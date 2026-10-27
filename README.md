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
| tools | `okld`, `okrun` (openkal-tools) |
| build | `-fPIC -femulated-tls -fvisibility=hidden -fasynchronous-unwind-tables`, stack-clash protection, `-ffunction-sections -fdata-sections`; on aarch64 `-ffixed-x18 -mno-outline-atomics` |
| link | `ld.lld`, entry `ok_main`, 64 KiB pages, `--gc-sections --icf=safe`, the image notes produced by `okld note-source` |
| seal | `okld note`, then `okld check --with` the shared images the program needs |
| naming | `{name}.oke`, `lib{name}.okso` |
| runner | `okrun {}`, on a machine of the image's architecture |
| carrier | openkal-run, section `.oke`, entry `carrier/main.cpp`: with nothing carried the runner of a package, with an image or a zip of images the program that runs them from memory |
| universal | `mcpp pack` of either target packs both into one package (mcpp SPEC-014 §3) |
| plugin | `build.mcpp` provides the `okb` package format through `okld bundle --package` |
| hosted | the toolchain and the layers for the host triples, over the system's openkal implementation |
| versions | the release group of the openkal family |

A project overrides any group under `[target.<selector>]`, for example
`[target.'cfg(platform = "openkal")'.layers] "c++-abi" = false`, and selects
another platform, or none, with `platform = "..."` or `--platform`.

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

## License

Apache-2.0.
