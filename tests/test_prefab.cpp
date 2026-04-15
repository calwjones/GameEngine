#include <catch2/catch_test_macros.hpp>
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityFactory.h"
#include "Engine/Entity/EntityTypeRegistry.h"
#include "Engine/Level/LevelLoader.h"
#include <filesystem>
#include <fstream>

using Engine::Entity;
using Engine::EntityFactory;
using Engine::LevelLoader;

namespace {
struct TempFile {
    std::filesystem::path path;
    TempFile() {
        auto dir = std::filesystem::temp_directory_path();
        path = dir / ("prefab_test_" + std::to_string(std::rand()) + ".json");
    }
    ~TempFile() { std::error_code ec; std::filesystem::remove(path, ec); }
};
}

TEST_CASE("prefab save and load preserves entity fields", "[prefab]") {
    EntityFactory factory;
    Engine::registerBuiltinTypes(factory);

    LevelLoader loader;
    loader.setFactory(&factory);

    auto* src = factory.create("shooting_enemy");
    src->name = "Turret";
    src->position = {120.f, 60.f};
    src->size = {40.f, 40.f};
    src->velocity = {0.f, 0.f};
    src->color = sf::Color(200, 80, 20, 255);
    src->isStatic = false;
    src->hasGravity = true;
    src->isTrigger = false;

    TempFile tmp;
    std::vector<Entity*> one{src};
    REQUIRE(loader.saveToJSON(tmp.path.string(), one, 0.f, 0.f, nullptr));

    auto loaded = loader.loadFromJSON(tmp.path.string());
    REQUIRE(loaded.size() == 1);

    Entity* dst = loaded.front();
    REQUIRE(dst->type == "shooting_enemy");
    REQUIRE(dst->name == "Turret");
    REQUIRE(dst->position.x == 120.f);
    REQUIRE(dst->position.y == 60.f);
    REQUIRE(dst->size.x == 40.f);
    REQUIRE(dst->size.y == 40.f);
    REQUIRE(dst->color == sf::Color(200, 80, 20, 255));
    REQUIRE(dst->hasGravity);
    REQUIRE_FALSE(dst->isStatic);

    delete src;
    for (auto* e : loaded) delete e;
}

TEST_CASE("prefab load survives missing optional fields", "[prefab]") {
    EntityFactory factory;
    Engine::registerBuiltinTypes(factory);

    LevelLoader loader;
    loader.setFactory(&factory);

    TempFile tmp;
    {
        std::ofstream f(tmp.path);
        f << R"({"entities":[{"type":"platform","name":"Slab","position":{"x":10,"y":20},"size":{"x":64,"y":16}}]})";
    }

    auto loaded = loader.loadFromJSON(tmp.path.string());
    REQUIRE(loaded.size() == 1);
    REQUIRE(loaded.front()->type == "platform");
    REQUIRE(loaded.front()->position.x == 10.f);
    REQUIRE(loaded.front()->size.y == 16.f);
    for (auto* e : loaded) delete e;
}
