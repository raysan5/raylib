# Revisão do Pull Request (feita pelo outro integrante)

> **Este arquivo é um roteiro + registro a preencher.** O enunciado exige revisão realizada pelo outro integrante;
> nenhum resultado abaixo foi preenchido por nós — só vale depois que a pessoa revisora **executar** os passos.

- **Autor do PR:** `TODO`
- **Revisor(a):** `TODO` (não pode ser o autor)
- **Link do PR:** https://github.com/raysan5/raylib/pull/6400 (PR ao projeto original, **fechado pelo mantenedor em 10/11/2026 sem mesclar**; o código revisado continua na branch `feat/headless-tests`. PR interno de revisão no fork: https://github.com/FaMarcolino/raylib/pull/1)
- **Data da revisão:** `TODO`

## Roteiro de revisão

### A. Ler e entender (sem executar)
- [ ] O PR toca apenas `tests/` e `.github/workflows/run_tests.yml` (nenhum `src/`)?
- [ ] Cada valor esperado em `test_hash.c` tem origem independente do raylib? (conferir comentário no topo e recalcular 2 valores à mão — ver C.)
- [ ] `KNOWN_BUG` (se houver) cita issue/PR real? Abrir #6348 e confirmar que foi mesclado (commit `7e626d65`) e que o teste correspondente hoje é `CHECK`, não `KNOWN_BUG`.
- [ ] O código dos testes segue `CONVENTIONS.md` (4 espaços, sem TAB, comentário antes da linha, `float x.xf`)?

### B. Executar
```bash
cd src && make PLATFORM=PLATFORM_DESKTOP
cd ../tests && make test
```
- [x] Saída final `ALL TEST FILES PASSED`. Contagens: `test_hash` 65 · `test_paths` 29 · `test_utf8` 26 (0 falhas, 0 XFAIL, 0 XPASS). Ambiente do revisor: Windows, MSYS2 UCRT64, gcc 16.2.0, GNU Make 4.4.1, em 10/11/2026.

### C. Conferir um valor por conta própria
PowerShell (SHA-256 de `'a'` × 56 deve ser `b35439a4…6738a`):
```powershell
$b=[byte[]]@(0x61)*56; ([BitConverter]::ToString([Security.Cryptography.SHA256]::Create().ComputeHash($b))).Replace('-','').ToLower()
```
- [ ] Bateu com o esperado em `test_hash.c`?

### D. Teste de mutação (o teste realmente pega o bug?)
1. Crie uma **cópia** de `src/` (não altere o original).
2. Em `rcore.c`, troque `unsigned long long paddedSize = dataSize + sizeof(bitLen);` por `... = dataSize + 4;`
3. Apague os objetos da cópia (`rm -f *.o libraylib.a`), recompile **gravando a biblioteca dentro da cópia** (`make PLATFORM=PLATFORM_DESKTOP RAYLIB_RELEASE_PATH=.`; sem isso o Makefile grava em `../src/` e sobrescreve a biblioteca **original**) e rode `test_hash` apontando `RAYLIB_SRC` para a cópia.
- [x] `test_hash` falhou nos comprimentos 56, 57, 59, 120 e 121 (`65 checks, 5 failed`; `make` terminou com `Error 1`); 0, 1, 3, 55, 60, 63, 64, 65 e 119 seguiram `ok`. Ao restaurar a biblioteca original, voltou a `ALL TEST FILES PASSED`.
  - **Nota honesta:** a 1ª tentativa foi **inválida** — o `make` gravou a biblioteca mutada em `../src/` e os testes linkaram com a cópia original, então passaram. O revisor percebeu, reconstruiu a biblioteca original e refez com `RAYLIB_RELEASE_PATH=.` (o roteiro acima já foi corrigido).

### E. Pontos de atenção que a revisão deve avaliar
- Testes de caminho absoluto dependem de `#ifdef _WIN32` — ok para Linux/macOS?
- O workflow de CI: abrir a aba *Actions* e conferir se está verde no commit mais recente do PR. (Histórico, com os hashes de antes do rebase de 10/11/2026: a 1ª execução, no commit `9657563`, falhou; a do commit `b83e2d3`, com `libglfw3-dev`, passou.) Revisor: confirmar isso você mesmo(a) na aba *Actions*.
- Algum teste "trava" o processo? (`GetFileExtension(NULL)` **não** deve estar na suíte.)

## Parecer

- **Resultado:** `TODO: aprovado / aprovado com ressalvas / mudanças solicitadas`
- **Comentários principais (copiar do PR):** `TODO`
- **Mudanças feitas após a revisão (commits):** `TODO`
