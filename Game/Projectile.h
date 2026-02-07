#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

// bullet. fired by ShootingEnemy, dies after m_lifetime seconds OR if it
// leaves the level OR if it hits something. all the "dies on hit" logic lives
// in GameplaySystem::tick() not here — projectiles are dumb + just travel
class Projectile : public Engine::Entity {
    float m_lifetime = 3.f;   // max seconds alive — fallback if it never hits anything
    float m_age = 0.f;        // seconds since spawn

public:
    Projectile() : Entity("Projectile", "projectile") {
        color = sf::Color(255, 100, 50);   // orange
        size = {8.f, 4.f};
        isStatic = false;
        hasGravity = false;
        // isTrigger is key — damage goes thru GameplaySystem's custom projectile-vs-player check, the resolver must NOT push the bullet out or we'd never detect the hit
        isTrigger = true;
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
        position += velocity * dt;   // velocity set once at spawn, never changes
        m_age += dt;
    }
};

}
