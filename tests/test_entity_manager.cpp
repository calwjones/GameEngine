#include <catch2/catch_test_macros.hpp>
#include "Engine/Entity/Entity.h"
#include "Engine/Entity/EntityManager.h"

using Engine::Entity;
using Engine::EntityManager;

namespace {
struct Zombie : public Entity {
    int* counter;
    explicit Zombie(int* c) : counter(c) { type = "zombie"; }
    ~Zombie() override { if (counter) (*counter)++; }
};
}

TEST_CASE("spawn<T> forwards args and inserts into manager", "[entity]") {
    EntityManager mgr;
    int destroyed = 0;
    auto* z = mgr.spawn<Zombie>(&destroyed);
    REQUIRE(z != nullptr);
    REQUIRE(mgr.getEntityCount() == 1);
    REQUIRE(mgr.getEntitiesByType("zombie").size() == 1);
}

TEST_CASE("clear deletes owned entities", "[entity]") {
    EntityManager mgr;
    int destroyed = 0;
    mgr.spawn<Zombie>(&destroyed);
    mgr.spawn<Zombie>(&destroyed);
    REQUIRE(mgr.getEntityCount() == 2);

    mgr.clear();
    REQUIRE(mgr.getEntityCount() == 0);
    REQUIRE(destroyed == 2);
}

TEST_CASE("detachEntity removes without deleting", "[entity]") {
    EntityManager mgr;
    int destroyed = 0;
    auto* z = mgr.spawn<Zombie>(&destroyed);

    REQUIRE(mgr.detachEntity(z));
    REQUIRE(mgr.getEntityCount() == 0);
    REQUIRE(destroyed == 0);
    REQUIRE(mgr.getEntitiesByType("zombie").empty());

    delete z;
    REQUIRE(destroyed == 1);
}

TEST_CASE("getEntitiesByType returns only matching bucket", "[entity]") {
    EntityManager mgr;
    auto* a = new Entity("a", "apple");
    auto* b = new Entity("b", "banana");
    auto* c = new Entity("c", "apple");
    mgr.addEntity(a);
    mgr.addEntity(b);
    mgr.addEntity(c);

    const auto& apples = mgr.getEntitiesByType("apple");
    REQUIRE(apples.size() == 2);
    REQUIRE(mgr.getEntitiesByType("banana").size() == 1);
    REQUIRE(mgr.getEntitiesByType("cherry").empty());
}

TEST_CASE("getEntityByName finds by name", "[entity]") {
    EntityManager mgr;
    auto* a = new Entity("target", "block");
    mgr.addEntity(a);
    mgr.addEntity(new Entity("other", "block"));

    REQUIRE(mgr.getEntityByName("target") == a);
    REQUIRE(mgr.getEntityByName("missing") == nullptr);
}
