#pragma once
#include <SFML/Graphics.hpp>
#include "../Engine/Core/Application.h"
#include <vector>
#include <unordered_map>

namespace Editor {

// returned by consumeClick() - which entity was clicked (null = clicked empty space), and whether ctrl was held
struct ViewportClick {
    Engine::Entity* entity = nullptr;
    bool occurred = false;
    bool ctrlHeld = false;
};

// optional HUD fields shown in the top-right overlay during play.
// score is always shown when running; the rest are only drawn when >= 0 / non-null,
// so individual-level play stays clean and world play shows the full stats.
struct HudInfo {
    int score = 0;              // coins collected this level
    int totalCoins = -1;        // -1 = hide (single-level play has no running total)
    int lives = -1;             // -1 = hide
    const char* worldLabel = nullptr;   // e.g. "World 1  -  Level 2 / 4"
};

// 9-state enum for which resize grabber the mouse is on (None = not on any).
// the edge ones resize 1 axis, the corner ones resize both at once. named
// after compass dirs bc its easier to reason about than "handle 3" / "handle 7"
enum class ResizeHandle { None, TL, T, TR, L, R, BL, B, BR };

// the game preview panel - renders to sf::RenderTexture then displays as an imgui image
// handles pan (left-empty/right/middle-drag), zoom (scroll), entity click-select,
// entity drag-to-move, and click-drag resize via the 8 handles on the primary selection
class GameViewport {
    sf::RenderTexture m_tex;   // game renders into this, then its displayed as an imgui image
    bool m_visible = true;
    unsigned int m_w = 640, m_h = 480;

    static constexpr float BASE_VIEW_H = 600.f;   // game coordinate space height
    float m_levelWidth = 800.f;    // updated via setLevelSize when a level is loaded/created
    float m_levelHeight = 600.f;

    float m_imgX = 0, m_imgY = 0, m_imgW = 0, m_imgH = 0;   // imgui image rect in screen space - needed for mouse hit testing

    sf::Vector2f m_viewOffset{0, 0};   // top-left corner of the visible region in world space
    float m_viewZoom = 1.f;
    bool m_panning = false;
    int m_panButton = -1;   // which mouse btn started the pan (0=left, 1=right, 2=middle)
    sf::Vector2f m_panStart{0, 0};
    sf::Vector2f m_panOffsetStart{0, 0};

    bool m_gridEnabled = true;    // on by default — designers want snap from frame 1. auto-hidden during play
    float m_gridSize = 16.f;

    ViewportClick m_click;   // pending click result, consumed once per frame by EditorApplication

    bool m_dragging = false;      // mouse is held on an entity
    bool m_dragActive = false;    // drag threshold exceeded, entity is actually moving
    float m_dragClickX = 0, m_dragClickY = 0;
    static constexpr float DRAG_DEAD_ZONE = 4.f;   // pixels before drag starts - prevents accidental moves on click
    std::unordered_map<Engine::Entity*, sf::Vector2f> m_dragOffsets;        // world-space offset from entity origin to mouse
    std::unordered_map<Engine::Entity*, sf::Vector2f> m_dragStartPositions; // positions before drag started - for undo
    // moving platforms in the drag set: stashed pointA/pointB at drag start
    // so we can slide the whole patrol path along with the platform
    std::unordered_map<Engine::Entity*, std::pair<sf::Vector2f, sf::Vector2f>> m_dragStartMpPoints;
    bool m_dragCompleted = false;

    Engine::Entity* m_hoverEntity = nullptr;

    sf::Vector2f m_panVelocity{0, 0};    // momentum for middle-drag panning
    sf::Vector2f m_panPrevMouse{0, 0};

    // resize state — only tracks ONE entity (the primary selection) bc u cant
    // reasonably scale a multi-select from a single handle
    bool m_resizing = false;
    ResizeHandle m_resizeHandle = ResizeHandle::None;
    ResizeHandle m_resizeHover = ResizeHandle::None;   // for cursor hint on hover
    Engine::Entity* m_resizeEntity = nullptr;
    sf::Vector2f m_resizeStartPos{0, 0};    // entity pos at start of drag — for clean math
    sf::Vector2f m_resizeStartSize{0, 0};
    bool m_resizeCompleted = false;         // 1-frame flag polled by EditorApplication

    // moving platform pointB drag — visual authoring of the patrol path endpoint.
    // only engages when the primary selection is a moving_platform and the user
    // grabs the little square on pointB
    bool m_mpDraggingB = false;
    bool m_mpHoverB = false;
    Engine::Entity* m_mpDragEntity = nullptr;
    sf::Vector2f m_mpDragStartB{0, 0};      // original pointB at drag start — for undo
    bool m_mpDragCompleted = false;

public:
    float viewW() const;
    float viewH() const;

    bool initialize(unsigned int w, unsigned int h);
    void render(Engine::Application& app, bool running,
                const std::vector<Engine::Entity*>& selection,
                const HudInfo& hud,
                Engine::Entity* primary = nullptr);
    void shutdown() {}
    bool& isVisible() { return m_visible; }
    void cancelDrag() { m_dragging = false; m_dragActive = false; m_dragOffsets.clear(); }

    [[nodiscard]] ViewportClick consumeClick();
    [[nodiscard]] bool consumeDragComplete(std::vector<std::pair<Engine::Entity*, sf::Vector2f>>& oldPositions);

    // polled once per frame — returns non-null if a resize just finished.
    // editor pushes a PropertyChangeCommand so the resize is undoable
    [[nodiscard]] Engine::Entity* consumeResizeComplete(sf::Vector2f& oldPos, sf::Vector2f& oldSize);

    // polled once per frame — returns non-null if a moving-platform pointB
    // endpoint drag just completed. editor uses it to push undo
    [[nodiscard]] Engine::Entity* consumeMovingPlatformBComplete(sf::Vector2f& oldB);

    // stuff like player/enemies have movement/AI tied to their dimensions, so
    // exposing resize handles for them would just confuse the designer. everything
    // else is free game (walls, platforms, hazards, coins, goal...)
    static bool isResizableType(const std::string& type);

    bool& gridEnabled() { return m_gridEnabled; }
    float& gridSize() { return m_gridSize; }

    float getZoom() const { return m_viewZoom; }
    void resetView() { m_viewOffset = {0, 0}; m_viewZoom = 1.f; }
    void setLevelSize(float w, float h) { m_levelWidth = w; m_levelHeight = h; }
    sf::Vector2f getViewOffset() const { return m_viewOffset; }
    void setViewOffset(sf::Vector2f off) { m_viewOffset = off; }
    void setZoom(float z) { m_viewZoom = z; }

    void setViewCenter(sf::Vector2f center) {
        m_viewOffset.x = center.x - viewW() / 2.f;
        m_viewOffset.y = center.y - viewH() / 2.f;
    }

    sf::Vector2f screenToWorld(float sx, float sy) const;

private:
    Engine::Entity* pickEntity(Engine::Application& app, sf::Vector2f world);
    ResizeHandle pickResizeHandle(Engine::Entity* e, sf::Vector2f world) const;
    void drawResizeHandles(Engine::Entity* e);
    // moving platform authoring viz: dashed path line + ghost rect at pointB + grab handle
    void drawMovingPlatformPath(Engine::Entity* e);
    bool pickMovingPlatformB(Engine::Entity* e, sf::Vector2f world) const;
    void drawGrid();
    void drawLevelBounds();
};

}
