#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

// patrol enemy that also shoots. timer raises a fire flag every fireRate seconds, GameplaySystem reads it + spawns the projectile — we dont spawn from here bc subclasses shouldnt know about EntityManager
class ShootingEnemy : public Engine::Entity {
    float m_speed = 40.f;
    float m_dir = 1.f;
    float m_fireRate = 2.f;      // seconds between shots
    float m_fireTimer = 0.f;
    float m_projSpeed = 200.f;
    float m_minX = 0.f, m_maxX = 0.f;
    bool m_hasBounds = false;
    bool m_fireFlag = false;     // set by update(), cleared by consumeFireFlag()

public:
    ShootingEnemy() : Entity("Shooting Enemy", "shooting_enemy") {
        color = sf::Color(220, 120, 20);
        size = {32.f, 32.f};
        hasGravity = true;
    }

    void setPatrolSpeed(float s) { m_speed = s; }
    void setDirection(float d) { m_dir = d; }
    void setFireRate(float r) { m_fireRate = r; }
    void setProjectileSpeed(float s) { m_projSpeed = s; }
    void setPatrolBounds(float min, float max) { m_minX = min; m_maxX = max; m_hasBounds = true; }
    void clearPatrolBounds() { m_hasBounds = false; }
    float getProjectileSpeed() const { return m_projSpeed; }
    float getDirection() const { return m_dir; }
    float getPatrolSpeed() const { return m_speed; }
    float getFireRate() const { return m_fireRate; }
    bool hasBounds() const { return m_hasBounds; }
    float getMinX() const { return m_minX; }
    float getMaxX() const { return m_maxX; }

    Properties serializeProperties() const override {
        Properties p;
        p["patrolSpeed"] = m_speed;
        p["direction"] = m_dir;
        p["fireRate"] = m_fireRate;
        p["projectileSpeed"] = m_projSpeed;
        if (m_hasBounds) { p["minX"] = m_minX; p["maxX"] = m_maxX; }
        return p;
    }
    void deserializeProperties(const Properties& p) override {
        auto it = p.find("patrolSpeed"); if (it != p.end()) m_speed = it->second;
        it = p.find("direction"); if (it != p.end()) m_dir = it->second;
        it = p.find("fireRate"); if (it != p.end()) m_fireRate = it->second;
        it = p.find("projectileSpeed"); if (it != p.end()) m_projSpeed = it->second;
        auto minIt = p.find("minX"), maxIt = p.find("maxX");
        if (minIt != p.end() && maxIt != p.end()) setPatrolBounds(minIt->second, maxIt->second);
    }

    // read-and-reset — named "consume" not "shouldFire" so the side effect is obvious (the old name looked pure + bit callers)
    bool consumeFireFlag() {
        bool f = m_fireFlag;
        m_fireFlag = false;
        return f;
    }

    void update(float dt) override {
        velocity.x = m_speed * m_dir;   // same patrol dance as Enemy
        if (m_hasBounds) {
            if (m_dir > 0 && position.x + size.x >= m_maxX) { m_dir = -m_dir; position.x = m_maxX - size.x; }
            else if (m_dir < 0 && position.x <= m_minX) { m_dir = -m_dir; position.x = m_minX; }
        }
        position += velocity * dt;

        m_fireTimer += dt;
        if (m_fireTimer >= m_fireRate) {
            m_fireTimer -= m_fireRate;   // -= not =0 so we dont drop fractional leftover time
            m_fireFlag = true;
        }
    }
};

}
