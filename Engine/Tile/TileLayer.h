#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

namespace sf { class RenderTarget; }

namespace Engine {

// dense row-major tile grid. id 0 = empty, nonzero = solid.
// phase 1 renders nonzero ids as flat colored cells; a palette-aware draw will
// come when tile textures land.
class TileLayer {
    int m_width = 0;
    int m_height = 0;
    float m_cellSize = 32.f;
    std::vector<int> m_cells;

public:
    TileLayer() = default;
    TileLayer(int w, int h, float cellSize);

    void resize(int w, int h, float cellSize);
    void clear();

    int width() const { return m_width; }
    int height() const { return m_height; }
    float cellSize() const { return m_cellSize; }

    int at(int cx, int cy) const;
    void set(int cx, int cy, int id);

    // world-space helpers
    sf::Vector2i worldToCell(float wx, float wy) const;
    sf::Vector2f cellToWorld(int cx, int cy) const;

    // Render nonzero cells in [viewMin, viewMax] world-space rect.
    void draw(sf::RenderTarget& target, sf::Vector2f viewMin, sf::Vector2f viewMax) const;

    // for serialization + iteration
    const std::vector<int>& cells() const { return m_cells; }
    std::vector<int>& cells() { return m_cells; }

    bool isSolid(int cx, int cy) const;
};

}
