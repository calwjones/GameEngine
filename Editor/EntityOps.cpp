#include "EntityOps.h"
#include "EditorContext.h"
#include "SelectionController.h"
#include "LevelIO.h"
#include "CommandHistory.h"
#include "GameViewport.h"
#include "../Engine/Core/Application.h"
#include "../Engine/Entity/Entity.h"
#include "../Engine/Entity/EntityManager.h"
#include "../Engine/Entity/EntityFactory.h"
#include "../Engine/Level/LevelLoader.h"
#include "../Game/MovingPlatform.h"
#include <filesystem>
#include <memory>
#include <string>

namespace Editor {

void EntityOps::addEntity() {
    int id = ctx.levelIO ? ctx.levelIO->nextEntityId() : 0;
    auto* e = new Engine::Entity("Platform " + std::to_string(id), "platform");
    sf::Vector2f viewOff = ctx.viewport.getViewOffset();
    e->position = {viewOff.x + ctx.viewport.viewW() / 2.f - 32.f,
                   viewOff.y + ctx.viewport.viewH() / 2.f - 8.f};
    e->size = {64.f, 16.f};
    e->color = sf::Color(100, 100, 120);
    e->isStatic = true;
    e->hasGravity = false;
    ctx.game.getEntityManager().addEntity(e);
    if (ctx.selection) ctx.selection->select(e);
    ctx.viewport.isVisible() = true;

    ctx.history.push(std::make_unique<AddEntityCommand>(ctx.game.getEntityManager(), e));
    ctx.markDirty();
    ctx.setStatus("Entity created");
}

void EntityOps::addFromTemplate(EntityTemplate tmpl) {
    int id = ctx.levelIO ? ctx.levelIO->nextEntityId() : 0;
    Engine::Entity* e = nullptr;

    switch (tmpl) {
    case EntityTemplate::Player:
        e = ctx.factory.create("player");
        e->name = "Player";
        break;
    case EntityTemplate::Enemy:
        e = ctx.factory.create("enemy");
        e->name = "Enemy " + std::to_string(id);
        e->velocity = {80.f, 0.f};
        break;
    case EntityTemplate::Platform:
        e = ctx.factory.create("platform");
        e->name = "Platform " + std::to_string(id);
        e->size = {64.f, 16.f};
        e->color = sf::Color(100, 100, 120);
        e->isStatic = true;
        e->hasGravity = false;
        break;
    case EntityTemplate::LargePlatform:
        e = ctx.factory.create("platform");
        e->name = "Large Platform " + std::to_string(id);
        e->size = {192.f, 16.f};
        e->color = sf::Color(100, 100, 120);
        e->isStatic = true;
        e->hasGravity = false;
        break;
    case EntityTemplate::Ground:
        e = ctx.factory.create("platform");
        e->name = "Ground";
        e->size = {800.f, 32.f};
        e->color = sf::Color(60, 60, 80);
        e->isStatic = true;
        e->hasGravity = false;
        e->position = {0.f, 568.f};
        break;
    case EntityTemplate::Wall:
        e = ctx.factory.create("platform");
        e->name = "Wall " + std::to_string(id);
        e->size = {16.f, 96.f};
        e->color = sf::Color(80, 80, 100);
        e->isStatic = true;
        e->hasGravity = false;
        break;
    case EntityTemplate::FlyingEnemy:
        e = ctx.factory.create("flying_enemy");
        e->name = "Flying Enemy " + std::to_string(id);
        e->velocity = {60.f, 0.f};
        break;
    case EntityTemplate::ShootingEnemy:
        e = ctx.factory.create("shooting_enemy");
        e->name = "Shooting Enemy " + std::to_string(id);
        break;
    case EntityTemplate::Collectible:
        e = ctx.factory.create("collectible");
        e->name = "Collectible " + std::to_string(id);
        break;
    case EntityTemplate::MovingPlatform:
        e = ctx.factory.create("moving_platform");
        e->name = "Moving Platform " + std::to_string(id);
        break;
    case EntityTemplate::Goal:
        e = ctx.factory.create("goal");
        e->name = "Goal " + std::to_string(id);
        break;
    case EntityTemplate::Hazard:
        e = ctx.factory.create("hazard");
        e->name = "Hazard " + std::to_string(id);
        break;
    default:
        return;
    }

    if (tmpl != EntityTemplate::Ground) {
        sf::Vector2f viewOff = ctx.viewport.getViewOffset();
        float vw = ctx.viewport.viewW(), vh = ctx.viewport.viewH();
        e->position = {viewOff.x + vw / 2.f - e->size.x / 2.f,
                       viewOff.y + vh / 2.f - e->size.y / 2.f};
    }

    if (auto* mp = dynamic_cast<Game::MovingPlatform*>(e)) {
        mp->setPointA(e->position);
        mp->setPointB({e->position.x + 200.f, e->position.y});
    }

    ctx.game.getEntityManager().addEntity(e);
    if (ctx.selection) {
        ctx.selection->select(e);
        ctx.selection->findPlayer();
    }
    ctx.viewport.isVisible() = true;

    ctx.history.push(std::make_unique<AddEntityCommand>(ctx.game.getEntityManager(), e));
    ctx.markDirty();
    ctx.setStatus(e->name + " created");
}

void EntityOps::duplicateEntity() {
    auto* src = ctx.selection ? ctx.selection->current() : nullptr;
    if (!src) return;

    auto* e = ctx.factory.create(src->type);
    std::string baseName = src->name;
    while (baseName.size() > 5 && baseName.substr(baseName.size() - 5) == " Copy")
        baseName = baseName.substr(0, baseName.size() - 5);
    e->name = baseName + " Copy";
    e->type = src->type;
    e->position = src->position + sf::Vector2f(20.f, 20.f);
    e->size = src->size;
    e->velocity = src->velocity;
    e->color = src->color;
    e->isStatic = src->isStatic;
    e->hasGravity = src->hasGravity;
    e->isTrigger = src->isTrigger;
    e->texturePath = src->texturePath;
    e->texture = src->texture;
    e->deserializeProperties(src->serializeProperties());
    ctx.game.getEntityManager().addEntity(e);
    ctx.selection->select(e);

    ctx.history.push(std::make_unique<AddEntityCommand>(ctx.game.getEntityManager(), e));
    ctx.markDirty();
    ctx.setStatus("Entity duplicated");
}

bool EntityOps::saveSelectionAsPrefab(const std::string& name) {
    auto* src = ctx.selection ? ctx.selection->current() : nullptr;
    if (!src) {
        ctx.setStatus("No selection to save as prefab");
        return false;
    }
    if (name.empty()) {
        ctx.setStatus("Prefab name required");
        return false;
    }

    std::filesystem::create_directories("assets/prefabs");
    std::string filename = name;
    if (filename.size() < 5 || filename.substr(filename.size() - 5) != ".json")
        filename += ".json";
    std::string path = "assets/prefabs/" + filename;

    std::vector<Engine::Entity*> one{src};
    if (!ctx.loader.saveToJSON(path, one, 0.f, 0.f, nullptr)) {
        ctx.setStatus("Prefab save failed");
        return false;
    }
    ctx.setStatus("Saved prefab " + filename);
    return true;
}

bool EntityOps::instantiatePrefab(const std::string& path) {
    auto loaded = ctx.loader.loadFromJSON(path);
    if (loaded.empty()) {
        ctx.setStatus("Prefab load failed");
        return false;
    }
    Engine::Entity* e = loaded.front();
    for (size_t i = 1; i < loaded.size(); ++i) delete loaded[i];

    sf::Vector2f viewOff = ctx.viewport.getViewOffset();
    float vw = ctx.viewport.viewW(), vh = ctx.viewport.viewH();
    e->position = {viewOff.x + vw / 2.f - e->size.x / 2.f,
                   viewOff.y + vh / 2.f - e->size.y / 2.f};

    if (auto* mp = dynamic_cast<Game::MovingPlatform*>(e)) {
        mp->setPointA(e->position);
        mp->setPointB({e->position.x + 200.f, e->position.y});
    }

    int id = ctx.levelIO ? ctx.levelIO->nextEntityId() : 0;
    if (!e->name.empty()) e->name += " " + std::to_string(id);

    ctx.game.getEntityManager().addEntity(e);
    if (ctx.selection) {
        ctx.selection->select(e);
        ctx.selection->findPlayer();
    }
    ctx.viewport.isVisible() = true;

    ctx.history.push(std::make_unique<AddEntityCommand>(ctx.game.getEntityManager(), e));
    ctx.markDirty();
    ctx.setStatus("Instantiated " + e->name);
    return true;
}

void EntityOps::deleteSelected() {
    if (!ctx.selection || ctx.selection->all().empty()) return;

    auto toDelete = ctx.selection->all();

    if (toDelete.size() == 1) {
        auto* e = toDelete[0];
        std::string name = e->name;
        if (e == ctx.selection->player()) ctx.selection->setPlayer(nullptr);

        auto cmd = std::make_unique<DeleteEntityCommand>(ctx.game.getEntityManager(), e);
        cmd->execute();
        ctx.history.push(std::move(cmd));

        ctx.selection->clear();
        ctx.viewport.cancelDrag();
        ctx.markDirty();
        ctx.setStatus("Deleted " + name);
    } else {
        std::vector<std::unique_ptr<Command>> cmds;
        for (auto* e : toDelete) {
            if (e == ctx.selection->player()) ctx.selection->setPlayer(nullptr);
            auto cmd = std::make_unique<DeleteEntityCommand>(ctx.game.getEntityManager(), e);
            cmd->execute();
            cmds.push_back(std::move(cmd));
        }
        std::string desc = "Delete " + std::to_string(toDelete.size()) + " entities";
        ctx.history.push(std::make_unique<CompoundCommand>(std::move(cmds), desc));

        ctx.selection->clear();
        ctx.viewport.cancelDrag();
        ctx.markDirty();
        ctx.setStatus("Deleted " + std::to_string(toDelete.size()) + " entities");
    }
}

}
