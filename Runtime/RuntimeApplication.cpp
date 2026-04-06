#include "RuntimeApplication.h"
#include "Engine/Entity/EntityTypeRegistry.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

namespace Runtime {

namespace {
constexpr float kFixedDt = 1.f / 60.f;

// first font that loads wins; keeps the runtime zero-asset
const char* kFontCandidates[] = {
    "/System/Library/Fonts/Supplemental/Arial.ttf",
    "/System/Library/Fonts/Helvetica.ttc",
    "/Library/Fonts/Arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "C:/Windows/Fonts/arial.ttf",
};
}

bool RuntimeApplication::initialize(const LaunchOptions& opts) {
    m_opts = opts;

    m_window.create(sf::VideoMode(opts.width, opts.height), "GameEngine Runtime",
                    sf::Style::Titlebar | sf::Style::Close);
    m_window.setFramerateLimit(60);

    if (!m_app.initialize(&m_window)) return false;

    Engine::registerBuiltinTypes(m_factory);
    m_loader.setFactory(&m_factory);

    for (const char* p : kFontCandidates) {
        if (m_font.loadFromFile(p)) { m_hasFont = true; break; }
    }

    m_camera.setViewSize((float)opts.width, (float)opts.height);

    if (opts.mode == Mode::Level) {
        if (!loadLevel(opts.path)) {
            std::cerr << "Failed to load level: " << opts.path << "\n";
            return false;
        }
    } else {
        if (!loadWorld(opts.path)) {
            std::cerr << "Failed to load world: " << opts.path << "\n";
            return false;
        }
    }

    startLevel();
    return true;
}

bool RuntimeApplication::loadLevel(const std::string& path) {
    auto ents = m_loader.loadFromJSON(path);
    if (ents.empty()) return false;

    m_app.getEntityManager().clear();
    for (auto* e : ents) {
        if (!e->texturePath.empty()) {
            auto* tex = m_textures.getTexture(e->texturePath);
            if (tex) e->texture = tex;
        }
        m_app.getEntityManager().addEntity(e);
    }

    m_levelWidth = m_loader.getWidth();
    m_levelHeight = m_loader.getHeight();
    return true;
}

bool RuntimeApplication::loadWorld(const std::string& path) {
    if (!Engine::LevelGroupLoader::loadFromJSON(path, m_group)) return false;
    if (m_group.levels.empty()) return false;
    m_worldIndex = 0;
    m_lives = m_group.lives > 0 ? m_group.lives : 3;

    // group entries are level filenames relative to assets/levels/
    return loadLevel("assets/levels/" + m_group.levels[m_worldIndex]);
}

void RuntimeApplication::startLevel() {
    Engine::GameplaySystem::Config cfg;
    cfg.app = &m_app;
    cfg.audio = &m_audio;
    cfg.camera = &m_camera;
    cfg.levelWidth = m_levelWidth;
    cfg.levelHeight = m_levelHeight;
    m_gameplay.begin(cfg);

    m_state = State::Playing;
    m_accumulator = 0.f;
}

void RuntimeApplication::restartLevel() {
    if (m_opts.mode == Mode::Level) {
        loadLevel(m_opts.path);
    } else {
        loadLevel("assets/levels/" + m_group.levels[m_worldIndex]);
    }
    startLevel();
}

void RuntimeApplication::advanceWorld() {
    if (m_opts.mode != Mode::World) return;
    if (m_worldIndex + 1 >= (int)m_group.levels.size()) {
        m_state = State::Win;
        return;
    }
    ++m_worldIndex;
    loadLevel("assets/levels/" + m_group.levels[m_worldIndex]);
    startLevel();
}

void RuntimeApplication::handleEvent(const sf::Event& ev) {
    if (ev.type == sf::Event::Closed) {
        m_window.close();
        return;
    }
    if (ev.type != sf::Event::KeyPressed) return;

    if (ev.key.code == sf::Keyboard::Escape) {
        if (m_state == State::Playing) m_state = State::Paused;
        else if (m_state == State::Paused) m_state = State::Playing;
        return;
    }

    if (m_state == State::GameOver && ev.key.code == sf::Keyboard::R) {
        m_lives = m_group.lives > 0 ? m_group.lives : 3;
        m_worldIndex = 0;
        restartLevel();
    } else if (m_state == State::Win && ev.key.code == sf::Keyboard::R) {
        m_worldIndex = 0;
        restartLevel();
    }
}

void RuntimeApplication::tick(float dt) {
    if (m_state != State::Playing) return;

    m_accumulator += dt;
    if (m_accumulator > 0.25f) m_accumulator = 0.25f;
    while (m_accumulator >= kFixedDt) {
        m_gameplay.tick(kFixedDt);
        m_accumulator -= kFixedDt;

        int deaths = m_gameplay.consumeDeaths();
        if (deaths > 0 && m_opts.mode == Mode::World) {
            m_lives -= deaths;
            if (m_lives <= 0) {
                m_state = State::GameOver;
                return;
            }
        }

        if (m_gameplay.won()) {
            if (m_opts.mode == Mode::Level) {
                m_state = State::Win;
            } else {
                advanceWorld();
            }
            return;
        }
    }
}

void RuntimeApplication::drawText(const std::string& s, float x, float y,
                                   unsigned int size, sf::Color color) {
    if (!m_hasFont) return;
    sf::Text t(s, m_font, size);
    t.setPosition(x, y);
    t.setFillColor(color);
    m_window.draw(t);
}

void RuntimeApplication::drawOverlay() {
    sf::View prev = m_window.getView();
    m_window.setView(m_window.getDefaultView());

    float w = (float)m_window.getSize().x;
    float h = (float)m_window.getSize().y;

    if (m_state == State::Paused) {
        sf::RectangleShape dim({w, h});
        dim.setFillColor(sf::Color(0, 0, 0, 150));
        m_window.draw(dim);
        drawText("PAUSED", w * 0.5f - 80.f, h * 0.5f - 30.f, 48);
        drawText("esc to resume", w * 0.5f - 75.f, h * 0.5f + 30.f, 18);
    } else if (m_state == State::Win) {
        sf::RectangleShape dim({w, h});
        dim.setFillColor(sf::Color(0, 40, 0, 180));
        m_window.draw(dim);
        drawText("YOU WIN", w * 0.5f - 90.f, h * 0.5f - 40.f, 52, sf::Color(255, 215, 0));
        drawText("r to restart", w * 0.5f - 65.f, h * 0.5f + 30.f, 18);
    } else if (m_state == State::GameOver) {
        sf::RectangleShape dim({w, h});
        dim.setFillColor(sf::Color(40, 0, 0, 180));
        m_window.draw(dim);
        drawText("GAME OVER", w * 0.5f - 120.f, h * 0.5f - 40.f, 52, sf::Color(220, 60, 60));
        drawText("r to restart", w * 0.5f - 65.f, h * 0.5f + 30.f, 18);
    }

    // HUD
    if (m_state == State::Playing || m_state == State::Paused) {
        drawText("Score: " + std::to_string(m_gameplay.score()), 16.f, 12.f, 22);
        if (m_opts.mode == Mode::World)
            drawText("Lives: " + std::to_string(m_lives), 16.f, 40.f, 22);
    }

    m_window.setView(prev);
}

void RuntimeApplication::render() {
    m_window.clear(sf::Color(40, 44, 52));

    sf::View view = m_window.getDefaultView();
    view.setSize((float)m_opts.width, (float)m_opts.height);
    view.setCenter(m_camera.getPosition());
    m_window.setView(view);

    m_app.render(m_window);
    drawOverlay();
    m_window.display();
}

void RuntimeApplication::run() {
    sf::Clock clock;
    while (m_window.isOpen()) {
        sf::Event ev;
        while (m_window.pollEvent(ev)) handleEvent(ev);

        float dt = clock.restart().asSeconds();
        tick(dt);
        render();
    }
}

}
