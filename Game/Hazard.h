#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

// spikes / lava — touch = instadeath, respawn. isTrigger so the resolver doesnt block the player, GameplaySystem picks up the overlap + calls respawnPlayer()
class Hazard : public Engine::Entity {
public:
    Hazard() : Entity("Hazard", "hazard") {
        color = sf::Color(180, 180, 200); // metallic grey
        size = {32.f, 32.f};
        isStatic = true;
        isTrigger = true;
    }
};

}
