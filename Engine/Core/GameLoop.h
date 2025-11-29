#pragma once
#include "Time.h"
#include <functional>

namespace Engine {

// the "fix your timestep" pattern from that famous Glenn Fiedler article.
// idea: physics MUST run at a fixed rate (1/60s here) or collision gets weird
// when framerate dips. so we pile up real time in an accumulator and burn it
// off in fixed-size chunks each frame — render can run at whatever tho.
// the 0.25s cap = "spiral of death" guard. if a frame takes forever (debug
// breakpoint, laptop lid closed, etc) we dont then try to catch up by running
// 1000 physics steps back to back. just drop the extra time on the floor.
// NB: the editor doesnt actually use this class directly — it rolls its own
// accumulator inside EditorApplication::update() bc it needs to peek at state
// mid-loop (win check, death drain). this class is here as the "standalone
// runtime" path + so the viva can point at one clean example of the pattern.
class GameLoop {
public:
  using UpdateFn = std::function<void(float dt)>;
  using RenderFn = std::function<void(float interpolation)>;

private:
  Time m_time;
  float m_timestep = 1.f / 60.f; // one physics tick = 16.6ms. hardcoded 60hz
  float m_accumulator = 0.f;     // time we owe physics, not yet consumed
  float m_maxFrameTime = 0.25f;  // clamp: no more than 0.25s worth of catch-up per frame
  int m_ups = 0, m_fps = 0;      // updates/sec and frames/sec tracked separately
  int m_updateCount = 0, m_frameCount = 0;
  float m_fpsTimer = 0.f;
  bool m_running = false, m_paused = false;

public:
  void setTimestep(float ts) { m_timestep = ts; }
  float timestep() const { return m_timestep; }

  // call this once per frame. runs 0..N physics updates depending on how much
  // real time has built up, then calls render once. returns #updates it ran
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

} // namespace Engine
