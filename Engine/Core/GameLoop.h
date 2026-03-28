#pragma once
#include "Time.h"
#include <functional>

namespace Engine {

// fixed-timestep loop with accumulator; clamped to avoid catch-up spirals
class GameLoop {
public:
  using UpdateFn = std::function<void(float dt)>;
  using RenderFn = std::function<void(float interpolation)>;

private:
  Time m_time;
  float m_timestep = 1.f / 60.f;
  float m_accumulator = 0.f;
  float m_maxFrameTime = 0.25f;
  int m_ups = 0, m_fps = 0;
  int m_updateCount = 0, m_frameCount = 0;
  float m_fpsTimer = 0.f;
  bool m_running = false, m_paused = false;

public:
  void setTimestep(float ts) { m_timestep = ts; }
  float timestep() const { return m_timestep; }

  int tick(const UpdateFn &update, const RenderFn &render);

  void start();
  void stop() { m_running = false; }
  void pause() { m_paused = true; }
  void resume();

  bool running() const { return m_running; }
  bool paused() const { return m_paused; }
  int ups() const { return m_ups; }
  int fps() const { return m_fps; }
  float timestepMs() const { return m_timestep * 1000.f; }
};

}
