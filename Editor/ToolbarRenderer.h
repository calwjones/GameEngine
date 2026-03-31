#pragma once

namespace Editor {

struct EditorContext;

class ToolbarRenderer {
    EditorContext& ctx;

public:
    explicit ToolbarRenderer(EditorContext& c) : ctx(c) {}
    void render();
};

}
