#include "GameViewport.h"
#include "../Game/MovingPlatform.h"
#include <imgui.h>
#include <imgui-SFML.h>
#include <cmath>
#include <algorithm>

namespace Editor {

bool GameViewport::initialize(unsigned int w, unsigned int h) {
    m_w = w; m_h = h;
    return m_tex.create(w, h);
}

float GameViewport::viewW() const {
    float aspect = (m_h > 0) ? (float)m_w / (float)m_h : 1.f;
    return (BASE_VIEW_H / m_viewZoom) * aspect;
}

float GameViewport::viewH() const {
    return BASE_VIEW_H / m_viewZoom;
}

sf::Vector2f GameViewport::screenToWorld(float sx, float sy) const {
    if (m_imgW <= 0 || m_imgH <= 0) return {-1e9f, -1e9f};
    float normX = (sx - m_imgX) / m_imgW;
    float normY = (sy - m_imgY) / m_imgH;
    return {m_viewOffset.x + normX * viewW(), m_viewOffset.y + normY * viewH()};
}

Engine::Entity* GameViewport::pickEntity(Engine::Application& app, sf::Vector2f world) {
    float tolerance = 6.f / m_viewZoom;

    auto& ents = app.getEntityManager().getAllEntities();
    // reverse order so top entity wins on overlap
    for (int i = (int)ents.size() - 1; i >= 0; i--) {
        auto b = ents[i]->getBounds();
        sf::FloatRect expanded(b.left - tolerance, b.top - tolerance,
                               b.width + tolerance * 2, b.height + tolerance * 2);
        if (expanded.contains(world.x, world.y))
            return ents[i];
    }
    return nullptr;
}

ViewportClick GameViewport::consumeClick() {
    auto c = m_click;
    m_click = {};
    return c;
}

bool GameViewport::isResizableType(const std::string& type) {
    return type != "player" && type != "enemy" && type != "flying_enemy" &&
           type != "shooting_enemy" && type != "projectile";
}

// screen-constant size, capped so handles can't swallow a small entity when zoomed out
static float handleHalfSizeWorld(float zoom, sf::Vector2f entSize, float basePx) {
    float screenConstant = basePx / zoom;
    float capBySmallest = std::min(entSize.x, entSize.y) * 0.28f;
    return std::min(screenConstant, capBySmallest);
}

ResizeHandle GameViewport::pickResizeHandle(Engine::Entity* e, sf::Vector2f world) const {
    if (!e || !isResizableType(e->type)) return ResizeHandle::None;
    float hs = handleHalfSizeWorld(m_viewZoom, e->size, 7.f);
    sf::Vector2f p = e->position;
    sf::Vector2f s = e->size;
    struct HP { ResizeHandle h; float x, y; };
    HP pts[] = {
        {ResizeHandle::TL, p.x,             p.y},
        {ResizeHandle::T,  p.x + s.x/2.f,   p.y},
        {ResizeHandle::TR, p.x + s.x,       p.y},
        {ResizeHandle::L,  p.x,             p.y + s.y/2.f},
        {ResizeHandle::R,  p.x + s.x,       p.y + s.y/2.f},
        {ResizeHandle::BL, p.x,             p.y + s.y},
        {ResizeHandle::B,  p.x + s.x/2.f,   p.y + s.y},
        {ResizeHandle::BR, p.x + s.x,       p.y + s.y},
    };
    for (auto& h : pts) {
        if (std::abs(world.x - h.x) <= hs && std::abs(world.y - h.y) <= hs)
            return h.h;
    }
    return ResizeHandle::None;
}

void GameViewport::drawResizeHandles(Engine::Entity* e) {
    float hs = handleHalfSizeWorld(m_viewZoom, e->size, 6.f);
    sf::Vector2f p = e->position;
    sf::Vector2f s = e->size;
    sf::Vector2f pts[8] = {
        {p.x,             p.y},
        {p.x + s.x/2.f,   p.y},
        {p.x + s.x,       p.y},
        {p.x,             p.y + s.y/2.f},
        {p.x + s.x,       p.y + s.y/2.f},
        {p.x,             p.y + s.y},
        {p.x + s.x/2.f,   p.y + s.y},
        {p.x + s.x,       p.y + s.y},
    };
    for (auto& pt : pts) {
        sf::RectangleShape h(sf::Vector2f(hs * 2.f, hs * 2.f));
        h.setPosition(pt.x - hs, pt.y - hs);
        h.setFillColor(sf::Color(255, 220, 50, 230));
        h.setOutlineColor(sf::Color(20, 20, 20, 220));
        h.setOutlineThickness(1.f / m_viewZoom);
        m_tex.draw(h);
    }
}

Engine::Entity* GameViewport::consumeResizeComplete(sf::Vector2f& oldPos, sf::Vector2f& oldSize) {
    if (!m_resizeCompleted) return nullptr;
    m_resizeCompleted = false;
    oldPos = m_resizeStartPos;
    oldSize = m_resizeStartSize;
    auto* e = m_resizeEntity;
    m_resizeEntity = nullptr;
    return e;
}

Engine::Entity* GameViewport::consumeMovingPlatformBComplete(sf::Vector2f& oldB) {
    if (!m_mpDragCompleted) return nullptr;
    m_mpDragCompleted = false;
    oldB = m_mpDragStartB;
    auto* e = m_mpDragEntity;
    m_mpDragEntity = nullptr;
    return e;
}

void GameViewport::drawMovingPlatformPath(Engine::Entity* e) {
    auto* mp = dynamic_cast<Game::MovingPlatform*>(e);
    if (!mp) return;

    sf::Vector2f aCenter = {mp->position.x + mp->size.x * 0.5f,
                             mp->position.y + mp->size.y * 0.5f};
    sf::Vector2f pB = mp->getPointB();
    // legacy default — fake it so the viz works before first save
    if ((pB.x == 0.f && pB.y == 0.f) || (pB.x == 200.f && pB.y == 0.f))
        pB = {mp->position.x + 200.f, mp->position.y};
    sf::Vector2f bCenter = {pB.x + mp->size.x * 0.5f,
                             pB.y + mp->size.y * 0.5f};

    sf::Color pathCol(255, 220, 50, 180);
    sf::Vector2f dir = bCenter - aCenter;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.1f) {
        sf::Vector2f unit = {dir.x / len, dir.y / len};
        float dash = 10.f / m_viewZoom;
        float gap  = 6.f  / m_viewZoom;
        float step = dash + gap;
        for (float d = 0.f; d < len; d += step) {
            float end = std::min(d + dash, len);
            sf::Vertex seg[] = {
                sf::Vertex(aCenter + unit * d, pathCol),
                sf::Vertex(aCenter + unit * end, pathCol),
            };
            m_tex.draw(seg, 2, sf::Lines);
        }
    }

    sf::RectangleShape ghost(mp->size);
    ghost.setPosition(pB);
    ghost.setFillColor(sf::Color(60, 120, 180, 70));
    ghost.setOutlineColor(sf::Color(255, 220, 50, 200));
    ghost.setOutlineThickness(1.5f / m_viewZoom);
    m_tex.draw(ghost);

    float hs = handleHalfSizeWorld(m_viewZoom, mp->size, 6.f);
    sf::RectangleShape grab(sf::Vector2f(hs * 2.f, hs * 2.f));
    grab.setPosition(bCenter.x - hs, bCenter.y - hs);
    grab.setFillColor(sf::Color(255, 180, 50, 240));
    grab.setOutlineColor(sf::Color(20, 20, 20, 220));
    grab.setOutlineThickness(1.f / m_viewZoom);
    m_tex.draw(grab);
}

bool GameViewport::pickMovingPlatformB(Engine::Entity* e, sf::Vector2f world) const {
    auto* mp = dynamic_cast<Game::MovingPlatform*>(e);
    if (!mp) return false;
    sf::Vector2f pB = mp->getPointB();
    if ((pB.x == 0.f && pB.y == 0.f) || (pB.x == 200.f && pB.y == 0.f))
        pB = {mp->position.x + 200.f, mp->position.y};
    sf::Vector2f bCenter = {pB.x + mp->size.x * 0.5f,
                             pB.y + mp->size.y * 0.5f};
    float hs = handleHalfSizeWorld(m_viewZoom, mp->size, 7.f);
    return std::abs(world.x - bCenter.x) <= hs && std::abs(world.y - bCenter.y) <= hs;
}

bool GameViewport::consumeDragComplete(std::vector<std::pair<Engine::Entity*, sf::Vector2f>>& oldPositions) {
    if (!m_dragCompleted) return false;
    m_dragCompleted = false;
    oldPositions.clear();
    for (auto& [entity, pos] : m_dragStartPositions)
        oldPositions.emplace_back(entity, pos);
    m_dragStartPositions.clear();
    return true;
}

void GameViewport::drawLevelBounds() {
    sf::RectangleShape gameArea(sf::Vector2f(m_levelWidth, m_levelHeight));
    gameArea.setPosition(0.f, 0.f);
    gameArea.setFillColor(sf::Color(48, 52, 60));
    m_tex.draw(gameArea);

    sf::RectangleShape border(sf::Vector2f(m_levelWidth, m_levelHeight));
    border.setPosition(0.f, 0.f);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(255, 255, 255, 25));
    border.setOutlineThickness(1.f / m_viewZoom);
    m_tex.draw(border);
}

void GameViewport::drawGrid() {
    if (m_gridSize < 1.f) m_gridSize = 16.f;
    float vw = viewW(), vh = viewH();
    float left = m_viewOffset.x;
    float top = m_viewOffset.y;
    float right = left + vw;
    float bottom = top + vh;

    sf::Color gridCol(255, 255, 255, 20);
    sf::Color originCol(255, 255, 255, 50);

    float startX = std::floor(left / m_gridSize) * m_gridSize;
    for (float x = startX; x <= right; x += m_gridSize) {
        sf::Color c = (std::abs(x) < 0.5f) ? originCol : gridCol;
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, top), c),
            sf::Vertex(sf::Vector2f(x, bottom), c)
        };
        m_tex.draw(line, 2, sf::Lines);
    }

    float startY = std::floor(top / m_gridSize) * m_gridSize;
    for (float y = startY; y <= bottom; y += m_gridSize) {
        sf::Color c = (std::abs(y) < 0.5f) ? originCol : gridCol;
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(left, y), c),
            sf::Vertex(sf::Vector2f(right, y), c)
        };
        m_tex.draw(line, 2, sf::Lines);
    }
}

void GameViewport::render(Engine::Application& app, bool running,
                          const std::vector<Engine::Entity*>& selection,
                          const HudInfo& hud,
                          Engine::Entity* primary) {
    if (!m_visible) return;

    ImGui::SetNextWindowSize(ImVec2(660, 520), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Game Viewport", &m_visible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::End();
        return;
    }

    ImVec2 region = ImGui::GetContentRegionAvail();
    unsigned int nw = std::max(1u, (unsigned int)region.x);
    unsigned int nh = std::max(1u, (unsigned int)region.y);
    if (nw != m_w || nh != m_h) {
        m_w = nw; m_h = nh;
        m_tex.create(nw, nh);
    }

    float vw = viewW(), vh = viewH();
    sf::View view(sf::FloatRect(m_viewOffset.x, m_viewOffset.y, vw, vh));
    m_tex.setView(view);

    m_tex.clear(sf::Color(32, 35, 42));
    drawLevelBounds();
    if (m_gridEnabled && !running) drawGrid();

    for (auto* e : app.getEntityManager().getAllEntities()) {
        if (e->texture) {
            sf::Sprite sprite(*e->texture);
            sprite.setPosition(e->position);
            sf::Vector2u texSize = e->texture->getSize();
            if (texSize.x > 0 && texSize.y > 0)
                sprite.setScale(e->size.x / texSize.x, e->size.y / texSize.y);
            m_tex.draw(sprite);
        } else {
            sf::RectangleShape s(e->size);
            s.setPosition(e->position);
            s.setFillColor(e->color);
            m_tex.draw(s);
        }
    }

    if (m_hoverEntity && !running) {
        bool alreadySelected = std::find(selection.begin(), selection.end(), m_hoverEntity) != selection.end();
        if (!alreadySelected) {
            sf::RectangleShape hoverOutline(m_hoverEntity->size);
            hoverOutline.setPosition(m_hoverEntity->position);
            hoverOutline.setFillColor(sf::Color::Transparent);
            hoverOutline.setOutlineColor(sf::Color(255, 255, 255, 60));
            hoverOutline.setOutlineThickness(1.5f / m_viewZoom);
            m_tex.draw(hoverOutline);
        }
    }

    for (auto* sel : selection) {
        sf::RectangleShape outline(sel->size);
        outline.setPosition(sel->position);
        outline.setFillColor(sf::Color::Transparent);
        bool isPrimary = (sel == primary);
        outline.setOutlineColor(isPrimary ? sf::Color(255, 220, 50, 255) : sf::Color(255, 200, 0, 140));
        outline.setOutlineThickness((isPrimary ? 2.5f : 1.5f) / m_viewZoom);
        m_tex.draw(outline);
    }

    if (primary && !running && primary->type == "moving_platform" && !m_dragActive)
        drawMovingPlatformPath(primary);

    if (primary && !running && isResizableType(primary->type) && !m_dragActive)
        drawResizeHandles(primary);

    m_tex.display();

    ImVec2 imgScreenPos = ImGui::GetCursorScreenPos();
    m_imgX = imgScreenPos.x;
    m_imgY = imgScreenPos.y;
    m_imgW = region.x;
    m_imgH = region.y;

    // InvisibleButton before Image claims mouse events before imgui treats them as window drag
    ImGui::InvisibleButton("##viewport_input", ImVec2(region.x, region.y));
    bool hovered = ImGui::IsItemHovered();

    ImGui::SetCursorScreenPos(imgScreenPos);
    ImGui::Image(m_tex, sf::Vector2f(region.x, region.y));

    if (!hovered) m_hoverEntity = nullptr;

    if (hovered && !running) {
        ImVec2 mouse = ImGui::GetMousePos();
        sf::Vector2f world = screenToWorld(mouse.x, mouse.y);

        m_resizeHover = ResizeHandle::None;
        m_mpHoverB = false;
        if (!m_dragging && !m_panning && !m_resizing && !m_mpDraggingB) {
            m_resizeHover = pickResizeHandle(primary, world);
            if (m_resizeHover == ResizeHandle::None && primary &&
                primary->type == "moving_platform")
                m_mpHoverB = pickMovingPlatformB(primary, world);
        }

        if (m_resizeHover != ResizeHandle::None || m_resizing) {
            ResizeHandle h = m_resizing ? m_resizeHandle : m_resizeHover;
            ImGuiMouseCursor cur = ImGuiMouseCursor_Arrow;
            switch (h) {
                case ResizeHandle::TL: case ResizeHandle::BR: cur = ImGuiMouseCursor_ResizeNWSE; break;
                case ResizeHandle::TR: case ResizeHandle::BL: cur = ImGuiMouseCursor_ResizeNESW; break;
                case ResizeHandle::T:  case ResizeHandle::B:  cur = ImGuiMouseCursor_ResizeNS;   break;
                case ResizeHandle::L:  case ResizeHandle::R:  cur = ImGuiMouseCursor_ResizeEW;   break;
                default: break;
            }
            ImGui::SetMouseCursor(cur);
        } else if (m_mpHoverB || m_mpDraggingB) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }

        if (!m_dragging && !m_panning && !m_resizing && !m_mpDraggingB &&
            m_resizeHover == ResizeHandle::None && !m_mpHoverB)
            m_hoverEntity = pickEntity(app, world);
        else
            m_hoverEntity = nullptr;

        bool handleClickConsumed = false;
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
            m_resizeHover != ResizeHandle::None && primary) {
            m_resizing = true;
            m_resizeHandle = m_resizeHover;
            m_resizeEntity = primary;
            m_resizeStartPos = primary->position;
            m_resizeStartSize = primary->size;
            m_dragging = false;
            m_dragActive = false;
            m_dragOffsets.clear();
            handleClickConsumed = true;
        }

        if (!handleClickConsumed && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
            m_mpHoverB && primary && primary->type == "moving_platform") {
            if (auto* mp = dynamic_cast<Game::MovingPlatform*>(primary)) {
                m_mpDraggingB = true;
                m_mpDragEntity = primary;
                m_mpDragStartB = mp->getPointB();
                if ((m_mpDragStartB.x == 0.f && m_mpDragStartB.y == 0.f) ||
                    (m_mpDragStartB.x == 200.f && m_mpDragStartB.y == 0.f)) {
                    m_mpDragStartB = {primary->position.x + 200.f, primary->position.y};
                    mp->setPointB(m_mpDragStartB);
                }
                m_dragging = false;
                m_dragActive = false;
                m_dragOffsets.clear();
                handleClickConsumed = true;
            }
        }

        if (!handleClickConsumed && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            auto* picked = pickEntity(app, world);
            bool ctrl = ImGui::GetIO().KeyCtrl;
            m_click = {picked, true, ctrl};

            if (picked) {
                bool inSelection = std::find(selection.begin(), selection.end(), picked) != selection.end();

                if (ctrl && inSelection) {
                    m_dragging = false;
                    m_dragActive = false;
                } else {
                    m_dragging = true;
                    m_dragActive = false;
                    m_dragClickX = mouse.x;
                    m_dragClickY = mouse.y;
                    m_dragOffsets.clear();
                    m_dragStartPositions.clear();

                    std::vector<Engine::Entity*> dragSet;
                    if (ctrl && !inSelection) {
                        dragSet = selection;
                        dragSet.push_back(picked);
                    } else if (!ctrl && inSelection) {
                        dragSet = selection;
                    } else {
                        dragSet.push_back(picked);
                    }

                    m_dragStartMpPoints.clear();
                    for (auto* e : dragSet) {
                        m_dragOffsets[e] = e->position - world;
                        m_dragStartPositions[e] = e->position;
                        if (auto* mp = dynamic_cast<Game::MovingPlatform*>(e))
                            m_dragStartMpPoints[e] = {mp->getPointA(), mp->getPointB()};
                    }
                }
            } else {
                // empty space: click deselects, drag pans
                m_dragging = false;
                m_dragActive = false;
                m_dragOffsets.clear();
                m_panning = true;
                m_panButton = ImGuiMouseButton_Left;
                m_panStart = {mouse.x, mouse.y};
                m_panPrevMouse = {mouse.x, mouse.y};
                m_panOffsetStart = m_viewOffset;
                m_panVelocity = {0.f, 0.f};
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            m_panning = true;
            m_panButton = ImGuiMouseButton_Right;
            m_panStart = {mouse.x, mouse.y};
            m_panPrevMouse = {mouse.x, mouse.y};
            m_panOffsetStart = m_viewOffset;
            m_panVelocity = {0.f, 0.f};
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
            m_panning = true;
            m_panButton = ImGuiMouseButton_Middle;
            m_panStart = {mouse.x, mouse.y};
            m_panPrevMouse = {mouse.x, mouse.y};
            m_panOffsetStart = m_viewOffset;
            m_panVelocity = {0.f, 0.f};
        }

        float scroll = ImGui::GetIO().MouseWheel;
        if (scroll != 0.f) {
            float normX = (mouse.x - m_imgX) / m_imgW;
            float normY = (mouse.y - m_imgY) / m_imgH;
            float worldX = m_viewOffset.x + normX * vw;
            float worldY = m_viewOffset.y + normY * vh;

            float newZoom = m_viewZoom * (1.f + scroll * 0.15f);
            newZoom = std::clamp(newZoom, 0.25f, 4.f);

            float aspect = (m_h > 0) ? (float)m_w / (float)m_h : 1.f;
            float newViewH = BASE_VIEW_H / newZoom;
            float newViewW = newViewH * aspect;
            // keep cursor world point stable across zoom
            m_viewOffset.x = worldX - normX * newViewW;
            m_viewOffset.y = worldY - normY * newViewH;
            m_viewZoom = newZoom;
        }
    }

    if (m_panning) {
        if (m_panButton >= 0 && ImGui::IsMouseDown((ImGuiMouseButton)m_panButton)) {
            ImVec2 mouse = ImGui::GetMousePos();
            float dx = (mouse.x - m_panStart.x) / m_imgW * vw;
            float dy = (mouse.y - m_panStart.y) / m_imgH * vh;
            m_viewOffset.x = m_panOffsetStart.x - dx;
            m_viewOffset.y = m_panOffsetStart.y - dy;
            float mdx = (mouse.x - m_panPrevMouse.x) / m_imgW * vw;
            float mdy = (mouse.y - m_panPrevMouse.y) / m_imgH * vh;
            m_panVelocity = {-mdx * 60.f, -mdy * 60.f};
            m_panPrevMouse = {mouse.x, mouse.y};
        } else {
            m_panning = false;
            m_panButton = -1;
        }
    } else if (m_panVelocity.x != 0.f || m_panVelocity.y != 0.f) {
        float friction = 0.9f;
        m_viewOffset.x += m_panVelocity.x * (1.f / 60.f);
        m_viewOffset.y += m_panVelocity.y * (1.f / 60.f);
        m_panVelocity.x *= friction;
        m_panVelocity.y *= friction;
        if (std::abs(m_panVelocity.x) < 0.5f && std::abs(m_panVelocity.y) < 0.5f)
            m_panVelocity = {0.f, 0.f};
    }

    if (m_dragging && !running) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ImVec2 mouse = ImGui::GetMousePos();

            if (!m_dragActive) {
                float dx = mouse.x - m_dragClickX;
                float dy = mouse.y - m_dragClickY;
                if (dx * dx + dy * dy >= DRAG_DEAD_ZONE * DRAG_DEAD_ZONE)
                    m_dragActive = true;
            }

            if (m_dragActive) {
                float edgeMargin = 30.f;
                float scrollSpeed = 200.f / 60.f / m_viewZoom;
                if (mouse.x - m_imgX < edgeMargin) m_viewOffset.x -= scrollSpeed;
                else if (m_imgX + m_imgW - mouse.x < edgeMargin) m_viewOffset.x += scrollSpeed;
                if (mouse.y - m_imgY < edgeMargin) m_viewOffset.y -= scrollSpeed;
                else if (m_imgY + m_imgH - mouse.y < edgeMargin) m_viewOffset.y += scrollSpeed;

                sf::Vector2f world = screenToWorld(mouse.x, mouse.y);

                if (m_gridEnabled && !m_dragOffsets.empty()) {
                    auto anchorIt = m_dragOffsets.begin();
                    sf::Vector2f anchorTarget = world + anchorIt->second;
                    sf::Vector2f snapped;
                    snapped.x = std::round(anchorTarget.x / m_gridSize) * m_gridSize;
                    snapped.y = std::round(anchorTarget.y / m_gridSize) * m_gridSize;
                    sf::Vector2f snapDelta = snapped - anchorTarget;

                    for (auto& [entity, offset] : m_dragOffsets)
                        entity->position = world + offset + snapDelta;
                } else {
                    for (auto& [entity, offset] : m_dragOffsets)
                        entity->position = world + offset;
                }

                // carry patrol path along with platform
                for (auto& [entity, startAB] : m_dragStartMpPoints) {
                    auto* mp = dynamic_cast<Game::MovingPlatform*>(entity);
                    if (!mp) continue;
                    auto startPosIt = m_dragStartPositions.find(entity);
                    if (startPosIt == m_dragStartPositions.end()) continue;
                    sf::Vector2f delta = entity->position - startPosIt->second;
                    mp->setPointA(startAB.first + delta);
                    mp->setPointB(startAB.second + delta);
                }
            }
        } else {
            bool moved = false;
            for (auto& [entity, startPos] : m_dragStartPositions) {
                if (entity->position.x != startPos.x || entity->position.y != startPos.y) {
                    moved = true;
                    break;
                }
            }
            if (moved) m_dragCompleted = true;
            m_dragging = false;
            m_dragActive = false;
            m_dragOffsets.clear();
            m_dragStartMpPoints.clear();
        }
    }

    if (m_resizing && m_resizeEntity && !running) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ImVec2 mouse = ImGui::GetMousePos();
            sf::Vector2f world = screenToWorld(mouse.x, mouse.y);

            float startLeft   = m_resizeStartPos.x;
            float startTop    = m_resizeStartPos.y;
            float startRight  = startLeft + m_resizeStartSize.x;
            float startBottom = startTop  + m_resizeStartSize.y;
            float newLeft = startLeft, newRight = startRight;
            float newTop  = startTop,  newBottom = startBottom;

            switch (m_resizeHandle) {
                case ResizeHandle::TL: newLeft = world.x; newTop = world.y; break;
                case ResizeHandle::T:  newTop = world.y; break;
                case ResizeHandle::TR: newRight = world.x; newTop = world.y; break;
                case ResizeHandle::L:  newLeft = world.x; break;
                case ResizeHandle::R:  newRight = world.x; break;
                case ResizeHandle::BL: newLeft = world.x; newBottom = world.y; break;
                case ResizeHandle::B:  newBottom = world.y; break;
                case ResizeHandle::BR: newRight = world.x; newBottom = world.y; break;
                case ResizeHandle::None: break;
            }

            if (m_gridEnabled && m_gridSize >= 1.f) {
                auto snap = [&](float v) { return std::round(v / m_gridSize) * m_gridSize; };
                if (m_resizeHandle == ResizeHandle::TL || m_resizeHandle == ResizeHandle::L || m_resizeHandle == ResizeHandle::BL)
                    newLeft = snap(newLeft);
                if (m_resizeHandle == ResizeHandle::TR || m_resizeHandle == ResizeHandle::R || m_resizeHandle == ResizeHandle::BR)
                    newRight = snap(newRight);
                if (m_resizeHandle == ResizeHandle::TL || m_resizeHandle == ResizeHandle::T || m_resizeHandle == ResizeHandle::TR)
                    newTop = snap(newTop);
                if (m_resizeHandle == ResizeHandle::BL || m_resizeHandle == ResizeHandle::B || m_resizeHandle == ResizeHandle::BR)
                    newBottom = snap(newBottom);
            }

            constexpr float MIN_SIZE = 8.f;
            if (newRight - newLeft < MIN_SIZE) {
                if (m_resizeHandle == ResizeHandle::L || m_resizeHandle == ResizeHandle::TL || m_resizeHandle == ResizeHandle::BL)
                    newLeft = newRight - MIN_SIZE;
                else
                    newRight = newLeft + MIN_SIZE;
            }
            if (newBottom - newTop < MIN_SIZE) {
                if (m_resizeHandle == ResizeHandle::T || m_resizeHandle == ResizeHandle::TL || m_resizeHandle == ResizeHandle::TR)
                    newTop = newBottom - MIN_SIZE;
                else
                    newBottom = newTop + MIN_SIZE;
            }

            m_resizeEntity->position = {newLeft, newTop};
            m_resizeEntity->size     = {newRight - newLeft, newBottom - newTop};
        } else {
            bool changed = (m_resizeEntity->position.x != m_resizeStartPos.x) ||
                           (m_resizeEntity->position.y != m_resizeStartPos.y) ||
                           (m_resizeEntity->size.x != m_resizeStartSize.x) ||
                           (m_resizeEntity->size.y != m_resizeStartSize.y);
            if (changed) m_resizeCompleted = true;
            m_resizing = false;
            m_resizeHandle = ResizeHandle::None;
        }
    }

    if (m_mpDraggingB && m_mpDragEntity && !running) {
        auto* mp = dynamic_cast<Game::MovingPlatform*>(m_mpDragEntity);
        if (!mp) { m_mpDraggingB = false; m_mpDragEntity = nullptr; }
        else if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ImVec2 mouse = ImGui::GetMousePos();
            sf::Vector2f world = screenToWorld(mouse.x, mouse.y);
            sf::Vector2f newB = {world.x - mp->size.x * 0.5f,
                                  world.y - mp->size.y * 0.5f};
            if (m_gridEnabled && m_gridSize >= 1.f) {
                newB.x = std::round(newB.x / m_gridSize) * m_gridSize;
                newB.y = std::round(newB.y / m_gridSize) * m_gridSize;
            }
            mp->setPointB(newB);
        } else {
            sf::Vector2f pB = mp->getPointB();
            bool changed = (pB.x != m_mpDragStartB.x) || (pB.y != m_mpDragStartB.y);
            if (changed) m_mpDragCompleted = true;
            m_mpDraggingB = false;
        }
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    ImVec2 pos = ImGui::GetWindowPos();
    bool gridBadge = m_gridEnabled && !running;
    bool showOverlay = (m_viewZoom != 1.f || gridBadge);
    if (showOverlay) {
        ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 30));
        ImGui::SetNextWindowBgAlpha(0.6f);
        if (ImGui::BeginChild("##overlay", ImVec2(150, 50), false, flags)) {
            if (m_viewZoom != 1.f)
                ImGui::Text("Zoom: %.0f%%", m_viewZoom * 100.f);
            if (gridBadge)
                ImGui::Text("Grid: %.0fpx", m_gridSize);
        }
        ImGui::EndChild();
    }

    if (running) {
        int hudLines = 1;
        if (hud.worldLabel) ++hudLines;
        if (hud.lives >= 0) ++hudLines;
        if (hud.totalCoins >= 0) ++hudLines;
        float hudH = 14.f + 18.f * hudLines;

        ImGui::SetNextWindowPos(ImVec2(pos.x + ImGui::GetWindowSize().x - 210, pos.y + 30));
        ImGui::SetNextWindowBgAlpha(0.6f);
        if (ImGui::BeginChild("##hud", ImVec2(200, hudH), false, flags)) {
            if (hud.worldLabel)
                ImGui::TextColored(ImVec4(0.55f, 0.85f, 1.0f, 1.f), "%s", hud.worldLabel);
            ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "Coins: %d", hud.score);
            if (hud.totalCoins >= 0)
                ImGui::TextColored(ImVec4(1.f, 0.70f, 0.20f, 1.f), "Total: %d", hud.totalCoins);
            if (hud.lives >= 0)
                ImGui::TextColored(ImVec4(1.f, 0.40f, 0.40f, 1.f), "Lives: %d", hud.lives);
        }
        ImGui::EndChild();

        ImGui::SetNextWindowPos(ImVec2(pos.x + ImGui::GetWindowSize().x - 200, pos.y + ImGui::GetWindowSize().y - 40));
        ImGui::SetNextWindowBgAlpha(0.6f);
        if (ImGui::BeginChild("##hint", ImVec2(220, 30), false, flags))
            ImGui::TextDisabled("WASD/Arrows: Move | Space: Jump");
        ImGui::EndChild();
    }

    ImGui::End();
}

}
