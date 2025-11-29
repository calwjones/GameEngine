#include "GameLoop.h"
#include <algorithm>

namespace Engine {

// this is the fixed-timestep loop the editor's own update() is modeled after
int GameLoop::tick(const UpdateFn& update, const RenderFn& render) {
    if (!m_running) return 0;

    m_time.update();
    // clamp frameTime bc if the prev frame took ages (lag spike, debugger paused us, etc)
    // we dont wanna then catch up 100 physics ticks in one frame — that crashes everything
    float frameTime = std::min(m_time.rawDelta(), m_maxFrameTime);

    // fps/ups counter. adds up frame time till it crosses 1 second, then snapshots
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
        // burn the accumulator in fixed chunks. this is THE key line for determinism —
        // update(dt) always sees the same dt so physics gives the same result every run
        while (m_accumulator >= m_timestep) {
            update(m_timestep);
            m_accumulator -= m_timestep;
            m_updateCount++;
            updates++;
        }
    }

    // alpha = how far into the next physics step we are, 0..1.
    // could be used for render interp (blend prev + curr pos) but i never hooked it up
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
    m_time.update();       // re-read the clock so the delta since pause doesn't count
    m_accumulator = 0.f;   // throw away any owed time — otherwise we'd speedrun thru paused time
}

}
