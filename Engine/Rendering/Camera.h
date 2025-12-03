#pragma once
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>

namespace Engine {

// smooth-follow camera. same exp-lerp trick as PhysicsEngine friction —
// every frame: pos += (target - pos) * (1 - e^(-speed*dt))
// the 1-e^(-k*dt) term is the magic bit, its what makes it framerate-independent.
// plain linear lerp (pos += (target-pos)*0.1) feels different at 30 vs 144 fps
class Camera {
    sf::Vector2f m_position{400.f, 300.f};
    sf::Vector2f m_target{400.f, 300.f};
    float m_lerpSpeed = 5.f;   // higher = snappier follow, lower = floatier drift

    // level-space bounds so we dont pan past the edge
    float m_left = 0.f, m_top = 0.f;
    float m_right = 800.f, m_bottom = 600.f;
    bool m_hasBounds = true;

    float m_viewW = 800.f, m_viewH = 600.f;   // viewport dims, for the clamp math

public:
    void setTarget(const sf::Vector2f& t) { m_target = t; }
    void setPosition(const sf::Vector2f& p) { m_position = p; m_target = p; }
    sf::Vector2f getPosition() const { return m_position; }

    void setLerpSpeed(float s) { m_lerpSpeed = s; }
    float getLerpSpeed() const { return m_lerpSpeed; }

    void setBounds(float left, float top, float right, float bottom) {
        m_left = left; m_top = top; m_right = right; m_bottom = bottom;
        m_hasBounds = true;
    }
    void clearBounds() { m_hasBounds = false; }
    bool hasBounds() const { return m_hasBounds; }

    void setViewSize(float w, float h) { m_viewW = w; m_viewH = h; }

    void update(float dt) {
        float t = 1.f - std::exp(-m_lerpSpeed * dt);   // 0..1, fraction of way to target
        m_position.x += (m_target.x - m_position.x) * t;
        m_position.y += (m_target.y - m_position.y) * t;
        clampToBounds();
    }

    // instant snap — use when u teleport/respawn so camera doesnt glide across the level
    void snapToTarget() {
        m_position = m_target;
        clampToBounds();
    }

private:
    // keep camera center at least half-viewport away from every edge, so we
    // never show the grey area outside the level. std::max guards the edge case
    // where the level is SMALLER than the viewport (clamp would flip args & UB)
    void clampToBounds() {
        if (!m_hasBounds) return;
        float halfW = m_viewW / 2.f;
        float halfH = m_viewH / 2.f;
        m_position.x = std::clamp(m_position.x,
            m_left + halfW, std::max(m_left + halfW, m_right - halfW));
        m_position.y = std::clamp(m_position.y,
            m_top + halfH, std::max(m_top + halfH, m_bottom - halfH));
    }
};

}
