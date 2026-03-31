#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <string>
#include "../Engine/Core/Application.h"
#include "../Engine/Entity/EntityFactory.h"
#include "../Engine/Level/LevelLoader.h"
#include "../Engine/Rendering/Camera.h"
#include "../Engine/Audio/AudioManager.h"
#include "../Engine/Rendering/TextureManager.h"
#include "../Engine/State/StateManager.h"
#include "GameViewport.h"
#include "ScenePanel.h"
#include "PropertiesPanel.h"
#include "EntityPalette.h"
#include "CommandHistory.h"

namespace Editor {

class SelectionController;
class LevelIO;
class PlaySessionController;
class EntityOps;
class PopupRenderer;

struct EditorContext {
    sf::RenderWindow&       window;
    Engine::Application&    game;
    Engine::EntityFactory&  factory;
    Engine::LevelLoader&    loader;
    Engine::Camera&         camera;
    Engine::AudioManager&   audio;
    Engine::TextureManager& textures;

    GameViewport&           viewport;
    ScenePanel&             scenePanel;
    PropertiesPanel&        propsPanel;
    EntityPalette&          palette;
    CommandHistory&         history;

    Engine::GameState&      state;
    bool&                   dirty;
    std::string&            statusMsg;
    float&                  statusTimer;

    SelectionController*    selection    = nullptr;
    LevelIO*                levelIO      = nullptr;
    PlaySessionController*  playSession  = nullptr;
    EntityOps*              entityOps    = nullptr;
    PopupRenderer*          popups       = nullptr;

    void setStatus(const std::string& m) { statusMsg = m; statusTimer = 3.f; }
    void markDirty() { dirty = true; }
};

}
