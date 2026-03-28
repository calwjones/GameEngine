#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

class Collectible : public Engine::Entity {
    int m_points = 10;

public:
    Collectible() : Entity("Collectible", "collectible") {
        color = sf::Color(255, 215, 0);
        size = {16.f, 16.f};
        isStatic = true;
        hasGravity = false;
        isTrigger = true;
    }

    void setPoints(int p) { m_points = p; }
    int getPoints() const { return m_points; }

    Properties serializeProperties() const override {
        return {{"points", static_cast<float>(m_points)}};
    }
    void deserializeProperties(const Properties& p) override {
        auto it = p.find("points"); if (it != p.end()) m_points = static_cast<int>(it->second);
    }
};

}
