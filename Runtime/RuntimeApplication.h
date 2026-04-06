#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include "Engine/Core/Application.h"
#include "Engine/Entity/EntityFactory.h"
#include "Engine/Level/LevelLoader.h"
#include "Engine/Level/LevelGroup.h"
#include "Engine/Rendering/Camera.h"
#include "Engine/Rendering/TextureManager.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/Gameplay/GameplaySystem.h"
#include <string>

namespace Runtime {

enum class Mode { Level, World };

struct LaunchOptions {
    Mode mode = Mode::Level;
    std::string path;
    unsigned int width = 1280;
    unsigned int height = 720;
};

class RuntimeApplication {
public:
    bool initialize(const LaunchOptions& opts);
    void run();

private:
    enum class State { Playing, Paused, Win, GameOver };

    bool loadLevel(const std::string& path);
    bool loadWorld(const std::string& path);
    void startLevel();
    void tick(float dt);
    void render();
    void handleEvent(const sf::Event& ev);
    void advanceWorld();
    void restartLevel();
    void drawOverlay();
    void drawText(const std::string& s, float x, float y, unsigned int size,
                  sf::Color color = sf::Color::White);

    LaunchOptions m_opts;
    sf::RenderWindow m_window;
    sf::Font m_font;
    bool m_hasFont = false;

    Engine::Application m_app;
    Engine::EntityFactory m_factory;
    Engine::LevelLoader m_loader;
    Engine::Camera m_camera;
    Engine::TextureManager m_textures;
    Engine::AudioManager m_audio;
    Engine::GameplaySystem m_gameplay;

    Engine::LevelGroup m_group;
    int m_worldIndex = 0;
    int m_lives = 3;

    State m_state = State::Playing;
    float m_accumulator = 0.f;
    float m_levelWidth = 800.f;
    float m_levelHeight = 600.f;
};

}
