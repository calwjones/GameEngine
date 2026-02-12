#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

// coin / pickup. isTrigger → overlaps reported but no pushout. GameplaySystem detaches it on pickup, keeping the ptr alive so Stop can re-attach (non-destructive playtest).
// since 2026-04-11: coins are cosmetic score only, only the Goal ends the level
class Collectible : public Engine::Entity {
    int m_points = 10;   // how much score this coin gives. stored as int, serialized as float bc the Properties map is float-only

public:
    Collectible() : Entity("Collectible", "collectible") {
        color = sf::Color(255, 215, 0);   // gold — obvious "pick me up" colour
        size = {16.f, 16.f};
        isStatic = true;
        hasGravity = false;
        isTrigger = true;                 // overlap-only, no physical collision
    }

    void setPoints(int p) { m_points = p; }
    int getPoints() const { return m_points; }

    // int↔float hack bc Properties map is float-only — coins are never worth >100 so precision loss doesnt matter
    Properties serializeProperties() const override {
        return {{"points", static_cast<float>(m_points)}};
    }
    void deserializeProperties(const Properties& p) override {
        auto it = p.find("points"); if (it != p.end()) m_points = static_cast<int>(it->second);
    }
};

}
