#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

class Projectile : public Engine::Entity {
    float m_lifetime = 3.f;
    float m_age = 0.f;

public:
    Projectile() : Entity("Projectile", "projectile") {
        color = sf::Color(255, 100, 50);
        size = {8.f, 4.f};
        isStatic = false;
        hasGravity = false;
        isTrigger = true;   // resolver must not push out
    }

    void setLifetime(float t) { m_lifetime = t; }
    float getLifetime() const { return m_lifetime; }
    bool isExpired() const { return m_age >= m_lifetime; }
    void resetAge() { m_age = 0.f; }

    Properties serializeProperties() const override {
        return {{"lifetime", m_lifetime}};
    }
    void deserializeProperties(const Properties& p) override {
        auto it = p.find("lifetime"); if (it != p.end()) m_lifetime = it->second;
    }

    void update(float dt) override {
        position += velocity * dt;
        m_age += dt;
    }
};

}
