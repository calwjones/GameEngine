#pragma once
#include "Entity.h"
#include "EntityFactory.h"
#include "../../Game/Player.h"
#include "../../Game/Enemy.h"
#include "../../Game/FlyingEnemy.h"
#include "../../Game/ShootingEnemy.h"
#include "../../Game/Collectible.h"
#include "../../Game/Projectile.h"
#include "../../Game/MovingPlatform.h"
#include "../../Game/Goal.h"
#include "../../Game/Hazard.h"
#include <array>
#include <cstddef>
#include <functional>

namespace Engine {

// VIVA POINT: this is the "one table to rule them all" thing i was talking about.
// originally i had the entity type knowledge spread across 4 files — EditorApplication
// registered constructors, ScenePanel hardcoded icons, PropertiesPanel had the type
// dropdown, EntityPalette hardcoded colours. every new type = edit 4 places + forget
// one + crash. now its ONE table and every place that needs to know just reads it.
//
// each row = {typeString, label, icon, r, g, b, ctor}. ctor is nullptr for
// "default"/"platform" (the base Entity is used directly, no subclass) and
// also for "projectile" which is registered separately bc its runtime-only.
struct EntityTypeInfo {
    const char* type;            // machine key, stored on Entity::type + in JSON
    const char* label;           // human label for dropdowns
    const char* icon;             // little bracket tag in ScenePanel — "[P]" for player etc
    float r, g, b;                // colour 0..1. floats bc ImGui wants floats, not sf::Color bytes
    Entity* (*create)();          // factory fn ptr. nullptr = no subclass, use base Entity
};

// the order of rows = the order u see in the PropertiesPanel "Type" combobox
inline constexpr std::array<EntityTypeInfo, 10> kEntityTypes = {{
    {"default",         "Default",         "[-]", 1.00f, 1.00f, 1.00f, nullptr},
    {"player",          "Player",          "[P]", 0.20f, 0.80f, 0.20f, []() -> Entity* { return new Game::Player(); }},
    {"enemy",           "Enemy",           "[E]", 0.80f, 0.20f, 0.20f, []() -> Entity* { return new Game::Enemy(); }},
    {"platform",        "Platform",        "[=]", 0.50f, 0.50f, 0.50f, nullptr},
    {"flying_enemy",    "Flying Enemy",    "[F]", 0.71f, 0.20f, 0.86f, []() -> Entity* { return new Game::FlyingEnemy(); }},
    {"shooting_enemy",  "Shooting Enemy",  "[S]", 0.86f, 0.47f, 0.08f, []() -> Entity* { return new Game::ShootingEnemy(); }},
    {"collectible",     "Collectible",     "[*]", 1.00f, 0.84f, 0.00f, []() -> Entity* { return new Game::Collectible(); }},
    {"moving_platform", "Moving Platform", "[~]", 0.24f, 0.47f, 0.71f, []() -> Entity* { return new Game::MovingPlatform(); }},
    {"goal",            "Goal",            "[G]", 1.00f, 0.85f, 0.00f, []() -> Entity* { return new Game::Goal(); }},
    {"hazard",          "Hazard",          "[X]", 0.70f, 0.70f, 0.80f, []() -> Entity* { return new Game::Hazard(); }},
}};

// manual strcmp loop bc constexpr cant use <cstring>. returns nullptr if not found.
// yes this is O(n*len) but n=10 so who cares lol
inline const EntityTypeInfo* findEntityType(const char* type) {
    if (!type) return nullptr;
    for (const auto& info : kEntityTypes) {
        const char* a = info.type;
        const char* b = type;
        while (*a && *a == *b) { ++a; ++b; }
        if (*a == 0 && *b == 0) return &info;
    }
    return nullptr;
}
inline const EntityTypeInfo* findEntityType(const std::string& type) {
    return findEntityType(type.c_str());
}

// registers every type with a non-null create fn against the given factory.
// projectile is deliberately skipped - its only ever spawned by ShootingEnemy at runtime.
inline void registerBuiltinTypes(EntityFactory& factory) {
    for (const auto& info : kEntityTypes) {
        if (!info.create) continue;
        auto ctor = info.create;
        factory.registerType(info.type, [ctor]() { return ctor(); });
    }
    factory.registerType("projectile", []() -> Entity* { return new Game::Projectile(); });
}

}
