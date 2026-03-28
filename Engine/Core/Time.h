#pragma once
#include <chrono>

namespace Engine {

class Time {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint m_lastFrame;
    float m_delta = 0.f;
    float m_total = 0.f;
    float m_scale = 1.f;

public:
    Time() : m_lastFrame(Clock::now()) {}

    void update() {
        auto now = Clock::now();
        m_delta = std::chrono::duration<float>(now - m_lastFrame).count();
        m_total += m_delta;
        m_lastFrame = now;
    }

    void reset() { m_lastFrame = Clock::now(); m_delta = m_total = 0.f; }

    float delta() const { return m_delta * m_scale; }
    float rawDelta() const { return m_delta; }
    float total() const { return m_total; }
    float scale() const { return m_scale; }
    void setScale(float s) { m_scale = s; }
};

}
