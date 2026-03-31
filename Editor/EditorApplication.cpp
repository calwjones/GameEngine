#include "EditorApplication.h"
#include "../Engine/Entity/EntityTypeRegistry.h"
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <filesystem>

namespace Editor {

static void applyEditorTheme() {
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowRounding    = 6.f;
    style.FrameRounding     = 4.f;
    style.GrabRounding      = 4.f;
    style.TabRounding       = 4.f;
    style.ScrollbarRounding = 4.f;
    style.ChildRounding     = 4.f;
    style.PopupRounding     = 4.f;
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 5);
    style.WindowPadding     = ImVec2(10, 10);
    style.WindowBorderSize  = 1.f;
    style.FrameBorderSize   = 0.f;
    style.IndentSpacing     = 14.f;
    style.ScrollbarSize     = 12.f;
    style.GrabMinSize       = 8.f;

    auto& c = style.Colors;
    c[ImGuiCol_WindowBg]             = {0.11f, 0.11f, 0.13f, 1.00f};
    c[ImGuiCol_ChildBg]              = {0.10f, 0.10f, 0.12f, 1.00f};
    c[ImGuiCol_PopupBg]              = {0.12f, 0.12f, 0.15f, 0.96f};
    c[ImGuiCol_MenuBarBg]            = {0.13f, 0.13f, 0.16f, 1.00f};
    c[ImGuiCol_Border]               = {0.22f, 0.22f, 0.28f, 1.00f};
    c[ImGuiCol_BorderShadow]         = {0.00f, 0.00f, 0.00f, 0.00f};
    c[ImGuiCol_TitleBg]              = {0.09f, 0.09f, 0.11f, 1.00f};
    c[ImGuiCol_TitleBgActive]        = {0.12f, 0.12f, 0.16f, 1.00f};
    c[ImGuiCol_TitleBgCollapsed]     = {0.09f, 0.09f, 0.11f, 0.60f};
    c[ImGuiCol_FrameBg]              = {0.16f, 0.16f, 0.20f, 1.00f};
    c[ImGuiCol_FrameBgHovered]       = {0.20f, 0.20f, 0.26f, 1.00f};
    c[ImGuiCol_FrameBgActive]        = {0.24f, 0.24f, 0.32f, 1.00f};
    c[ImGuiCol_Button]               = {0.18f, 0.32f, 0.36f, 1.00f};
    c[ImGuiCol_ButtonHovered]        = {0.22f, 0.42f, 0.48f, 1.00f};
    c[ImGuiCol_ButtonActive]         = {0.16f, 0.50f, 0.56f, 1.00f};
    c[ImGuiCol_Header]               = {0.18f, 0.32f, 0.36f, 0.60f};
    c[ImGuiCol_HeaderHovered]        = {0.22f, 0.42f, 0.48f, 0.80f};
    c[ImGuiCol_HeaderActive]         = {0.16f, 0.50f, 0.56f, 1.00f};
    c[ImGuiCol_Tab]                  = {0.14f, 0.14f, 0.18f, 1.00f};
    c[ImGuiCol_TabHovered]           = {0.22f, 0.42f, 0.48f, 0.80f};
    c[ImGuiCol_TabActive]            = {0.18f, 0.32f, 0.36f, 1.00f};
    c[ImGuiCol_CheckMark]            = {0.30f, 0.78f, 0.70f, 1.00f};
    c[ImGuiCol_SliderGrab]           = {0.30f, 0.78f, 0.70f, 0.80f};
    c[ImGuiCol_SliderGrabActive]     = {0.36f, 0.88f, 0.80f, 1.00f};
    c[ImGuiCol_ScrollbarBg]          = {0.10f, 0.10f, 0.12f, 1.00f};
    c[ImGuiCol_ScrollbarGrab]        = {0.22f, 0.22f, 0.28f, 1.00f};
    c[ImGuiCol_ScrollbarGrabHovered] = {0.30f, 0.30f, 0.38f, 1.00f};
    c[ImGuiCol_ScrollbarGrabActive]  = {0.36f, 0.36f, 0.44f, 1.00f};
    c[ImGuiCol_Separator]            = {0.22f, 0.22f, 0.28f, 1.00f};
    c[ImGuiCol_SeparatorHovered]     = {0.30f, 0.78f, 0.70f, 0.60f};
    c[ImGuiCol_SeparatorActive]      = {0.30f, 0.78f, 0.70f, 1.00f};
    c[ImGuiCol_ResizeGrip]           = {0.22f, 0.22f, 0.28f, 0.40f};
    c[ImGuiCol_ResizeGripHovered]    = {0.30f, 0.78f, 0.70f, 0.60f};
    c[ImGuiCol_ResizeGripActive]     = {0.30f, 0.78f, 0.70f, 1.00f};
    c[ImGuiCol_Text]                 = {0.88f, 0.88f, 0.92f, 1.00f};
    c[ImGuiCol_TextDisabled]         = {0.44f, 0.44f, 0.50f, 1.00f};
    c[ImGuiCol_TextSelectedBg]       = {0.22f, 0.42f, 0.48f, 0.60f};
}

bool EditorApplication::initialize(unsigned int w, unsigned int h) {
    m_ctx.selection = &m_selection;
    m_ctx.levelIO = &m_levelIO;
    m_ctx.playSession = &m_playSession;
    m_ctx.entityOps = &m_entityOps;
    m_ctx.popups = &m_popups;

    m_window.create(sf::VideoMode(w, h), "2D Game Engine Editor");
    m_window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(m_window)) {
        std::cerr << "Failed to init ImGui-SFML" << std::endl;
        return false;
    }
    m_imguiInitialized = true;

    applyEditorTheme();
    m_viewport.initialize(640, 480);
    m_clock.restart();

    m_audio.loadSound("jump", "assets/audio/jump.wav");
    m_audio.loadSound("collect", "assets/audio/collect.wav");
    m_audio.loadSound("shoot", "assets/audio/shoot.wav");

    Engine::registerBuiltinTypes(m_factory);
    m_loader.setFactory(&m_factory);

    m_levelIO.scanLevelFiles();
    m_levelIO.scanLevelGroups();
    if (std::filesystem::exists("assets/levels/demo_level.json"))
        m_levelIO.loadLevel("assets/levels/demo_level.json");

    return true;
}

void EditorApplication::run() {
    while (m_window.isOpen()) {
        m_dt = m_clock.restart().asSeconds();
        processEvents();
        update(m_dt);
        render();
    }
}

void EditorApplication::shutdown() {
    m_viewport.shutdown();
    if (m_imguiInitialized) {
        ImGui::SFML::Shutdown();
        m_imguiInitialized = false;
    }
}

void EditorApplication::processEvents() {
    sf::Event e;
    while (m_window.pollEvent(e)) {
        ImGui::SFML::ProcessEvent(m_window, e);
        if (e.type == sf::Event::Closed) {
            if (m_dirty) {
                m_popups.requestCloseWindow();
            } else {
                m_window.close();
            }
        }
        if (e.type == sf::Event::KeyPressed) handleInput(e.key.code);
    }
}

void EditorApplication::handleInput(sf::Keyboard::Key key) {
    using GS = Engine::GameState;

    if (ImGui::GetIO().WantTextInput) return;

    bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
    bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

    if (key == sf::Keyboard::Z && ctrl && m_state == GS::MENU) {
        if (shift) {
            if (m_history.canRedo()) {
                m_history.redo();
                m_selection.validate();
                m_selection.findPlayer();
                markDirty();
                setStatus("Redo: " + m_history.undoDescription());
            }
        } else {
            if (m_history.canUndo()) {
                std::string desc = m_history.undoDescription();
                m_history.undo();
                m_selection.validate();
                m_selection.findPlayer();
                markDirty();
                setStatus("Undo: " + desc);
            }
        }
        return;
    }

    if (key == sf::Keyboard::S && ctrl && m_state == GS::MENU) {
        m_popups.saveLevel();
        return;
    }

    if (key == sf::Keyboard::N && ctrl && m_state == GS::MENU) {
        if (m_dirty) {
            m_popups.requestNewLevel();
        } else {
            m_levelIO.resetToNewLevel();
        }
        return;
    }

    if (key == sf::Keyboard::D && ctrl && m_selection.current() && m_state == GS::MENU) {
        m_entityOps.duplicateEntity();
        return;
    }

    if (key == sf::Keyboard::Escape && m_state == GS::MENU) {
        if (!m_selection.all().empty()) {
            m_selection.clear();
            m_viewport.cancelDrag();
        }
        return;
    }

    if ((key == sf::Keyboard::Delete || key == sf::Keyboard::BackSpace) && !m_selection.all().empty() && m_state == GS::MENU) {
        m_entityOps.deleteSelected();
        return;
    }

    if (key == sf::Keyboard::G && !ctrl && m_state == GS::MENU) {
        m_viewport.gridEnabled() = !m_viewport.gridEnabled();
        setStatus(m_viewport.gridEnabled() ? "Grid enabled" : "Grid disabled");
        return;
    }

    if (key == sf::Keyboard::F && !ctrl && m_state == GS::MENU && m_selection.current()) {
        m_viewport.setViewCenter(m_selection.current()->getCenter());
        setStatus("Focused on " + m_selection.current()->name);
        return;
    }

    bool hasEntities = m_game.getEntityManager().getEntityCount() > 0;

    if (m_state == GS::MENU && key == sf::Keyboard::Space && hasEntities) {
        m_viewport.isVisible() = true;
        m_playSession.startPlaying();
    } else if (m_state == GS::PLAYING && key == sf::Keyboard::Escape) {
        m_state = GS::PAUSED;
    } else if (m_state == GS::PAUSED) {
        if (key == sf::Keyboard::Escape) m_state = GS::PLAYING;
        else if (key == sf::Keyboard::Q) m_playSession.stopPlaying();
    }
}

void EditorApplication::update(float dt) {
    ImGui::SFML::Update(m_window, sf::seconds(dt));

    for (auto* e : m_game.getEntityManager().getAllEntities()) {
        if (e->texturePath == e->resolvedTexturePath) continue;
        e->texture = e->texturePath.empty() ? nullptr : m_textures.getTexture(e->texturePath);
        e->resolvedTexturePath = e->texturePath;
    }

    if (!m_viewport.isVisible() && m_state != Engine::GameState::MENU)
        m_playSession.stopPlaying();

    if (m_state == Engine::GameState::PLAYING)
        m_playSession.tick(dt);
}

void EditorApplication::render() {
    m_window.clear(sf::Color(28, 28, 33));
    renderImGui();
    ImGui::SFML::Render(m_window);
    m_window.display();
}

void EditorApplication::renderImGui() {
    m_menuBar.render(m_dt);
    m_toolbar.render();

    float w = (float)m_window.getSize().x, h = (float)m_window.getSize().y;
    float panel = 220.f, toolbar = 60.f, content = h - toolbar;

    ImGui::SetNextWindowPos(ImVec2(0, toolbar), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panel, content), ImGuiCond_FirstUseEver);
    {
        auto& sel = m_selection.mutableAll();
        auto* primary = m_selection.current();
        m_scenePanel.render(m_game.getEntityManager(), sel, primary, isPlaying());
        m_selection.setCurrent(primary);
    }

    if (auto* toDelete = m_scenePanel.consumeDeleteRequest()) {
        std::string name = toDelete->name;
        m_selection.remove(toDelete);

        auto cmd = std::make_unique<DeleteEntityCommand>(m_game.getEntityManager(), toDelete);
        cmd->execute();
        m_history.push(std::move(cmd));
        m_viewport.cancelDrag();
        markDirty();
        setStatus("Deleted " + name);
    }
    if (auto* toDuplicate = m_scenePanel.consumeDuplicateRequest()) {
        m_selection.setCurrent(toDuplicate);
        m_entityOps.duplicateEntity();
    }
    if (m_scenePanel.consumeAddRequest()) {
        m_entityOps.addEntity();
    }
    if (auto* toMoveUp = m_scenePanel.consumeMoveUpRequest()) {
        m_game.getEntityManager().moveEntity(toMoveUp, 1);
        markDirty();
    }
    if (auto* toMoveDown = m_scenePanel.consumeMoveDownRequest()) {
        m_game.getEntityManager().moveEntity(toMoveDown, -1);
        markDirty();
    }

    auto click = m_viewport.consumeClick();
    if (click.occurred) {
        if (click.entity) {
            if (click.ctrlHeld)
                m_selection.toggle(click.entity);
            else
                m_selection.select(click.entity);
        } else {
            m_selection.clear();
        }
    }

    std::vector<std::pair<Engine::Entity*, sf::Vector2f>> dragOldPositions;
    if (m_viewport.consumeDragComplete(dragOldPositions)) {
        std::vector<MoveCommand::Entry> entries;
        for (auto& [entity, oldPos] : dragOldPositions)
            entries.push_back({entity, oldPos, entity->position});
        if (!entries.empty()) {
            m_history.push(std::make_unique<MoveCommand>(std::move(entries)));
            markDirty();
        }
    }

    // swap-back so pos+size become a single undo step
    sf::Vector2f resizeOldPos, resizeOldSize;
    if (auto* resized = m_viewport.consumeResizeComplete(resizeOldPos, resizeOldSize)) {
        EntityState newState = EntityState::capture(resized);
        sf::Vector2f curPos = resized->position;
        sf::Vector2f curSize = resized->size;
        resized->position = resizeOldPos;
        resized->size = resizeOldSize;
        EntityState oldState = EntityState::capture(resized);
        resized->position = curPos;
        resized->size = curSize;
        m_history.push(std::make_unique<PropertyChangeCommand>(resized, oldState, newState));
        markDirty();
    }

    sf::Vector2f mpOldB;
    if (auto* mpEnt = m_viewport.consumeMovingPlatformBComplete(mpOldB)) {
        if (auto* mp = dynamic_cast<Game::MovingPlatform*>(mpEnt)) {
            EntityState newState = EntityState::capture(mpEnt);
            sf::Vector2f curB = mp->getPointB();
            mp->setPointB(mpOldB);
            EntityState oldState = EntityState::capture(mpEnt);
            mp->setPointB(curB);
            m_history.push(std::make_unique<PropertyChangeCommand>(mpEnt, oldState, newState));
            markDirty();
        }
    }

    ImGui::SetNextWindowPos(ImVec2(0, toolbar + content * 0.6f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panel, content * 0.4f), ImGuiCond_FirstUseEver);
    m_palette.render();
    auto tmpl = m_palette.consumeCreateRequest();
    if (tmpl != EntityTemplate::None) m_entityOps.addFromTemplate(tmpl);

    ImGui::SetNextWindowPos(ImVec2(w - panel, toolbar), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panel, content), ImGuiCond_FirstUseEver);
    // null history during play so live edits don't pollute undo
    size_t histBefore = m_history.size();
    m_propsPanel.render(m_selection.current(), isPlaying() ? nullptr : &m_history);
    if (m_history.size() != histBefore) markDirty();

    ImGui::SetNextWindowPos(ImVec2(panel, toolbar), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(w - panel * 2, content), ImGuiCond_FirstUseEver);
    HudInfo hud;
    hud.score = m_playSession.score();
    static thread_local char worldLabelBuf[64];
    const auto& gs = m_playSession.groupSession();
    if (gs.active && gs.groupIndex >= 0 &&
        gs.groupIndex < (int)m_levelIO.groups().size()) {
        auto& g = m_levelIO.groups()[gs.groupIndex];
        snprintf(worldLabelBuf, sizeof(worldLabelBuf),
                 "%s  -  Level %d / %d",
                 g.name.c_str(),
                 gs.currentLevel + 1,
                 (int)g.levels.size());
        hud.worldLabel = worldLabelBuf;
        hud.lives = gs.livesRemaining;
        hud.totalCoins = gs.totalCoins + m_playSession.score();
    }
    m_viewport.render(m_game, isPlaying(), m_selection.all(), hud, m_selection.current());

    m_overlay.render();
    m_popups.render();
}


void EditorApplication::setStatus(const std::string& msg) {
    m_statusMsg = msg;
    m_statusTimer = 3.f;
}

void EditorApplication::markDirty() {
    if (!m_dirty) {
        m_dirty = true;
        m_levelIO.updateWindowTitle();
    }
}

}
