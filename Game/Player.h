#pragma once
#include "../Engine/Entity/Entity.h"
#include "../Engine/Input/InputManager.h"

namespace Game {

class Player : public Engine::Entity {
    float m_speed = 250.f;
    float m_jump = -450.f;   // negative y = up in SFML
    Engine::InputManager* m_input = nullptr;

public:
    Player() : Entity("Player", "player") {
        color = sf::Color(50, 205, 50);
        size = {32.f, 48.f};
        hasGravity = true;
    }

    void setInputManager(Engine::InputManager* i) { m_input = i; }
    void setMoveSpeed(float s) { m_speed = s; }
    void setJumpForce(float f) { m_jump = f; }
    float getMoveSpeed() const { return m_speed; }
    float getJumpForce() const { return m_jump; }

    Properties serializeProperties() const override {
        return {{"moveSpeed", m_speed}, {"jumpForce", m_jump}};
    }
    void deserializeProperties(const Properties& p) override {
        auto it = p.find("moveSpeed"); if (it != p.end()) m_speed = it->second;
        it = p.find("jumpForce"); if (it != p.end()) m_jump = it->second;
    }

    void update(float dt) override {
        if (m_input) {
            velocity.x = 0.f;
            if (m_input->isKeyPressed(sf::Keyboard::Left) || m_input->isKeyPressed(sf::Keyboard::A))
                velocity.x = -m_speed;
            if (m_input->isKeyPressed(sf::Keyboard::Right) || m_input->isKeyPressed(sf::Keyboard::D))
                velocity.x = m_speed;
            if (isOnGround && (m_input->isKeyPressed(sf::Keyboard::Space) ||
                               m_input->isKeyPressed(sf::Keyboard::W) ||
                               m_input->isKeyPressed(sf::Keyboard::Up))) {
                velocity.y = m_jump;
                isOnGround = false;
            }
        }
        if (velocity.y > 0) isOnGround = false;
        position += velocity * dt;
    }
};

}
