#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <string>
#include <unordered_map>

namespace sf { class Texture; }

namespace Engine {

class Renderer;

class Entity {
public:
    std::string name = "Entity";
    std::string type = "default";
    sf::Vector2f position{0.f, 0.f};
    sf::Vector2f size{32.f, 32.f};
    sf::Vector2f velocity{0.f, 0.f};
    sf::Color color = sf::Color::White;
    bool isStatic = false;
    bool hasGravity = true;
    bool isOnGround = false;
    bool isTrigger = false;

    std::string texturePath;
    sf::Texture* texture = nullptr;    // borrowed, manager-owned
    std::string resolvedTexturePath;

    Entity() = default;
    Entity(const std::string& n, const std::string& t) : name(n), type(t) {}
    virtual ~Entity() = default;

    virtual void update(float dt) { if (!isStatic) position += velocity * dt; }
    virtual void render(Renderer& renderer);

    using Properties = std::unordered_map<std::string, float>;
    using StringProperties = std::unordered_map<std::string, std::string>;
    virtual Properties serializeProperties() const { return {}; }
    virtual void deserializeProperties(const Properties& props) { (void)props; }
    virtual StringProperties serializeStringProperties() const { return {}; }
    virtual void deserializeStringProperties(const StringProperties& props) { (void)props; }

    sf::FloatRect getBounds() const { return {position.x, position.y, size.x, size.y}; }
    sf::Vector2f getCenter() const { return position + size * 0.5f; }
    void setCenter(const sf::Vector2f& c) { position = c - size * 0.5f; }
};

}
