# openkal-platform

The platform package of openkal (mcpp SPEC-013). It states, in its
`mcpp.toml`, the configuration of the targets `x86_64-openkal` and
`aarch64-openkal`: the toolchain, the C library, the C++ run-time, the
compiler run-time, the tools, the flags, the link, the seal, the runner and
the carrier of a self-contained program. mcpp selects it for both targets
through its target table, so a project for an ok image declares nothing but
its sources:

```toml
[package]
name    = "app"
version = "0.1.0"
```

```bash
mcpp run  --target x86_64-openkal      # built as app.oke, run by okrun
mcpp test --target aarch64-openkal
mcpp pack --target x86_64-openkal      # one package: every architecture, launchers
mcpp pack --target x86_64-openkal --mode self-contained   # the runner inside, nothing to install
mcpp pack --target x86_64-openkal --pkg-format single-file
mcpp pack --target x86_64-openkal --pkg-format okb        # one file okrun runs on any system
```

## What the platform states

| Group | Value |
|---|---|
| targets | `x86_64-openkal`, `aarch64-openkal` |
| toolchain | `llvm@23.1.3` |
| layers | `c-abi` openkal-musl (`posix`), `c++-abi` and `compiler-runtime` openkal-llvm-runtime |
| tools | `okld`, `okrun`, `okprof` (openkal-tools) |
| build | `-fPIC -femulated-tls -fvisibility=hidden -fasynchronous-unwind-tables`, stack-clash protection, `-ffunction-sections -fdata-sections`; frame pointers, `-fstack-protector-strong -mstack-protector-guard=global`, `-ftrivial-auto-var-init=zero`, libc++'s fast hardening (0.23.0); on x86_64 `-fcf-protection=full`; on aarch64 `-ffixed-x18 -mno-outline-atomics -mbranch-protection=pac-ret+bti` (not `standard`, which clang 23 counts the guarded control stack in) |
| link | `ld.lld`, entry `ok_main`, 64 KiB pages, relative relocations packed, `--gc-sections --icf=safe`, `--build-id=sha1`, retained debug relocations (0.24.0), the image notes produced by `okld note-source` |
| seal | the selected LLVM toolchain's `llvm-objcopy` prunes ordinary static relocations (0.24.0); `okld note`; `okld place`, the writable segments computed for the base the image's bytes choose (openkal 0.23); then `okld check --with` the shared images the program needs |
| naming | `{name}.oke`, `lib{name}.okso` |
| runner | `okrun {}`, on a machine of the image's architecture; `hosts` the systems and architectures okrun is built for, where a package that needs it can run |
| runner `debuggee` | `okrun --debugger {}` for a host debugger (0.24.0) |
| carrier | openkal-run, section `.oke`, entry `carrier/main.cpp`: with nothing carried the runner of a package, with an image or a zip of images the program that runs them from memory |
| plugin | `build.mcpp` provides the `okb` package format through `okld bundle --package` and selects the toolchain program used by the image seal |
| hosted | the toolchain and the layers for the host triples, over the system's openkal implementation |
| versions | the release group of the openkal family |

Platform 0.23.6 also sets emulated TLS for the linker's LTO code generation.
Compiling bitcode with `-femulated-tls` alone does not retain that choice at the
final link. Both ordinary object files and ThinLTO builds use the platform's
emulated thread-local storage.

Platform 0.24.0 selects openkal-musl 0.28.0, openkal-llvm-runtime 0.19.0
and image bindings 0.5.0 over the OpenKal 0.24 implementations.
The C library calls the optional resolved-name operation directly, preserving
its fallback, error and output-bound checks.

Platform 0.24.1 corrects that group: openkal-musl 0.28.1 keeps a directory's
whole name however long, openkal-llvm-runtime 0.19.1 reads a name beneath a
Windows volume as absolute, openkal-macos 0.22.1 sizes its confinement rules to
the grants of each start, and openkal-image 0.5.1 builds okrun over it.

Platform 0.24.2 changes one member: openkal-llvm-runtime 0.19.2 keeps the volume
of a name it rebuilds part by part (`lexically_normal`, `weakly_canonical`, and
`D:/a` / `/b`), which 0.19.1 lost on Windows, so that a launcher over the hosted
arrangement could not start its runner (openkal-llvm-runtime#68). It is the first
group released as a leaf and a patch of this group, through one index change.

Platform 0.24.3 changes one member: openkal-image 0.5.2, whose okrun.exe reserves
8 MiB + 256 KiB for the image's main thread, a thread's stack under
openkal-windows, where the linker's default was 1 MiB (an image recursing deeply
on its main thread, clangd.okb with `--sync` over the MSVC headers, overflowed
it on Windows; openkal-image#32).

Platform 0.24.4 changes one member: openkal-image 0.5.3, whose runner enters an
image in its convention's floating-point state (openkal SPEC 12.2): the x87 at
extended precision and MXCSR 0x1F80 on x86-64, FPCR zero on AArch64. A thread of
Windows begins with the x87 at double precision, under which openkal-musl's
`strtod("0.4")` read `0x1.99999999998p-2` (openkal-image#33).

Platform 0.24.5 changes one member: openkal-image 0.5.4, whose runner states
its openkal implementation as a capability (`mcpp:kernel-abi=openkal`) with a
default per system: an implementation the graph already has is the one bound,
and a system a platform defines needs no patch of openkal-run. The platform
requires mcpp 2026.11.9.1, which reads `[defaults]`.

Platform 0.25.0 is the openkal 0.25 family: openkal 0.25.0 (a second name for a
file, `kal_fs_hard_link`; the requests to end counted), openkal-linux 0.26.0,
openkal-macos 0.23.0, openkal-windows 0.21.0, openkal-musl 0.29.0, which finds a
program by the search rules of the system `kal_env_system` names and implements
`link` over `kal_fs_hard_link`, openkal-llvm-runtime 0.20.0 above it, and
openkal-image 0.5.5, built against the same family. The platform requires mcpp
2026.11.9.1, as 0.24.5 does.

Platform 0.26.0 adds FreeBSD and the interface-bound runner. Its hosted
arrangement serves `x86_64-freebsd-musl` and `aarch64-freebsd-musl`: openkal-musl
0.29.2 over openkal-freebsd 0.1.0 in its standalone arrangement, with
openkal-llvm-runtime 0.20.1 above it, as one static program with no C library of
the system beneath. openkal-image 0.6.1 binds only the interfaces its
implementation provides, and its okrun is built for FreeBSD as well, so
`freebsd-x86_64` and `freebsd-aarch64` are hosts of the runner. The platform
requires mcpp 2026.11.10.1; mcpp 2026.11.10.2 and later select it for the
FreeBSD musl rows by default.

Platform 0.27.0 states the identity its runner is built as on FreeBSD
(`runner.built-as`): `x86_64-freebsd-musl` and `aarch64-freebsd-musl`. mcpp
2026.11.12.1 reads the statement, and a package that carries its runner for
every system (`mcpp pack --mode self-contained --systems all`) carries
FreeBSD's; before, which identity a system's runner is was a table of the
engine. The version group is that of 0.26.0. An mcpp earlier than 2026.11.12.1
names the key it does not read and ignores it.

Platform 0.27.1 is a patch of the group. The implementations retire a handle
before closing it: openkal-linux 0.26.1, openkal-macos 0.23.1 and
openkal-windows 0.21.1. A connection, a listener and a datagram socket were
closed first, so a file another thread opened at that moment was left with a
stale handle, and above openkal-musl its reads and writes reached descriptor 0
(openkal-linux#53). openkal-musl 0.29.4 delivers the byte a `poll` held together
with what else has arrived from `recv`, `recvfrom` and `recvmsg`, as `read`
does; before, a program that polls before each `recv` took a stream one byte
per call (openkal-musl#52). The group stays on openkal-windows 0.21, the line
openkal-run and mcpplibs.os name. The platform's statements are those of 0.27.0.

Platform 0.28.0 adds Linux's thirty-two bit row, `i686-linux-musl`, to the hosted
arrangement (openkal 0.25.1, clause 5.5): openkal-musl 0.29.5 over openkal-linux
0.26.3's i386 system-call layer, with openkal-llvm-runtime 0.20.2 above it. A
Linux x86_64 machine runs its programs itself; CI builds and runs a hosted
program there and the specification's conformance suite through the platform.
The group follows openkal 0.25.1, and its implementations declare the
specification by its minor version (openkal-macos 0.23.2, openkal-windows 0.21.2,
openkal-freebsd 0.1.2; openkal-image 0.6.2), so a project that names openkal
0.25.1 resolves with them.

A project overrides any group under `[target.<selector>]`, for example
`[target.'cfg(platform = "openkal")'.layers] "c++-abi" = false`, and selects
another platform, or none, with `platform = "..."` or `--platform`.

## Source debugging

Platform 0.24.0 retains the relocation information used by image 0.5.0's
debugger object. Debug information follows the selected build profile, including
custom profiles; the seal preserves ELF zlib compression. A profile with
`debug = false` produces no source debug information.

The `debuggee` runner supplies the command for a host debugger:

```bash
mcpp run --target x86_64-openkal --runner debuggee --print-launch
mcpp run --target x86_64-openkal --profile inspect --runner debuggee --print-launch
```

The second command uses a custom profile declared as follows:

```toml
[profile.inspect]
opt = 0
debug = true
split_debuginfo = "off"
```

The launch descriptor includes the runner, `--debugger`, the image, working
directory and environment. A debugger starts that command. GDB reads the JIT
interface; an LLDB session enables it with
`settings set plugin.jit-loader.gdb.enable on` before starting the program.
This named runner is a debugged program, not a board debug-server command.

The verified source-debugger combination is Linux x86-64 with the GDB and LLDB
bundled in CLion 2026.1.5.2. A release without DWARF retains symbol-level
breakpoints where its symbols remain. External debug sidecars and native
debugger frontends on other hosts are not qualified by this check.

## What an image is observed and held by (0.23.0)

Every frame of an image is a link of the frame-pointer chain, which the
implementation's sampler and fault report walk (openkal 0.23); a frame with an
array or an address taken is guarded by a variable the C library sets from the
environment's randomness, since an image has no thread pointer to keep a guard
in; a local variable read before it is written reads zero; libc++ checks what
its fast mode checks; indirect branches land on marks and returns are checked
(CET on x86_64; pac-ret and BTI on aarch64) where the system enforces them, and
are no-ops where it does not. An image is named by its build-id, by which a
debugger finds what `okld debug-split` keeps apart.

0.23.1 names openkal-macos 0.21.3, whose answers to an initialiser that runs
before its own are the loader's record: a hosted program on macOS, whose C
library's initialiser reads its directories first, is given the directories it
was granted rather than the implementation's defaults (openkal-musl 0.27.1,
openkal-llvm-runtime 0.18.2 and openkal-image 0.4.2 are built over it).

0.23.2 names openkal-musl 0.27.2 and openkal-llvm-runtime 0.18.3. The C
library's image dependency selects openkal-ld 0.4.2, matching the loader in
this platform's version group. The earlier 0.23.1 archive remains unchanged.

0.23.3 names openkal-linux 0.24.1, openkal-musl 0.27.3,
openkal-llvm-runtime 0.18.4 and all four image packages at 0.4.3. The Linux
implementation and the native runner require Landlock ABI 3 before claiming
confinement of read-only grants; earlier ABIs cannot restrict file truncation.
The ABI probe cache uses atomic accesses during concurrent first queries.

These words reach every unit of the target, the std module included, from mcpp
2026.11.3.1 on, which this release requires (`[package] mcpp`): clang records
`-fcf-protection` in a module's BMI, and an earlier mcpp compiled the std module
without the platform's words, so every unit that imported it on x86_64 was
refused.

An image of this platform states its placement, so a runner places it at a base
of the boot and can make a snapshot of its initialisers, which openkal-musl and
openkal-llvm-runtime take part in (`checks/snapshot`; openkal-image
`docs/snapshot.md`). The pages a start executes first are a property of a
program's use, not of its build: the producer of an image that has measured them
(`okprof record --what touch`) writes them with `okld touch` before it signs or
publishes the image, and lays its code out by `okprof report --order` through
its own link flag (`--symbol-ordering-file`).

## The hosted arrangement and the host graph

A host triple that selects this platform takes `[platform.hosted]`: the
toolchain, the C library and the C++ run-time of the family over the system's
own openkal implementation (`checks/hosted`). `[host] platform` selects it for
the host graph: build programs are then compiled as ok images of the machine's
architecture and run by okrun (`checks/host-graph`).

## Version group

`[platform.versions]` names one version of each package of the family. From
0.24.1 the packages name one another by compatible ranges (`^0.28.0`), and the
group chooses: mcpp 2026.11.7.1 and later resolve each member to the group's
version, and an earlier mcpp refuses a graph holding another version of one of
them, naming the group and the two remedies. A patch version of a member keeps
every symbol its previous patch defines.

A fix of one member is that member's release and a patch of this group. The
group is released as one index change:

```bash
python3 tools/family_release.py check            # tags, GitCode mirrors, the index
python3 tools/family_release.py defects          # open issues, for the release notes
python3 tools/family_release.py index ../mcpp-index   # the group's one index change
python3 tools/family_release.py exports openkal-platform@0.24.1 openkal-platform@0.24.2
```

`.github/workflows/family-release.yml` runs them on every change of the group:
it keeps the index change as an artifact, resolves an image of the group with
the released mcpp and the one under review over the candidate index, and
compares the global symbols of the C library and the C++ run-time with the
previous patch's.

## Checks

| Check | What it asserts |
|---|---|
| `mcpp test` | the test image runs on the machine's architecture; the other architecture is built |
| `checks/image` | a project without declarations builds, runs and packs (M2 and M3) |
| `checks/hosted` | the hosted arrangement runs on Linux, macOS and Windows |
| `checks/host-graph` | a build program runs as an ok image |
| `checks/snapshot` | a C++ program starts from a snapshot of its initialisers, and a start whose named value differs does not |
| family release | the group's members are tagged and mirrored, both mcpp releases resolve an image of it, and a patch removes no symbol |

## License

Apache-2.0.
