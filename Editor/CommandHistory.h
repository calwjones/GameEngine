#pragma once
#include "../Engine/Entity/Entity.h"
#include "../Engine/Entity/EntityManager.h"
#include "../Engine/Tile/TileLayer.h"
#include <vector>
#include <memory>
#include <string>
#include <utility>

namespace Editor {

struct EntityState {
    std::string name, type;
    sf::Vector2f position, size, velocity;
    sf::Color color;
    bool isStatic, hasGravity, isTrigger;
    Engine::Entity::Properties properties;

    static EntityState capture(const Engine::Entity* e);
    void apply(Engine::Entity* e) const;
    bool operator!=(const EntityState& other) const;
};

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string description() const = 0;
};

class CommandHistory {
    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;

public:
    void push(std::unique_ptr<Command> cmd);
    void undo();
    void redo();
    bool canUndo() const { return !m_undoStack.empty(); }
    bool canRedo() const { return !m_redoStack.empty(); }
    size_t size() const { return m_undoStack.size(); }
    std::string undoDescription() const;
    std::string redoDescription() const;
    void clear();
};

class MoveCommand : public Command {
public:
    struct Entry {
        Engine::Entity* entity;
        sf::Vector2f oldPos, newPos;
    };
private:
    std::vector<Entry> m_entries;
public:
    MoveCommand(Engine::Entity* e, sf::Vector2f oldPos, sf::Vector2f newPos);
    MoveCommand(std::vector<Entry> entries);
    void execute() override;
    void undo() override;
    std::string description() const override;
};

// ownership ping-pongs between manager and command based on m_ownsEntity
class AddEntityCommand : public Command {
    Engine::EntityManager& m_mgr;
    Engine::Entity* m_entity;
    std::string m_name;
    bool m_ownsEntity = false;
public:
    AddEntityCommand(Engine::EntityManager& mgr, Engine::Entity* e);
    ~AddEntityCommand() override;
    void execute() override;
    void undo() override;
    std::string description() const override;
    Engine::Entity* getEntity() const { return m_entity; }
};

class DeleteEntityCommand : public Command {
    Engine::EntityManager& m_mgr;
    Engine::Entity* m_entity;
    std::string m_name;
    bool m_ownsEntity = false;
public:
    DeleteEntityCommand(Engine::EntityManager& mgr, Engine::Entity* e);
    ~DeleteEntityCommand() override;
    void execute() override;
    void undo() override;
    std::string description() const override;
    Engine::Entity* getEntity() const { return m_entity; }
};

class PropertyChangeCommand : public Command {
    Engine::Entity* m_entity;
    EntityState m_oldState, m_newState;
public:
    PropertyChangeCommand(Engine::Entity* e, EntityState oldState, EntityState newState);
    void execute() override;
    void undo() override;
    std::string description() const override;
};

class PaintTilesCommand : public Command {
public:
    struct Edit { int cx, cy, oldId, newId; };
private:
    Engine::TileLayer* m_tiles;
    std::vector<Edit> m_edits;
public:
    PaintTilesCommand(Engine::TileLayer* tiles, std::vector<Edit> edits)
        : m_tiles(tiles), m_edits(std::move(edits)) {}
    void execute() override {
        for (const auto& e : m_edits) m_tiles->set(e.cx, e.cy, e.newId);
    }
    void undo() override {
        for (const auto& e : m_edits) m_tiles->set(e.cx, e.cy, e.oldId);
    }
    std::string description() const override {
        return "Paint tiles (" + std::to_string(m_edits.size()) + ")";
    }
};

class CompoundCommand : public Command {
    std::vector<std::unique_ptr<Command>> m_commands;
    std::string m_desc;
public:
    CompoundCommand(std::vector<std::unique_ptr<Command>> cmds, std::string desc)
        : m_commands(std::move(cmds)), m_desc(std::move(desc)) {}
    void execute() override { for (auto& c : m_commands) c->execute(); }
    void undo() override { for (int i = (int)m_commands.size() - 1; i >= 0; i--) m_commands[i]->undo(); }
    std::string description() const override { return m_desc; }
};

}
