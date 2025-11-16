#pragma once
#include "../Entity/Entity.h"
#include <cmath>

namespace Engine {

// gravity + friction + impulses. no state of its own (other than tunables),
// just mutates whatever entity you pass in. kinda functional in style
class PhysicsEngine {
    float m_gravity = 980.f;         // px/s² — tuned by feel, roughly 1g in pixel space
    float m_maxFall = 800.f;         // terminal velocity. without this fast falls tunnel thru floors
    float m_groundFriction = 10.f;   // exp decay on ground — stops near-instantly
    float m_airFriction = 0.5f;      // 20x weaker in air so u can drift through jumps

public:
    // call this BEFORE entity->update() each tick. just adds g*dt to y velocity
    void applyGravity(Entity& e, float dt) {
        if (e.isStatic || !e.hasGravity) return;
        e.velocity.y = std::min(e.velocity.y + m_gravity * dt, m_maxFall);
    }

    // this is the exponential friction trick — vel *= e^(-k*dt).
    // the cool thing is its framerate independent: same stop distance at 30fps or 144fps
    // bc integrating over time gives the same result regardless of step size. linear
    // friction (vel -= k*dt) does NOT have this property — try it if ur curious
    void updateVelocity(Entity& e, float dt) {
        if (e.isStatic) return;
        float friction = e.isOnGround ? m_groundFriction : m_airFriction;
        if (std::abs(e.velocity.x) > 0.1f)
            e.velocity.x *= std::exp(-friction * dt);
        else
            e.velocity.x = 0.f;   // snap to 0 so we dont have tiny residual drift forever
    }

    // just adds to velocity. used for jumps mostly
    void applyImpulse(Entity& e, const sf::Vector2f& impulse) {
        if (!e.isStatic) e.velocity += impulse;
    }

    void setGravity(float g) { m_gravity = g; }
    float getGravity() const { return m_gravity; }
    void setMaxFallSpeed(float s) { m_maxFall = s; }
    void setGroundFriction(float f) { m_groundFriction = f; }
    void setAirFriction(float f) { m_airFriction = f; }
};

}
