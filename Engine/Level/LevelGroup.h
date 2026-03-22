#pragma once
#include <string>
#include <vector>

namespace Engine {

// a "World" (or "Level Group") — basically a mario world style container.
// its just a LIST of level files + a lives count. manifest files live in
// assets/levels/groups/*.json, scanned once at editor startup.
//
// runtime state for a playthrough (where u are, lives left, running coin total)
// lives in EditorApplication::m_groupSession, NOT here — this struct is just
// the static manifest. think of LevelGroup as the "recipe" and GroupSession
// as the "current cooking state"
struct LevelGroup {
    std::string id;                      // filename stem, stable — used as a key
    std::string name;                    // what u see in the Level Browser
    std::vector<std::string> levels;     // level filenames (relative to assets/levels/)
    int lives = 3;                       // deaths allowed per run before the world resets to level 1
};

class LevelGroupLoader {
public:
    static bool loadFromJSON(const std::string& path, LevelGroup& out);
    static std::vector<LevelGroup> scanDirectory(const std::string& dir);
};

}
