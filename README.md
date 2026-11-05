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
| link | `ld.lld`, entry `ok_main`, 64 KiB pages, relative relocations packed, `--gc-sections --icf=safe`, `--build-id=sha1`, the image notes produced by `okld note-source` |
| seal | `okld note`; `okld place`, the writable segments computed for the base the image's bytes choose (openkal 0.23); then `okld check --with` the shared images the program needs |
| naming | `{name}.oke`, `lib{name}.okso` |
| runner | `okrun {}`, on a machine of the image's architecture; `hosts` the systems and architectures okrun is built for, where a package that needs it can run |
| carrier | openkal-run, section `.oke`, entry `carrier/main.cpp`: with nothing carried the runner of a package, with an image or a zip of images the program that runs them from memory |
| plugin | `build.mcpp` provides the `okb` package format through `okld bundle --package` |
| hosted | the toolchain and the layers for the host triples, over the system's openkal implementation |
| versions | the release group of the openkal family |

Platform 0.23.6 also sets emulated TLS for the linker's LTO code generation.
Compiling bitcode with `-femulated-tls` alone does not retain that choice at the
final link. Both ordinary object files and ThinLTO builds use the platform's
emulated thread-local storage.

Platform 0.23.7 selects openkal-musl 0.27.6 and openkal-llvm-runtime 0.18.7.
The C library calls the optional resolved-name operation directly, preserving
its fallback, error and output-bound checks.

A project overrides any group under `[target.<selector>]`, for example
`[target.'cfg(platform = "openkal")'.layers] "c++-abi" = false`, and selects
another platform, or none, with `platform = "..."` or `--platform`.

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

`[platform.versions]` names one version of each package of the family. The
packages pin one another exactly; a graph holding another version of one of
them is refused, naming the group and the two remedies.

## Checks

| Check | What it asserts |
|---|---|
| `mcpp test` | the test image runs on the machine's architecture; the other architecture is built |
| `checks/image` | a project without declarations builds, runs and packs (M2 and M3) |
| `checks/hosted` | the hosted arrangement runs on Linux, macOS and Windows |
| `checks/host-graph` | a build program runs as an ok image |
| `checks/snapshot` | a C++ program starts from a snapshot of its initialisers, and a start whose named value differs does not |

## License

Apache-2.0.
