#pragma once
#include "../Engine/Entity/Entity.h"

namespace Game {

// end-of-level flag. nextLevel can chain to another level (editor swaps + re-enters play) so multi-stage demos run as one session. empty = show win overlay
class Goal : public Engine::Entity {
public:
    std::string nextLevel;   // "" = end of run, else chain to that level

    Goal() : Entity("Goal", "goal") {
        color = sf::Color(255, 215, 0);   // gold flag
        size = {32.f, 48.f};              // flag-ish proportions
        isStatic = true;
        isTrigger = true;
    }

    // nextLevel is a string so it goes thru the separate StringProperties map (base Properties is float-only). only emit when set so we dont pollute the json
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
