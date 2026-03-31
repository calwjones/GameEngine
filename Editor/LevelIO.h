#pragma once
#include <string>
#include <vector>
#include "../Engine/Level/LevelGroup.h"

namespace Editor {

struct EditorContext;

class LevelIO {
public:
    static constexpr float DEFAULT_W = 1600.f;
    static constexpr float DEFAULT_H = 720.f;

private:
    EditorContext& ctx;
    std::string m_levelPath;
    std::vector<std::string> m_levelFiles;
    std::vector<Engine::LevelGroup> m_groups;
    float m_levelWidth = DEFAULT_W;
    float m_levelHeight = DEFAULT_H;
    int m_entityCounter = 0;

public:
    explicit LevelIO(EditorContext& c) : ctx(c) {}

    void loadLevel(const std::string& path);
    void saveLevel();
    void saveLevelAs(const std::string& filename);
    void scanLevelFiles();
    void scanLevelGroups();
    void fitLevelToContent();
    void resetToNewLevel();
    void updateWindowTitle();

    const std::string& currentPath() const { return m_levelPath; }
    const std::vector<std::string>& levelFiles() const { return m_levelFiles; }
    const std::vector<Engine::LevelGroup>& groups() const { return m_groups; }

    float levelWidth() const { return m_levelWidth; }
    float levelHeight() const { return m_levelHeight; }
    void setLevelSize(float w, float h) { m_levelWidth = w; m_levelHeight = h; }

    int nextEntityId() { return ++m_entityCounter; }
    int entityCounter() const { return m_entityCounter; }
    void setEntityCounter(int v) { m_entityCounter = v; }
};

}
