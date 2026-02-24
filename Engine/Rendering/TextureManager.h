#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <map>
#include <iostream>

namespace Engine {

// loads textures once + caches em by path. also does chroma keying (the
// "white = transparent" trick — paint ur sprite bg white in aseprite + it
// vanishes at load time). used instead of actual PNG alpha bc most of the
// placeholder art was BMP lol
//
// WHY std::map not unordered_map: map gives pointer stability across inserts,
// so the raw sf::Texture* i return stays valid even after more textures load.
// unordered_map rehashes on grow = dangling ptrs = hours of debugging
class TextureManager {
    std::map<std::string, sf::Texture> m_textures;
    std::string m_basePath = "assets/textures/";

public:
    // takes a path relative to assets/textures/. returns cached ptr if seen before,
    // otherwise loads from disk, applies the chroma key, caches + returns
    sf::Texture* getTexture(const std::string& path, sf::Color maskColor = sf::Color::White) {
        if (path.empty()) return nullptr;

        auto it = m_textures.find(path);
        if (it != m_textures.end()) return &it->second;

        sf::Image img;
        if (!img.loadFromFile(m_basePath + path)) {
            std::cerr << "TextureManager: Failed to load " << path << std::endl;
            return nullptr;
        }

        img.createMaskFromColor(maskColor);

        sf::Texture tex;
        if (!tex.loadFromImage(img)) {
            std::cerr << "TextureManager: Failed to convert image to texture: " << path << std::endl;
            return nullptr;
        }

        auto inserted = m_textures.emplace(path, std::move(tex));
        return &inserted.first->second;
    }

    void clear() { m_textures.clear(); }
};

}
