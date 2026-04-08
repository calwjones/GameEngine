#include "TileLayer.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <algorithm>
#include <cmath>

namespace Engine {

TileLayer::TileLayer(int w, int h, float cellSize) {
    resize(w, h, cellSize);
}

void TileLayer::resize(int w, int h, float cellSize) {
    m_width = std::max(0, w);
    m_height = std::max(0, h);
    m_cellSize = cellSize > 0.f ? cellSize : 32.f;
    m_cells.assign((size_t)m_width * (size_t)m_height, 0);
}

void TileLayer::clear() {
    std::fill(m_cells.begin(), m_cells.end(), 0);
}

int TileLayer::at(int cx, int cy) const {
    if (cx < 0 || cy < 0 || cx >= m_width || cy >= m_height) return 0;
    return m_cells[(size_t)cy * m_width + cx];
}

void TileLayer::set(int cx, int cy, int id) {
    if (cx < 0 || cy < 0 || cx >= m_width || cy >= m_height) return;
    m_cells[(size_t)cy * m_width + cx] = id;
}

bool TileLayer::isSolid(int cx, int cy) const {
    return at(cx, cy) != 0;
}

sf::Vector2i TileLayer::worldToCell(float wx, float wy) const {
    return { (int)std::floor(wx / m_cellSize), (int)std::floor(wy / m_cellSize) };
}

sf::Vector2f TileLayer::cellToWorld(int cx, int cy) const {
    return { cx * m_cellSize, cy * m_cellSize };
}

void TileLayer::draw(sf::RenderTarget& target, sf::Vector2f viewMin, sf::Vector2f viewMax) const {
    if (m_cells.empty()) return;

    int xStart = std::max(0, (int)std::floor(viewMin.x / m_cellSize));
    int yStart = std::max(0, (int)std::floor(viewMin.y / m_cellSize));
    int xEnd = std::min(m_width,  (int)std::ceil(viewMax.x / m_cellSize) + 1);
    int yEnd = std::min(m_height, (int)std::ceil(viewMax.y / m_cellSize) + 1);

    sf::RectangleShape cell(sf::Vector2f(m_cellSize, m_cellSize));
    for (int y = yStart; y < yEnd; ++y) {
        for (int x = xStart; x < xEnd; ++x) {
            int id = m_cells[(size_t)y * m_width + x];
            if (id == 0) continue;
            cell.setPosition(x * m_cellSize, y * m_cellSize);
            // deterministic color per id for phase 1 — swap for atlas lookup later
            sf::Uint8 r = 80 + (id * 53) % 160;
            sf::Uint8 g = 80 + (id * 97) % 160;
            sf::Uint8 b = 80 + (id * 151) % 160;
            cell.setFillColor(sf::Color(r, g, b));
            cell.setOutlineColor(sf::Color(20, 20, 20, 180));
            cell.setOutlineThickness(1.f);
            target.draw(cell);
        }
    }
}

}
