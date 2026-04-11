#pragma once
#include <SFML/Graphics.hpp>
#include "../Engine/Core/Application.h"
#include <vector>
#include <unordered_map>

namespace Editor {

struct ViewportClick {
    Engine::Entity* entity = nullptr;
    bool occurred = false;
    bool ctrlHeld = false;
};

struct HudInfo {
    int score = 0;
    int totalCoins = -1;
    int lives = -1;
    const char* worldLabel = nullptr;
};

enum class ResizeHandle { None, TL, T, TR, L, R, BL, B, BR };
enum class ToolMode { Select, PaintTile };

struct PaintEdit { int cx; int cy; int oldId; int newId; };

class GameViewport {
    sf::RenderTexture m_tex;
    bool m_visible = true;
    unsigned int m_w = 640, m_h = 480;

    static constexpr float BASE_VIEW_H = 600.f;
    float m_levelWidth = 800.f;
    float m_levelHeight = 600.f;

    float m_imgX = 0, m_imgY = 0, m_imgW = 0, m_imgH = 0;

    sf::Vector2f m_viewOffset{0, 0};
    float m_viewZoom = 1.f;
    bool m_panning = false;
    int m_panButton = -1;
    sf::Vector2f m_panStart{0, 0};
    sf::Vector2f m_panOffsetStart{0, 0};

    bool m_gridEnabled = true;
    float m_gridSize = 16.f;

    ViewportClick m_click;

    bool m_dragging = false;
    bool m_dragActive = false;
    float m_dragClickX = 0, m_dragClickY = 0;
    static constexpr float DRAG_DEAD_ZONE = 4.f;
    std::unordered_map<Engine::Entity*, sf::Vector2f> m_dragOffsets;
    std::unordered_map<Engine::Entity*, sf::Vector2f> m_dragStartPositions;
    std::unordered_map<Engine::Entity*, std::pair<sf::Vector2f, sf::Vector2f>> m_dragStartMpPoints;
    bool m_dragCompleted = false;

    Engine::Entity* m_hoverEntity = nullptr;

    sf::Vector2f m_panVelocity{0, 0};
    sf::Vector2f m_panPrevMouse{0, 0};

    bool m_resizing = false;
    ResizeHandle m_resizeHandle = ResizeHandle::None;
    ResizeHandle m_resizeHover = ResizeHandle::None;
    Engine::Entity* m_resizeEntity = nullptr;
    sf::Vector2f m_resizeStartPos{0, 0};
    sf::Vector2f m_resizeStartSize{0, 0};
    bool m_resizeCompleted = false;

    bool m_mpDraggingB = false;
    bool m_mpHoverB = false;
    Engine::Entity* m_mpDragEntity = nullptr;
    sf::Vector2f m_mpDragStartB{0, 0};
    bool m_mpDragCompleted = false;

    ToolMode m_tool = ToolMode::Select;
    int m_paintTileId = 1;
    bool m_painting = false;
    std::vector<PaintEdit> m_paintStroke;
    std::vector<PaintEdit> m_paintCompleted;

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
    [[nodiscard]] Engine::Entity* consumeResizeComplete(sf::Vector2f& oldPos, sf::Vector2f& oldSize);
    [[nodiscard]] Engine::Entity* consumeMovingPlatformBComplete(sf::Vector2f& oldB);
    [[nodiscard]] bool consumePaintComplete(std::vector<PaintEdit>& out);

    ToolMode& tool() { return m_tool; }
    int& paintTileId() { return m_paintTileId; }

    // player/enemies tie movement to size, so resizing them breaks the gameplay feel
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
    void drawMovingPlatformPath(Engine::Entity* e);
    bool pickMovingPlatformB(Engine::Entity* e, sf::Vector2f world) const;
    void drawGrid();
    void drawLevelBounds();
    void clampViewOffset();
};

}
