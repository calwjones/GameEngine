#include "EditorApplication.h"
// EntityTypeRegistry.h transitively pulls every Game::* subclass header via the factory table so downcasts below resolve cleanly
#include "../Engine/Entity/EntityTypeRegistry.h"
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace Editor {

// dark theme — hand-tuned colour values, dont touch
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
    m_window.create(sf::VideoMode(w, h), "2D Game Engine Editor");
    m_window.setFramerateLimit(60);   // soft cap - doesnt block, imgui will render at whatever the gpu allows

    if (!ImGui::SFML::Init(m_window)) {
        std::cerr << "Failed to init ImGui-SFML" << std::endl;
        return false;
    }
    m_imguiInitialized = true;

    applyEditorTheme();
    m_viewport.initialize(640, 480);   // render texture size - game content is 800x600 but viewport panel is smaller
    m_clock.restart();

    // preload audio - fails silently if files missing (conditional compile fallback in AudioManager)
    m_audio.loadSound("jump", "assets/audio/jump.wav");
    m_audio.loadSound("collect", "assets/audio/collect.wav");
    m_audio.loadSound("shoot", "assets/audio/shoot.wav");

    // factory regs live in EntityTypeRegistry.h — adding a new entity type = one row in kEntityTypes
    Engine::registerBuiltinTypes(m_factory);
    m_loader.setFactory(&m_factory);   // loader needs the factory to construct the right subclass from json

    scanLevelFiles();
    scanLevelGroups();
    // auto-load demo level on startup if it exists so theres something to show immediately
    if (std::filesystem::exists("assets/levels/demo_level.json"))
        loadLevel("assets/levels/demo_level.json");

    return true;
}

// main loop - dt is wall clock time, fixed timestep accumulator lives in update()
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
        ImGui::SFML::ProcessEvent(m_window, e);   // imgui needs to see raw events first
        if (e.type == sf::Event::Closed) {
            // dirty flag check - show unsaved changes popup instead of just closing
            if (m_dirty) {
                m_pendingAction = PendingAction::CloseWindow;
            } else {
                m_window.close();
            }
        }
        if (e.type == sf::Event::KeyPressed) handleInput(e.key.code);
    }
}

// keyboard shortcuts - WantTextInput guard prevents triggering whilst typing in an imgui widget
void EditorApplication::handleInput(sf::Keyboard::Key key) {
    using GS = Engine::GameState;

    if (ImGui::GetIO().WantTextInput) return;   // imgui has focus, dont steal the keypress

    bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
    bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

    if (key == sf::Keyboard::Z && ctrl && m_state == GS::MENU) {
        if (shift) {
            if (m_history.canRedo()) {
                m_history.redo();
                validateSelection();
                findPlayer();
                markDirty();
                setStatus("Redo: " + m_history.undoDescription());
            }
        } else {
            if (m_history.canUndo()) {
                std::string desc = m_history.undoDescription();
                m_history.undo();
                validateSelection();
                findPlayer();
                markDirty();
                setStatus("Undo: " + desc);
            }
        }
        return;
    }

    if (key == sf::Keyboard::S && ctrl && m_state == GS::MENU) {
        saveLevel();
        return;
    }

    if (key == sf::Keyboard::N && ctrl && m_state == GS::MENU) {
        if (m_dirty) {
            m_pendingAction = PendingAction::NewLevel;
        } else {
            resetToNewLevel();
        }
        return;
    }

    if (key == sf::Keyboard::D && ctrl && m_selected && m_state == GS::MENU) {
        duplicateEntity();
        return;
    }

    if (key == sf::Keyboard::Escape && m_state == GS::MENU) {
        if (!m_selection.empty()) {
            clearSelection();
            m_viewport.cancelDrag();
        }
        return;
    }

    if ((key == sf::Keyboard::Delete || key == sf::Keyboard::BackSpace) && !m_selection.empty() && m_state == GS::MENU) {
        deleteSelected();
        return;
    }

    // Grid toggle shortcut (G)
    if (key == sf::Keyboard::G && !ctrl && m_state == GS::MENU) {
        m_viewport.gridEnabled() = !m_viewport.gridEnabled();
        setStatus(m_viewport.gridEnabled() ? "Grid enabled" : "Grid disabled");
        return;
    }

    // Focus on selection shortcut (F)
    if (key == sf::Keyboard::F && !ctrl && m_state == GS::MENU && m_selected) {
        m_viewport.setViewCenter(m_selected->getCenter());
        setStatus("Focused on " + m_selected->name);
        return;
    }

    bool hasEntities = m_game.getEntityManager().getEntityCount() > 0;

    // space to play, esc to pause/resume, q to stop - only wired in relevant states
    if (m_state == GS::MENU && key == sf::Keyboard::Space && hasEntities) {
        m_viewport.isVisible() = true;
        startPlaying();
    } else if (m_state == GS::PLAYING && key == sf::Keyboard::Escape) {
        m_state = GS::PAUSED;
    } else if (m_state == GS::PAUSED) {
        if (key == sf::Keyboard::Escape) m_state = GS::PLAYING;
        else if (key == sf::Keyboard::Q) stopPlaying();
    }
}

// simulation only runs when PLAYING — edit mode just does imgui bookkeeping. hot loop delegates to GameplaySystem::tick
void EditorApplication::update(float dt) {
    ImGui::SFML::Update(m_window, sf::seconds(dt));   // feeds dt to imgui for animations/input

    // texture resolution — only hit TextureManager when path actually changed
    for (auto* e : m_game.getEntityManager().getAllEntities()) {
        if (e->texturePath == e->resolvedTexturePath) continue;
        e->texture = e->texturePath.empty() ? nullptr : m_textures.getTexture(e->texturePath);
        e->resolvedTexturePath = e->texturePath;
    }

    // if the viewport was closed mid-play, auto stop - stops the game getting stuck in limbo
    if (!m_viewport.isVisible() && m_state != Engine::GameState::MENU)
        stopPlaying();

    if (m_state != Engine::GameState::PLAYING) return;   // editor mode - no simulation needed

    // transition overlay freezes sim until user dismisses it (rendered in renderStateOverlay)
    if (m_playOverlay != PlayOverlay::None) return;

    constexpr float fixedDt = 1.f / 60.f;
    constexpr float maxFrameTime = 0.25f;  // spiral of death cap - same as GameLoop
    m_accumulator += std::min(dt, maxFrameTime);

    while (m_accumulator >= fixedDt) {
        if (m_gameplay.won() || m_playOverlay != PlayOverlay::None) break;

        m_camera.setViewSize(m_viewport.viewW(), m_viewport.viewH());
        m_gameplay.tick(fixedDt);

        // death tracking — group run chips lives, outside a group deaths are infinite retries
        int deaths = m_gameplay.consumeDeaths();
        if (m_groupSession.active && deaths > 0) {
            m_groupSession.livesRemaining -= deaths;
            if (m_groupSession.livesRemaining <= 0) {
                m_groupSession.livesRemaining = 0;
                m_playOverlay = PlayOverlay::GameOver;
                break;
            }
        }

        // goal reached — group mode uses overlay transitions, non-group honours Goal.nextLevel chain or shows win screen
        if (m_gameplay.won()) {
            m_levelCoinsSnapshot = m_gameplay.score();
            if (m_groupSession.active) {
                m_groupSession.totalCoins += m_gameplay.score();
                bool lastLevel =
                    m_groupSession.groupIndex >= 0 &&
                    m_groupSession.groupIndex < (int)m_groups.size() &&
                    m_groupSession.currentLevel + 1 >=
                        (int)m_groups[m_groupSession.groupIndex].levels.size();
                m_playOverlay = lastLevel ? PlayOverlay::WorldComplete
                                          : PlayOverlay::LevelComplete;
            } else if (!m_gameplay.nextLevel().empty()) {
                m_pendingNextLevel = "assets/levels/" + m_gameplay.nextLevel();
            } else {
                m_playOverlay = PlayOverlay::SingleWin;
            }
            break;
        }

        m_viewport.setViewCenter(m_camera.getPosition());
        m_accumulator -= fixedDt;
    }

    // deferred level chain — stop, load nextLevel, re-enter play. done outside the fixed-step loop so we dont load mid-iteration
    if (!m_pendingNextLevel.empty()) {
        std::string next = std::move(m_pendingNextLevel);
        m_pendingNextLevel.clear();
        stopPlaying();
        loadLevel(next);
        if (m_game.getEntityManager().getEntityCount() > 0)
            startPlaying();
    }
}

void EditorApplication::render() {
    m_window.clear(sf::Color(28, 28, 33));   // dark bg shows through any imgui gaps
    renderImGui();
    ImGui::SFML::Render(m_window);
    m_window.display();
}

// lays out the whole ui — positions panels + processes their consume requests
void EditorApplication::renderImGui() {
    renderMenuBar();
    renderToolbar();

    float w = (float)m_window.getSize().x, h = (float)m_window.getSize().y;
    float panel = 220.f, toolbar = 60.f, content = h - toolbar;   // panel = left/right column width

    ImGui::SetNextWindowPos(ImVec2(0, toolbar), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panel, content), ImGuiCond_FirstUseEver);
    m_scenePanel.render(m_game.getEntityManager(), m_selection, m_selected, isPlaying());

    if (auto* toDelete = m_scenePanel.consumeDeleteRequest()) {
        std::string name = toDelete->name;
        m_selection.erase(std::remove(m_selection.begin(), m_selection.end(), toDelete), m_selection.end());
        if (toDelete == m_selected)
            m_selected = m_selection.empty() ? nullptr : m_selection.back();
        if (toDelete == m_player) m_player = nullptr;

        auto cmd = std::make_unique<DeleteEntityCommand>(m_game.getEntityManager(), toDelete);
        cmd->execute();
        m_history.push(std::move(cmd));
        m_viewport.cancelDrag();
        markDirty();
        setStatus("Deleted " + name);
    }
    if (auto* toDuplicate = m_scenePanel.consumeDuplicateRequest()) {
        m_selected = toDuplicate;
        duplicateEntity();
    }
    if (m_scenePanel.consumeAddRequest()) {
        addEntity();
    }
    if (auto* toMoveUp = m_scenePanel.consumeMoveUpRequest()) {
        m_game.getEntityManager().moveEntity(toMoveUp, 1);
        markDirty();
    }
    if (auto* toMoveDown = m_scenePanel.consumeMoveDownRequest()) {
        m_game.getEntityManager().moveEntity(toMoveDown, -1);
        markDirty();
    }

    // viewport click - ctrl+click = multi-select, plain click = single select or deselect
    auto click = m_viewport.consumeClick();
    if (click.occurred) {
        if (click.entity) {
            if (click.ctrlHeld)
                toggleSelect(click.entity);
            else
                selectEntity(click.entity);
        } else {
            clearSelection();
        }
    }

    // drag complete - push a MoveCommand so the drag is undoable
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

    // resize complete — swap-back pattern: capture new state, temp-restore old, capture old, put new back. one undo step for pos+size
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

    // mp pointB drag complete — same swap-back pattern. pointB is in the serialized properties sidecar
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
    if (tmpl != EntityTemplate::None) addFromTemplate(tmpl);

    ImGui::SetNextWindowPos(ImVec2(w - panel, toolbar), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panel, content), ImGuiCond_FirstUseEver);
    // pass null history during play so property edits dont create undo entries mid-game
    size_t histBefore = m_history.size();
    m_propsPanel.render(m_selected, isPlaying() ? nullptr : &m_history);
    if (m_history.size() != histBefore) markDirty();

    ImGui::SetNextWindowPos(ImVec2(panel, toolbar), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(w - panel * 2, content), ImGuiCond_FirstUseEver);
    // HUD — individual play shows coin count, group play adds world label + lives + running total
    HudInfo hud;
    hud.score = m_gameplay.score();
    static thread_local char worldLabelBuf[64];
    if (m_groupSession.active && m_groupSession.groupIndex >= 0 &&
        m_groupSession.groupIndex < (int)m_groups.size()) {
        auto& g = m_groups[m_groupSession.groupIndex];
        snprintf(worldLabelBuf, sizeof(worldLabelBuf),
                 "%s  -  Level %d / %d",
                 g.name.c_str(),
                 m_groupSession.currentLevel + 1,
                 (int)g.levels.size());
        hud.worldLabel = worldLabelBuf;
        hud.lives = m_groupSession.livesRemaining;
        hud.totalCoins = m_groupSession.totalCoins + m_gameplay.score();
    }
    m_viewport.render(m_game, isPlaying(), m_selection, hud, m_selected);

    renderStateOverlay();
    renderPopups();
}

// overlay shown in viewport - menu prompt in edit mode, pause screen during play
void EditorApplication::renderStateOverlay() {
    if (!m_viewport.isVisible()) return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
    float cx = m_window.getSize().x / 2.f, cy = m_window.getSize().y / 2.f;

    if (m_state == Engine::GameState::MENU && m_game.getEntityManager().getEntityCount() > 0) {
        findPlayer();

        ImGui::SetNextWindowPos(ImVec2(cx - 150, cy - 50));
        ImGui::SetNextWindowBgAlpha(0.85f);
        if (ImGui::Begin("##Menu", nullptr, flags)) {
            ImGui::Dummy(ImVec2(300, 10));
            ImGui::SetCursorPosX((300 - ImGui::CalcTextSize("2D GAME ENGINE").x) / 2);
            ImGui::TextColored(ImVec4(0.30f, 0.78f, 0.70f, 1.f), "2D GAME ENGINE");
            ImGui::Spacing(); ImGui::Spacing();
            if (m_player) {
                ImGui::SetCursorPosX((300 - ImGui::CalcTextSize("Press SPACE to play").x) / 2);
                ImGui::TextDisabled("Press SPACE to play");
            } else {
                const char* hint = "Add a Player entity to play";
                ImGui::SetCursorPosX((300 - ImGui::CalcTextSize(hint).x) / 2);
                ImGui::TextColored(ImVec4(0.90f, 0.65f, 0.20f, 0.8f), "%s", hint);
            }
            ImGui::Dummy(ImVec2(300, 10));
        }
        ImGui::End();
    }

    if (m_state == Engine::GameState::PAUSED) {
        ImGui::SetNextWindowPos(ImVec2(cx - 120, cy - 60));
        ImGui::SetNextWindowBgAlpha(0.85f);
        if (ImGui::Begin("##Pause", nullptr, flags)) {
            ImGui::Dummy(ImVec2(240, 10));
            ImGui::SetCursorPosX((240 - ImGui::CalcTextSize("PAUSED").x) / 2);
            ImGui::TextColored(ImVec4(1.f, 0.8f, 0.2f, 1.f), "PAUSED");
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
            ImGui::TextDisabled("ESC  - Resume");
            ImGui::TextDisabled("Q    - Stop & Edit");
            ImGui::Dummy(ImVec2(240, 10));
        }
        ImGui::End();
    }

    // transition overlay — blocks sim via early-return in update() until dismissed
    if (m_playOverlay != PlayOverlay::None &&
        (m_state == Engine::GameState::PLAYING || m_state == Engine::GameState::PAUSED)) {

        ImGui::SetNextWindowPos(ImVec2(cx - 180, cy - 110));
        ImGui::SetNextWindowBgAlpha(0.92f);
        if (ImGui::Begin("##Overlay", nullptr, flags)) {
            constexpr float PANEL_W = 360.f;
            ImGui::Dummy(ImVec2(PANEL_W, 8));

            const char* title = "";
            ImVec4 titleCol{1.f, 1.f, 1.f, 1.f};
            const char* btnLabel = "Continue";
            switch (m_playOverlay) {
                case PlayOverlay::LevelComplete:
                    title = "LEVEL COMPLETE"; titleCol = {0.55f, 0.90f, 1.0f, 1.f};
                    btnLabel = "Next Level"; break;
                case PlayOverlay::WorldComplete:
                    title = "WORLD COMPLETE!"; titleCol = {0.30f, 0.98f, 0.60f, 1.f};
                    btnLabel = "Back to Editor"; break;
                case PlayOverlay::GameOver:
                    title = "GAME OVER"; titleCol = {1.00f, 0.35f, 0.35f, 1.f};
                    btnLabel = "Retry World"; break;
                case PlayOverlay::SingleWin:
                    title = "YOU WIN!"; titleCol = {0.30f, 0.98f, 0.60f, 1.f};
                    btnLabel = "Back to Editor"; break;
                case PlayOverlay::None: break;
            }
            ImGui::SetCursorPosX((PANEL_W - ImGui::CalcTextSize(title).x) / 2);
            ImGui::TextColored(titleCol, "%s", title);

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            float playTime = m_gameplay.playTime();
            int mins = (int)(playTime / 60.f);
            float secs = playTime - mins * 60;
            char timeBuf[32];
            if (mins > 0) snprintf(timeBuf, sizeof(timeBuf), "%dm %.1fs", mins, secs);
            else          snprintf(timeBuf, sizeof(timeBuf), "%.1fs", secs);

            // group run shows level coins + world total + lives, non-group falls back to score + time
            auto centerLine = [&](const char* text, ImVec4 col) {
                ImGui::SetCursorPosX((PANEL_W - ImGui::CalcTextSize(text).x) / 2);
                ImGui::TextColored(col, "%s", text);
            };

            char buf[80];
            if (m_groupSession.active &&
                m_groupSession.groupIndex >= 0 &&
                m_groupSession.groupIndex < (int)m_groups.size()) {
                auto& g = m_groups[m_groupSession.groupIndex];
                snprintf(buf, sizeof(buf), "%s", g.name.c_str());
                centerLine(buf, ImVec4(0.75f, 0.85f, 1.f, 1.f));

                if (m_playOverlay == PlayOverlay::LevelComplete ||
                    m_playOverlay == PlayOverlay::WorldComplete) {
                    snprintf(buf, sizeof(buf), "Level %d / %d cleared",
                             m_groupSession.currentLevel + 1, (int)g.levels.size());
                    centerLine(buf, ImVec4(0.85f, 0.85f, 0.85f, 1.f));
                } else if (m_playOverlay == PlayOverlay::GameOver) {
                    snprintf(buf, sizeof(buf), "Died on Level %d / %d",
                             m_groupSession.currentLevel + 1, (int)g.levels.size());
                    centerLine(buf, ImVec4(0.9f, 0.5f, 0.5f, 1.f));
                }

                ImGui::Spacing();

                snprintf(buf, sizeof(buf), "Level Coins: %d", m_levelCoinsSnapshot);
                centerLine(buf, ImVec4(1.f, 0.84f, 0.f, 1.f));

                snprintf(buf, sizeof(buf), "World Total: %d", m_groupSession.totalCoins);
                centerLine(buf, ImVec4(1.f, 0.70f, 0.20f, 1.f));

                snprintf(buf, sizeof(buf), "Lives Remaining: %d", m_groupSession.livesRemaining);
                centerLine(buf, ImVec4(1.f, 0.45f, 0.45f, 1.f));
            } else {
                snprintf(buf, sizeof(buf), "Score: %d", m_gameplay.score());
                centerLine(buf, ImVec4(1.f, 0.84f, 0.f, 1.f));
                snprintf(buf, sizeof(buf), "Time:  %s", timeBuf);
                centerLine(buf, ImVec4(0.7f, 0.7f, 0.7f, 1.f));
            }

            ImGui::Spacing(); ImGui::Spacing();
            ImGui::SetCursorPosX((PANEL_W - 180.f) / 2.f);
            if (ImGui::Button(btnLabel, ImVec2(180, 0))) {
                switch (m_playOverlay) {
                    case PlayOverlay::LevelComplete:
                        m_playOverlay = PlayOverlay::None;
                        advanceGroupLevel();
                        break;
                    case PlayOverlay::GameOver:
                        m_playOverlay = PlayOverlay::None;
                        restartGroup();
                        break;
                    case PlayOverlay::WorldComplete:
                    case PlayOverlay::SingleWin:
                        m_playOverlay = PlayOverlay::None;
                        stopPlaying();
                        break;
                    case PlayOverlay::None: break;
                }
            }

            ImGui::Dummy(ImVec2(PANEL_W, 8));
        }
        ImGui::End();
    }
}

void EditorApplication::renderPopups() {
    showUnsavedChangesDialog();

    if (m_showSaveAs) {
        ImGui::OpenPopup("Save Level As");
        m_showSaveAs = false;
    }
    if (ImGui::BeginPopupModal("Save Level As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Filename:");
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##saveas", m_saveAsBuf, sizeof(m_saveAsBuf));
        ImGui::TextDisabled("Saved to assets/levels/");

        // Filename validation
        std::string name(m_saveAsBuf);
        bool nameValid = !name.empty();
        if (nameValid) {
            for (char c : name) {
                if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
                    c == '"' || c == '<' || c == '>' || c == '|') {
                    nameValid = false;
                    break;
                }
            }
        }
        if (!nameValid && !name.empty())
            ImGui::TextColored(ImVec4(1.f, 0.4f, 0.3f, 1.f), "Invalid filename characters");

        ImGui::Spacing();
        if (!nameValid) ImGui::BeginDisabled();
        if (ImGui::Button("Save", ImVec2(140, 0))) {
            if (name.size() < 5 || name.substr(name.size() - 5) != ".json")
                name += ".json";
            saveLevelAs(name);
            ImGui::CloseCurrentPopup();
        }
        if (!nameValid) ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(140, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (m_showLevelBrowser) {
        ImGui::OpenPopup("Level Browser");
        m_showLevelBrowser = false;
    }
    ImGui::SetNextWindowSize(ImVec2(500, 560), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Level Browser", nullptr, ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextColored(ImVec4(0.30f, 0.78f, 0.70f, 1.f), "Level Browser");
        ImGui::TextDisabled("Start a world or open a single level.");
        ImGui::Separator();
        ImGui::Spacing();

        // ========== Worlds section ==========
        ImGui::TextColored(ImVec4(0.55f, 0.85f, 1.0f, 1.f), "Worlds");
        ImGui::TextDisabled("Chained levels with lives and a running coin total.");
        ImGui::Spacing();

        ImGui::BeginChild("##WorldList", ImVec2(0, 180), true);
        if (m_groups.empty()) {
            ImGui::TextDisabled("No worlds found in assets/levels/groups/");
        } else {
            for (int i = 0; i < (int)m_groups.size(); ++i) {
                auto& g = m_groups[i];
                ImGui::PushID(i);

                ImGui::TextColored(ImVec4(1.f, 0.92f, 0.65f, 1.f), "%s", g.name.c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("(%d levels - %d lives)", (int)g.levels.size(), g.lives);

                // show level names disabled - only the first is actually reachable
                for (size_t li = 0; li < g.levels.size(); ++li) {
                    if (li == 0)
                        ImGui::TextDisabled("  1. %s  (start)", g.levels[li].c_str());
                    else
                        ImGui::TextDisabled("  %d. %s  (locked)", (int)li + 1, g.levels[li].c_str());
                }

                if (ImGui::Button("Start World", ImVec2(140, 0))) {
                    if (m_dirty) {
                        m_pendingAction = PendingAction::StartGroup;
                        m_pendingGroupIndex = i;
                    } else {
                        startGroup(i);
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                ImGui::PopID();
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();

        // ========== Single levels section ==========
        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.f), "Single Levels");
        ImGui::TextDisabled("Open any level for editing or solo play (no lives).");
        ImGui::Spacing();

        if (m_levelFiles.empty()) {
            ImGui::TextDisabled("No levels found in assets/levels/");
        } else {
            ImGui::BeginChild("##LevelList", ImVec2(0, -40), true);
            for (auto& file : m_levelFiles) {
                std::string display = file;
                auto slash = display.find_last_of("/\\");
                if (slash != std::string::npos) display = display.substr(slash + 1);

                bool current = (file == m_levelPath);
                if (current) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.42f, 0.38f, 1.f));
                if (ImGui::Button(display.c_str(), ImVec2(-1, 28))) {
                    // picking a single level always kills any lingering group session
                    endGroupSession();
                    if (m_dirty) {
                        m_pendingAction = PendingAction::LoadLevel;
                        m_pendingLoadPath = file;
                    } else {
                        loadLevel(file);
                    }
                    ImGui::CloseCurrentPopup();
                }
                if (current) ImGui::PopStyleColor();
                if (current) {
                    ImGui::SameLine();
                    ImGui::TextDisabled("(current)");
                }
            }
            ImGui::EndChild();
        }

        ImGui::Spacing();
        if (ImGui::Button("Refresh", ImVec2(120, 0))) {
            scanLevelFiles();
            scanLevelGroups();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (m_showAbout) {
        ImGui::OpenPopup("About");
        m_showAbout = false;
    }
    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.30f, 0.78f, 0.70f, 1.f), "2D Game Engine Editor");
        ImGui::TextDisabled("v1.0.0");
        ImGui::Separator();
        ImGui::Text("C++17 / SFML 2.x / Dear ImGui");
        ImGui::Text("UWE Bristol - Digital Systems Project");
        ImGui::Spacing();
        ImGui::TextDisabled("Engine / Editor / Game architecture");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(200, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

// menu bar - file/edit/view/help menus plus fps counter / status message on the right
void EditorApplication::renderMenuBar() {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Level", "Ctrl+N", false, !isPlaying())) {
            if (m_dirty) {
                m_pendingAction = PendingAction::NewLevel;
            } else {
                resetToNewLevel();
            }
        }

        if (ImGui::BeginMenu("Load Level")) {
            if (m_levelFiles.empty()) {
                ImGui::TextDisabled("No levels found in assets/levels/");
            } else {
                for (auto& file : m_levelFiles) {
                    std::string display = file;
                    if (file == m_levelPath) display += "  (current)";
                    if (ImGui::MenuItem(display.c_str())) {
                        if (m_dirty) {
                            m_pendingAction = PendingAction::LoadLevel;
                            m_pendingLoadPath = file;
                        } else {
                            loadLevel(file);
                        }
                    }
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Refresh")) scanLevelFiles();
            ImGui::EndMenu();
        }

        ImGui::Separator();

        bool canSave = !m_levelPath.empty() && !isPlaying();
        if (ImGui::MenuItem("Save", "Ctrl+S", false, canSave)) saveLevel();
        if (ImGui::MenuItem("Save As...", nullptr, false, !isPlaying())) {
            std::string defaultName = "my_level.json";
            if (!m_levelPath.empty()) {
                auto pos = m_levelPath.find_last_of('/');
                defaultName = (pos != std::string::npos) ? m_levelPath.substr(pos + 1) : m_levelPath;
            }
            snprintf(m_saveAsBuf, sizeof(m_saveAsBuf), "%s", defaultName.c_str());
            m_showSaveAs = true;
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4")) m_window.close();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, m_history.canUndo() && m_state == Engine::GameState::MENU)) {
            std::string desc = m_history.undoDescription();
            m_history.undo();
            validateSelection();
            findPlayer();
            markDirty();
            setStatus("Undo: " + desc);
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, m_history.canRedo() && m_state == Engine::GameState::MENU)) {
            m_history.redo();
            validateSelection();
            findPlayer();
            markDirty();
            setStatus("Redo: " + m_history.undoDescription());
        }
        ImGui::Separator();

        bool hasSelected = m_selected != nullptr && !isPlaying();
        if (ImGui::MenuItem("Add Entity", nullptr, false, !isPlaying())) addEntity();
        if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, hasSelected)) duplicateEntity();
        if (ImGui::MenuItem("Delete", "Del", false, !m_selection.empty() && m_state == Engine::GameState::MENU)) deleteSelected();
        ImGui::Separator();
        if (ImGui::MenuItem("Deselect", "Esc", false, !m_selection.empty())) {
            clearSelection();
            m_viewport.cancelDrag();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Scene Panel", nullptr, &m_scenePanel.isVisible());
        ImGui::MenuItem("Properties Panel", nullptr, &m_propsPanel.isVisible());
        ImGui::MenuItem("Game Viewport", nullptr, &m_viewport.isVisible());
        ImGui::MenuItem("Entity Palette", nullptr, &m_palette.isVisible());
        ImGui::Separator();
        ImGui::MenuItem("Grid", nullptr, &m_viewport.gridEnabled());
        if (ImGui::BeginMenu("Grid Size")) {
            float sizes[] = {8, 16, 32, 64};
            for (float s : sizes) {
                char label[16];
                snprintf(label, sizeof(label), "%.0fpx", s);
                if (ImGui::MenuItem(label, nullptr, m_viewport.gridSize() == s))
                    m_viewport.gridSize() = s;
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Reset Viewport")) m_viewport.resetView();
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        if (ImGui::BeginMenu("Shortcuts")) {
            ImGui::TextDisabled("Ctrl+N       New Level");
            ImGui::TextDisabled("Ctrl+S       Save Level");
            ImGui::TextDisabled("Ctrl+Z       Undo");
            ImGui::TextDisabled("Ctrl+Shift+Z Redo");
            ImGui::TextDisabled("Ctrl+D       Duplicate Entity");
            ImGui::TextDisabled("Ctrl+Click   Multi-select");
            ImGui::TextDisabled("Del          Delete Entity");
            ImGui::TextDisabled("Esc          Deselect / Pause");
            ImGui::TextDisabled("Space        Play");
            ImGui::TextDisabled("G            Toggle grid");
            ImGui::TextDisabled("F            Focus on selection");
            ImGui::TextDisabled("Middle-drag  Pan viewport");
            ImGui::TextDisabled("Scroll       Zoom viewport");
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("About")) m_showAbout = true;
        ImGui::EndMenu();
    }

    // right side of menu bar - status message fades out after 3s, shows fps when idle
    float rightEdge = ImGui::GetWindowWidth() - 20;
    if (m_statusTimer > 0) {
        float alpha = m_statusTimer < 0.5f ? m_statusTimer * 2.f : 1.f;   // fade out in last 0.5s
        ImGui::SameLine(rightEdge - ImGui::CalcTextSize(m_statusMsg.c_str()).x);
        ImGui::TextColored(ImVec4(0.30f, 0.78f, 0.70f, alpha), "%s", m_statusMsg.c_str());
    } else {
        // fps averaged over 0.5s window to prevent flickering
        m_fpsAccum += m_dt;
        m_fpsFrames++;
        if (m_fpsAccum >= 0.5f) {
            m_displayFps = m_fpsAccum > 0 ? (int)(m_fpsFrames / m_fpsAccum) : 60;
            m_fpsAccum = 0.f;
            m_fpsFrames = 0;
        }
        std::string txt = "FPS: " + std::to_string(m_displayFps);
        ImGui::SameLine(rightEdge - ImGui::CalcTextSize(txt.c_str()).x);
        ImGui::Text("%s", txt.c_str());
    }

    ImGui::EndMainMenuBar();
}

// toolbar strip below menu bar - play/stop, entity actions, undo/redo, grid, panel toggles
void EditorApplication::renderToolbar() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowPos(ImVec2(0, 19));
    ImGui::SetNextWindowSize(ImVec2((float)m_window.getSize().x, 40));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 5));

    if (ImGui::Begin("Toolbar", nullptr, flags)) {
        bool canPlay = m_viewport.isVisible() && m_game.getEntityManager().getEntityCount() > 0;

        if (!isPlaying()) {
            if (!canPlay) ImGui::BeginDisabled();
            if (ImGui::Button("Play") && canPlay) startPlaying();
            if (!canPlay) ImGui::EndDisabled();
            if (!canPlay && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip(m_game.getEntityManager().getEntityCount() == 0
                    ? "Load or create entities first" : "Open viewport first");
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.50f, 0.20f, 0.20f, 1.f));
            if (ImGui::Button("Stop Playing")) stopPlaying();
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        bool canStop = m_state == Engine::GameState::PLAYING || m_state == Engine::GameState::PAUSED;
        if (!canStop) ImGui::BeginDisabled();
        if (ImGui::Button("Stop") && canStop) stopPlaying();
        if (!canStop) ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Stop and restore entity positions");

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        const char* stateTxt = "EDITING";
        ImVec4 stateCol{0.50f, 0.55f, 0.80f, 1.f};
        if (m_state == Engine::GameState::PLAYING) { stateTxt = "PLAYING"; stateCol = {0.30f, 0.78f, 0.70f, 1.f}; }
        else if (m_state == Engine::GameState::PAUSED) { stateTxt = "PAUSED"; stateCol = {0.90f, 0.80f, 0.25f, 1.f}; }
        ImGui::TextColored(stateCol, "%s", stateTxt);

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        bool editing = !isPlaying();

        if (!editing) ImGui::BeginDisabled();
        if (ImGui::Button("+ Entity") && editing) addEntity();
        if (!editing) ImGui::EndDisabled();

        ImGui::SameLine();

        bool hasSelected = m_selected != nullptr && editing;
        if (!hasSelected) ImGui::BeginDisabled();
        if (ImGui::Button("Duplicate") && hasSelected) duplicateEntity();
        if (!hasSelected) ImGui::EndDisabled();

        ImGui::SameLine();

        bool canDelete = !m_selection.empty() && editing;
        if (!canDelete) ImGui::BeginDisabled();
        if (ImGui::Button("Delete") && canDelete) deleteSelected();
        if (!canDelete) ImGui::EndDisabled();

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        bool canUndo = m_history.canUndo() && m_state == Engine::GameState::MENU;
        bool canRedo = m_history.canRedo() && m_state == Engine::GameState::MENU;
        if (!canUndo) ImGui::BeginDisabled();
        if (ImGui::Button("Undo") && canUndo) {
            std::string desc = m_history.undoDescription();
            m_history.undo();
            validateSelection();
            findPlayer();
            setStatus("Undo: " + desc);
        }
        if (!canUndo) ImGui::EndDisabled();
        if (canUndo && ImGui::IsItemHovered())
            ImGui::SetTooltip("Undo: %s", m_history.undoDescription().c_str());

        ImGui::SameLine();
        if (!canRedo) ImGui::BeginDisabled();
        if (ImGui::Button("Redo") && canRedo) {
            m_history.redo();
            validateSelection();
            findPlayer();
            setStatus("Redo: " + m_history.undoDescription());
        }
        if (!canRedo) ImGui::EndDisabled();
        if (canRedo && ImGui::IsItemHovered())
            ImGui::SetTooltip("Redo: %s", m_history.redoDescription().c_str());

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        // disabled during play — cant hot-swap levels mid-sim (snapshot would be stale)
        if (isPlaying()) ImGui::BeginDisabled();
        if (ImGui::Button("Levels")) {
            scanLevelFiles();
            m_showLevelBrowser = true;
        }
        if (isPlaying()) ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Browse levels in assets/levels/");

        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();

        bool gridOn = m_viewport.gridEnabled();
        if (gridOn) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.42f, 0.38f, 1.f));
        if (ImGui::Button("Grid")) m_viewport.gridEnabled() = !m_viewport.gridEnabled();
        if (gridOn) ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) {
            if (gridOn)
                ImGui::SetTooltip("Grid: ON (%.0fpx) [G] — click to disable", m_viewport.gridSize());
            else
                ImGui::SetTooltip("Grid: OFF [G] — click to enable snap");
        }

        ImGui::SameLine();
        ImGui::Text("| Entities: %zu", m_game.getEntityManager().getEntityCount());
        if (m_selection.size() > 1) {
            ImGui::SameLine();
            ImGui::TextDisabled("(%zu sel)", m_selection.size());
        }

        // Zoom percentage input
        ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();
        int zoomPct = (int)(m_viewport.getZoom() * 100.f + 0.5f);
        ImGui::SetNextItemWidth(60);
        if (ImGui::DragInt("##Zoom", &zoomPct, 1, 25, 400, "%d%%")) {
            float newZoom = std::clamp(zoomPct / 100.f, 0.25f, 4.f);
            m_viewport.setZoom(newZoom);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Viewport zoom (scroll in viewport)");

        // level size spinners — hidden during play
        if (!isPlaying()) {
            ImGui::SameLine(); ImGui::Text("|"); ImGui::SameLine();
            ImGui::TextDisabled("Lvl:"); ImGui::SameLine();
            int lw = (int)m_levelWidth;
            ImGui::SetNextItemWidth(65);
            if (ImGui::DragInt("##lvlw", &lw, 4, 200, 10000, "%dw")) {
                m_levelWidth = (float)std::max(200, lw);
                m_viewport.setLevelSize(m_levelWidth, m_levelHeight);
                markDirty();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Level width (px) — drag to resize");
            ImGui::SameLine();
            int lh = (int)m_levelHeight;
            ImGui::SetNextItemWidth(65);
            if (ImGui::DragInt("##lvlh", &lh, 4, 200, 10000, "%dh")) {
                m_levelHeight = (float)std::max(200, lh);
                m_viewport.setLevelSize(m_levelWidth, m_levelHeight);
                markDirty();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Level height (px) — drag to resize");
            ImGui::SameLine();
            if (ImGui::Button("Fit")) fitLevelToContent();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Auto-resize level bounds to fit all entities (+ padding)");
        }

        // panel toggle buttons - only show on wide enough windows bc they'd overflow otherwise
        float toolbarW = ImGui::GetWindowWidth();
        if (toolbarW < 700) { ImGui::End(); ImGui::PopStyleVar(); return; }

        ImGui::SameLine(toolbarW - 320);
        ImGui::Text("|");
        ImGui::SameLine();

        auto activeCol = ImVec4(0.18f, 0.32f, 0.36f, 1.f);
        auto hiddenCol = ImVec4(0.30f, 0.16f, 0.16f, 1.f);

        ImGui::PushStyleColor(ImGuiCol_Button, m_scenePanel.isVisible() ? activeCol : hiddenCol);
        if (ImGui::Button("Scene")) m_scenePanel.isVisible() = !m_scenePanel.isVisible();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, m_propsPanel.isVisible() ? activeCol : hiddenCol);
        if (ImGui::Button("Properties")) m_propsPanel.isVisible() = !m_propsPanel.isVisible();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, m_viewport.isVisible() ? activeCol : hiddenCol);
        if (ImGui::Button("Viewport")) m_viewport.isVisible() = !m_viewport.isVisible();
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, m_palette.isVisible() ? activeCol : hiddenCol);
        if (ImGui::Button("Palette")) m_palette.isVisible() = !m_palette.isVisible();
        ImGui::PopStyleColor();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorApplication::loadLevel(const std::string& path) {
    m_history.clear();
    m_game.getEntityManager().clear();
    clearSelection();
    m_player = nullptr;
    m_playSnapshot.clear();

    auto entities = m_loader.loadFromJSON(path);
    if (entities.empty() && !m_loader.getLastError().empty()) {
        setStatus("Load failed!");
        std::cerr << "Load failed: " << m_loader.getLastError() << std::endl;
        return;
    }

    m_levelWidth = m_loader.getWidth();
    m_levelHeight = m_loader.getHeight();
    m_viewport.setLevelSize(m_levelWidth, m_levelHeight);   // viewport's bounds rect tracks actual level size

    for (auto* e : entities) m_game.getEntityManager().addEntity(e);

    // advance counter past the highest name suffix so new entities dont clash (resetting to entities.size() was wrong — "Platform 42" + 3 entities would reuse "4")
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
    findPlayer();
    m_levelPath = path;
    m_state = Engine::GameState::MENU;
    m_dirty = false;
    m_viewport.isVisible() = true;
    updateWindowTitle();

    auto pos = path.find_last_of('/');
    std::string name = (pos != std::string::npos) ? path.substr(pos + 1) : path;
    setStatus("Loaded " + name);
}

void EditorApplication::saveLevel() {
    if (m_levelPath.empty()) {
        snprintf(m_saveAsBuf, sizeof(m_saveAsBuf), "%s", "my_level.json");
        m_showSaveAs = true;
        return;
    }
    if (m_loader.saveToJSON(m_levelPath, m_game.getEntityManager().getAllEntities(), m_levelWidth, m_levelHeight)) {
        m_dirty = false;
        auto pos = m_levelPath.find_last_of('/');
        std::string name = (pos != std::string::npos) ? m_levelPath.substr(pos + 1) : m_levelPath;
        updateWindowTitle();
        setStatus("Saved " + name);
    } else {
        updateWindowTitle();
        setStatus("Save failed!");
        std::cerr << "Save failed: " << m_loader.getLastError() << std::endl;
    }
}

void EditorApplication::saveLevelAs(const std::string& filename) {
    m_levelPath = "assets/levels/" + filename;
    if (m_loader.saveToJSON(m_levelPath, m_game.getEntityManager().getAllEntities(), m_levelWidth, m_levelHeight)) {
        m_dirty = false;
        updateWindowTitle();
        scanLevelFiles();
        setStatus("Saved " + filename);
    } else {
        updateWindowTitle();
        setStatus("Save failed!");
        std::cerr << "Save failed: " << m_loader.getLastError() << std::endl;
    }
}

// generic add - spawns a plain platform at viewport centre, user can change type in properties
void EditorApplication::addEntity() {
    ++m_entityCounter;
    auto* e = new Engine::Entity("Platform " + std::to_string(m_entityCounter), "platform");
    sf::Vector2f viewOff = m_viewport.getViewOffset();
    // spawn at viewport centre so its visible immediately
    e->position = {viewOff.x + m_viewport.viewW() / 2.f - 32.f,
                   viewOff.y + m_viewport.viewH() / 2.f - 8.f};
    e->size = {64.f, 16.f};
    e->color = sf::Color(100, 100, 120);
    e->isStatic = true;
    e->hasGravity = false;
    m_game.getEntityManager().addEntity(e);
    selectEntity(e);
    m_viewport.isVisible() = true;

    m_history.push(std::make_unique<AddEntityCommand>(m_game.getEntityManager(), e));
    markDirty();
    setStatus("Entity created");
}

// template add from palette - creates a properly configured entity based on the chosen type
void EditorApplication::addFromTemplate(EntityTemplate tmpl) {
    ++m_entityCounter;
    Engine::Entity* e = nullptr;

    switch (tmpl) {
    case EntityTemplate::Player:
        e = m_factory.create("player");
        e->name = "Player";
        break;
    case EntityTemplate::Enemy:
        e = m_factory.create("enemy");
        e->name = "Enemy " + std::to_string(m_entityCounter);
        e->velocity = {80.f, 0.f};
        break;
    case EntityTemplate::Platform:
        e = m_factory.create("platform");
        e->name = "Platform " + std::to_string(m_entityCounter);
        e->size = {64.f, 16.f};
        e->color = sf::Color(100, 100, 120);
        e->isStatic = true;
        e->hasGravity = false;
        break;
    case EntityTemplate::LargePlatform:
        e = m_factory.create("platform");
        e->name = "Large Platform " + std::to_string(m_entityCounter);
        e->size = {192.f, 16.f};
        e->color = sf::Color(100, 100, 120);
        e->isStatic = true;
        e->hasGravity = false;
        break;
    case EntityTemplate::Ground:
        e = m_factory.create("platform");
        e->name = "Ground";
        e->size = {800.f, 32.f};
        e->color = sf::Color(60, 60, 80);
        e->isStatic = true;
        e->hasGravity = false;
        e->position = {0.f, 568.f};
        break;
    case EntityTemplate::Wall:
        e = m_factory.create("platform");
        e->name = "Wall " + std::to_string(m_entityCounter);
        e->size = {16.f, 96.f};
        e->color = sf::Color(80, 80, 100);
        e->isStatic = true;
        e->hasGravity = false;
        break;
    case EntityTemplate::FlyingEnemy:
        e = m_factory.create("flying_enemy");
        e->name = "Flying Enemy " + std::to_string(m_entityCounter);
        e->velocity = {60.f, 0.f};
        break;
    case EntityTemplate::ShootingEnemy:
        e = m_factory.create("shooting_enemy");
        e->name = "Shooting Enemy " + std::to_string(m_entityCounter);
        break;
    case EntityTemplate::Collectible:
        e = m_factory.create("collectible");
        e->name = "Collectible " + std::to_string(m_entityCounter);
        break;
    case EntityTemplate::MovingPlatform:
        e = m_factory.create("moving_platform");
        e->name = "Moving Platform " + std::to_string(m_entityCounter);
        break;
    case EntityTemplate::Goal:
        e = m_factory.create("goal");
        e->name = "Goal " + std::to_string(m_entityCounter);
        break;
    case EntityTemplate::Hazard:
        e = m_factory.create("hazard");
        e->name = "Hazard " + std::to_string(m_entityCounter);
        break;
    default:
        return;
    }

    // ground is pre-placed at y=568, everything else spawns at viewport centre
    if (tmpl != EntityTemplate::Ground) {
        sf::Vector2f viewOff = m_viewport.getViewOffset();
        float vw = m_viewport.viewW(), vh = m_viewport.viewH();
        e->position = {viewOff.x + vw / 2.f - e->size.x / 2.f,
                       viewOff.y + vh / 2.f - e->size.y / 2.f};
    }

    // snap pointA to spawn pos + offset pointB by 200, else the ghost endpoint shows at the level origin
    if (auto* mp = dynamic_cast<Game::MovingPlatform*>(e)) {
        mp->setPointA(e->position);
        mp->setPointB({e->position.x + 200.f, e->position.y});
    }

    m_game.getEntityManager().addEntity(e);
    selectEntity(e);
    m_viewport.isVisible() = true;
    findPlayer();

    m_history.push(std::make_unique<AddEntityCommand>(m_game.getEntityManager(), e));
    markDirty();
    setStatus(e->name + " created");
}

// duplicate copies all base fields + subclass properties via serialize/deserialize round-trip
void EditorApplication::duplicateEntity() {
    if (!m_selected) return;

    auto* e = m_factory.create(m_selected->type);
    std::string baseName = m_selected->name;
    while (baseName.size() > 5 && baseName.substr(baseName.size() - 5) == " Copy")
        baseName = baseName.substr(0, baseName.size() - 5);
    e->name = baseName + " Copy";
    e->type = m_selected->type;
    e->position = m_selected->position + sf::Vector2f(20.f, 20.f);
    e->size = m_selected->size;
    e->velocity = m_selected->velocity;
    e->color = m_selected->color;
    e->isStatic = m_selected->isStatic;
    e->hasGravity = m_selected->hasGravity;
    e->isTrigger = m_selected->isTrigger;
    e->texturePath = m_selected->texturePath;   // keep sprite - resolved against TextureManager on save+reload
    e->texture = m_selected->texture;           // pointer is stable (TextureManager uses std::map)
    e->deserializeProperties(m_selected->serializeProperties());
    m_game.getEntityManager().addEntity(e);
    selectEntity(e);

    m_history.push(std::make_unique<AddEntityCommand>(m_game.getEntityManager(), e));
    markDirty();
    setStatus("Entity duplicated");
}

// delete - single entity gets one DeleteEntityCommand, multi-select wrapped in CompoundCommand for single undo step
void EditorApplication::deleteSelected() {
    if (m_selection.empty()) return;

    auto toDelete = m_selection;   // copy bc clearSelection() wipes m_selection before we're done with it

    if (toDelete.size() == 1) {
        auto* e = toDelete[0];
        std::string name = e->name;
        if (e == m_player) m_player = nullptr;

        auto cmd = std::make_unique<DeleteEntityCommand>(m_game.getEntityManager(), e);
        cmd->execute();
        m_history.push(std::move(cmd));

        clearSelection();
        m_viewport.cancelDrag();
        markDirty();
        setStatus("Deleted " + name);
    } else {
        std::vector<std::unique_ptr<Command>> cmds;
        for (auto* e : toDelete) {
            if (e == m_player) m_player = nullptr;
            auto cmd = std::make_unique<DeleteEntityCommand>(m_game.getEntityManager(), e);
            cmd->execute();
            cmds.push_back(std::move(cmd));
        }
        std::string desc = "Delete " + std::to_string(toDelete.size()) + " entities";
        m_history.push(std::make_unique<CompoundCommand>(std::move(cmds), desc));

        clearSelection();
        m_viewport.cancelDrag();
        markDirty();
        setStatus("Deleted " + std::to_string(toDelete.size()) + " entities");
    }
}

void EditorApplication::setStatus(const std::string& msg) {
    m_statusMsg = msg;
    m_statusTimer = 3.f;
}

// scan for group manifests — called on init + browser refresh
void EditorApplication::scanLevelGroups() {
    m_groups = Engine::LevelGroupLoader::scanDirectory("assets/levels/groups");
}

// non-recursive scan — group manifests in assets/levels/groups/ dont end up here
void EditorApplication::scanLevelFiles() {
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

// capture position/velocity/isOnGround for every entity just before play starts
void EditorApplication::snapshotEntities() {
    m_playSnapshot.clear();
    for (auto* e : m_game.getEntityManager().getAllEntities()) {
        m_playSnapshot.push_back({e, e->position, e->velocity, e->isOnGround});
    }
}

// undo everything from play — re-add collected/defeated entities first (they were detached mid-play), then restore positions from snapshot
void EditorApplication::restoreSnapshot() {
    auto& collected = m_gameplay.collectedEntities();
    for (auto* e : collected) m_game.getEntityManager().addEntity(e);
    collected.clear();

    auto& defeated = m_gameplay.defeatedEntities();
    for (auto* e : defeated) m_game.getEntityManager().addEntity(e);
    defeated.clear();

    for (auto& snap : m_playSnapshot) {
        auto& ents = m_game.getEntityManager().getAllEntities();
        // entity might have been deleted mid-play (e.g. projectile hit) so check it still exists
        if (std::find(ents.begin(), ents.end(), snap.entity) != ents.end()) {
            snap.entity->position = snap.position;
            snap.entity->velocity = snap.velocity;
            snap.entity->isOnGround = snap.isOnGround;
        }
    }
    m_playSnapshot.clear();
}

void EditorApplication::updateWindowTitle() {
    std::string title = "2D Game Engine Editor";
    if (!m_levelPath.empty()) {
        auto pos = m_levelPath.find_last_of('/');
        std::string name = (pos != std::string::npos) ? m_levelPath.substr(pos + 1) : m_levelPath;
        title += " - " + name;
    }
    if (m_dirty) title += " *";
    m_window.setTitle(title);
}

// snapshot entities for non-destructive stop, save editor view, hand off to GameplaySystem::begin
void EditorApplication::startPlaying() {
    findPlayer();
    if (!m_player) {
        setStatus("No player entity in level!");
        return;
    }

    clearSelection();
    m_viewport.cancelDrag();
    snapshotEntities();
    m_savedViewOffset = m_viewport.getViewOffset();
    m_savedViewZoom = m_viewport.getZoom();

    Engine::GameplaySystem::Config cfg;
    cfg.app = &m_game;
    cfg.audio = &m_audio;
    cfg.camera = &m_camera;
    cfg.levelWidth = m_levelWidth;
    cfg.levelHeight = m_levelHeight;
    cfg.respawnYMargin = RESPAWN_Y_MARGIN;
    m_gameplay.begin(cfg);

    m_state = Engine::GameState::PLAYING;
}

// tear down gameplay, restore snapshot, reset view. stopping mid-group = abandon the run
void EditorApplication::stopPlaying() {
    m_state = Engine::GameState::MENU;
    m_gameplay.end();
    restoreSnapshot();
    m_viewport.setViewOffset(m_savedViewOffset);
    m_viewport.setZoom(m_savedViewZoom);
    m_audio.stopAll();
    m_playOverlay = PlayOverlay::None;
    endGroupSession();
}

// clear group state, does NOT stop playing — called on natural end or abandon
void EditorApplication::endGroupSession() {
    m_groupSession = GroupSession{};
}

// start a world from level 0 — loads first level and enters play immediately so its one click from the browser
void EditorApplication::startGroup(int groupIndex) {
    if (groupIndex < 0 || groupIndex >= (int)m_groups.size()) return;
    auto& g = m_groups[groupIndex];
    if (g.levels.empty()) return;

    m_groupSession.active = true;
    m_groupSession.groupIndex = groupIndex;
    m_groupSession.currentLevel = 0;
    m_groupSession.livesRemaining = g.lives;
    m_groupSession.totalCoins = 0;
    m_levelCoinsSnapshot = 0;
    m_playOverlay = PlayOverlay::None;

    loadLevel("assets/levels/" + g.levels.front());
    if (m_game.getEntityManager().getEntityCount() > 0) {
        startPlaying();
    } else {
        endGroupSession();
        setStatus("Failed to load first level of world");
    }
}

// advance to the next level in the group, preserving lives and running total
void EditorApplication::advanceGroupLevel() {
    if (!m_groupSession.active) return;
    if (m_groupSession.groupIndex < 0 ||
        m_groupSession.groupIndex >= (int)m_groups.size()) return;

    auto& g = m_groups[m_groupSession.groupIndex];
    m_groupSession.currentLevel++;
    if (m_groupSession.currentLevel >= (int)g.levels.size()) {
        // should only happen if called after the final level - defensive.
        stopPlaying();
        return;
    }

    // preserve camera state across transition — startPlaying re-snapshots every call so without this it drifts
    auto savedOff = m_savedViewOffset;
    float savedZoom = m_savedViewZoom;

    // reenter play on the next level without touching lives/totalCoins.
    m_state = Engine::GameState::MENU;
    m_gameplay.end();
    restoreSnapshot();
    m_audio.stopAll();

    loadLevel("assets/levels/" + g.levels[m_groupSession.currentLevel]);
    if (m_game.getEntityManager().getEntityCount() > 0) {
        startPlaying();
        m_savedViewOffset = savedOff;
        m_savedViewZoom = savedZoom;
    } else {
        endGroupSession();
        setStatus("Failed to load next level");
    }
}

// all lives lost — full world restart from level 0
void EditorApplication::restartGroup() {
    if (!m_groupSession.active) return;
    if (m_groupSession.groupIndex < 0 ||
        m_groupSession.groupIndex >= (int)m_groups.size()) return;

    int groupIdx = m_groupSession.groupIndex;
    // preserve pre-group editor camera so returning to the editor lands where the user left it
    auto savedOff = m_savedViewOffset;
    float savedZoom = m_savedViewZoom;

    // stop cleanly, then restart via startGroup which resets all counters
    m_state = Engine::GameState::MENU;
    m_gameplay.end();
    restoreSnapshot();
    m_audio.stopAll();
    endGroupSession();

    startGroup(groupIdx);
    if (m_groupSession.active) {
        m_savedViewOffset = savedOff;
        m_savedViewZoom = savedZoom;
    }
}


// linear scan for the first entity with type=="player" - called after any structural change
void EditorApplication::findPlayer() {
    m_player = nullptr;
    for (auto* e : m_game.getEntityManager().getAllEntities())
        if (e->type == "player") { m_player = e; return; }
}

void EditorApplication::markDirty() {
    if (!m_dirty) {
        m_dirty = true;
        updateWindowTitle();
    }
}

// find bottom-right corner that encloses all entities + pad for breathing room
void EditorApplication::fitLevelToContent() {
    auto& ents = m_game.getEntityManager().getAllEntities();
    if (ents.empty()) {
        setStatus("No entities to fit to");
        return;
    }
    float maxX = 0.f, maxY = 0.f;
    for (auto* e : ents) {
        float right = e->position.x + e->size.x;
        float bottom = e->position.y + e->size.y;
        if (right > maxX) maxX = right;
        if (bottom > maxY) maxY = bottom;
    }
    constexpr float PAD = 120.f;   // breathing room on right/bottom
    m_levelWidth = std::max(400.f, maxX + PAD);
    m_levelHeight = std::max(300.f, maxY + PAD);
    m_viewport.setLevelSize(m_levelWidth, m_levelHeight);
    markDirty();
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Level resized to %.0f x %.0f", m_levelWidth, m_levelHeight);
    setStatus(buf);
}

void EditorApplication::resetToNewLevel() {
    m_history.clear();
    m_game.getEntityManager().clear();
    clearSelection();
    m_player = nullptr;
    m_levelPath.clear();
    m_levelWidth = DEFAULT_W;
    m_levelHeight = DEFAULT_H;
    m_viewport.setLevelSize(m_levelWidth, m_levelHeight);
    m_state = Engine::GameState::MENU;
    m_dirty = false;
    updateWindowTitle();
    setStatus("New level");
}

void EditorApplication::showUnsavedChangesDialog() {
    if (m_pendingAction == PendingAction::None) return;

    ImGui::OpenPopup("Unsaved Changes");
    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have unsaved changes. What would you like to do?");
        ImGui::Spacing();

        // run the stashed action (same handler for Save + Discard, only difference is whether we saved first)
        auto runPending = [&]() {
            auto action = m_pendingAction;
            auto path = m_pendingLoadPath;
            int groupIdx = m_pendingGroupIndex;
            m_pendingAction = PendingAction::None;
            m_pendingLoadPath.clear();
            m_pendingGroupIndex = -1;

            if (action == PendingAction::NewLevel) {
                resetToNewLevel();
            } else if (action == PendingAction::LoadLevel) {
                loadLevel(path);
            } else if (action == PendingAction::StartGroup) {
                startGroup(groupIdx);
            } else if (action == PendingAction::CloseWindow) {
                m_window.close();
            }
        };

        if (ImGui::Button("Save", ImVec2(120, 0))) {
            saveLevel();
            ImGui::CloseCurrentPopup();
            runPending();
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            m_dirty = false;
            runPending();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            m_pendingAction = PendingAction::None;
            m_pendingLoadPath.clear();
            m_pendingGroupIndex = -1;
        }
        ImGui::EndPopup();
    }
}

void EditorApplication::selectEntity(Engine::Entity* e) {
    m_selection.clear();
    if (e) m_selection.push_back(e);
    m_selected = e;
}

void EditorApplication::toggleSelect(Engine::Entity* e) {
    auto it = std::find(m_selection.begin(), m_selection.end(), e);
    if (it != m_selection.end()) {
        m_selection.erase(it);
        if (m_selected == e)
            m_selected = m_selection.empty() ? nullptr : m_selection.back();
    } else {
        m_selection.push_back(e);
        m_selected = e;
    }
}

void EditorApplication::clearSelection() {
    m_selection.clear();
    m_selected = nullptr;
}

bool EditorApplication::isEntitySelected(Engine::Entity* e) const {
    return std::find(m_selection.begin(), m_selection.end(), e) != m_selection.end();
}

// removes dangling pointers from selection after undo/redo - entities might have been deleted
void EditorApplication::validateSelection() {
    auto& ents = m_game.getEntityManager().getAllEntities();
    m_selection.erase(
        std::remove_if(m_selection.begin(), m_selection.end(),
            [&](Engine::Entity* e) {
                return std::find(ents.begin(), ents.end(), e) == ents.end();
            }),
        m_selection.end()
    );
    if (m_selected && std::find(ents.begin(), ents.end(), m_selected) == ents.end())
        m_selected = m_selection.empty() ? nullptr : m_selection.back();
    if (m_player && std::find(ents.begin(), ents.end(), m_player) == ents.end())
        m_player = nullptr;
}

}
