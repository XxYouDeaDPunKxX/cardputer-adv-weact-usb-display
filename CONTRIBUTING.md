# Contributing

This repository includes a reusable USB display driver, a Cardputer ADV demo,
and a prebuilt 8 MiB factory image. Each has different verification needs.
Passing native tests or compiling a sketch does not prove that a command works
on the physical WeAct panel.

## Report a problem or propose a feature

Open a GitHub issue with the expected result, the actual result, reproduction
steps, and the relevant file or command. For a hardware problem, include the
Cardputer model, WeAct panel model, reported panel firmware, and what the
**Info** page shows. Attach a log or photo when useful, after removing any
credentials, serial numbers, or personal paths.

For protocol changes or larger features, describe the intended behavior and
available hardware evidence in an issue before writing the implementation.
Check [supported hardware](docs/HARDWARE.md) and [protocol notes](docs/PROTOCOL.md).
Behavior seen on another display or in upstream source is not proof that it
works on this panel.

## Find the right files

| Change | Files | Reference |
| --- | --- | --- |
| USB commands and driver | `library/WeActUSBDisplay/src/` | [API](docs/LIBRARY_API.md), [protocol](docs/PROTOCOL.md) |
| Standalone fill example | `library/WeActUSBDisplay/examples/ColorCycle/` | [Library README](library/WeActUSBDisplay/README.md) |
| Cardputer controls and rendering | `demo/CardputerADV_WeActDemo/` | [Demo README](demo/README.md), [quick start](docs/QUICKSTART.md) |
| Native regression tests | `tests/` | `tools/run_native_tests.py` |
| Build, packaging, CI | `tools/`, `.github/workflows/` | [Build guide](docs/BUILD.md), [validation](docs/VALIDATION.md) |
| Factory firmware and redistribution | `firmware/`, `sources/`, `LICENSES/` | [Firmware README](firmware/README.md), [notices](THIRD_PARTY_NOTICES.md) |

The Cardputer demo sends full, uncompressed RGB565 frames through
`panel.drawRect()`. Its menu does **not** offer solid-color fills or compressed
drawing. The separate `ColorCycle` example calls `panel.fillScreen()`;
`drawRectFastLz()` is an unvalidated library path. Keep these distinctions
accurate in code, examples, screenshots, and docs.

## Prepare a pull request

1. Fork the repository and create a focused branch from `main`. Keep unrelated
   refactors in separate changes.
2. Add or update tests when changing protocol encoding, bounds, connection
   handling, UI actions, orientation, or rendering. The existing test programs
   are `tests/weact-api-tests.cpp` and `tests/demo-tests.cpp`.
3. Update the documentation closest to the change: behavior and keys in
   [Quickstart](docs/QUICKSTART.md), API details in
   [Library API](docs/LIBRARY_API.md), and tested scope in
   [Validation](docs/VALIDATION.md). Keep the root README concise.
4. Run the relevant checks below. Record both passing results and checks you
   could not run, with the reason.
5. Open a PR against `main` explaining what changed, why, and how it was
   checked. Include a screenshot or short video for visible behavior, or a
   reproducible byte sequence/test for protocol changes.

## Verify the change

For native driver and demo tests, run this from the repository root. It needs
Python and a C++17 compiler (`g++`, `clang++`, or MSVC):

```sh
python tools/run_native_tests.py
```

For Arduino changes, follow the pinned dependencies and build commands in
[the build guide](docs/BUILD.md). CI compiles both the Cardputer demo and
`ColorCycle`; compilation alone does not verify USB or panel behavior.

**Firmware and source changes are checked differently.**
`python tools/verify_release.py` verifies the *existing* binary hashes, image
layout, and the revision hash of their build inputs. CI runs it on each PR.
Changing a demo, driver, or build-script input can therefore make this check
fail even when the code is correct. `python tools/package_release.py` also
calls that verifier. Report the source/firmware mismatch in the PR; do not
edit manifest hashes merely to silence it or replace the factory image in an
ordinary source change. A matching firmware rebuild requires separate review
and hardware verification.

If you have the supported Cardputer ADV and WeAct FS V1 panel, report its
firmware version and exactly what you tested: for example, reconnection,
orientations, brightness, or animation. Separate observations on hardware
from native-test results. **Hardware access is not required** to submit a
source or documentation PR.

Flashing a factory image at `0x0` overwrites **all 8 MiB of flash**. Never
flash just to satisfy a contribution checklist. Read the
[flash guide](docs/FLASH.md) before a hardware test.

## Firmware, docs, and licensing

Do not commit generated `.build/` or `dist/` output. A proposal to update
prebuilt firmware needs matching binary hashes, source revision, license
notices, and recorded hardware tests of **that exact image**. See
[validation](docs/VALIDATION.md), [corresponding source](sources/README.md),
and [third-party notices](THIRD_PARTY_NOTICES.md).

Document only behavior supported by code or recorded hardware evidence. Do
not claim an experimental opcode works, attribute a library-only feature to
the Cardputer demo, or call an unflashed build hardware-tested. Correct the
relevant existing section instead of adding a second explanation elsewhere.

The project's original source and documentation use the [MIT license](LICENSE).
Retain applicable third-party copyright and license notices when adding or
changing dependencies.
