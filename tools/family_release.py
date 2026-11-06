#!/usr/bin/env python3
"""The release of the openkal family's version group (mcpp design 2026-11-07, §4.2.2, A5 and A6).

The group is `[platform.versions]` of this repository's mcpp.toml, with the
platform itself; `--also NAME@VERSION` adds a package released with it (os). A
member is released by its own repository: a tag equal to its version, the
GitHub archive of the tag, and the same bytes on GitCode (mcpp-res/<repo>). The
commands state facts about the group; only `index` writes, into a checkout of
mcpp-index it is given.

  check              each member's tag, archive and mirror, and whether the
                     published index lists it already
  index DIR          adds the members the index lacks to the mcpp-index
                     checkout DIR: the whole group as one change, the content
                     of its one index PR
  defects            the open issues of each member's repository, for the
                     release notes
  exports OLD NEW    builds one image with the platform at OLD and at NEW and
                     compares the global symbols of the C library and the C++
                     run-time objects it links: a patch version removes none

Network reads use GITHUB_TOKEN when it is set.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import tomllib
import urllib.error
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
# The packages one repository releases under one tag and one archive.
REPOSITORY = {"openkal-ld": "openkal-image", "openkal-run": "openkal-image",
              "openkal-tools": "openkal-image", "openkal-bind": "openkal-image"}
INDEX_RAW = "https://raw.githubusercontent.com/mcpplibs/mcpp-index/main/pkgs/o/{}.lua"
# The layers whose objects an image links, by the directory mcpp builds them into.
LAYERS = ("mcpplibs_openkal-musl", "mcpplibs_openkal-llvm-runtime")


def group(also):
    manifest = tomllib.loads((ROOT / "mcpp.toml").read_text(encoding="utf-8"))
    members = dict(manifest["platform"]["versions"])
    members["openkal-platform"] = manifest["package"]["version"]
    for spec in also:
        name, version = spec.split("@", 1)
        members[name] = version
    return members


def fetch(url, binary=True):
    request = urllib.request.Request(url, headers={"User-Agent": "openkal-family-release"})
    token = os.environ.get("GITHUB_TOKEN")
    if token and url.startswith("https://api.github.com/"):
        request.add_header("Authorization", f"Bearer {token}")
    try:
        with urllib.request.urlopen(request, timeout=120) as response:
            data = response.read()
    except (urllib.error.HTTPError, urllib.error.URLError):
        return None
    return data if binary else data.decode("utf-8")


def urls(name, version):
    repo = REPOSITORY.get(name, name)
    return (f"https://github.com/mcpplibs/{repo}/archive/refs/tags/{version}.tar.gz",
            f"https://gitcode.com/mcpp-res/{repo}/releases/download/{version}/{repo}-{version}.tar.gz")


def listed(descriptor, version):
    """The sha256 the descriptor states for `version`, or None."""
    m = re.search(r'\["' + re.escape(version) + r'"\]\s*=\s*\{.*?sha256\s*=\s*"([0-9a-f]{64})"', descriptor, re.S)
    return m.group(1) if m else None


def check(members):
    rows, problems = [], 0
    digests = {}
    for name, version in members.items():
        github, gitcode = urls(name, version)
        repo = REPOSITORY.get(name, name)
        if repo in digests and digests[repo][0] == version:
            a = b = digests[repo][1]
        else:
            data = fetch(github)
            a = hashlib.sha256(data).hexdigest() if data else None
            mirror = fetch(gitcode)
            b = hashlib.sha256(mirror).hexdigest() if mirror else None
            digests[repo] = (version, a)
        descriptor = fetch(INDEX_RAW.format(name), binary=False) or ""
        indexed = listed(descriptor, version)
        state = ("no tag" if a is None else "no mirror" if b is None else "mirror differs" if a != b
                 else "indexed" if indexed == a else "index differs" if indexed else "not indexed")
        problems += state in ("no tag", "no mirror", "mirror differs", "index differs")
        rows.append((name, version, state, a or "-"))
    print("| member | version | state | sha256 |\n|---|---|---|---|")
    for row in rows:
        print("| {} | {} | {} | `{}` |".format(*row))
    return 1 if problems else 0


def entry(version, github, gitcode, sha):
    return (f'            ["{version}"] = {{\n'
            f'                url = {{\n'
            f'                    GLOBAL = "{github}",\n'
            f'                    CN = "{gitcode}",\n'
            f'                }},\n'
            f'                sha256 = "{sha}",\n'
            f'            }},\n')


def index(members, checkout):
    added = []
    for name, version in members.items():
        path = checkout / "pkgs/o" / f"{name}.lua"
        text = path.read_text(encoding="utf-8")
        if listed(text, version):
            continue
        github, gitcode = urls(name, version)
        data, mirror = fetch(github), fetch(gitcode)
        if not data or not mirror or hashlib.sha256(data).digest() != hashlib.sha256(mirror).digest():
            sys.exit(f"{name} {version}: the tag or its mirror is missing, or they differ; run `check`")
        sha = hashlib.sha256(data).hexdigest()
        # Newest first in each system's table, as the descriptors are written.
        text, n = re.subn(r'(\n\s*(?:linux|macosx|windows)\s*=\s*\{\n)',
                          lambda m: m.group(1) + entry(version, github, gitcode, sha), text)
        if n == 0:
            sys.exit(f"{path}: no system table to add {version} to")
        path.write_text(text, encoding="utf-8")
        added.append(f"{name} {version}")
    print("added: " + (", ".join(added) if added else "nothing, the index lists the group"))
    return 0


def defects(members):
    seen = set()
    print("## Open issues of the group's repositories\n")
    for name in members:
        repo = REPOSITORY.get(name, name)
        if repo in seen:
            continue
        seen.add(repo)
        text = fetch(f"https://api.github.com/repos/mcpplibs/{repo}/issues?state=open&per_page=100", binary=False)
        if text is None:
            print(f"- {repo}: the issues could not be read")
            continue
        issues = [i for i in json.loads(text) if "pull_request" not in i]
        for issue in issues:
            print(f"- {repo}#{issue['number']}: {issue['title']}")
        if not issues:
            print(f"- {repo}: none")
    return 0


def symbols(mcpp, platform, target, work, nm):
    project = work / platform.replace("@", "-")
    (project / "src").mkdir(parents=True, exist_ok=True)
    (project / "mcpp.toml").write_text('[package]\nname = "exports"\nversion = "0.1.0"\n', encoding="utf-8")
    (project / "src/main.cpp").write_text(
        'import std;\nint main() { std::vector<std::string> v{"a"}; std::println("{}", v.size()); }\n',
        encoding="utf-8")
    subprocess.run([mcpp, "build", "--target", target, "--platform", platform], cwd=project, check=True)
    found = {}
    for layer in LAYERS:
        objects = [p for p in (project / "target").rglob("*.o") if layer in p.parts]
        if not objects:
            sys.exit(f"{platform}: the build linked no objects of {layer}")
        out = subprocess.run([nm, "--defined-only", "--extern-only", "--format=just-symbols", *map(str, objects)],
                             check=True, capture_output=True, text=True).stdout
        found[layer] = {s for s in out.split() if s and not s.endswith(":")}
    return found


def exports(old, new, mcpp, target, nm):
    with tempfile.TemporaryDirectory(prefix="openkal-exports-") as directory:
        work = Path(directory)
        before = symbols(mcpp, old, target, work, nm)
        after = symbols(mcpp, new, target, work, nm)
    problems = 0
    for layer in LAYERS:
        removed = sorted(before[layer] - after[layer])
        gained = sorted(after[layer] - before[layer])
        print(f"{layer}: {len(before[layer])} -> {len(after[layer])} global symbols, "
              f"{len(gained)} added, {len(removed)} removed")
        for symbol in removed:
            print(f"  removed: {symbol}")
        problems += len(removed)
    return 1 if problems else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--also", action="append", default=[], metavar="NAME@VERSION")
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("check")
    p = sub.add_parser("index")
    p.add_argument("checkout", type=Path)
    sub.add_parser("defects")
    p = sub.add_parser("exports")
    p.add_argument("old", help="openkal-platform@<version> released before")
    p.add_argument("new", help="openkal-platform@<version> of this group")
    p.add_argument("--mcpp", default="mcpp")
    p.add_argument("--target", default="x86_64-openkal")
    p.add_argument("--nm", default=shutil.which("llvm-nm") or "llvm-nm")
    args = parser.parse_args()
    members = group(args.also)
    if args.command == "check":
        return check(members)
    if args.command == "index":
        return index(members, args.checkout.resolve())
    if args.command == "defects":
        return defects(members)
    return exports(args.old, args.new, args.mcpp, args.target, args.nm)


if __name__ == "__main__":
    sys.exit(main())
