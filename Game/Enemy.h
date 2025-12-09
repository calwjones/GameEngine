#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

// ground-pounding patrol enemy (the goomba). walks between minX<->maxX bounds
// and turns around when it hits one. if u dont set bounds it just marches off
// the screen edge — always set bounds in level json or u get sad enemies lost
// in the void
class Enemy : public Engine::Entity {
    float m_speed = 80.f;
    float m_dir = 1.f;         // direction multiplier. 1 = right, -1 = left
    float m_minX = 0.f, m_maxX = 0.f;
    bool m_hasBounds = false;  // if false, patrol bounds are ignored (free march)

public:
    Enemy() : Entity("Enemy", "enemy") {
        color = sf::Color(220, 20, 60);   // crimson — looks dangerous
        size = {32.f, 32.f};
        hasGravity = true;                // ground enemies fall, flyers dont
    }

    void setPatrolSpeed(float s) { m_speed = s; }
    void setDirection(float d) { m_dir = d; }
    void setPatrolBounds(float min, float max) { m_minX = min; m_maxX = max; m_hasBounds = true; }
    void clearPatrolBounds() { m_hasBounds = false; }
    void reverseDirection() { m_dir = -m_dir; }

    float getPatrolSpeed() const { return m_speed; }
    float getDirection() const { return m_dir; }
    bool hasBounds() const { return m_hasBounds; }
    float getMinX() const { return m_minX; }
    float getMaxX() const { return m_maxX; }

    Properties serializeProperties() const override {
        Properties p;
        p["patrolSpeed"] = m_speed;
        p["direction"] = m_dir;
        // only emit bounds when theyre actually set — otherwise level json gets
        // polluted with "minX: 0, maxX: 0" which deserialize would interpret as valid
        if (m_hasBounds) { p["minX"] = m_minX; p["maxX"] = m_maxX; }
        return p;
    }
    void deserializeProperties(const Properties& p) override {
        auto it = p.find("patrolSpeed"); if (it != p.end()) m_speed = it->second;
        it = p.find("direction"); if (it != p.end()) m_dir = it->second;
        // min AND max must both be present — half-bounds make no sense
        auto minIt = p.find("minX"), maxIt = p.find("maxX");
        if (minIt != p.end() && maxIt != p.end()) setPatrolBounds(minIt->second, maxIt->second);
    }

    void update(float dt) override {
        velocity.x = m_speed * m_dir;
        // bounce at bounds + snap to the exact bound so we cant drift past it
        // over multiple frames (position += velocity * dt can overshoot)
        if (m_hasBounds) {
            if (m_dir > 0 && position.x + size.x >= m_maxX) { reverseDirection(); position.x = m_maxX - size.x; }
            else if (m_dir < 0 && position.x <= m_minX) { reverseDirection(); position.x = m_minX; }
        }
        position += velocity * dt;
    }
};

}
