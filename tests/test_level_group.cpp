#include <catch2/catch_test_macros.hpp>
#include "Engine/Level/LevelGroup.h"
#include <filesystem>
#include <fstream>

using Engine::LevelGroup;
using Engine::LevelGroupLoader;

namespace {
struct TempPath {
    std::filesystem::path path;
    explicit TempPath(const std::string& suffix) {
        path = std::filesystem::temp_directory_path() /
               ("level_group_test_" + std::to_string(std::rand()) + suffix);
    }
    ~TempPath() { std::error_code ec; std::filesystem::remove_all(path, ec); }
};
}

TEST_CASE("level group loader parses valid json", "[level_group]") {
    TempPath tmp(".json");
    {
        std::ofstream f(tmp.path);
        f << R"({"name":"Tour","lives":5,"levels":["a.json","b.json","c.json"]})";
    }

    LevelGroup g;
    REQUIRE(LevelGroupLoader::loadFromJSON(tmp.path.string(), g));
    REQUIRE(g.name == "Tour");
    REQUIRE(g.lives == 5);
    REQUIRE(g.levels.size() == 3);
    REQUIRE(g.levels[0] == "a.json");
    REQUIRE(g.levels[2] == "c.json");
}

TEST_CASE("level group id derives from filename stem", "[level_group]") {
    auto dir = std::filesystem::temp_directory_path();
    auto file = dir / "my_world.json";
    {
        std::ofstream f(file);
        f << R"({"levels":["x.json"]})";
    }

    LevelGroup g;
    REQUIRE(LevelGroupLoader::loadFromJSON(file.string(), g));
    REQUIRE(g.id == "my_world");
    REQUIRE(g.name == "my_world");

    std::error_code ec; std::filesystem::remove(file, ec);
}

TEST_CASE("level group loader rejects missing file", "[level_group]") {
    LevelGroup g;
    REQUIRE_FALSE(LevelGroupLoader::loadFromJSON("/tmp/definitely_does_not_exist_xyz.json", g));
}

TEST_CASE("level group loader rejects malformed json", "[level_group]") {
    TempPath tmp(".json");
    {
        std::ofstream f(tmp.path);
        f << "{ not valid json";
    }

    LevelGroup g;
    REQUIRE_FALSE(LevelGroupLoader::loadFromJSON(tmp.path.string(), g));
}

TEST_CASE("level group loader rejects empty levels array", "[level_group]") {
    TempPath tmp(".json");
    {
        std::ofstream f(tmp.path);
        f << R"({"name":"empty","levels":[]})";
    }

    LevelGroup g;
    REQUIRE_FALSE(LevelGroupLoader::loadFromJSON(tmp.path.string(), g));
}

TEST_CASE("level group loader defaults lives when missing", "[level_group]") {
    TempPath tmp(".json");
    {
        std::ofstream f(tmp.path);
        f << R"({"name":"Default","levels":["x.json"]})";
    }

    LevelGroup g;
    REQUIRE(LevelGroupLoader::loadFromJSON(tmp.path.string(), g));
    REQUIRE(g.lives == 3);
}

TEST_CASE("level group scanDirectory finds all json files and sorts by name", "[level_group]") {
    TempPath dir("");
    std::filesystem::create_directories(dir.path);

    auto write = [&](const std::string& filename, const std::string& body) {
        std::ofstream f(dir.path / filename);
        f << body;
    };
    write("zeta.json", R"({"name":"Zeta","levels":["z1.json"]})");
    write("alpha.json", R"({"name":"Alpha","levels":["a1.json"]})");
    write("readme.txt", "not a group");

    auto groups = LevelGroupLoader::scanDirectory(dir.path.string());
    REQUIRE(groups.size() == 2);
    REQUIRE(groups[0].name == "Alpha");
    REQUIRE(groups[1].name == "Zeta");
}

TEST_CASE("level group scanDirectory returns empty for missing dir", "[level_group]") {
    auto groups = LevelGroupLoader::scanDirectory("/tmp/definitely_does_not_exist_abc");
    REQUIRE(groups.empty());
}
