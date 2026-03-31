#pragma once

namespace Editor {

struct EditorContext;

class OverlayRenderer {
    EditorContext& ctx;

public:
    explicit OverlayRenderer(EditorContext& c) : ctx(c) {}
    void render();
};

}
