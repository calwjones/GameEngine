#include <catch2/catch_test_macros.hpp>
#include "Editor/CommandHistory.h"
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityManager.h"

using Editor::CommandHistory;
using Editor::MoveCommand;
using Editor::AddEntityCommand;
using Editor::DeleteEntityCommand;
using Editor::CompoundCommand;
using Engine::Entity;
using Engine::EntityManager;

TEST_CASE("CommandHistory undo/redo", "[commands]") {
    CommandHistory h;
    REQUIRE_FALSE(h.canUndo());
    REQUIRE_FALSE(h.canRedo());

    // MoveCommand records an already-applied move; push does not execute
    Entity e;
    e.position = {10.f, 5.f};

    h.push(std::make_unique<MoveCommand>(&e, sf::Vector2f{0, 0}, sf::Vector2f{10, 5}));
    REQUIRE(h.canUndo());
    REQUIRE(h.size() == 1);

    h.undo();
    REQUIRE(e.position == sf::Vector2f{0, 0});
    REQUIRE_FALSE(h.canUndo());
    REQUIRE(h.canRedo());

    h.redo();
    REQUIRE(e.position == sf::Vector2f{10, 5});
    REQUIRE(h.canUndo());
    REQUIRE_FALSE(h.canRedo());
}

TEST_CASE("pushing clears the redo stack", "[commands]") {
    CommandHistory h;
    Entity e;
    e.position = {2.f, 0.f};

    h.push(std::make_unique<MoveCommand>(&e, sf::Vector2f{0, 0}, sf::Vector2f{1, 0}));
    h.push(std::make_unique<MoveCommand>(&e, sf::Vector2f{1, 0}, sf::Vector2f{2, 0}));
    h.undo();
    REQUIRE(h.canRedo());

    h.push(std::make_unique<MoveCommand>(&e, sf::Vector2f{1, 0}, sf::Vector2f{5, 5}));
    REQUIRE_FALSE(h.canRedo());
}

TEST_CASE("AddEntityCommand flips ownership on undo", "[commands]") {
    EntityManager mgr;
    // caller has already added the entity; command records that for undo
    auto* e = new Entity("a", "block");
    mgr.addEntity(e);
    auto cmd = std::make_unique<AddEntityCommand>(mgr, e);
    REQUIRE(mgr.getEntityCount() == 1);

    cmd->undo();
    REQUIRE(mgr.getEntityCount() == 0);

    cmd->execute();
    REQUIRE(mgr.getEntityCount() == 1);
}

TEST_CASE("DeleteEntityCommand restores entity", "[commands]") {
    EntityManager mgr;
    auto* e = new Entity("a", "block");
    mgr.addEntity(e);
    REQUIRE(mgr.getEntityCount() == 1);

    auto cmd = std::make_unique<DeleteEntityCommand>(mgr, e);
    cmd->execute();
    REQUIRE(mgr.getEntityCount() == 0);

    cmd->undo();
    REQUIRE(mgr.getEntityCount() == 1);
}

TEST_CASE("CompoundCommand undoes in reverse order", "[commands]") {
    Entity e;
    e.position = {0.f, 0.f};

    std::vector<std::unique_ptr<Editor::Command>> parts;
    parts.push_back(std::make_unique<MoveCommand>(&e, sf::Vector2f{0, 0}, sf::Vector2f{5, 0}));
    parts.push_back(std::make_unique<MoveCommand>(&e, sf::Vector2f{5, 0}, sf::Vector2f{5, 10}));

    CompoundCommand compound(std::move(parts), "move");
    compound.execute();
    REQUIRE(e.position == sf::Vector2f{5, 10});
    compound.undo();
    REQUIRE(e.position == sf::Vector2f{0, 0});
}
