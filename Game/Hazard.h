#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

class Hazard : public Engine::Entity {
public:
    Hazard() : Entity("Hazard", "hazard") {
        color = sf::Color(180, 180, 200);
        size = {32.f, 32.f};
        isStatic = true;
        isTrigger = true;
    }
};

}
