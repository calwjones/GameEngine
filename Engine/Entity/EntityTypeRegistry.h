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

struct EntityTypeInfo {
    const char* type;
    const char* label;
    const char* icon;
    float r, g, b;
    Entity* (*create)();
};

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

inline void registerBuiltinTypes(EntityFactory& factory) {
    for (const auto& info : kEntityTypes) {
        if (!info.create) continue;
        auto ctor = info.create;
        factory.registerType(info.type, [ctor]() { return ctor(); });
    }
    factory.registerType("projectile", []() -> Entity* { return new Game::Projectile(); });
}

}
