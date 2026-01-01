#pragma once
#include <SFML/Graphics.hpp>
#include "../Engine/Core/Application.h"
#include "../Engine/State/StateManager.h"
#include "../Engine/Level/LevelLoader.h"
#include "../Engine/Level/LevelGroup.h"
#include "../Engine/Entity/EntityFactory.h"
#include "../Engine/Rendering/Camera.h"
#include "../Engine/Audio/AudioManager.h"
#include "../Engine/Rendering/TextureManager.h"
#include "../Engine/Gameplay/GameplaySystem.h"
#include "ScenePanel.h"
#include "PropertiesPanel.h"
#include "GameViewport.h"
#include "EntityPalette.h"
#include "CommandHistory.h"
#include <vector>

namespace Editor {

// per-entity snapshot taken at play start — rolled back on stop so playtest is non-destructive (die, bounce around, hit stop, everything restored)
struct EntitySnapshot {
    Engine::Entity* entity;
    sf::Vector2f position;
    sf::Vector2f velocity;
    bool isOnGround;
};

// runtime state for a multi-level world run. active=false means single-level playtest, all the group logic is bypassed
struct GroupSession {
    bool active = false;
    int groupIndex = -1;              // index into m_groups
    int currentLevel = 0;             // within the group
    int livesRemaining = 3;
    int totalCoins = 0;               // running sum across levels of the world
};

// which full-screen transition overlay (if any) is blocking gameplay this frame
enum class PlayOverlay {
    None,
    LevelComplete,   // between levels of a world — "Continue" advances
    WorldComplete,   // final level cleared — "Back to Editor"
    GameOver,        // out of lives mid-world — "Retry World" restarts at level 1
    SingleWin,       // individual-level goal hit — "YOU WIN" overlay
};

// the main class — owns everything. m_game is a subsystem container, editor drives its own loop + never calls m_game.run()
class EditorApplication {
    sf::RenderWindow m_window;
    Engine::Application m_game;           // subsystem bag — see Application.h
    Engine::EntityFactory m_factory;      // type-string → subclass ctor lookup
    Engine::LevelLoader m_loader;
    Engine::Camera m_camera;              // only moves during play mode, ignored while editing
    Engine::AudioManager m_audio;
    Engine::TextureManager m_textures;

    ScenePanel m_scenePanel;
    PropertiesPanel m_propsPanel;
    GameViewport m_viewport;
    EntityPalette m_palette;
    CommandHistory m_history;

    Engine::GameplaySystem m_gameplay;    // runs the per-tick sim in play mode (extracted from update() on 2026-04-11)

    Engine::GameState m_state = Engine::GameState::MENU;
    sf::Clock m_clock;
    float m_dt = 0.f;
    float m_accumulator = 0.f;   // fixed-timestep accumulator — same pattern as Engine::GameLoop
    bool m_dirty = false;        // unsaved changes flag — shows a * in the window title
    bool m_imguiInitialized = false;

    std::vector<Engine::Entity*> m_selection;   // everything thats selected rn
    Engine::Entity* m_selected = nullptr;        // primary — the one shown in the properties panel (selection.back() basically)
    Engine::Entity* m_player = nullptr;          // weak menu-mode cache, just for the "press space to play" hint

    std::string m_levelPath;
    std::vector<std::string> m_levelFiles;
    std::vector<Engine::LevelGroup> m_groups;
    GroupSession m_groupSession;
    PlayOverlay m_playOverlay = PlayOverlay::None;
    int m_levelCoinsSnapshot = 0;   // coins earned on the level just cleared - shown on the transition overlay
    bool m_showSaveAs = false;
    char m_saveAsBuf[256] = {0};
    bool m_showAbout = false;
    bool m_showLevelBrowser = false;

    // unsaved-changes modal stashes the original intent here, replayed once user picks save/discard/cancel
    enum class PendingAction { None, NewLevel, LoadLevel, CloseWindow, StartGroup };
    PendingAction m_pendingAction = PendingAction::None;
    std::string m_pendingLoadPath;
    int m_pendingGroupIndex = -1;

    std::string m_statusMsg;
    float m_statusTimer = 0.f;
    float m_fpsAccum = 0.f;
    int m_fpsFrames = 0;
    int m_displayFps = 60;

    // snapshot/restore used for non-destructive playtesting
    std::vector<EntitySnapshot> m_playSnapshot;
    sf::Vector2f m_savedViewOffset{0, 0};
    float m_savedViewZoom = 1.f;

    // deferred level transition: if a Goal trigger hands us a nextLevel we stash
    // the target here and switch levels after the fixed-timestep loop exits so
    // we don't load while still iterating the current entity list.
    std::string m_pendingNextLevel;

    // bigger default canvas so theres room to actually build something.
    // old default (800x600) left designers fighting the bounds from the first click
    static constexpr float DEFAULT_W = 1600.f;
    static constexpr float DEFAULT_H = 720.f;
    float m_levelWidth = DEFAULT_W;
    float m_levelHeight = DEFAULT_H;
    static constexpr float RESPAWN_Y_MARGIN = 100.f;   // how far below levelHeight before respawn triggers

    int m_entityCounter = 0;   // monotonic, never resets - avoids duplicate names after delete

public:
    ~EditorApplication() { shutdown(); }

    bool initialize(unsigned int w = 1280, unsigned int h = 720);
    void run();
    void shutdown();

private:
    void processEvents();
    void update(float dt);
    void render();

    void renderImGui();
    void renderMenuBar();
    void renderToolbar();
    void renderStateOverlay();
    void renderPopups();

    void fitLevelToContent();   // expand level bounds to encompass all entities + padding
    void loadLevel(const std::string& path);
    void saveLevel();
    void saveLevelAs(const std::string& filename);
    void scanLevelFiles();
    void scanLevelGroups();

    // group playthrough lifecycle. startGroup loads level 0 and begins play with
    // a fresh GroupSession; advanceGroupLevel moves to the next level in the
    // sequence after a LevelComplete overlay; restartGroup rewinds to level 0
    // when all lives are gone; endGroupSession clears state on stop/editor return.
    void startGroup(int groupIndex);
    void advanceGroupLevel();
    void restartGroup();
    void endGroupSession();

    void addEntity();
    void addFromTemplate(EntityTemplate tmpl);
    void duplicateEntity();
    void deleteSelected();

    void handleInput(sf::Keyboard::Key key);
    void startPlaying();
    void stopPlaying();
    void snapshotEntities();
    void restoreSnapshot();

    void findPlayer();
    void setStatus(const std::string& msg);
    void updateWindowTitle();
    void markDirty();
    void resetToNewLevel();
    void showUnsavedChangesDialog();
    bool isPlaying() const { return m_state == Engine::GameState::PLAYING || m_state == Engine::GameState::PAUSED; }

    void selectEntity(Engine::Entity* e);
    void toggleSelect(Engine::Entity* e);
    void clearSelection();
    bool isEntitySelected(Engine::Entity* e) const;
    void validateSelection();
};

}
