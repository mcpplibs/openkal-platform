#!/usr/bin/env python3
"""Exercise the selected platform's real seal and named debuggee runner."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tomllib


def elf_sections(image: Path) -> dict[str, tuple[int, int]]:
    data = image.read_bytes()
    assert data[:6] == b"\x7fELF\x02\x01", "the produced image is little-endian ELF64"
    offset = struct.unpack_from("<Q", data, 40)[0]
    width, count, strings = struct.unpack_from("<HHH", data, 58)
    assert width == 64 and 0 < strings < count and offset + width * count <= len(data)
    headers = [struct.unpack_from("<IIQQQQIIQQ", data, offset + i * width) for i in range(count)]
    names_header = headers[strings]
    names = data[names_header[4]:names_header[4] + names_header[5]]
    assert len(names) == names_header[5]
    result = {}
    for header in headers:
        start = header[0]
        assert start < len(names)
        end = names.find(b"\0", start)
        assert end >= start
        name = names[start:end].decode("ascii")
        if header[1] != 8:  # NOBITS has no file bytes.
            assert header[4] + header[5] <= len(data)
        result[name] = (header[1], header[5])
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--arch", required=True, choices=("x86_64", "aarch64"))
    parser.add_argument("--mcpp", default=os.environ.get("MCPP", "mcpp"))
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    fixture = root / "checks/debug-producer"
    logs = fixture / "target/producer-contract"
    logs.mkdir(parents=True, exist_ok=True)
    triple = args.arch + "-openkal"
    manifest = tomllib.loads((root / "mcpp.toml").read_text(encoding="utf-8"))
    assert manifest["platform"]["link"]["seal"] == [
        ["okld", "note", "{}"], ["okld", "place", "{}"],
        ["okld", "check", "{}", "--with", "{needed}"],
    ], "the build-program bootstrap retains its declarative seal"

    def command(label: str, words: list[str]) -> dict:
        result = subprocess.run([args.mcpp, *words], cwd=fixture, text=True, encoding="utf-8",
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=180)
        (logs / (label + ".stdout")).write_text(result.stdout, encoding="utf-8")
        (logs / (label + ".stderr")).write_text(result.stderr, encoding="utf-8")
        assert result.returncode == 0, f"{label} failed with exit {result.returncode}"
        return json.loads(result.stdout)["data"]

    environment = command("environment", ["self", "env", "--target", triple, "--format", "json"])
    selected_bin = Path(environment["toolchain"]["compiler"]).parent
    rows = []
    for profile, switches in [("dev", []), ("inspect", ["--profile", "inspect"]),
                              ("release", ["--release"])]:
        launch = command(profile, ["run", "--target", triple, *switches,
                                   "--runner", "debuggee", "--print-launch"])
        image = Path(launch["program"])
        assert image.is_file()
        assert launch["via"] == "runner" and Path(launch["argv"][0]).stem == "okrun"
        assert len(launch["argv"]) == 3 and launch["argv"][1] == "--debugger"
        assert Path(launch["argv"][2]).samefile(image)
        ninja = (image.parent.parent / "build.ninja").read_text(encoding="utf-8")
        matches = re.findall(r"platform-step seal --steps-hex ([0-9a-f]+)", ninja)
        assert len(matches) == 1
        steps = [line.split("\t") for line in bytes.fromhex(matches[0]).decode().splitlines()]
        assert len(steps) == 4, "the dynamic seal replaces rather than appends to the bootstrap"
        assert Path(steps[0][0]).stem == "llvm-objcopy"
        assert Path(steps[0][0]).parent.samefile(selected_bin), "objcopy belongs to the selected toolchain"
        assert steps[0][1:] == ["--remove-section=.rela.*", "--remove-section=!.rela.debug_*",
                                 "--remove-section=!.rela.dyn", "--remove-section=!.rela.plt", "{}"]
        for step, expected in zip(steps[1:], [["note", "{}"], ["place", "{}"],
                                             ["check", "{}", "--with", "{needed}"]]):
            assert Path(step[0]).stem == "okld" and step[1:] == expected
        sections = elf_sections(image)
        assert all(not name.startswith(".rela.") or name.startswith(".rela.debug_")
                   or name in {".rela.dyn", ".rela.plt"} for name in sections), "ordinary static relocations are pruned"
        if profile == "inspect":
            assert sections.get(".debug_info", (0, 0))[1] > 0
            assert sections.get(".rela.debug_info", (0, 0))[0] == 4
            assert sections[".rela.debug_info"][1] > 0
        if profile == "release":
            assert ".debug_info" not in sections and ".rela.debug_info" not in sections
        env = os.environ.copy()
        env.update(launch["env"])
        result = subprocess.run(launch["argv"], cwd=launch["cwd"], env=env,
                                capture_output=True, timeout=30)
        assert result.returncode == 0, f"{profile} actual debuggee admission/execution failed"
        rows.append({"profile": profile, "seal_steps": len(steps), "actual_debuggee_exit": result.returncode,
                     "retained_debug_relocations": any(name.startswith(".rela.debug_") for name in sections)})
    ordinary = command("ordinary", ["run", "--target", triple, "--profile", "inspect",
                                     "--print-launch"])
    assert len(ordinary["argv"]) == 2, "the ordinary runner has no debuggee argument"
    assert Path(ordinary["argv"][1]).samefile(ordinary["program"])
    env = os.environ.copy()
    env.update(ordinary["env"])
    result = subprocess.run(ordinary["argv"], cwd=ordinary["cwd"], env=env,
                            capture_output=True, timeout=30)
    assert result.returncode == 0
    receipt = {"profiles": rows, "ordinary_runner_exit": result.returncode,
               "scope": "Real producer/ELF/seal/launch/native runner checks; native debugger frontends are qualified separately."}
    (logs / "verified.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(receipt))


if __name__ == "__main__":
    main()
