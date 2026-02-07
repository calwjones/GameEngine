#pragma once
#include "../Engine/Entity/Entity.h"
#include <cmath>

namespace Game {

// floats + bobs. horizontal patrol like Enemy, vertical is y = baseY + amp*sin(phase). baseY is snapshotted at play start, phase is staggered per-flyer so they dont bob in sync
class FlyingEnemy : public Engine::Entity {
    float m_speed = 60.f;
    float m_dir = 1.f;
    float m_amplitude = 50.f;   // bob height in px
    float m_frequency = 2.f;    // rad/s, not hz
    float m_baseY = 0.f;        // vertical midpoint, snapshotted at play start
    float m_phase = 0.f;
    float m_minX = 0.f, m_maxX = 0.f;
    bool m_hasBounds = false;

public:
    FlyingEnemy() : Entity("Flying Enemy", "flying_enemy") {
        color = sf::Color(180, 50, 220);   // purple
        size = {28.f, 28.f};
        hasGravity = false;
    }

    void setPatrolSpeed(float s) { m_speed = s; }
    void setDirection(float d) { m_dir = d; }
    void setAmplitude(float a) { m_amplitude = a; }
    void setFrequency(float f) { m_frequency = f; }
    void setBaseY(float y) { m_baseY = y; }
    void setPhase(float p) { m_phase = p; }
    void setPatrolBounds(float min, float max) { m_minX = min; m_maxX = max; m_hasBounds = true; }
    void clearPatrolBounds() { m_hasBounds = false; }

    float getPatrolSpeed() const { return m_speed; }
    float getDirection() const { return m_dir; }
    float getAmplitude() const { return m_amplitude; }
    float getFrequency() const { return m_frequency; }
    float getBaseY() const { return m_baseY; }
    bool hasBounds() const { return m_hasBounds; }
    float getMinX() const { return m_minX; }
    float getMaxX() const { return m_maxX; }

    Properties serializeProperties() const override {
        Properties p;
        p["patrolSpeed"] = m_speed;
        p["direction"] = m_dir;
        p["amplitude"] = m_amplitude;
        p["frequency"] = m_frequency;
        p["baseY"] = m_baseY;
        if (m_hasBounds) { p["minX"] = m_minX; p["maxX"] = m_maxX; }
        return p;
    }
    void deserializeProperties(const Properties& p) override {
        auto it = p.find("patrolSpeed"); if (it != p.end()) m_speed = it->second;
        it = p.find("direction"); if (it != p.end()) m_dir = it->second;
        it = p.find("amplitude"); if (it != p.end()) m_amplitude = it->second;
        it = p.find("frequency"); if (it != p.end()) m_frequency = it->second;
        it = p.find("baseY"); if (it != p.end()) m_baseY = it->second;
        auto minIt = p.find("minX"), maxIt = p.find("maxX");
        if (minIt != p.end() && maxIt != p.end()) setPatrolBounds(minIt->second, maxIt->second);
    }

    void update(float dt) override {
        velocity.x = m_speed * m_dir;   // horizontal patrol, same logic as Enemy
        if (m_hasBounds) {
            if (m_dir > 0 && position.x + size.x >= m_maxX) { m_dir = -m_dir; position.x = m_maxX - size.x; }
            else if (m_dir < 0 && position.x <= m_minX) { m_dir = -m_dir; position.x = m_minX; }
        }
        position.x += velocity.x * dt;

        // set y ABSOLUTELY, not += — else sine drifts from accumulated float error
        m_phase += m_frequency * dt;
        position.y = m_baseY + m_amplitude * std::sin(m_phase);
    }
};

}
