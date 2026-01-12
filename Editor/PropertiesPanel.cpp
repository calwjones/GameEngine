#include "PropertiesPanel.h"
#include "../Engine/Entity/EntityTypeRegistry.h"
#include "../Game/Player.h"
#include "../Game/Enemy.h"
#include "../Game/FlyingEnemy.h"
#include "../Game/ShootingEnemy.h"
#include "../Game/Collectible.h"
#include "../Game/MovingPlatform.h"
#include "../Game/Goal.h"
#include <imgui.h>
#include <cstring>

namespace Editor {

// snap on activate, push on deactivate — whole drag becomes one undo step. history=null means play mode, no undo entries
void PropertiesPanel::checkUndo(Engine::Entity* e, CommandHistory* history) {
    if (!history) return;
    if (ImGui::IsItemActivated()) {
        m_editSnapshot = EntityState::capture(e);
        m_editing = true;
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && m_editing) {
        auto newState = EntityState::capture(e);
        if (m_editSnapshot != newState)
            history->push(std::make_unique<PropertyChangeCommand>(e, m_editSnapshot, newState));
        m_editing = false;
    }
}

// renders name/type/transform/physics/appearance/debug sections for the selected entity
void PropertiesPanel::render(Engine::Entity* e, CommandHistory* history) {
    if (!m_visible) return;

    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Properties", &m_visible)) { ImGui::End(); return; }

    if (!e) {
        ImGui::Spacing();
        ImGui::TextDisabled("Select an entity to edit its properties.");
        ImGui::Spacing();
        ImGui::TextDisabled("Click in the viewport or scene panel.");
        m_lastEntity = nullptr;
        ImGui::End();
        return;
    }

    // selection changed mid-drag → bail on edit state or wed push a command w/ the wrong entity
    if (e != m_lastEntity) {
        m_lastEntity = e;
        m_editing = false;
    }

    // name change is undoable, type change also resets physics/size/colour defaults
    ImGui::Spacing();
    char nameBuf[128] = {0};
    snprintf(nameBuf, sizeof(nameBuf), "%s", e->name.c_str());
    ImGui::Text("Name");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##Name", nameBuf, sizeof(nameBuf))) {
        if (strlen(nameBuf) > 0)
            e->name = nameBuf;
    }
    checkUndo(e, history);

    // type dropdown — fed from EntityTypeRegistry so adding a new type is one row in the registry table
    static constexpr int typeCount = (int)Engine::kEntityTypes.size();
    static const char* types[typeCount] = {
        Engine::kEntityTypes[0].type, Engine::kEntityTypes[1].type, Engine::kEntityTypes[2].type,
        Engine::kEntityTypes[3].type, Engine::kEntityTypes[4].type, Engine::kEntityTypes[5].type,
        Engine::kEntityTypes[6].type, Engine::kEntityTypes[7].type, Engine::kEntityTypes[8].type,
        Engine::kEntityTypes[9].type
    };
    int typeIdx = -1;
    for (int i = 0; i < typeCount; i++)
        if (e->type == types[i]) { typeIdx = i; break; }
    ImGui::Text("Type");
    ImGui::SetNextItemWidth(-1);
    if (typeIdx < 0) {
        // unknown type → read-only, dont silently overwrite something we cant round-trip
        ImGui::TextDisabled("%s", e->type.c_str());
    } else if (ImGui::Combo("##Type", &typeIdx, types, typeCount)) {
        std::string newType = types[typeIdx];
        if (newType != e->type) {
            e->type = newType;
            // slap on sensible defaults for the new type — else u end up w/ a wall-shaped green block marked as player
            if (newType == "player") {
                e->size = {32.f, 48.f};
                e->color = sf::Color(50, 205, 50);
                e->isStatic = false;
                e->hasGravity = true;
                e->isTrigger = false;
            } else if (newType == "enemy") {
                e->size = {32.f, 32.f};
                e->color = sf::Color(220, 20, 60);
                e->isStatic = false;
                e->hasGravity = true;
                e->isTrigger = false;
                e->velocity = {80.f, 0.f};
            } else if (newType == "platform") {
                e->color = sf::Color(100, 100, 120);
                e->isStatic = true;
                e->hasGravity = false;
                e->isTrigger = false;
                e->velocity = {0.f, 0.f};
            } else if (newType == "flying_enemy") {
                e->size = {28.f, 28.f};
                e->color = sf::Color(180, 50, 220);
                e->isStatic = false;
                e->hasGravity = false;
                e->isTrigger = false;
                e->velocity = {60.f, 0.f};
            } else if (newType == "shooting_enemy") {
                e->size = {32.f, 32.f};
                e->color = sf::Color(220, 120, 20);
                e->isStatic = false;
                e->hasGravity = true;
                e->isTrigger = false;
            } else if (newType == "collectible") {
                e->size = {16.f, 16.f};
                e->color = sf::Color(255, 215, 0);
                e->isStatic = true;
                e->hasGravity = false;
                e->isTrigger = true;
            } else if (newType == "moving_platform") {
                e->size = {128.f, 16.f};
                e->color = sf::Color(60, 120, 180);
                e->isStatic = true;
                e->hasGravity = false;
                e->isTrigger = false;
            } else if (newType == "goal") {
                e->size = {32.f, 48.f};
                e->color = sf::Color(255, 215, 0);
                e->isStatic = true;
                e->hasGravity = false;
                e->isTrigger = true;
            } else if (newType == "hazard") {
                e->size = {32.f, 32.f};
                e->color = sf::Color(180, 180, 200);
                e->isStatic = true;
                e->hasGravity = false;
                e->isTrigger = true;
            }
        }
    }
    checkUndo(e, history);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Transform
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        float pos[2] = {e->position.x, e->position.y};
        ImGui::Text("Position"); ImGui::SameLine(); ImGui::TextDisabled("(X, Y)");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::DragFloat2("##Pos", pos, 1.0f))
            e->position = {pos[0], pos[1]};
        checkUndo(e, history);

        float sz[2] = {e->size.x, e->size.y};
        ImGui::Text("Size"); ImGui::SameLine(); ImGui::TextDisabled("(W, H)");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::DragFloat2("##Size", sz, 1.0f, 1.0f, 10000.f))
            e->size = {sz[0], sz[1]};
        checkUndo(e, history);
    }

    // Physics
    if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
        float vel[2] = {e->velocity.x, e->velocity.y};
        ImGui::Text("Velocity"); ImGui::SameLine(); ImGui::TextDisabled("(X, Y)");
        if (e->type == "player") {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.9f, 0.65f, 0.2f, 0.8f), "(overridden at runtime)");
        }
        ImGui::SetNextItemWidth(-1);
        if (ImGui::DragFloat2("##Vel", vel, 1.0f))
            e->velocity = {vel[0], vel[1]};
        checkUndo(e, history);

        ImGui::Spacing();

        // imgui checkboxes dont fire IsItemActivated/Deactivated so checkUndo() doesnt work — this lambda handles undo manually via pointer-to-member
        auto undoableCheckbox = [&](const char* label, bool& field, bool EntityState::* member) {
            bool prev = field;
            if (ImGui::Checkbox(label, &field) && history) {
                auto oldState = EntityState::capture(e);
                oldState.*member = prev;
                history->push(std::make_unique<PropertyChangeCommand>(e, oldState, EntityState::capture(e)));
            }
        };

        undoableCheckbox("Static", e->isStatic, &EntityState::isStatic);
        ImGui::SameLine(0, 20);
        undoableCheckbox("Gravity", e->hasGravity, &EntityState::hasGravity);
        undoableCheckbox("Trigger", e->isTrigger, &EntityState::isTrigger);

        if (!e->isStatic) {
            ImGui::SameLine(0, 20);
            ImGui::TextDisabled("On Ground: %s", e->isOnGround ? "Yes" : "No");
        }
    }

    // Colour
    if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
        float col[4] = {e->color.r / 255.f, e->color.g / 255.f, e->color.b / 255.f, e->color.a / 255.f};
        ImGui::Text("Colour");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::ColorEdit4("##Col", col, ImGuiColorEditFlags_NoAlpha)) {
            e->color.r = (sf::Uint8)(col[0] * 255.f);
            e->color.g = (sf::Uint8)(col[1] * 255.f);
            e->color.b = (sf::Uint8)(col[2] * 255.f);
            e->color.a = (sf::Uint8)(col[3] * 255.f);
        }
        checkUndo(e, history);

        ImGui::Spacing();
        ImGui::Text("Sprite Texture");
        char texBuf[128] = {0};
        snprintf(texBuf, sizeof(texBuf), "%s", e->texturePath.c_str());
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 60);
        if (ImGui::InputText("##TexturePath", texBuf, sizeof(texBuf))) {
            e->texturePath = texBuf;
            // renderer picks this up next frame — TextureManager caches by path + the resolved ptr is cached on the entity
        }
        checkUndo(e, history);
        ImGui::SameLine();
        if (ImGui::Button("Clear", ImVec2(50, 0))) {
            e->texturePath = "";
            e->texture = nullptr;
        }
        ImGui::TextDisabled("Path relative to assets/textures/");
    }

    // subclass-specific sections, gated by dynamic_cast. ordered by specificity — ShootingEnemy before Enemy so the right UI shows up even if inheritance changes later
    if (auto* player = dynamic_cast<Game::Player*>(e)) {
        if (ImGui::CollapsingHeader("Player Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            float speed = player->getMoveSpeed();
            ImGui::Text("Move Speed"); ImGui::SameLine(); ImGui::TextDisabled("px/s");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##MoveSpeed", &speed, 1.f, 50.f, 800.f, "%.0f"))
                player->setMoveSpeed(speed);
            checkUndo(e, history);

            float jump = player->getJumpForce();
            ImGui::Text("Jump Force"); ImGui::SameLine(); ImGui::TextDisabled("negative = up");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##JumpForce", &jump, 1.f, -1200.f, -100.f, "%.0f"))
                player->setJumpForce(jump);
            checkUndo(e, history);
        }
    } else if (auto* enemy = dynamic_cast<Game::ShootingEnemy*>(e)) {
        // ShootingEnemy before Enemy — see note above the first dynamic_cast
        if (ImGui::CollapsingHeader("Shooting Enemy Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            float speed = enemy->getPatrolSpeed();
            ImGui::Text("Patrol Speed"); ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##SESpeed", &speed, 1.f, 0.f, 400.f, "%.0f"))
                enemy->setPatrolSpeed(speed);
            checkUndo(e, history);

            float rate = enemy->getFireRate();
            ImGui::Text("Fire Rate"); ImGui::SameLine(); ImGui::TextDisabled("seconds");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##FireRate", &rate, 0.1f, 0.2f, 10.f, "%.1fs"))
                enemy->setFireRate(rate);
            checkUndo(e, history);

            float projSpeed = enemy->getProjectileSpeed();
            ImGui::Text("Projectile Speed"); ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##ProjSpeed", &projSpeed, 1.f, 50.f, 600.f, "%.0f"))
                enemy->setProjectileSpeed(projSpeed);
            checkUndo(e, history);

            ImGui::Spacing();
            ImGui::TextDisabled("Patrol Bounds");
            bool hasBounds = enemy->hasBounds();
            bool oldHasBounds = hasBounds;
            ImGui::Checkbox("Enable Bounds##SE", &hasBounds);
            if (hasBounds && !oldHasBounds) enemy->setPatrolBounds(e->position.x - 100.f, e->position.x + 100.f);
            else if (!hasBounds && oldHasBounds) enemy->clearPatrolBounds();
            if (hasBounds) {
                float mn = enemy->getMinX(), mx = enemy->getMaxX();
                ImGui::SetNextItemWidth(-1);
                if (ImGui::DragFloat2("##SEBounds", &mn, 1.f)) enemy->setPatrolBounds(mn, mx);
                checkUndo(e, history);
            }
        }
    } else if (auto* enemy = dynamic_cast<Game::Enemy*>(e)) {
        if (ImGui::CollapsingHeader("Enemy Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            float speed = enemy->getPatrolSpeed();
            ImGui::Text("Patrol Speed"); ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##ESpeed", &speed, 1.f, 0.f, 400.f, "%.0f"))
                enemy->setPatrolSpeed(speed);
            checkUndo(e, history);

            ImGui::Spacing();
            ImGui::TextDisabled("Patrol Bounds");
            bool hasBounds = enemy->hasBounds();
            bool oldHasBounds = hasBounds;
            ImGui::Checkbox("Enable Bounds##E", &hasBounds);
            if (hasBounds && !oldHasBounds) enemy->setPatrolBounds(e->position.x - 100.f, e->position.x + 100.f);
            else if (!hasBounds && oldHasBounds) enemy->clearPatrolBounds();
            if (hasBounds) {
                float mn = enemy->getMinX(), mx = enemy->getMaxX();
                float bounds[2] = {mn, mx};
                ImGui::SetNextItemWidth(-1);
                if (ImGui::DragFloat2("##EBounds", bounds, 1.f)) enemy->setPatrolBounds(bounds[0], bounds[1]);
                checkUndo(e, history);
            }
        }
    } else if (auto* flyer = dynamic_cast<Game::FlyingEnemy*>(e)) {
        if (ImGui::CollapsingHeader("Flying Enemy Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            float speed = flyer->getPatrolSpeed();
            ImGui::Text("Patrol Speed"); ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##FSpeed", &speed, 1.f, 0.f, 400.f, "%.0f"))
                flyer->setPatrolSpeed(speed);
            checkUndo(e, history);

            float amp = flyer->getAmplitude();
            ImGui::Text("Amplitude"); ImGui::SameLine(); ImGui::TextDisabled("px");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##Amp", &amp, 1.f, 0.f, 300.f, "%.0f"))
                flyer->setAmplitude(amp);
            checkUndo(e, history);

            float freq = flyer->getFrequency();
            ImGui::Text("Frequency"); ImGui::SameLine(); ImGui::TextDisabled("hz");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##Freq", &freq, 0.1f, 0.1f, 10.f, "%.1f"))
                flyer->setFrequency(freq);
            checkUndo(e, history);

            ImGui::Spacing();
            ImGui::TextDisabled("Patrol Bounds");
            bool hasBounds = flyer->hasBounds();
            bool oldHasBounds = hasBounds;
            ImGui::Checkbox("Enable Bounds##F", &hasBounds);
            if (hasBounds && !oldHasBounds) flyer->setPatrolBounds(e->position.x - 100.f, e->position.x + 100.f);
            else if (!hasBounds && oldHasBounds) flyer->clearPatrolBounds();
            if (hasBounds) {
                float mn = flyer->getMinX(), mx = flyer->getMaxX();
                float bounds[2] = {mn, mx};
                ImGui::SetNextItemWidth(-1);
                if (ImGui::DragFloat2("##FBounds", bounds, 1.f)) flyer->setPatrolBounds(bounds[0], bounds[1]);
                checkUndo(e, history);
            }
        }
    } else if (auto* mp = dynamic_cast<Game::MovingPlatform*>(e)) {
        if (ImGui::CollapsingHeader("Moving Platform Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            float speed = mp->getSpeed();
            ImGui::Text("Speed"); ImGui::SameLine(); ImGui::TextDisabled("px/s");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##MPSpeed", &speed, 1.f, 10.f, 500.f, "%.0f"))
                mp->setSpeed(speed);
            checkUndo(e, history);

            float a[2] = {mp->getPointA().x, mp->getPointA().y};
            ImGui::Text("Point A"); ImGui::SameLine(); ImGui::TextDisabled("start");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat2("##MPA", a, 1.f))
                mp->setPointA({a[0], a[1]});
            checkUndo(e, history);

            float b[2] = {mp->getPointB().x, mp->getPointB().y};
            ImGui::Text("Point B"); ImGui::SameLine(); ImGui::TextDisabled("end");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat2("##MPB", b, 1.f))
                mp->setPointB({b[0], b[1]});
            checkUndo(e, history);

            // visual guide
            sf::Vector2f diff = mp->getPointB() - mp->getPointA();
            float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            ImGui::Spacing();
            ImGui::TextDisabled("Distance: %.0fpx", dist);
        }
    } else if (auto* col = dynamic_cast<Game::Collectible*>(e)) {
        if (ImGui::CollapsingHeader("Collectible Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            int pts = col->getPoints();
            ImGui::Text("Points Value");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragInt("##Points", &pts, 1, 1, 1000))
                col->setPoints(pts);
            checkUndo(e, history);
        }
    } else if (auto* goal = dynamic_cast<Game::Goal*>(e)) {
        // nextLevel is a string so it bypasses undo (EntityState only snapshots floats) — same deal as texturePath
        if (ImGui::CollapsingHeader("Goal Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            char nextBuf[128] = {0};
            snprintf(nextBuf, sizeof(nextBuf), "%s", goal->nextLevel.c_str());
            ImGui::Text("Next Level"); ImGui::SameLine(); ImGui::TextDisabled("file in assets/levels/");
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##NextLevel", nextBuf, sizeof(nextBuf)))
                goal->nextLevel = nextBuf;
            if (goal->nextLevel.empty())
                ImGui::TextDisabled("(empty = show win overlay)");
            else
                ImGui::TextDisabled("chains to: %s", goal->nextLevel.c_str());
        }
    }

    // debug section — collapsed by default, shows bounds + centre for sanity-checking placements
    if (ImGui::CollapsingHeader("Debug")) {
        auto b = e->getBounds();
        ImGui::Text("Bounds: %.1f, %.1f  %.1f x %.1f", b.left, b.top, b.width, b.height);
        auto center = e->getCenter();
        ImGui::Text("Centre: %.1f, %.1f", center.x, center.y);
    }

    ImGui::End();
}

}
