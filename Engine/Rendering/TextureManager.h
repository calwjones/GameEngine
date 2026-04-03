#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <map>

namespace Engine {

// std::map for pointer stability across inserts
class TextureManager {
    std::map<std::string, sf::Texture> m_textures;
    std::string m_basePath = "assets/textures/";

public:
    sf::Texture* getTexture(const std::string& path, sf::Color maskColor = sf::Color::White);
    void clear() { m_textures.clear(); }
};

}
