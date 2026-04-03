#pragma once
#include "../Entity/Entity.h"

namespace Engine {

class PhysicsEngine {
    float m_gravity = 980.f;
    float m_maxFall = 800.f;
    float m_groundFriction = 10.f;
    float m_airFriction = 0.5f;

public:
    void applyGravity(Entity& e, float dt);
    void updateVelocity(Entity& e, float dt);
    void applyImpulse(Entity& e, const sf::Vector2f& impulse);

    void setGravity(float g) { m_gravity = g; }
    float getGravity() const { return m_gravity; }
    void setMaxFallSpeed(float s) { m_maxFall = s; }
    void setGroundFriction(float f) { m_groundFriction = f; }
    void setAirFriction(float f) { m_airFriction = f; }
};

}
