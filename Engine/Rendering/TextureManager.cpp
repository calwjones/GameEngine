#include "TextureManager.h"
#include <SFML/Graphics/Image.hpp>
#include <iostream>

namespace Engine {

sf::Texture* TextureManager::getTexture(const std::string& path, sf::Color maskColor) {
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

}
