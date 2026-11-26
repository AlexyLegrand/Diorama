# Tools

Helpers to test the game, export models and keep assets clean. They need `uv` (already installed) and Blender.

## 1. Drive the game — `play.py`

Launches `./diorama`, sends key presses to its window only, and saves screenshots in `tools/out/`.
Your `progress.dat` and `settings.dat` are backed up before the run and restored after it.

```sh
make
uv run tools/play.py "key Return" "wait 3.5" "shot map" "hold z 0.8" "burst walk 8 0.1" "key Escape" "shot pause"
```

| Step | Effect |
|---|---|
| `key NAME` | Press and release a key (X key names: `Return`, `Escape`, `space`, `z`, `Left`...) |
| `hold NAME SECONDS` | Hold a key down |
| `wait SECONDS` | Wait |
| `shot NAME` | Save one screenshot `NAME.png` |
| `burst NAME COUNT DELAY` | Save `COUNT` screenshots, `DELAY` seconds apart, to judge a movement |

Options: `--out DIR` (screenshot folder), `--start SECONDS` (wait after the window appears, default 4).
Movement keys are physical positions: on AZERTY, forward is `z`.
Do not type in other windows while it runs: key presses go to the focused window.

## 2. Export from Blender — `export_glb.sh` and `glb_info.py`

```sh
tools/export_glb.sh assets/sprout/sprout.blend assets/sprout/sprout.glb
tools/export_glb.sh scene.blend assets/rocket/rocket.glb RocketRig RocketMesh
python3 tools/glb_info.py assets/rocket/rocket.glb
```

- `export_glb.sh FILE.blend OUTPUT.glb [OBJECT...]` exports the named objects (and their children), or every object
  except cameras and lights, with all their actions. It then runs `glb_info.py` on the result.
- `glb_info.py` lists meshes, materials and animations with their durations, and warns when an animation was mixed
  with other actions at export (the rocket bug). It exits with status 1 on a warning.
- Blender's glTF exporter needs `numpy`: it is installed once in `tools/.deps/` (not committed).

Export one model per `.blend` scene, or name its objects explicitly, so actions from other characters are not mixed in.

### Optional: Blender connector (MCP)

To let Claude edit a scene live in the Blender window:

1. Install the `blender-mcp` add-on in Blender (Edit > Preferences > Add-ons > Install, from the project's
   `addon.py`), enable it, then click "Connect to Claude" in the 3D view sidebar (N panel, BlenderMCP tab).
2. Register the server once: `claude mcp add blender -- uvx blender-mcp`
3. Restart Claude Code.

This is a third-party project: read its code before installing it.

## 3. Free assets

See `assets/CREDITS.md`: where to find CC0 assets, and the list every third-party asset must be added to.

## 4. References

See `docs/references/`: screenshots and notes from other games. The images stay local and are not committed.
