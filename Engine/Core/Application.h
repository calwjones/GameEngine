#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include "../Entity/EntityManager.h"
#include "../Physics/PhysicsEngine.h"
#include "../Collision/CollisionSystem.h"
#include "../Input/InputManager.h"

namespace Engine {

// bag of subsystems — entities, physics, collision, input. used to have its own run()/update() loop for standalone mode but killed it, editor drives everything now
class Application {
    EntityManager m_entities;
    PhysicsEngine m_physics;
    CollisionSystem m_collision;
    InputManager m_input;

    sf::RenderWindow* m_window = nullptr;   // borrowed, editor owns the window
    unsigned int m_width = 800, m_height = 600;
    bool m_initialized = false;

public:
    ~Application() { shutdown(); }

    bool initialize(sf::RenderWindow* window);
    void render(sf::RenderTarget& target);
    void shutdown();

    EntityManager& getEntityManager() { return m_entities; }
    PhysicsEngine& getPhysics() { return m_physics; }
    CollisionSystem& getCollision() { return m_collision; }
    InputManager& getInput() { return m_input; }

    sf::RenderWindow* getWindow() { return m_window; }
    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }
};

}
