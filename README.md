# Metal Slug — OOP Project (Spring)

A C++ recreation of **Metal Slug** built with [SFML](https://www.sfml-dev.org/) as a semester project for our Object-Oriented Programming course. The game features multiple playable soldiers, drivable vehicles, several enemy types with their own AI, procedurally generated biomes, a survival level structure, and a full HUD/game-loop system — all designed around core OOP principles (inheritance, polymorphism, abstraction, and encapsulation).

## Authors

- **Abdurehman Rafi**
- **Izaan Ahmad**

## Features

- **4 playable soldiers** — Marco Rossi, Eri Kasamoto, Tarma Roving, and Fiolina Germi — each with unique weapons, melee attacks, and power-ups.
- **Drivable vehicles** — Metal Slug tank, Slug Flyer, Slug Mariner, and Amphibious Slug, each with their own movement and firing behavior.
- **Enemy roster with individual AI** — Rebel Soldier, Shielded Soldier, Bazooka Soldier, Grenade Soldier, Zombie, Mummy Warrior, Martian, Flying Tara, Bradley Vehicle, Enemy Sub, and Paratrooper, driven by a state machine (Patrol, Chase, Attack, Idle, Descent states).
- **Procedural biomes** — Plains, Aerial, and Aquatic biomes generate terrain and destructible/indestructible/water blocks.
- **3 survival levels** with predefined enemy waves and POW prisoner rescues.
- **Collectibles** — turkeys, fruit, supply crates, and rescuable POW prisoners.
- **Weapons system** — Pistol, Knife, Heavy Machine Gun, Rocket Launcher, Flame Shot, and Laser Gun.
- **Full game loop** — main menu, game mode selection, HUD, camera system, high score tracking, and pause/level-select controls.

## Tech Stack

- **Language:** C++
- **Graphics/Audio/Windowing:** [SFML 2](https://www.sfml-dev.org/) (Graphics, Window, System, Audio, Network)
- **IDE / Build system:** Visual Studio (`.vcxproj` project, `v145` platform toolset)

## Project Structure

```
Metal-Slug-main/
├── Project1/
│   ├── Project1.slnx                 # Visual Studio solution
│   └── Project1/
│       ├── Source.cpp                # Entry point (creates and runs the Game)
│       ├── Header.h                  # All game classes (see Architecture below)
│       ├── highscore.txt             # Persisted high score
│       ├── Sprites/                  # Character, enemy, vehicle, item, and tile art
│       └── *.dll                     # SFML / OpenAL runtime dependencies
└── *.dll                             # SFML / OpenAL runtime dependencies (root copy)
```

## Architecture / Class Overview

The entire game logic lives in `Header.h`, organized around a clear class hierarchy:

| Category | Classes |
|---|---|
| **Core / Base** | `Entity` (abstract base), `DamagableEntity`, `AnimationController`, `Environment`, `Camera`, `HUD`, `GameStats`, `InputManager`, `Game` |
| **World** | `Block`, `NormalBlock`, `IndestructibleBlock`, `WaterBlock`, `Biome` → `PlainsBiome`, `AerialBiome`, `AquaticBiome` |
| **Levels** | `Level` (abstract) → `SurvivalLevel` (abstract) → `Level1`, `Level2`, `Level3`; `SpawnPoint`, `GameModeMenu` |
| **Player** | `Soldier` (abstract) → `PlayerSoldier` → `MarcoRossi`, `EriKasamoto`, `Tarma`, `FiolinaGermi`; `TransformationState`, `PlayerProjectilePool` |
| **Enemies** | `Enemy` (abstract, extends `DamagableEntity`) → `RebelSoldier`, `ShieldedSoldier`, `BazookaSoldier`, `GrenadeSoldier`, `Zombie`, `MummyWarrior`, `Martian`, `FlyingTara`, `BradleyVehicle`, `EnemySub`, `Paratrooper`; `EnemyManager`, `EnemyProjectiles` |
| **Enemy AI** | `EnemyAIState` (abstract) → `PatrolState`, `ChaseState`, `AttackState`, `IdleState`, `DescentState` |
| **Vehicles** | `Vehicle` (abstract, extends `Entity`) → `MetalSlug`, `SlugFlyer`, `SlugMariner`, `AmphibiousSlug` |
| **Weapons / Projectiles** | `Weapon` → `Pistol`, `Knife`, `HeavyMachineGun`, `RocketLauncher`, `FlameShotWeapon`, `LaserGun`; `Projectile` (abstract) → `Bullet`, `BallisticProjectile` |
| **Collectibles** | `Collectible` (abstract, extends `Entity`) → `Turkey`, `Fruit`, `SupplyCrate`, `POWPrisoner` |

This structure demonstrates the four pillars of OOP:
- **Abstraction** via pure virtual base classes (`Entity`, `Weapon`, `Block`, `Biome`, `Level`, `Projectile`, `Enemy`, `Vehicle`, `Collectible`, `EnemyAIState`).
- **Inheritance** across soldiers, enemies, vehicles, and weapons that share a common interface.
- **Polymorphism** through virtual `update()`/`render()`/`attack()` overrides used uniformly by the game loop and managers.
- **Encapsulation** of state (health, ammo, animation, AI state) inside each class.

## Controls

| Action | Key |
|---|---|
| Move Left / Right | `Left Arrow` / `Right Arrow` |
| Move Up / Down (vehicles, flying, swimming) | `Up Arrow` / `Down Arrow` |
| Jump | `Space` |
| Fire weapon | `X` or Left Mouse Button |
| Melee attack | `F` |
| Throw grenade | `G` |
| Enter / exit vehicle | `E` |
| Fire vehicle secondary / surface missile | `C` / `V` |
| Power-up / special (varies by character) | `P` |
| Select menu option | `Up` / `Down` |
| Confirm menu selection | `Enter` |
| Pause / Menu | `Enter` or `M` |
| Quick level select (debug) | `1`, `2`, `3` |
| Exit game / close menu | `Escape` |

> Some keys are context-sensitive and vary slightly by character/vehicle — see `Header.h` for exact per-class bindings.

## Getting Started

### Prerequisites

- Windows with **Visual Studio** (2022 or later recommended, toolset `v145`)
- [SFML 2.x](https://www.sfml-dev.org/download.php) (Graphics, Window, System, Audio, Network modules)

### Setup

1. Clone or download this repository.
2. Download SFML 2.x and note its `include` and `lib` folder paths.
3. Open `Project1/Project1.slnx` in Visual Studio.
4. Update the project's **Additional Include Directories** and **Additional Library Directories** to point to your local SFML `include` and `lib` folders (the project currently references a local path and will need to be repointed to your machine).
5. Ensure the following are linked under **Additional Dependencies**:
   ```
   sfml-graphics-d.lib; sfml-window-d.lib; sfml-system-d.lib; sfml-network-d.lib; sfml-audio-d.lib
   ```
   (drop the `-d` suffix for Release builds).
6. Make sure the SFML/OpenAL `.dll` files (already included alongside the project) sit next to the built executable.
7. Build and run (`Ctrl+F5`).

## Assets

All character, enemy, vehicle, item, background, and tile sprites are stored under `Project1/Project1/Sprites/`, including 4x-upscaled variants for higher-resolution rendering.

## Notes

- High scores are persisted locally in `highscore.txt`.
- This project was built for educational purposes to demonstrate OOP design patterns in a game engine context and is a fan-made tribute to SNK's *Metal Slug* — not affiliated with or endorsed by SNK.
