#include "GameLoop.h"
#include <algorithm>

namespace Engine {

int GameLoop::tick(const UpdateFn& update, const RenderFn& render) {
    if (!m_running) return 0;

    m_time.update();
    float frameTime = std::min(m_time.rawDelta(), m_maxFrameTime);

    m_fpsTimer += frameTime;
    m_frameCount++;

    if (m_fpsTimer >= 1.f) {
        m_fps = m_frameCount;
        m_ups = m_updateCount;
        m_frameCount = m_updateCount = 0;
        m_fpsTimer -= 1.f;
    }

    int updates = 0;
    if (!m_paused) {
        m_accumulator += frameTime;
        while (m_accumulator >= m_timestep) {
            update(m_timestep);
            m_accumulator -= m_timestep;
            m_updateCount++;
            updates++;
        }
    }

    render(m_accumulator / m_timestep);
    return updates;
}

void GameLoop::start() {
    m_running = true;
    m_paused = false;
    m_time.reset();
    m_accumulator = m_fpsTimer = 0.f;
    m_frameCount = m_updateCount = 0;
}

void GameLoop::resume() {
    m_paused = false;
    m_time.update();
    m_accumulator = 0.f;
}

}
