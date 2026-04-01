#include "GameplaySystem.h"
#include "../Core/Application.h"
#include "../Audio/AudioManager.h"
#include "../Rendering/Camera.h"
#include "../Entity/Entity.h"
#include "../Entity/EntityManager.h"
#include "../Physics/PhysicsEngine.h"
#include "../Collision/CollisionSystem.h"
#include "../Input/InputManager.h"
#include "../../Game/Player.h"
#include "../../Game/FlyingEnemy.h"
#include "../../Game/ShootingEnemy.h"
#include "../../Game/Projectile.h"
#include "../../Game/Collectible.h"
#include "../../Game/MovingPlatform.h"
#include "../../Game/Goal.h"
#include <algorithm>
#include <cmath>

namespace Engine {

void GameplaySystem::begin(const Config& cfg) {
    m_cfg = cfg;

    m_player = nullptr;
    m_playerTyped = nullptr;
    auto& em = m_cfg.app->getEntityManager();
    const auto& players = em.getEntitiesByType("player");
    if (!players.empty()) {
        m_player = players.front();
        m_playerTyped = static_cast<Game::Player*>(m_player);
        m_playerSpawnPos = m_player->position;
    }

    m_score = 0;
    m_playTime = 0.f;
    m_won = false;
    m_nextLevel.clear();
    m_projectiles.clear();
    m_collected.clear();
    m_defeated.clear();
    m_playerWasOnGround = false;
    m_deathsThisTick = 0;

    if (m_cfg.camera && m_player) {
        m_cfg.camera->setPosition(m_player->getCenter());
        m_cfg.camera->setBounds(0.f, 0.f, m_cfg.levelWidth, m_cfg.levelHeight);
        m_cfg.camera->setLerpSpeed(5.f);
    }

    if (m_playerTyped)
        m_playerTyped->setInputManager(&m_cfg.app->getInput());

    // stagger flyer phases so a row doesn't bob in sync
    int flyerIndex = 0;
    for (auto* e : em.getEntitiesByType("flying_enemy")) {
        auto* flyer = static_cast<Game::FlyingEnemy*>(e);
        flyer->setBaseY(e->position.y);
        flyer->setPhase(flyerIndex * 1.2f);
        ++flyerIndex;
    }

    // default moving platform path: 200px right of current pos
    for (auto* e : em.getEntitiesByType("moving_platform")) {
        auto* mp = static_cast<Game::MovingPlatform*>(e);
        if (mp->getPointA().x == 0.f && mp->getPointA().y == 0.f)
            mp->setPointA(e->position);
        if ((mp->getPointB().x == 200.f && mp->getPointB().y == 0.f) ||
            (mp->getPointB().x == 0.f && mp->getPointB().y == 0.f)) {
            mp->setPointB({mp->getPointA().x + 200.f, mp->getPointA().y});
        }
        mp->resetToStart();
    }
}

void GameplaySystem::tick(float fixedDt) {
    if (m_won) return;

    auto& em = m_cfg.app->getEntityManager();
    auto& physics = m_cfg.app->getPhysics();
    auto& collision = m_cfg.app->getCollision();

    m_playTime += fixedDt;
    m_cfg.app->getInput().update();

    m_playerWasOnGround = m_player && m_player->isOnGround;
    auto& ents = em.getAllEntities();

    for (auto* e : ents) {
        if (!e->isStatic && e->hasGravity) physics.applyGravity(*e, fixedDt);
        e->update(fixedDt);
    }

    // rising edge → jump sfx
    if (m_player && m_playerWasOnGround && m_player->velocity.y < 0)
        m_cfg.audio->playSound("jump");

    {
        bool anyFired = false;
        for (auto* e : em.getEntitiesByType("shooting_enemy")) {
            auto* shooter = static_cast<Game::ShootingEnemy*>(e);
            if (!shooter->consumeFireFlag()) continue;
            auto* proj = em.spawn<Game::Projectile>();
            proj->position = {e->position.x + e->size.x / 2.f - proj->size.x / 2.f,
                              e->position.y + e->size.y / 2.f - proj->size.y / 2.f};
            float dir = (m_player && m_player->position.x < e->position.x) ? -1.f : 1.f;
            proj->velocity = {shooter->getProjectileSpeed() * dir, 0.f};
            m_projectiles.push_back(proj);
            anyFired = true;
        }
        if (anyFired) m_cfg.audio->playSound("shoot");
    }

    // moving platform rider carry
    if (m_player) {
        for (auto* e : em.getEntitiesByType("moving_platform")) {
            auto* mp = static_cast<Game::MovingPlatform*>(e);
            sf::Vector2f delta = mp->getDelta();
            if (delta.x == 0.f && delta.y == 0.f) continue;
            float playerBottom = m_player->position.y + m_player->size.y;
            float platformTop  = mp->position.y;
            bool verticallyOnTop = std::abs(playerBottom - platformTop) <= 4.f;
            bool horizontalOverlap = m_player->position.x + m_player->size.x > mp->position.x &&
                                     m_player->position.x < mp->position.x + mp->size.x;
            if (verticallyOnTop && horizontalOverlap)
                m_player->position += delta;
        }
    }

    // projectile hits run before resolver so bullets aren't pushed out
    std::vector<Game::Projectile*> deadProjectiles;
    for (auto* p : m_projectiles) {
        for (auto* e : ents) {
            if (e == p || e->type == "projectile" || e->type == "shooting_enemy" ||
                e->type == "enemy" || e->type == "flying_enemy") continue;
            if (e->isTrigger && e != m_player) continue;

            bool hit = collision.checkCollision(*p, *e);
            if (!hit && e == m_player) {
                // 2px fudge for fairness
                Entity expanded = *m_player;
                expanded.position -= {2.f, 2.f};
                expanded.size += {4.f, 4.f};
                hit = collision.checkCollision(*p, expanded);
            }

            if (hit) {
                deadProjectiles.push_back(p);
                if (e == m_player) {
                    m_player->position = m_playerSpawnPos;
                    m_player->velocity = {0.f, 0.f};
                    m_cfg.audio->playSound("shoot");
                    ++m_deathsThisTick;
                }
                break;
            }
        }
    }
    for (auto* dp : deadProjectiles) {
        em.removeEntity(dp);
        m_projectiles.erase(std::remove(m_projectiles.begin(), m_projectiles.end(), dp), m_projectiles.end());
    }

    // stomp vs death
    if (m_player) {
        std::vector<Entity*> deadEnemies;
        for (auto* e : ents) {
            if (e == m_player) continue;
            if (e->type != "enemy" && e->type != "flying_enemy" && e->type != "shooting_enemy") continue;

            if (collision.checkCollision(*m_player, *e)) {
                float playerBottom = m_player->position.y + m_player->size.y;
                float enemyMid = e->position.y + e->size.y * 0.5f;

                if (m_player->velocity.y > 0 && playerBottom <= enemyMid + 8.f) {
                    deadEnemies.push_back(e);
                    m_player->velocity.y = -400.f;
                    m_cfg.audio->playSound("collect");
                } else {
                    m_player->position = m_playerSpawnPos;
                    m_player->velocity = {0.f, 0.f};
                    ++m_deathsThisTick;
                    break;
                }
            }
        }
        for (auto* de : deadEnemies) {
            em.detachEntity(de);
            m_defeated.push_back(de);
        }
    }

    collision.findPotentialPairs(ents, [&collision](Entity& a, Entity& b) {
        if (a.type == "projectile" || b.type == "projectile") return;
        if (collision.checkCollision(a, b))
            collision.resolveCollision(a, b);
    });

    // coins detach so Stop can re-attach them
    if (m_player) {
        std::vector<Entity*> collected;
        for (auto* e : ents) {
            if (e->type == "collectible" && e->isTrigger &&
                collision.checkCollision(*m_player, *e)) {
                collected.push_back(e);
            }
        }
        for (auto* c : collected) {
            auto* col = static_cast<Game::Collectible*>(c);
            m_score += col->getPoints();
            em.detachEntity(c);
            m_collected.push_back(c);
            m_cfg.audio->playSound("collect");
        }
    }

    deadProjectiles.clear();
    for (auto* p : m_projectiles) {
        if (p->isExpired() ||
            p->position.x < -m_cfg.levelWidth * 0.5f || p->position.x > m_cfg.levelWidth * 1.5f ||
            p->position.y < -m_cfg.levelHeight * 0.5f || p->position.y > m_cfg.levelHeight * 1.5f) {
            deadProjectiles.push_back(p);
        }
    }
    for (auto* dp : deadProjectiles) {
        em.removeEntity(dp);
        m_projectiles.erase(std::remove(m_projectiles.begin(), m_projectiles.end(), dp), m_projectiles.end());
    }

    if (m_player) {
        m_player->position.x = std::clamp(m_player->position.x, 0.f, m_cfg.levelWidth - m_player->size.x);
        if (m_player->position.y > m_cfg.levelHeight + m_cfg.respawnYMargin) {
            m_player->position = m_playerSpawnPos;
            m_player->velocity = {0.f, 0.f};
            ++m_deathsThisTick;
        }
    }

    if (m_player) {
        for (auto* e : ents) {
            if (!e->isTrigger) continue;
            if (!collision.checkCollision(*m_player, *e)) continue;
            if (e->type == "goal") {
                m_won = true;
                m_nextLevel = static_cast<Game::Goal*>(e)->nextLevel;
                break;
            } else if (e->type == "hazard") {
                m_player->position = m_playerSpawnPos;
                m_player->velocity = {0.f, 0.f};
                ++m_deathsThisTick;
            }
        }
    }

    if (m_cfg.camera && m_player) {
        m_cfg.camera->setTarget(m_player->getCenter());
        m_cfg.camera->update(fixedDt);
    }
}

void GameplaySystem::end() {
    auto& em = m_cfg.app->getEntityManager();

    for (auto* p : m_projectiles)
        em.removeEntity(p);
    m_projectiles.clear();

    if (m_playerTyped)
        m_playerTyped->setInputManager(nullptr);

    m_player = nullptr;
    m_playerTyped = nullptr;
    m_won = false;
    m_nextLevel.clear();
    // editor drains m_collected + m_defeated during snapshot restore
}

}
