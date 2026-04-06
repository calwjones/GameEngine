#include "Camera.h"
#include <algorithm>
#include <cmath>

namespace Engine {

void Camera::setBounds(float left, float top, float right, float bottom) {
    m_left = left; m_top = top; m_right = right; m_bottom = bottom;
    m_hasBounds = true;
}

void Camera::update(float dt) {
    float t = 1.f - std::exp(-m_lerpSpeed * dt);
    m_position.x += (m_target.x - m_position.x) * t;
    m_position.y += (m_target.y - m_position.y) * t;
    clampToBounds();
}

void Camera::snapToTarget() {
    m_position = m_target;
    clampToBounds();
}

void Camera::clampToBounds() {
    if (!m_hasBounds) return;
    float halfW = m_viewW / 2.f;
    float halfH = m_viewH / 2.f;
    float levelW = m_right - m_left;
    float levelH = m_bottom - m_top;

    if (m_viewW >= levelW)
        m_position.x = (m_left + m_right) * 0.5f;
    else
        m_position.x = std::clamp(m_position.x, m_left + halfW, m_right - halfW);

    if (m_viewH >= levelH)
        m_position.y = (m_top + m_bottom) * 0.5f;
    else
        m_position.y = std::clamp(m_position.y, m_top + halfH, m_bottom - halfH);
}

}
