# 2D Game Engine & Visual Editor

A 2D platformer game engine with a built-in visual level editor, written in C++17. Built as a final-year project at UWE Bristol.

The editor lets you build platformer levels by placing and configuring entities from a palette, then playtesting them in-editor without recompiling.

## Features

### Engine
- Fixed-timestep game loop at 60 Hz with accumulator-based timing
- Gravity, exponential friction, and terminal velocity physics
- AABB collision detection with uniform grid spatial partitioning
- Smooth-follow camera with exponential lerp
- Audio system with conditional compilation fallback for systems without SFML Audio

### Editor
- Scene hierarchy panel with search, type icons, and entity reordering
- Properties inspector for position, size, physics flags, colour, and type-specific behaviour
- Entity palette with templates: Player, Enemy, Flying Enemy, Shooting Enemy, Collectible, Platform, Wall, Ground
- Live game viewport (render-to-texture) with pan, zoom, grid snapping, hover highlight, and edge-scroll
- Full undo/redo via the command pattern (Ctrl+Z / Ctrl+Shift+Z)
- JSON save/load with dirty flag tracking and unsaved-changes protection

### Gameplay
- Player: WASD / arrow keys to move, Space to jump
- Patrol enemies, flying enemies (sinusoidal path), shooting enemies (fires projectiles)
- Collectibles with score tracking
- Player respawns on enemy contact or falling out of bounds

## Building

The project uses CMake's `FetchContent` to automatically download and configure dependencies (ImGui, ImGui-SFML, RapidJSON). You only need to have SFML 2.x installed on your system.

**macOS (Homebrew)**
```bash
brew install sfml@2
cmake -B build
cmake --build build
./build/GameEngine
```

**Windows (Visual Studio)**
Ensure you have SFML 2.x installed. If SFML is not in a standard location, you can point CMake to it:
```powershell
cmake -B build -DSFML_DIR="C:/path/to/SFML/lib/cmake/SFML"
cmake --build build --config Release
.\build\Release\GameEngine.exe
```

## Editor Controls

| Input | Action |
|-------|--------|
| Space | Play / start simulation |
| Esc | Pause (while playing) |
| Q | Stop and return to editor |
| Ctrl+Z | Undo |
| Ctrl+Shift+Z | Redo |
| Ctrl+S | Save level |
| Ctrl+N | New level |
| Ctrl+D | Duplicate selected entity |
| Del | Delete selected entity |
| G | Toggle grid |
| F | Focus viewport on selection |
| Middle-drag | Pan viewport |
| Scroll | Zoom viewport |
| Ctrl+Click | Multi-select |

## Project Structure

```
GameEngine/
├── Engine/     # Core systems: physics, collision, camera, audio, entity management, level I/O
├── Editor/     # ImGui editor: panels, viewport, undo/redo, palette
├── Game/       # Entity subclasses: Player, Enemy, FlyingEnemy, ShootingEnemy, Collectible, Projectile
├── assets/     # Level JSON files and audio
└── external/   # SFML, Dear ImGui, ImGui-SFML, RapidJSON
```

## Dependencies

- [SFML 2.x](https://www.sfml-dev.org/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [ImGui-SFML](https://github.com/SFML/imgui-sfml)
- [RapidJSON](https://rapidjson.org/)
