#include <catch2/catch_test_macros.hpp>
#include "Engine/Core/Application.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/Gameplay/GameplaySystem.h"
#include "Engine/Entity/EntityManager.h"
#include "Game/Player.h"
#include "Game/Enemy.h"
#include "Game/Hazard.h"
#include "Game/Goal.h"
#include "Game/Projectile.h"

using Engine::Application;
using Engine::AudioManager;
using Engine::GameplaySystem;

namespace {
GameplaySystem::Config makeCfg(Application& app, AudioManager& audio) {
    GameplaySystem::Config cfg;
    cfg.app = &app;
    cfg.audio = &audio;
    cfg.camera = nullptr;
    cfg.levelWidth = 2000.f;
    cfg.levelHeight = 1000.f;
    cfg.respawnYMargin = 100.f;
    return cfg;
}
}

TEST_CASE("player respawns on hazard touch", "[gameplay]") {
    Application app;
    AudioManager audio;
    audio.setEnabled(false);

    auto& em = app.getEntityManager();
    auto* player = em.spawn<Game::Player>();
    player->position = {100.f, 100.f};
    player->hasGravity = false;

    auto* hazard = em.spawn<Game::Hazard>();
    hazard->position = {200.f, 200.f};

    GameplaySystem gs;
    gs.begin(makeCfg(app, audio));

    player->position = {200.f, 200.f};
    gs.tick(1.f / 60.f);

    REQUIRE(player->position == sf::Vector2f{100.f, 100.f});
    REQUIRE(gs.consumeDeaths() == 1);

    gs.end();
}

TEST_CASE("player wins on goal touch", "[gameplay]") {
    Application app;
    AudioManager audio;
    audio.setEnabled(false);

    auto& em = app.getEntityManager();
    auto* player = em.spawn<Game::Player>();
    player->position = {100.f, 100.f};
    player->hasGravity = false;

    auto* goal = em.spawn<Game::Goal>();
    goal->position = {300.f, 300.f};
    goal->nextLevel = "level2.json";

    GameplaySystem gs;
    gs.begin(makeCfg(app, audio));

    REQUIRE_FALSE(gs.won());

    player->position = {300.f, 300.f};
    gs.tick(1.f / 60.f);

    REQUIRE(gs.won());
    REQUIRE(gs.nextLevel() == "level2.json");

    gs.end();
}

TEST_CASE("falling player stomps enemy", "[gameplay]") {
    Application app;
    AudioManager audio;
    audio.setEnabled(false);

    auto& em = app.getEntityManager();
    auto* player = em.spawn<Game::Player>();
    player->hasGravity = false;
    player->position = {100.f, 100.f};
    player->size = {32.f, 48.f};
    player->velocity = {0.f, 50.f};

    auto* enemy = em.spawn<Game::Enemy>();
    enemy->hasGravity = false;
    enemy->position = {100.f, 140.f};

    GameplaySystem gs;
    gs.begin(makeCfg(app, audio));
    gs.tick(1.f / 60.f);

    REQUIRE(gs.defeatedEntities().size() == 1);
    REQUIRE(gs.defeatedEntities().front() == enemy);
    REQUIRE(player->velocity.y < 0.f);

    // defeated detached; test owns it now
    delete enemy;
    gs.defeatedEntities().clear();
    gs.end();
}

TEST_CASE("projectile ages and expires", "[gameplay]") {
    Game::Projectile p;
    p.setLifetime(0.25f);
    p.velocity = {100.f, 0.f};

    REQUIRE_FALSE(p.isExpired());
    for (int i = 0; i < 10; ++i) p.update(1.f / 60.f);
    REQUIRE_FALSE(p.isExpired());
    for (int i = 0; i < 10; ++i) p.update(1.f / 60.f);
    REQUIRE(p.isExpired());

    p.resetAge();
    REQUIRE_FALSE(p.isExpired());
}

TEST_CASE("falling off the level respawns player", "[gameplay]") {
    Application app;
    AudioManager audio;
    audio.setEnabled(false);

    auto& em = app.getEntityManager();
    auto* player = em.spawn<Game::Player>();
    player->hasGravity = false;
    player->position = {100.f, 100.f};

    GameplaySystem gs;
    gs.begin(makeCfg(app, audio));

    player->position = {100.f, 2000.f};
    gs.tick(1.f / 60.f);

    REQUIRE(player->position == sf::Vector2f{100.f, 100.f});
    REQUIRE(gs.consumeDeaths() == 1);

    gs.end();
}
