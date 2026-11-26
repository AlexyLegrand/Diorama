import json
import struct
import sys

SAMPLER_WARNING = "uses fewer samplers than it contains: actions were probably mixed at export"


def load(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] != b"glTF":
        sys.exit(f"{path}: not a .glb file")
    length = struct.unpack("<I", data[12:16])[0]
    return json.loads(data[20:20 + length])


def animation_lines(gltf):
    accessors = gltf["accessors"]
    lines = []
    problems = []
    for anim in gltf.get("animations", []):
        used = {channel["sampler"] for channel in anim["channels"]}
        duration = max(accessors[anim["samplers"][i]["input"]]["max"][0] for i in used)
        lines.append(f"  {anim['name']}: {duration:.2f}s, {len(anim['channels'])} channels")
        if len(anim["samplers"]) > len(used):
            problems.append(f"  {anim['name']} {SAMPLER_WARNING} ({len(used)}/{len(anim['samplers'])})")
    return lines, problems


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: python3 tools/glb_info.py FILE.glb...")
    status = 0
    for path in sys.argv[1:]:
        gltf = load(path)
        meshes = [m.get("name") for m in gltf.get("meshes", [])]
        lines, problems = animation_lines(gltf)
        print(f"{path}")
        print(f"  meshes: {', '.join(n or '?' for n in meshes)}")
        print(f"  materials: {len(gltf.get('materials', []))}, skins: {len(gltf.get('skins', []))}")
        print("\n".join(lines) if lines else "  no animation")
        if problems:
            print("WARNING")
            print("\n".join(problems))
            status = 1
    sys.exit(status)


if __name__ == "__main__":
    main()
