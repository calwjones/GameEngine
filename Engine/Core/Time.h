#pragma once
#include <chrono>

namespace Engine {

// tiny wrapper around std::chrono. basically "how long was the last frame"
// also has a scale knob bc i wanted slow-mo (never actually used it tho lol)
class Time {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint m_lastFrame;
    float m_delta = 0.f;   // seconds since last update() call
    float m_total = 0.f;   // total seconds alive
    float m_scale = 1.f;   // 1 = normal, 0.5 = half speed, 0 = paused

public:
    Time() : m_lastFrame(Clock::now()) {}

    // call once per frame. reads the clock, diffs against last frame, stores both
    void update() {
        auto now = Clock::now();
        m_delta = std::chrono::duration<float>(now - m_lastFrame).count();
        m_total += m_delta;
        m_lastFrame = now;
    }

    void reset() { m_lastFrame = Clock::now(); m_delta = m_total = 0.f; }

    float delta() const { return m_delta * m_scale; }   // use this for gameplay (gets slowmo'd)
    float rawDelta() const { return m_delta; }           // unscaled — game loop accumulator needs real time
    float total() const { return m_total; }
    float scale() const { return m_scale; }
    void setScale(float s) { m_scale = s; }
};

}
