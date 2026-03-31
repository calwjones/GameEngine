#include "LevelIO.h"
#include "EditorContext.h"
#include "SelectionController.h"
#include "GameViewport.h"
#include "CommandHistory.h"
#include "../Engine/Core/Application.h"
#include "../Engine/Entity/Entity.h"
#include "../Engine/Entity/EntityManager.h"
#include "../Engine/Level/LevelLoader.h"
#include "../Engine/Level/LevelGroup.h"
#include "../Engine/State/StateManager.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <iostream>

namespace Editor {

void LevelIO::loadLevel(const std::string& path) {
    ctx.history.clear();
    ctx.game.getEntityManager().clear();
    if (ctx.selection) {
        ctx.selection->clear();
        ctx.selection->setPlayer(nullptr);
    }

    auto entities = ctx.loader.loadFromJSON(path);
    if (entities.empty() && !ctx.loader.getLastError().empty()) {
        ctx.setStatus("Load failed!");
        std::cerr << "Load failed: " << ctx.loader.getLastError() << std::endl;
        return;
    }

    m_levelWidth = ctx.loader.getWidth();
    m_levelHeight = ctx.loader.getHeight();
    ctx.viewport.setLevelSize(m_levelWidth, m_levelHeight);

    for (auto* e : entities) ctx.game.getEntityManager().addEntity(e);

    int maxSuffix = m_entityCounter;
    for (auto* e : entities) {
        size_t i = e->name.size();
        while (i > 0 && std::isdigit((unsigned char)e->name[i - 1])) --i;
        if (i < e->name.size()) {
            try {
                int v = std::stoi(e->name.substr(i));
                if (v > maxSuffix) maxSuffix = v;
            } catch (...) {}
        }
    }
    m_entityCounter = maxSuffix;
    if (ctx.selection) ctx.selection->findPlayer();

    m_levelPath = path;
    ctx.state = Engine::GameState::MENU;
    ctx.dirty = false;
    ctx.viewport.isVisible() = true;
    updateWindowTitle();

    auto pos = path.find_last_of('/');
    std::string name = (pos != std::string::npos) ? path.substr(pos + 1) : path;
    ctx.setStatus("Loaded " + name);
}

void LevelIO::saveLevel() {
    if (m_levelPath.empty()) return;
    if (ctx.loader.saveToJSON(m_levelPath,
                              ctx.game.getEntityManager().getAllEntities(),
                              m_levelWidth, m_levelHeight)) {
        ctx.dirty = false;
        auto pos = m_levelPath.find_last_of('/');
        std::string name = (pos != std::string::npos) ? m_levelPath.substr(pos + 1) : m_levelPath;
        updateWindowTitle();
        ctx.setStatus("Saved " + name);
    } else {
        updateWindowTitle();
        ctx.setStatus("Save failed!");
        std::cerr << "Save failed: " << ctx.loader.getLastError() << std::endl;
    }
}

void LevelIO::saveLevelAs(const std::string& filename) {
    m_levelPath = "assets/levels/" + filename;
    if (ctx.loader.saveToJSON(m_levelPath,
                              ctx.game.getEntityManager().getAllEntities(),
                              m_levelWidth, m_levelHeight)) {
        ctx.dirty = false;
        updateWindowTitle();
        scanLevelFiles();
        ctx.setStatus("Saved " + filename);
    } else {
        updateWindowTitle();
        ctx.setStatus("Save failed!");
        std::cerr << "Save failed: " << ctx.loader.getLastError() << std::endl;
    }
}

void LevelIO::scanLevelFiles() {
    m_levelFiles.clear();
    std::string dir = "assets/levels/";
    if (!std::filesystem::exists(dir)) return;

    for (auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            m_levelFiles.push_back(entry.path().string());
        }
    }
    std::sort(m_levelFiles.begin(), m_levelFiles.end());
}

void LevelIO::scanLevelGroups() {
    m_groups = Engine::LevelGroupLoader::scanDirectory("assets/levels/groups");
}

void LevelIO::fitLevelToContent() {
    auto& ents = ctx.game.getEntityManager().getAllEntities();
    if (ents.empty()) {
        ctx.setStatus("No entities to fit to");
        return;
    }
    float maxX = 0.f, maxY = 0.f;
    for (auto* e : ents) {
        float right = e->position.x + e->size.x;
        float bottom = e->position.y + e->size.y;
        if (right > maxX) maxX = right;
        if (bottom > maxY) maxY = bottom;
    }
    constexpr float PAD = 120.f;
    m_levelWidth = std::max(400.f, maxX + PAD);
    m_levelHeight = std::max(300.f, maxY + PAD);
    ctx.viewport.setLevelSize(m_levelWidth, m_levelHeight);
    ctx.markDirty();
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Level resized to %.0f x %.0f", m_levelWidth, m_levelHeight);
    ctx.setStatus(buf);
}

void LevelIO::resetToNewLevel() {
    ctx.history.clear();
    ctx.game.getEntityManager().clear();
    if (ctx.selection) {
        ctx.selection->clear();
        ctx.selection->setPlayer(nullptr);
    }
    m_levelPath.clear();
    m_levelWidth = DEFAULT_W;
    m_levelHeight = DEFAULT_H;
    ctx.viewport.setLevelSize(m_levelWidth, m_levelHeight);
    ctx.state = Engine::GameState::MENU;
    ctx.dirty = false;
    updateWindowTitle();
    ctx.setStatus("New level");
}

void LevelIO::updateWindowTitle() {
    std::string title = "2D Game Engine Editor";
    if (!m_levelPath.empty()) {
        auto pos = m_levelPath.find_last_of('/');
        std::string name = (pos != std::string::npos) ? m_levelPath.substr(pos + 1) : m_levelPath;
        title += " - " + name;
    }
    if (ctx.dirty) title += " *";
    ctx.window.setTitle(title);
}

}
