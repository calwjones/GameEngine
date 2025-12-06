#pragma once
#include "../Engine/Entity/Entity.h"
#include "../Engine/Input/InputManager.h"

namespace Game {

// player entity — overrides update() to poll input. input ptr is null in edit mode so clicks on the viewport dont trigger jumps (GameplaySystem wires it on play)
class Player : public Engine::Entity {
    float m_speed = 250.f;
    float m_jump = -450.f;    // NEGATIVE — sfml y-axis points down, so "up" is negative. trips ppl up
    Engine::InputManager* m_input = nullptr;   // null in edit mode

public:
    Player() : Entity("Player", "player") {
        color = sf::Color(50, 205, 50);    // lime green
        size = {32.f, 48.f};
        hasGravity = true;
    }

    void setInputManager(Engine::InputManager* i) { m_input = i; }
    void setMoveSpeed(float s) { m_speed = s; }
    void setJumpForce(float f) { m_jump = f; }
    float getMoveSpeed() const { return m_speed; }
    float getJumpForce() const { return m_jump; }

    // moveSpeed + jumpForce round-trip thru level json via the "properties" sidecar object
    Properties serializeProperties() const override {
        return {{"moveSpeed", m_speed}, {"jumpForce", m_jump}};
    }
    // guarded find → missing field just keeps the default
    void deserializeProperties(const Properties& p) override {
        auto it = p.find("moveSpeed"); if (it != p.end()) m_speed = it->second;
        it = p.find("jumpForce"); if (it != p.end()) m_jump = it->second;
    }

    void update(float dt) override {
        if (m_input) {
            velocity.x = 0.f;   // reset each tick → no inertia, instant stop on key release (mario-ish)
            if (m_input->isKeyPressed(sf::Keyboard::Left) || m_input->isKeyPressed(sf::Keyboard::A))
                velocity.x = -m_speed;
            if (m_input->isKeyPressed(sf::Keyboard::Right) || m_input->isKeyPressed(sf::Keyboard::D))
                velocity.x = m_speed;
            // isOnGround set by collision system when something pushes us up
            if (isOnGround && (m_input->isKeyPressed(sf::Keyboard::Space) ||
                               m_input->isKeyPressed(sf::Keyboard::W) ||
                               m_input->isKeyPressed(sf::Keyboard::Up))) {
                velocity.y = m_jump;
                isOnGround = false;   // clear so we cant double-jump in one frame
            }
        }
        if (velocity.y > 0) isOnGround = false;   // falling = airborne, clear stickiness on fall edge
        position += velocity * dt;   // euler integration, fine at 60hz fixed step
    }
};

}
