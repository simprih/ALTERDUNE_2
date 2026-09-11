# ALTERDUNE

A turn-based combat RPG written from scratch in C++17, with both a terminal and an SFML graphical build sharing a single game-logic core.

Built as an object-oriented programming project at ESILV (Léonard de Vinci Graduate School of Engineering), April 2026.

![ALTERDUNE combat screen](https://github.com/user-attachments/assets/0b8195ff-c72d-4fc8-bd3b-64acfeb1b9a8)

## Gameplay

Combat is turn-based against three tiers of enemies — regular monsters, minibosses and bosses. Each turn the player chooses to fight, use an item, or ACT.

The ACT system is the core mechanic: instead of only reducing an enemy's HP, the player can raise a **mercy** counter until it reaches that enemy's `mercyGoal`, at which point the enemy can be spared. Every fight is therefore winnable by force or by persuasion.

Each enemy exposes its own subset of actions, and the number available scales with its tier — two for regular monsters, three for minibosses, four for bosses:

| Action | Notes |
|---|---|
| `COMPLIMENT`, `JOKE`, `DANCE`, `PET`, `DISCUSS`, `OFFER_SNACK` | Standard mercy-building actions |
| `OBSERVE` | Minibosses only |
| `REASON` | Bosses only |
| `INSULT` | Bosses only — mercy impact can be negative, so it reads as a risk/reward option |

Because actions carry a signed `impactMercy`, the right approach differs per enemy rather than being a single dominant strategy.

## Architecture

The codebase is roughly 3,000 lines across 11 classes.

```
Monstre  (abstract base)
├── MonstreNormal
├── MiniBoss
└── Boss
```

`Monstre` declares pure virtual `afficher()` and `cloner()` plus a virtual destructor. `cloner()` implements a **prototype pattern**: the `Bestiaire` holds one template instance per enemy type and clones it on demand, so spawning never needs a switch on enemy type. Enemies are held as `vector<Monstre*>` with manual lifetime management.

Other classes:

| Class | Role |
|---|---|
| `Joueur` | Player state, inventory, combat actions |
| `Bestiaire` | Enemy catalogue and cloning source |
| `Item` | Consumables and their effects |
| `ActionACT` | ACT command definitions and mercy resolution |
| `ChargeurCSV` | Static loaders building `Item` and `Monstre*` collections from CSV |
| `Jeu` | Terminal game loop |
| `JeuSFML` | Graphical game loop |

Enemy and item statistics live in `monsters.csv` and `items.csv` rather than in code, so balancing is a data edit and not a recompile.

A class diagram is included as `UML_Diagram.png`.

## Building

Requires CMake 3.16+, a C++17 compiler, and SFML 3 (bundled under `lib/` and `include/`).

```bash
cmake -B build
cmake --build build
```

This produces two executables:

- `alterdune` — terminal version
- `alterdune_sfml` — graphical version

Run from the project root so the CSV data files are found:

```bash
./build/alterdune
```

macOS links SFML statically against the system frameworks; Windows (MinGW) copies the SFML DLLs next to the executable automatically. Both are handled in `CMakeLists.txt`.

## Repository layout

```
├── main.cpp / jeu.cpp          Terminal entry point and loop
├── main_sfml.cpp / jeu_sfml.cpp Graphical entry point and loop
├── monstre.*, monstreNormal.*, miniboss.*, boss.*
├── joueur.*, item.*, actionAct.*, bestiaire.*
├── chargeurCSV.*               CSV loading
├── monsters.csv, items.csv     Game data
├── UML_Diagram.png             Class diagram
└── CMakeLists.txt
```

## Authors

Simon Prihnenko and Pierre Rançon.
