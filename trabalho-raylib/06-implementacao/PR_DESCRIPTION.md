# [tests] Add headless regression tests for core utility functions

## Problema

raylib não tem testes automatizados das **funções da biblioteca**: o CI compila e roda análise estática; o passo `ctest` existe mas não há nenhum `add_test()`.
O `rexm test` valida exemplos (inicializa/fecha/carrega), não o resultado das funções. Resultado: bugs em funções puras só aparecem depois do uso, p.ex.

- `ComputeSHA256()` com hash errado para `tamanho mod 64 ∈ 56..59` — issue #6143 / PR #6144
- `GetFileExtension()` corrigida duas vezes em 3 dias — PRs #6312 e #6326
- `GetCodepoint()` devolvendo tamanho além do `'\0'` em sequência truncada — PR #6348 (aberto)

## O que muda

Somente arquivos novos; **nenhuma alteração em `src/`**.

- `tests/rtest.h`: helper mínimo (C99, sem dependências) com `CHECK*`, `KNOWN_BUG` (XFAIL/XPASS).
- `tests/test_hash.c` (65 verificações), `tests/test_paths.c` (29), `tests/test_utf8.c` (26, com 3 `KNOWN_BUG` do PR #6348).
- `tests/Makefile`: `make test`.
- `.github/workflows/run_tests.yml`: roda a suíte no Linux.

## Como testar

```bash
cd src && make PLATFORM=PLATFORM_DESKTOP     # Windows/MSYS2: mingw32-make
cd ../tests && make test
```

Resultado local (Windows 11, gcc 14.2.0): `ALL TEST FILES PASSED` — 120 verificações, 0 falhas, 3 XFAIL.

## Evidências

- **Oráculo independente:** hashes esperados gerados com .NET `System.Security.Cryptography` (não com o raylib); CRC-32 com valor padrão `0xCBF43926`; Base64 com RFC 4648.
- **Teste de mutação:** com `paddedSize = dataSize + 4` em `ComputeSHA256()` (cópia descartável), `test_hash` falha em 56, 57, 59, 120 e 121 e retorna código 1.
- **CI:** o workflow `Run headless tests` (Ubuntu) ficou **verde** no commit `b83e2d3`; a 1ª execução falhou por falta de `libglfw3-dev` nas dependências (corrigido neste PR).
- **Limitações:** testado no Windows/MSYS2 (local) e Linux/Ubuntu (CI), **não** no macOS; `GetFileExtension(NULL)` trava (achado, **não** coberto por teste automático).

## Fora do escopo

Testes de janela/GPU/áudio; matriz multiplataforma; integração com `add_test()`/CMake (próximo passo); correção dos bugs encontrados.

## Checklist

- [x] `make test` passa localmente (Windows/MSYS2)
- [x] Nenhum arquivo de `src/` alterado
- [ ] Revisado pelo outro integrante (ver `06-implementacao/REVISAO.md`)
- [x] Workflow de CI executado e verde em *Actions* (commit `b83e2d3`)
