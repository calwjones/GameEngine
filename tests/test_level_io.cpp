#include <catch2/catch_test_macros.hpp>
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityFactory.h"
#include "Engine/Entity/EntityTypeRegistry.h"
#include "Engine/Level/LevelLoader.h"
#include <filesystem>

using Engine::Entity;
using Engine::EntityFactory;
using Engine::LevelLoader;

namespace {
struct TempFile {
    std::filesystem::path path;
    TempFile() {
        auto dir = std::filesystem::temp_directory_path();
        path = dir / ("level_io_test_" + std::to_string(std::rand()) + ".json");
    }
    ~TempFile() { std::error_code ec; std::filesystem::remove(path, ec); }
};

struct EntityBag {
    std::vector<Entity*> entities;
    ~EntityBag() { for (auto* e : entities) delete e; }
};
}

TEST_CASE("save/load round-trip preserves core fields", "[level]") {
    EntityFactory factory;
    Engine::registerBuiltinTypes(factory);

    LevelLoader loader;
    loader.setFactory(&factory);

    EntityBag src;
    auto* player = factory.create("player");
    player->name = "p1";
    player->position = {32.f, 64.f};
    player->size = {24.f, 48.f};
    player->color = sf::Color(10, 20, 30, 40);
    src.entities.push_back(player);

    auto* block = new Entity("block1", "platform");
    block->position = {100.f, 200.f};
    block->size = {64.f, 16.f};
    block->isStatic = true;
    block->hasGravity = false;
    src.entities.push_back(block);

    TempFile tmp;
    REQUIRE(loader.saveToJSON(tmp.path.string(), src.entities, 1024.f, 768.f));

    EntityBag dst;
    dst.entities = loader.loadFromJSON(tmp.path.string());
    REQUIRE(dst.entities.size() == 2);
    REQUIRE(loader.getWidth() == 1024.f);
    REQUIRE(loader.getHeight() == 768.f);

    Entity* p2 = nullptr;
    Entity* b2 = nullptr;
    for (auto* e : dst.entities) {
        if (e->name == "p1") p2 = e;
        if (e->name == "block1") b2 = e;
    }
    REQUIRE(p2 != nullptr);
    REQUIRE(p2->type == "player");
    REQUIRE(p2->position == sf::Vector2f{32.f, 64.f});
    REQUIRE(p2->size == sf::Vector2f{24.f, 48.f});
    REQUIRE(p2->color == sf::Color(10, 20, 30, 40));

    REQUIRE(b2 != nullptr);
    REQUIRE(b2->type == "platform");
    REQUIRE(b2->isStatic);
    REQUIRE_FALSE(b2->hasGravity);
}

TEST_CASE("load reports error for missing file", "[level]") {
    LevelLoader loader;
    auto ents = loader.loadFromJSON("/does/not/exist/nope.json");
    REQUIRE(ents.empty());
    REQUIRE_FALSE(loader.getLastError().empty());
}

TEST_CASE("unknown types fall back to generic Entity", "[level]") {
    EntityFactory factory;
    Engine::registerBuiltinTypes(factory);
    LevelLoader loader;
    loader.setFactory(&factory);

    EntityBag src;
    auto* weird = new Entity("mystery", "weird_thing");
    weird->position = {5.f, 5.f};
    src.entities.push_back(weird);

    TempFile tmp;
    REQUIRE(loader.saveToJSON(tmp.path.string(), src.entities));

    EntityBag dst;
    dst.entities = loader.loadFromJSON(tmp.path.string());
    REQUIRE(dst.entities.size() == 1);
    REQUIRE(dst.entities[0]->type == "weird_thing");
}
