#include "PhysicsEngine.h"
#include <algorithm>
#include <cmath>

namespace Engine {

void PhysicsEngine::applyGravity(Entity& e, float dt) {
    if (e.isStatic || !e.hasGravity) return;
    e.velocity.y = std::min(e.velocity.y + m_gravity * dt, m_maxFall);
}

void PhysicsEngine::updateVelocity(Entity& e, float dt) {
    if (e.isStatic) return;
    float friction = e.isOnGround ? m_groundFriction : m_airFriction;
    if (std::abs(e.velocity.x) > 0.1f)
        e.velocity.x *= std::exp(-friction * dt);
    else
        e.velocity.x = 0.f;
}

void PhysicsEngine::applyImpulse(Entity& e, const sf::Vector2f& impulse) {
    if (!e.isStatic) e.velocity += impulse;
}

}
