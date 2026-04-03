#pragma once
#include <SFML/System/Vector2.hpp>

namespace Engine {

class Camera {
    sf::Vector2f m_position{400.f, 300.f};
    sf::Vector2f m_target{400.f, 300.f};
    float m_lerpSpeed = 5.f;

    float m_left = 0.f, m_top = 0.f;
    float m_right = 800.f, m_bottom = 600.f;
    bool m_hasBounds = true;

    float m_viewW = 800.f, m_viewH = 600.f;

public:
    void setTarget(const sf::Vector2f& t) { m_target = t; }
    void setPosition(const sf::Vector2f& p) { m_position = p; m_target = p; }
    sf::Vector2f getPosition() const { return m_position; }

    void setLerpSpeed(float s) { m_lerpSpeed = s; }
    float getLerpSpeed() const { return m_lerpSpeed; }

    void setBounds(float left, float top, float right, float bottom);
    void clearBounds() { m_hasBounds = false; }
    bool hasBounds() const { return m_hasBounds; }

    void setViewSize(float w, float h) { m_viewW = w; m_viewH = h; }

    void update(float dt);
    void snapToTarget();

private:
    void clampToBounds();
};

}
