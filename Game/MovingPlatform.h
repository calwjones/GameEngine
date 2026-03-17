#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

// patrols between pointA/pointB at constant speed. kinematic body — isStatic so physics cant push it, but update() moves it manually.
// player riding: GameplaySystem reads getDelta() each tick + adds it to the player if they're standing on top, else they'd instantly fall off
class MovingPlatform : public Engine::Entity {
    sf::Vector2f m_pointA{0.f, 0.f};
    sf::Vector2f m_pointB{200.f, 0.f};
    float m_speed = 80.f;              // px/s along the path
    float m_t = 0.f;                   // parametric, 0=A 1=B
    float m_dir = 1.f;                 // +1 → B, -1 → A
    sf::Vector2f m_delta{0.f, 0.f};    // movement this frame, GameplaySystem reads for rider carry

public:
    MovingPlatform() : Entity("Moving Platform", "moving_platform") {
        color = sf::Color(60, 120, 180);   // steely blue
        size = {128.f, 16.f};
        isStatic = true;                   // we move it manually, physics must leave it alone
        hasGravity = false;
    }

    void setPointA(sf::Vector2f a) { m_pointA = a; }
    void setPointB(sf::Vector2f b) { m_pointB = b; }
    void setSpeed(float s) { m_speed = s; }
    sf::Vector2f getPointA() const { return m_pointA; }
    sf::Vector2f getPointB() const { return m_pointB; }
    float getSpeed() const { return m_speed; }
    sf::Vector2f getDelta() const { return m_delta; }

    // called on play start — snap back to A, else the platform would teleport if we started mid-cycle
    void resetToStart() {
        m_t = 0.f;
        m_dir = 1.f;
        position = m_pointA;
        m_delta = {0.f, 0.f};
    }

    Properties serializeProperties() const override {
        return {
            {"ax", m_pointA.x}, {"ay", m_pointA.y},
            {"bx", m_pointB.x}, {"by", m_pointB.y},
            {"speed", m_speed}
        };
    }
    void deserializeProperties(const Properties& p) override {
        auto it = p.find("ax"); if (it != p.end()) m_pointA.x = it->second;
        it = p.find("ay"); if (it != p.end()) m_pointA.y = it->second;
        it = p.find("bx"); if (it != p.end()) m_pointB.x = it->second;
        it = p.find("by"); if (it != p.end()) m_pointB.y = it->second;
        it = p.find("speed"); if (it != p.end()) m_speed = it->second;
    }

    void update(float dt) override {
        sf::Vector2f diff = m_pointB - m_pointA;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (dist < 0.1f) { m_delta = {0.f, 0.f}; return; }   // A≡B → no motion, also guards div-by-zero

        // convert px/s into a parametric t-step using path length → constant speed regardless of distance
        float tStep = (m_speed / dist) * dt;
        m_t += m_dir * tStep;

        if (m_t >= 1.f) { m_t = 1.f; m_dir = -1.f; }   // bounce at endpoints
        else if (m_t <= 0.f) { m_t = 0.f; m_dir = 1.f; }

        sf::Vector2f newPos = m_pointA + diff * m_t;   // lerp A→B, stash delta for player riding
        m_delta = newPos - position;
        position = newPos;
    }
};

}
