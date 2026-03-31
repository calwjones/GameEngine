#pragma once
#include <SFML/Graphics.hpp>
#include "../Engine/Core/Application.h"
#include "../Engine/State/StateManager.h"
#include "../Engine/Level/LevelLoader.h"
#include "../Engine/Entity/EntityFactory.h"
#include "../Engine/Rendering/Camera.h"
#include "../Engine/Audio/AudioManager.h"
#include "../Engine/Rendering/TextureManager.h"
#include "ScenePanel.h"
#include "PropertiesPanel.h"
#include "GameViewport.h"
#include "EntityPalette.h"
#include "CommandHistory.h"
#include "EditorContext.h"
#include "SelectionController.h"
#include "LevelIO.h"
#include "PlaySessionController.h"
#include "EntityOps.h"
#include "PopupRenderer.h"
#include "MenuBarRenderer.h"
#include "ToolbarRenderer.h"
#include "OverlayRenderer.h"
#include <string>

namespace Editor {

class EditorApplication {
    sf::RenderWindow m_window;
    Engine::Application m_game;
    Engine::EntityFactory m_factory;
    Engine::LevelLoader m_loader;
    Engine::Camera m_camera;
    Engine::AudioManager m_audio;
    Engine::TextureManager m_textures;

    ScenePanel m_scenePanel;
    PropertiesPanel m_propsPanel;
    GameViewport m_viewport;
    EntityPalette m_palette;
    CommandHistory m_history;

    Engine::GameState m_state = Engine::GameState::MENU;
    sf::Clock m_clock;
    float m_dt = 0.f;
    bool m_dirty = false;
    bool m_imguiInitialized = false;

    std::string m_statusMsg;
    float m_statusTimer = 0.f;

    EditorContext m_ctx{
        m_window, m_game, m_factory, m_loader, m_camera, m_audio, m_textures,
        m_viewport, m_scenePanel, m_propsPanel, m_palette, m_history,
        m_state, m_dirty, m_statusMsg, m_statusTimer
    };
    SelectionController m_selection{m_ctx};
    LevelIO m_levelIO{m_ctx};
    PlaySessionController m_playSession{m_ctx};
    EntityOps m_entityOps{m_ctx};
    PopupRenderer m_popups{m_ctx};
    MenuBarRenderer m_menuBar{m_ctx};
    ToolbarRenderer m_toolbar{m_ctx};
    OverlayRenderer m_overlay{m_ctx};

public:
    ~EditorApplication() { shutdown(); }

    bool initialize(unsigned int w = 1280, unsigned int h = 720);
    void run();
    void shutdown();

private:
    void processEvents();
    void update(float dt);
    void render();

    void renderImGui();

    void handleInput(sf::Keyboard::Key key);

    void setStatus(const std::string& msg);
    void markDirty();
    bool isPlaying() const { return m_state == Engine::GameState::PLAYING || m_state == Engine::GameState::PAUSED; }
};

}
