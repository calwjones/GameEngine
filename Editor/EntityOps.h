#pragma once
#include "EntityPalette.h"
#include <string>

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

    // Prefabs: save the current selection to assets/prefabs/<name>.json, or
    // instantiate from a prefab file at viewport center.
    bool saveSelectionAsPrefab(const std::string& name);
    bool instantiatePrefab(const std::string& path);
};

}
