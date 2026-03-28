#pragma once
#include <string>
#include <vector>

namespace Engine {

struct LevelGroup {
    std::string id;
    std::string name;
    std::vector<std::string> levels;
    int lives = 3;
};

class LevelGroupLoader {
public:
    static bool loadFromJSON(const std::string& path, LevelGroup& out);
    static std::vector<LevelGroup> scanDirectory(const std::string& dir);
};

}
