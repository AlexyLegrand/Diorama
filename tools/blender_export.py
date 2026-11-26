import sys

import bpy

args = sys.argv[sys.argv.index("--") + 1:]
output = args[0]
names = set(args[1:])

bpy.ops.object.select_all(action="DESELECT")
for obj in bpy.data.objects:
    if obj.type in {"CAMERA", "LIGHT"}:
        continue
    if not names or obj.name in names or (obj.parent is not None and obj.parent.name in names):
        obj.select_set(True)

bpy.ops.export_scene.gltf(
    filepath=output,
    export_format="GLB",
    use_selection=True,
    export_animations=True,
    export_animation_mode="ACTIONS",
    export_force_sampling=True,
    export_cameras=False,
    export_lights=False,
)
