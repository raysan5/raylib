# [tests] Add headless regression tests for core utility functions

<!-- Texto do Pull Request para raysan5/raylib (em inglês, porque é lido pelo mantenedor).
     Cole a 1ª linha acima como TÍTULO do PR e o restante como DESCRIÇÃO. -->

## Problem

raylib has no automated tests for the **library functions**. CI builds and runs static analysis, and a `ctest` step exists in `build_cmake.yml`, but there is no `add_test()` anywhere. `rexm test` validates the examples (they initialize/close/load), not the results of the functions.

As a consequence, bugs in small pure functions are only found after users hit them, for example:

- `ComputeSHA256()` returning a wrong hash for lengths where `size mod 64` is in `56..59` — issue #6143 / PR #6144
- `GetFileExtension()` fixed twice in 3 days — PRs #6312 and #6326
- `GetCodepoint()` returning a size past the `'\0'` on truncated UTF-8 — PR #6348 (fixed in 7e626d65)

## What changes

Only new files. **Nothing in `src/` is modified.**

- `tests/rtest.h`: minimal helper (C99, no dependencies) with `CHECK`, `CHECK_INT`, `CHECK_STR` and `KNOWN_BUG` (reports XFAIL/XPASS so a known upstream bug does not turn CI red, and a fix is noticed automatically).
- `tests/test_hash.c` (65 checks): `ComputeSHA256/SHA1/MD5` at 14 boundary lengths, `ComputeCRC32`, Base64.
- `tests/test_paths.c` (29 checks): `GetFileName`, `GetFileExtension`, `IsFileExtension`, `GetFileNameWithoutExt`, `IsPathAbsolute`.
- `tests/test_utf8.c` (26 checks): `GetCodepoint` (valid / invalid / truncated), `GetCodepointCount`, `CodepointToUTF8`. It includes a regression test for #6348.
- `tests/Makefile`: `make test`.
- `.github/workflows/run_tests.yml`: runs the suite on Linux.

All tests are **headless**: no window and no OpenGL context are created.

## How to test

```bash
cd src && make PLATFORM=PLATFORM_DESKTOP     # Windows/MSYS2: mingw32-make
cd ../tests && make test
```

Expected result: `ALL TEST FILES PASSED` (120 checks, 0 failed). `RAYLIB_SRC` can point to another folder containing `raylib.h` and `libraylib.a` (default `../src`).

## Evidence

- **Independent oracle:** expected SHA-256/SHA-1/MD5 values were generated with .NET `System.Security.Cryptography` (not with raylib); CRC-32 uses the standard check value `0xCBF43926`; Base64 uses the RFC 4648 vectors.
- **Mutation test:** changing `paddedSize = dataSize + sizeof(bitLen)` to `dataSize + 4` in `ComputeSHA256()` (on a throw-away copy) makes `test_hash` fail at lengths 56, 57, 59, 120 and 121 and exit with code 1.
- **Known-bug tracking works:** while #6348 was open, `test_utf8` reported 3 XFAIL. After rebasing on the current `master` (which contains the fix), they became XPASS, and they were promoted to regular `CHECK`s in this PR.
- **CI:** the `Run headless tests` workflow (Ubuntu) runs green on my fork (the first run failed only because `libglfw3-dev` was missing; it is installed now, like the other workflows do).
- **Limitations:** tested on Windows 11 (MSYS2 gcc 14.2) and Ubuntu (CI), **not** on macOS. While writing the tests I also noticed that `GetFileExtension(NULL)` crashes (`GetFileName(NULL)` returns `NULL` and `strrchr(NULL, '.')` follows); this is **not** covered by an automated test because it would abort the process. I can open a separate issue for it if that is useful.

## Out of scope

Window/GPU/audio tests; a multi-platform CI matrix; integrating the tests with `add_test()` / CMake (a natural next step); fixing the bugs found.

## Notes

- Parts of this change (tests and documentation) were drafted with AI assistance (Claude Code). The authors reviewed the code, built it and ran it before submitting.

## Checklist

- [x] `make test` passes locally (Windows/MSYS2)
- [x] No file in `src/` is changed
- [x] CI workflow runs green on my fork
