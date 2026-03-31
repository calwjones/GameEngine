#pragma once
#include "EntityPalette.h"

namespace Editor {

struct EditorContext;

class EntityOps {
    EditorContext& ctx;

public:
    explicit EntityOps(EditorContext& c) : ctx(c) {}

    void addEntity();
    void addFromTemplate(EntityTemplate tmpl);
    void duplicateEntity();
    void deleteSelected();
};

}
