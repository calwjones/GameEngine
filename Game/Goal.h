#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

class Goal : public Engine::Entity {
public:
    std::string nextLevel;

    Goal() : Entity("Goal", "goal") {
        color = sf::Color(255, 215, 0);
        size = {32.f, 48.f};
        isStatic = true;
        isTrigger = true;
    }

    StringProperties serializeStringProperties() const override {
        StringProperties p;
        if (!nextLevel.empty()) p["nextLevel"] = nextLevel;
        return p;
    }
    void deserializeStringProperties(const StringProperties& p) override {
        auto it = p.find("nextLevel");
        if (it != p.end()) nextLevel = it->second;
    }
};

}
