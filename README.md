# Diorama

A 3D voxel puzzle game written in C99 with [raylib](https://www.raylib.com/).
Guide Sprout through small floating dioramas: push crates, climb stairs and ladders,
turn cranks to move platforms, and reach the golden seed.

## Build and run

Requirements: `gcc`, `make` and raylib installed in `/usr/local` (Linux / X11).

```sh
make        # build ./diorama
make run    # build and launch
make test   # movement tests
make clean  # remove obj/ (fclean also removes the binary)
```

Run the game from the project root (it loads `levels/` and `assets/` with relative paths).

## Controls

| Action | Default key |
|---|---|
| Move | W A S D (Z Q S D on AZERTY) |
| Restart level | R |
| Rotate camera / navigate menus | Arrow keys |
| Confirm / enter a level | Enter or Space |
| Back | Escape |

Movement and restart keys can be rebound in **Settings**.
Settings are saved to `settings.dat`.

## Structure

```
src/
  core/       orbit camera, settings
  world/      level loading, movement, platforms, block types
  render/     blocks, lighting, vegetation, sky, backdrop
  entities/   player, crank
  game/       game states, per-frame update and rendering
  ui/         menus, HUD, settings screen
levels/       levels (.txt) and sky decor (.sky)
assets/       .glb models and Blender sources
tests/        unit tests
```

## Level format

A level `levels/<name>.txt` stacks horizontal layers separated by a blank line,
from bottom to top. Each layer is a grid of characters (max 8×6×8).

| Char | Block | Char | Vegetation |
|---|---|---|---|
| `.` | empty | `T` | round tree |
| `@` | player start | `P` | pine tree |
| `#` | ground | `B` | bush |
| `c` | crate | `F` | fern |
| `*` | golden seed (goal) | `g` | grass |
| `k` | crank | `f` | flowers |
| `p` | platform | `m` | mushrooms |
| `^ > v <` | stairs (N E S W) | `I` | ivy (on ground) |
| `n e s w` | ladder (N E S W) | `V` | vine (on ground) |
| `1`–`9` | level entrance (world map) | | |

The `.sky` file with the same name places the decor: one object per line,
`kind x y z scale yaw` (position relative to the level center), `#` for comments.

To add a level: create `level_N.txt` (+ `.sky`), add it to `LEVEL_PATHS`
in `src/game/game.c`, increment `LEVEL_COUNT` in `src/game/game.h`, and place
entrance `N` on `levels/world_map.txt`.
