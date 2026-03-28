#include "LevelGroup.h"
#include <rapidjson/document.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace Engine {

bool LevelGroupLoader::loadFromJSON(const std::string& path, LevelGroup& out) {
    std::ifstream file(path);
    if (!file) return false;

    std::stringstream buf;
    buf << file.rdbuf();

    rapidjson::Document doc;
    doc.Parse(buf.str().c_str());
    if (doc.HasParseError() || !doc.IsObject()) return false;

    if (doc.HasMember("name") && doc["name"].IsString())
        out.name = doc["name"].GetString();
    if (doc.HasMember("lives") && doc["lives"].IsNumber())
        out.lives = doc["lives"].GetInt();

    if (doc.HasMember("levels") && doc["levels"].IsArray()) {
        for (auto& v : doc["levels"].GetArray()) {
            if (v.IsString()) out.levels.emplace_back(v.GetString());
        }
    }

    std::filesystem::path p(path);
    out.id = p.stem().string();
    if (out.name.empty()) out.name = out.id;

    return !out.levels.empty();
}

std::vector<LevelGroup> LevelGroupLoader::scanDirectory(const std::string& dir) {
    std::vector<LevelGroup> groups;
    namespace fs = std::filesystem;
    if (!fs::exists(dir)) return groups;

    for (auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        LevelGroup g;
        if (loadFromJSON(entry.path().string(), g))
            groups.push_back(std::move(g));
    }
    std::sort(groups.begin(), groups.end(),
              [](const LevelGroup& a, const LevelGroup& b) { return a.name < b.name; });
    return groups;
}

}
