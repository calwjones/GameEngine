#pragma once
// NOTE: including the 3 SFML sub-headers we actually need instead of
// <SFML/Graphics.hpp>. huge compile time win bc Entity.h is in everything.
// Graphics.hpp pulls in like 40 files transitively, these 3 pull in maybe 5
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <string>
#include <unordered_map>

// forward decl: we only store a ptr to sf::Texture here, dont need the full class
namespace sf { class Texture; }

namespace Engine {

class Renderer;

// THE base class. everything in the game IS an entity — player, enemies,
// platforms, coins, spikes. inheritance tree is 1 level deep (Game::Player,
// Game::Enemy, etc all extend this directly).
//
// the 2 things subclasses override:
//   - update(dt)                  -> per-frame behaviour (walk, shoot, sine-wave)
//   - serialize/deserializeProps  -> so save/load can roundtrip subclass-specific fields
class Entity {
public:
    std::string name = "Entity";
    std::string type = "default";   // "player", "enemy", "coin", etc — factory + collision key off this
    sf::Vector2f position{0.f, 0.f};
    sf::Vector2f size{32.f, 32.f};
    sf::Vector2f velocity{0.f, 0.f};
    sf::Color color = sf::Color::White;
    bool isStatic = false;      // true = physics/collision leaves it alone (ground, walls)
    bool hasGravity = true;     // false = platforms, flying enemies
    bool isOnGround = false;    // collision sys sets this when we're standing on something. player reads it for jump
    bool isTrigger = false;     // overlap but dont block. coins + goal + hazards all use this

    // texture support. if texturePath is set + texture* resolved we draw the sprite,
    // else fall back to a flat coloured rect. TextureManager owns the actual sf::Texture
    std::string texturePath;
    sf::Texture* texture = nullptr;    // borrowed ptr — DONT delete, manager owns it
    std::string resolvedTexturePath;   // cache: last path we resolved. lets us skip the map lookup next frame

    Entity() = default;
    Entity(const std::string& n, const std::string& t) : name(n), type(t) {}
    virtual ~Entity() = default;

    // default update = euler integration. subclasses call this from their own update() typically
    virtual void update(float dt) { if (!isStatic) position += velocity * dt; }
    virtual void render(Renderer& renderer);

    // the serialization contract. two separate maps (float + string) bc i didnt
    // wanna deal with std::variant in json land. when saving, subclasses fill
    // both maps from their fields; when loading, they read whichever key exists.
    // each subclass impls only what it needs — base returns empty = no extra fields
    using Properties = std::unordered_map<std::string, float>;
    using StringProperties = std::unordered_map<std::string, std::string>;
    virtual Properties serializeProperties() const { return {}; }
    virtual void deserializeProperties(const Properties& props) { (void)props; }
    virtual StringProperties serializeStringProperties() const { return {}; }
    virtual void deserializeStringProperties(const StringProperties& props) { (void)props; }

    // AABB helpers — used all over the collision system
    sf::FloatRect getBounds() const { return {position.x, position.y, size.x, size.y}; }
    sf::Vector2f getCenter() const { return position + size * 0.5f; }
    void setCenter(const sf::Vector2f& c) { position = c - size * 0.5f; }
};

}
