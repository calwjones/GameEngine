#include "CommandHistory.h"

namespace Editor {

static constexpr size_t MAX_UNDO_HISTORY = 100;

EntityState EntityState::capture(const Engine::Entity* e) {
    return {e->name, e->type, e->position, e->size, e->velocity,
            e->color, e->isStatic, e->hasGravity, e->isTrigger,
            e->serializeProperties()};
}

void EntityState::apply(Engine::Entity* e) const {
    e->name = name; e->type = type;
    e->position = position; e->size = size; e->velocity = velocity;
    e->color = color;
    e->isStatic = isStatic; e->hasGravity = hasGravity; e->isTrigger = isTrigger;
    e->deserializeProperties(properties);
}

bool EntityState::operator!=(const EntityState& other) const {
    if (name != other.name || type != other.type ||
        position.x != other.position.x || position.y != other.position.y ||
        size.x != other.size.x || size.y != other.size.y ||
        velocity.x != other.velocity.x || velocity.y != other.velocity.y ||
        color.r != other.color.r || color.g != other.color.g ||
        color.b != other.color.b || color.a != other.color.a ||
        isStatic != other.isStatic || hasGravity != other.hasGravity ||
        isTrigger != other.isTrigger)
        return true;
    if (properties.size() != other.properties.size()) return true;
    for (auto& [k, v] : properties) {
        auto it = other.properties.find(k);
        if (it == other.properties.end() || it->second != v) return true;
    }
    return false;
}

void CommandHistory::push(std::unique_ptr<Command> cmd) {
    m_redoStack.clear();
    m_undoStack.push_back(std::move(cmd));
    if (m_undoStack.size() > MAX_UNDO_HISTORY)
        m_undoStack.erase(m_undoStack.begin());
}

void CommandHistory::undo() {
    if (m_undoStack.empty()) return;
    auto cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    cmd->undo();
    m_redoStack.push_back(std::move(cmd));
}

void CommandHistory::redo() {
    if (m_redoStack.empty()) return;
    auto cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();
    cmd->execute();
    m_undoStack.push_back(std::move(cmd));
}

std::string CommandHistory::undoDescription() const {
    if (m_undoStack.empty()) return "";
    return m_undoStack.back()->description();
}

std::string CommandHistory::redoDescription() const {
    if (m_redoStack.empty()) return "";
    return m_redoStack.back()->description();
}

void CommandHistory::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
}

MoveCommand::MoveCommand(Engine::Entity* e, sf::Vector2f oldPos, sf::Vector2f newPos) {
    m_entries.push_back({e, oldPos, newPos});
}

MoveCommand::MoveCommand(std::vector<Entry> entries) : m_entries(std::move(entries)) {}

void MoveCommand::execute() {
    for (auto& entry : m_entries)
        entry.entity->position = entry.newPos;
}

void MoveCommand::undo() {
    for (auto& entry : m_entries)
        entry.entity->position = entry.oldPos;
}

std::string MoveCommand::description() const {
    if (m_entries.size() == 1) return "Move " + m_entries[0].entity->name;
    return "Move " + std::to_string(m_entries.size()) + " entities";
}

AddEntityCommand::AddEntityCommand(Engine::EntityManager& mgr, Engine::Entity* e)
    : m_mgr(mgr), m_entity(e), m_name(e->name) {}

AddEntityCommand::~AddEntityCommand() {
    if (m_ownsEntity && m_entity) delete m_entity;
}

void AddEntityCommand::execute() {
    if (m_ownsEntity) {
        m_mgr.addEntity(m_entity);
        m_ownsEntity = false;
    }
}

void AddEntityCommand::undo() {
    m_mgr.detachEntity(m_entity);
    m_ownsEntity = true;
}

std::string AddEntityCommand::description() const {
    return "Add " + m_name;
}

DeleteEntityCommand::DeleteEntityCommand(Engine::EntityManager& mgr, Engine::Entity* e)
    : m_mgr(mgr), m_entity(e), m_name(e->name) {}

DeleteEntityCommand::~DeleteEntityCommand() {
    if (m_ownsEntity && m_entity) delete m_entity;
}

void DeleteEntityCommand::execute() {
    m_mgr.detachEntity(m_entity);
    m_ownsEntity = true;
}

void DeleteEntityCommand::undo() {
    if (m_ownsEntity) {
        m_mgr.addEntity(m_entity);
        m_ownsEntity = false;
    }
}

std::string DeleteEntityCommand::description() const {
    return "Delete " + m_name;
}

PropertyChangeCommand::PropertyChangeCommand(Engine::Entity* e, EntityState oldState, EntityState newState)
    : m_entity(e), m_oldState(std::move(oldState)), m_newState(std::move(newState)) {}

void PropertyChangeCommand::execute() {
    m_newState.apply(m_entity);
}

void PropertyChangeCommand::undo() {
    m_oldState.apply(m_entity);
}

std::string PropertyChangeCommand::description() const {
    return "Edit " + m_entity->name;
}

}
