#include "PlaySessionController.h"
#include "EditorContext.h"
#include "SelectionController.h"
#include "LevelIO.h"
#include "GameViewport.h"
#include "../Engine/Core/Application.h"
#include "../Engine/Entity/Entity.h"
#include "../Engine/Entity/EntityManager.h"
#include "../Engine/Rendering/Camera.h"
#include "../Engine/Audio/AudioManager.h"
#include "../Engine/State/StateManager.h"
#include <algorithm>

namespace Editor {

bool PlaySessionController::isPlaying() const {
    return ctx.state == Engine::GameState::PLAYING ||
           ctx.state == Engine::GameState::PAUSED;
}

void PlaySessionController::startPlaying() {
    if (ctx.selection) ctx.selection->findPlayer();
    if (ctx.selection && !ctx.selection->player()) {
        ctx.setStatus("No player entity in level!");
        return;
    }

    if (ctx.selection) ctx.selection->clear();
    ctx.viewport.cancelDrag();
    snapshotEntities();
    m_savedViewOffset = ctx.viewport.getViewOffset();
    m_savedViewZoom = ctx.viewport.getZoom();

    Engine::GameplaySystem::Config cfg;
    cfg.app = &ctx.game;
    cfg.audio = &ctx.audio;
    cfg.camera = &ctx.camera;
    cfg.levelWidth = ctx.levelIO ? ctx.levelIO->levelWidth() : 1600.f;
    cfg.levelHeight = ctx.levelIO ? ctx.levelIO->levelHeight() : 720.f;
    cfg.respawnYMargin = RESPAWN_Y_MARGIN;
    m_gameplay.begin(cfg);

    ctx.state = Engine::GameState::PLAYING;
}

void PlaySessionController::stopPlaying() {
    ctx.state = Engine::GameState::MENU;
    m_gameplay.end();
    restoreSnapshot();
    ctx.viewport.setViewOffset(m_savedViewOffset);
    ctx.viewport.setZoom(m_savedViewZoom);
    ctx.audio.stopAll();
    m_playOverlay = PlayOverlay::None;
    endGroupSession();
}

void PlaySessionController::tick(float dt) {
    if (m_playOverlay != PlayOverlay::None) return;

    constexpr float fixedDt = 1.f / 60.f;
    constexpr float maxFrameTime = 0.25f;
    m_accumulator += std::min(dt, maxFrameTime);

    while (m_accumulator >= fixedDt) {
        if (m_gameplay.won() || m_playOverlay != PlayOverlay::None) break;

        ctx.camera.setViewSize(ctx.viewport.viewW(), ctx.viewport.viewH());
        m_gameplay.tick(fixedDt);

        int deaths = m_gameplay.consumeDeaths();
        if (m_groupSession.active && deaths > 0) {
            m_groupSession.livesRemaining -= deaths;
            if (m_groupSession.livesRemaining <= 0) {
                m_groupSession.livesRemaining = 0;
                m_playOverlay = PlayOverlay::GameOver;
                break;
            }
        }

        if (m_gameplay.won()) {
            m_levelCoinsSnapshot = m_gameplay.score();
            if (m_groupSession.active) {
                m_groupSession.totalCoins += m_gameplay.score();
                const auto& groups = ctx.levelIO->groups();
                bool lastLevel =
                    m_groupSession.groupIndex >= 0 &&
                    m_groupSession.groupIndex < (int)groups.size() &&
                    m_groupSession.currentLevel + 1 >=
                        (int)groups[m_groupSession.groupIndex].levels.size();
                m_playOverlay = lastLevel ? PlayOverlay::WorldComplete
                                          : PlayOverlay::LevelComplete;
            } else if (!m_gameplay.nextLevel().empty()) {
                m_pendingNextLevel = "assets/levels/" + m_gameplay.nextLevel();
            } else {
                m_playOverlay = PlayOverlay::SingleWin;
            }
            break;
        }

        ctx.viewport.setViewCenter(ctx.camera.getPosition());
        m_accumulator -= fixedDt;
    }

    if (!m_pendingNextLevel.empty()) {
        std::string next = std::move(m_pendingNextLevel);
        m_pendingNextLevel.clear();
        stopPlaying();
        clearSnapshot();
        ctx.levelIO->loadLevel(next);
        if (ctx.game.getEntityManager().getEntityCount() > 0)
            startPlaying();
    }
}

void PlaySessionController::endGroupSession() {
    m_groupSession = GroupSession{};
}

void PlaySessionController::startGroup(int groupIndex) {
    if (!ctx.levelIO) return;
    const auto& groups = ctx.levelIO->groups();
    if (groupIndex < 0 || groupIndex >= (int)groups.size()) return;
    auto& g = groups[groupIndex];
    if (g.levels.empty()) return;

    m_groupSession.active = true;
    m_groupSession.groupIndex = groupIndex;
    m_groupSession.currentLevel = 0;
    m_groupSession.livesRemaining = g.lives;
    m_groupSession.totalCoins = 0;
    m_levelCoinsSnapshot = 0;
    m_playOverlay = PlayOverlay::None;

    clearSnapshot();
    ctx.levelIO->loadLevel("assets/levels/" + g.levels.front());
    if (ctx.game.getEntityManager().getEntityCount() > 0) {
        startPlaying();
    } else {
        endGroupSession();
        ctx.setStatus("Failed to load first level of world");
    }
}

void PlaySessionController::advanceGroupLevel() {
    if (!m_groupSession.active || !ctx.levelIO) return;
    const auto& groups = ctx.levelIO->groups();
    if (m_groupSession.groupIndex < 0 ||
        m_groupSession.groupIndex >= (int)groups.size()) return;

    auto& g = groups[m_groupSession.groupIndex];
    m_groupSession.currentLevel++;
    if (m_groupSession.currentLevel >= (int)g.levels.size()) {
        stopPlaying();
        return;
    }

    auto savedOff = m_savedViewOffset;
    float savedZoom = m_savedViewZoom;

    ctx.state = Engine::GameState::MENU;
    m_gameplay.end();
    restoreSnapshot();
    ctx.audio.stopAll();

    clearSnapshot();
    ctx.levelIO->loadLevel("assets/levels/" + g.levels[m_groupSession.currentLevel]);
    if (ctx.game.getEntityManager().getEntityCount() > 0) {
        startPlaying();
        m_savedViewOffset = savedOff;
        m_savedViewZoom = savedZoom;
    } else {
        endGroupSession();
        ctx.setStatus("Failed to load next level");
    }
}

void PlaySessionController::restartGroup() {
    if (!m_groupSession.active || !ctx.levelIO) return;
    const auto& groups = ctx.levelIO->groups();
    if (m_groupSession.groupIndex < 0 ||
        m_groupSession.groupIndex >= (int)groups.size()) return;

    int groupIdx = m_groupSession.groupIndex;
    auto savedOff = m_savedViewOffset;
    float savedZoom = m_savedViewZoom;

    ctx.state = Engine::GameState::MENU;
    m_gameplay.end();
    restoreSnapshot();
    ctx.audio.stopAll();
    endGroupSession();

    startGroup(groupIdx);
    if (m_groupSession.active) {
        m_savedViewOffset = savedOff;
        m_savedViewZoom = savedZoom;
    }
}

void PlaySessionController::snapshotEntities() {
    m_playSnapshot.clear();
    for (auto* e : ctx.game.getEntityManager().getAllEntities()) {
        m_playSnapshot.push_back({e, e->position, e->velocity, e->isOnGround});
    }
}

void PlaySessionController::restoreSnapshot() {
    auto& collected = m_gameplay.collectedEntities();
    for (auto* e : collected) ctx.game.getEntityManager().addEntity(e);
    collected.clear();

    auto& defeated = m_gameplay.defeatedEntities();
    for (auto* e : defeated) ctx.game.getEntityManager().addEntity(e);
    defeated.clear();

    for (auto& snap : m_playSnapshot) {
        auto& ents = ctx.game.getEntityManager().getAllEntities();
        if (std::find(ents.begin(), ents.end(), snap.entity) != ents.end()) {
            snap.entity->position = snap.position;
            snap.entity->velocity = snap.velocity;
            snap.entity->isOnGround = snap.isOnGround;
        }
    }
    m_playSnapshot.clear();
}

}
