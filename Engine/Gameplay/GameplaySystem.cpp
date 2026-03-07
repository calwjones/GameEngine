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

    // re-scan every play bc editor couldve moved/replaced/deleted the player since last run
    m_player = nullptr;
    auto& em = m_cfg.app->getEntityManager();
    for (auto* e : em.getAllEntities())
        if (e->type == "player") { m_player = e; break; }

    if (m_player)
        m_playerSpawnPos = m_player->position;   // respawn point = current editor pos, not json pos

    // reset transient play state
    m_score = 0;
    m_playTime = 0.f;
    m_won = false;
    m_nextLevel.clear();
    m_projectiles.clear();
    m_collected.clear();
    m_defeated.clear();
    m_playerWasOnGround = false;
    m_deathsThisTick = 0;

    // snap camera on frame 1 so u dont see a lerp from wherever the editor left it
    if (m_cfg.camera && m_player) {
        m_cfg.camera->setPosition(m_player->getCenter());
        m_cfg.camera->setBounds(0.f, 0.f, m_cfg.levelWidth, m_cfg.levelHeight);
        m_cfg.camera->setLerpSpeed(5.f);
    }

    // wire input only during play, end() clears it so editor clicks dont jump the player
    if (auto* player = dynamic_cast<Game::Player*>(m_player))
        player->setInputManager(&m_cfg.app->getInput());

    // snapshot flyer Y as sine midpoint + stagger phases so a row of them doesnt bob in sync
    int flyerIndex = 0;
    for (auto* e : em.getAllEntities()) {
        if (auto* flyer = dynamic_cast<Game::FlyingEnemy*>(e)) {
            flyer->setBaseY(e->position.y);
            flyer->setPhase(flyerIndex * 1.2f);
            ++flyerIndex;
        }
    }

    // moving platform defaults: (0,0) pointA means "use current pos", (200,0) means "200px right of A" — saves tedious clicking for simple L↔R patrols
    for (auto* e : em.getAllEntities()) {
        if (auto* mp = dynamic_cast<Game::MovingPlatform*>(e)) {
            if (mp->getPointA().x == 0.f && mp->getPointA().y == 0.f)
                mp->setPointA(e->position);
            if ((mp->getPointB().x == 200.f && mp->getPointB().y == 0.f) ||
                (mp->getPointB().x == 0.f && mp->getPointB().y == 0.f)) {
                mp->setPointB({mp->getPointA().x + 200.f, mp->getPointA().y});
            }
            mp->resetToStart();
        }
    }
}

void GameplaySystem::tick(float fixedDt) {
    if (m_won) return;   // freeze sim until editor consumes the win flag

    auto& em = m_cfg.app->getEntityManager();
    auto& physics = m_cfg.app->getPhysics();
    auto& collision = m_cfg.app->getCollision();

    m_playTime += fixedDt;
    m_cfg.app->getInput().update();   // edge-detection bookkeeping

    // cache "was grounded" BEFORE update() clears isOnGround — used for jump sfx edge
    m_playerWasOnGround = m_player && m_player->isOnGround;
    auto& ents = em.getAllEntities();

    // gravity before update() so velocity already has this tick's gravity when subclasses run
    for (auto* e : ents) {
        if (!e->isStatic && e->hasGravity) physics.applyGravity(*e, fixedDt);
        e->update(fixedDt);
    }

    // jump sfx = rising edge: grounded last tick + moving up now
    if (m_player && m_playerWasOnGround && m_player->velocity.y < 0)
        m_cfg.audio->playSound("jump");

    // per-type index = O(shooters) not O(n) w/ dynamic_cast. collect-then-add avoids mid-iteration mutation
    {
        std::vector<Game::Projectile*> newProjectiles;
        for (auto* e : em.getEntitiesByType("shooting_enemy")) {
            auto* shooter = static_cast<Game::ShootingEnemy*>(e);   // safe, index guarantees type
            if (shooter->consumeFireFlag()) {
                auto* proj = new Game::Projectile();
                // spawn centred so the bullet doesnt clip thru walls
                proj->position = {e->position.x + e->size.x / 2.f - proj->size.x / 2.f,
                                  e->position.y + e->size.y / 2.f - proj->size.y / 2.f};
                float dir = (m_player && m_player->position.x < e->position.x) ? -1.f : 1.f;
                proj->velocity = {shooter->getProjectileSpeed() * dir, 0.f};
                newProjectiles.push_back(proj);
            }
        }
        for (auto* proj : newProjectiles) {
            em.addEntity(proj);
            m_projectiles.push_back(proj);   // local list for easy cleanup on end()
        }
        if (!newProjectiles.empty())
            m_cfg.audio->playSound("shoot");
    }

    // moving platform riding — without this the platform moves and the player falls off instantly. 4px tolerance bc of sub-pixel drift from the resolver
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

    // projectile hits BEFORE the collision resolver or the bullet gets pushed out + we miss the overlap
    std::vector<Entity*> deadProjectiles;
    for (auto* p : m_projectiles) {
        for (auto* e : ents) {
            if (e == p || e->type == "projectile" || e->type == "shooting_enemy" ||
                e->type == "enemy" || e->type == "flying_enemy") continue;
            if (e->isTrigger && e != m_player) continue;   // bullets only hit walls + player, not coins/goals

            bool hit = collision.checkCollision(*p, *e);
            if (!hit && e == m_player) {
                // 2px fudge box on the player — feels fairer than pixel-perfect
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

    // stomp vs death — mario rules. falling onto enemy from above = stomp, anything else = die. runs BEFORE the resolver or both checks miss. detach not remove so Stop can restore
    if (m_player) {
        std::vector<Entity*> deadEnemies;
        for (auto* e : ents) {
            if (e == m_player) continue;
            if (e->type != "enemy" && e->type != "flying_enemy" && e->type != "shooting_enemy") continue;

            if (collision.checkCollision(*m_player, *e)) {
                float playerBottom = m_player->position.y + m_player->size.y;
                float enemyMid = e->position.y + e->size.y * 0.5f;

                if (m_player->velocity.y > 0 && playerBottom <= enemyMid + 8.f) {
                    // stomp — enemy dies, small bounce
                    deadEnemies.push_back(e);
                    m_player->velocity.y = -400.f;
                    m_cfg.audio->playSound("collect");   // reuse coin sfx, nice zing
                } else {
                    m_player->position = m_playerSpawnPos;
                    m_player->velocity = {0.f, 0.f};
                    ++m_deathsThisTick;
                    break;   // dead, stop checking more enemies
                }
            }
        }
        for (auto* de : deadEnemies) {
            em.detachEntity(de);          // keeps ptr alive
            m_defeated.push_back(de);     // editor will re-attach on Stop
        }
    }

    // collision pass — spatial hash broad-phase → AABB narrow → MTV pushout. skip projectiles (triggers, handled above)
    collision.findPotentialPairs(ents, [&collision](Entity& a, Entity& b) {
        if (a.type == "projectile" || b.type == "projectile") return;
        if (collision.checkCollision(a, b))
            collision.resolveCollision(a, b);
    });

    // collectible pickup — detach not remove so Stop can re-attach. coins are cosmetic score, only Goal ends the level
    if (m_player) {
        std::vector<Entity*> collected;
        for (auto* e : ents) {
            if (e->type == "collectible" && e->isTrigger &&
                collision.checkCollision(*m_player, *e)) {
                collected.push_back(e);
            }
        }
        for (auto* c : collected) {
            auto* col = dynamic_cast<Game::Collectible*>(c);
            m_score += col ? col->getPoints() : 10;   // fallback 10 if cast failed
            em.detachEntity(c);                       // keeps ptr alive for restore
            m_collected.push_back(c);
            m_cfg.audio->playSound("collect");
        }
    }

    // projectile expiry — lifetime ran out OR flew way out of bounds (safety net)
    deadProjectiles.clear();
    for (auto* p : m_projectiles) {
        auto* proj = dynamic_cast<Game::Projectile*>(p);
        if ((proj && proj->isExpired()) ||
            p->position.x < -m_cfg.levelWidth * 0.5f || p->position.x > m_cfg.levelWidth * 1.5f ||
            p->position.y < -m_cfg.levelHeight * 0.5f || p->position.y > m_cfg.levelHeight * 1.5f) {
            deadProjectiles.push_back(p);
        }
    }
    for (auto* dp : deadProjectiles) {
        em.removeEntity(dp);
        m_projectiles.erase(std::remove(m_projectiles.begin(), m_projectiles.end(), dp), m_projectiles.end());
    }

    // fall-off respawn + horizontal clamp (cant walk off the sides, thats just frustrating)
    if (m_player) {
        m_player->position.x = std::clamp(m_player->position.x, 0.f, m_cfg.levelWidth - m_player->size.x);
        if (m_player->position.y > m_cfg.levelHeight + m_cfg.respawnYMargin) {
            m_player->position = m_playerSpawnPos;
            m_player->velocity = {0.f, 0.f};
            ++m_deathsThisTick;
        }
    }

    // goal = win (optionally chains to nextLevel), hazard = death. break after goal so 2 goals in 1 tick doesnt double-win
    if (m_player) {
        for (auto* e : ents) {
            if (!e->isTrigger) continue;
            if (!collision.checkCollision(*m_player, *e)) continue;
            if (e->type == "goal") {
                m_won = true;
                if (auto* goal = dynamic_cast<Game::Goal*>(e))
                    m_nextLevel = goal->nextLevel;   // "" = end, else = chain to that file
                break;
            } else if (e->type == "hazard") {
                m_player->position = m_playerSpawnPos;
                m_player->velocity = {0.f, 0.f};
                ++m_deathsThisTick;
            }
        }
    }

    // camera follow — lerps toward player, editor pulls the smoothed pos for the sfml view
    if (m_cfg.camera && m_player) {
        m_cfg.camera->setTarget(m_player->getCenter());
        m_cfg.camera->update(fixedDt);
    }
}

void GameplaySystem::end() {
    auto& em = m_cfg.app->getEntityManager();

    // kill projectiles — they spawned during play so theyre not in the pre-play snapshot
    for (auto* p : m_projectiles)
        em.removeEntity(p);
    m_projectiles.clear();

    // unwire input so editor clicks post-stop dont jump the player
    if (auto* player = dynamic_cast<Game::Player*>(m_player))
        player->setInputManager(nullptr);

    m_player = nullptr;
    m_won = false;
    m_nextLevel.clear();
    // m_collected + m_defeated intentionally NOT cleared — editor drains them in restoreSnapshot() to re-attach killed enemies + picked-up coins
}

}
