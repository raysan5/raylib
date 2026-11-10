# [tests] Add headless regression tests for core utility functions

<!-- Texto do Pull Request enviado a raysan5/raylib (em inglês, curto, como pede o CONTRIBUTING.md).
     Título = 1ª linha acima. Descrição = o bloco abaixo, sem este comentário. -->

raylib has no automated tests for its library functions. Bugs in small pure
functions have reached users before (#6143 SHA-256, #6312/#6326 GetFileExtension,
#6348 GetCodepoint).

This PR adds headless tests (no window/GPU) for hash/encoding functions, path
helpers and UTF-8 functions, a `make test` target in `tests/`, and a small Linux
CI workflow. No changes in `src/`.

To run: `cd src && make PLATFORM=PLATFORM_DESKTOP`, then `cd ../tests && make test`
(120 checks pass). Tested on Windows (MSYS2) and Ubuntu (CI), not on macOS.

Parts of the code were written with AI assistance (Claude Code); we reviewed and ran everything.
