#pragma once

namespace Editor {

struct EditorContext;

class MenuBarRenderer {
    EditorContext& ctx;
    float m_fpsAccum = 0.f;
    int m_fpsFrames = 0;
    int m_displayFps = 60;

public:
    explicit MenuBarRenderer(EditorContext& c) : ctx(c) {}
    void render(float dt);
};

}
