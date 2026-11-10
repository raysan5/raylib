# 5 — A evolução proposta: testes de regressão headless para o núcleo utilitário 

## 5.1 Proposta

| Campo | Conteúdo |
|---|---|
| **Problema** | O raylib **não possui testes automatizados das funções da biblioteca**. As verificações existentes são: build no CI, análise estática (clang-tidy, CodeQL), *smoke test* dos exemplos (`tools/rexm`) e testes manuais da comunidade (#5664) — todas verificam "compila/inicializa/fecha", **não se o resultado de uma função está correto**. O exigido de um PR é "o build não quebra" (`CONTRIBUTING.md:63-64`). Funções puras e simples — hash, caminhos, UTF-8 — regridem e são corrigidas só depois que um usuário reporta. |
| **Usuário afetado** | **Persona 3** (contribuidor/mantenedor): sem rede de segurança, cada PR depende de revisão humana. **Persona 4** (autor de binding): comportamento que muda entre versões sem ser fixado por teste. **Persona 1/2** (usuários finais): recebem funções com bug. |
| **Evidência** | (1) **"Gancho" de testes vazio**: `enable_testing()` em `CMakeLists.txt:57` e `src/CMakeLists.txt:194`; o CI executa `ctest` em `build_cmake.yml:67` e `:111`; porém **nenhum `add_test()`** existe em `.txt/.cmake/.zig/.yml` (fora de `src/external`) nem há pasta/arquivo de testes `[L]` (busca refeita após uma primeira busca incompleta — ver `08-ia/ia.md`). Os demais workflows só compilam/analisam. (2) **`ComputeSHA256()` errada** para tamanhos `mod 64 ∈ 56..59`: issue **#6143** + PR **#6144** (12/09/2026), registrado no CHANGELOG 6.2 com `-WARNING-` `[GH][L]`. (3) **`GetFileExtension()` corrigida duas vezes em 3 dias**: PRs **#6312** (21/10) e **#6326** (24/10/2026) `[GH]`. (4) **`GetCodepoint()` com tamanho inválido** lendo além do fim da string: PR **#6348** (aberto, 30/10/2026) `[GH]`. (5) Outros ajustes de funções utilitárias do núcleo no CHANGELOG 6.2: `GetPrevDirectoryPath()` (#6073), `FileText*()` (#6210), `IsPathAbsolute()` (remoção de `isalpha()`), `GetFPS()` (#6265) `[L]`. (6) O próprio `rcore.c` declara `SUPPORT_AUTOMATION_EVENTS` "useful for automated testing systems" (`rcore.c:62-63`), mas isso é para jogos, não para a biblioteca. |
| **Solução proposta** | Suíte de testes em `tests/` na raiz do repositório: (a) *mini-helper* `rtest.h` (C99, sem dependências, em linha com a filosofia "sem dependências externas"), (b) testes **headless** — não criam janela nem contexto OpenGL — das funções puras de `rcore.c` e `rtext.c`, (c) `Makefile` com `make test`, (d) workflow de CI que roda a suíte, (e) mecanismo `KNOWN_BUG` (XFAIL) para registrar bugs conhecidos com o número da issue/PR. |
| **Benefício esperado** | Detectar regressões antes do merge; documentar o comportamento esperado de cada função (os testes funcionam como especificação executável); reduzir a carga da revisão humana; facilitar PRs pequenos de novos contribuidores; dar a autores de *bindings* um contrato verificável. |
| **Prioridade** | **Alta (Should-have, MoSCoW).** Justificativa: custo baixo, risco baixo (não altera `src/`), retorno contínuo, e destrava correções seguras nos back-ends. Não é *Must* porque a biblioteca funciona sem isso há 13 anos. |

### Achados novos feitos ao escrever os testes `[M]`

1. **`GetCodepoint()` ainda apresenta o problema do PR #6348 no `master` atual:** para `"\xC3"`, `"\xE2\x82"` e `"\xF0\x9F\x98"` (sequências truncadas) o tamanho devolvido excede `strlen`, então `texto += tamanho` passaria do `'\0'`. Está registrado como `XFAIL` (3 ocorrências).
2. **`GetFileExtension(NULL)` provoca violação de acesso** (código de saída `0xC0000005` no Windows): `GetFileName(NULL)` devolve `NULL` (`rcore.c:2522-2531`) e em seguida `strrchr(NULL, '.')` é chamado (`rcore.c:2503-2504`). Já `GetFileName(NULL)` e `IsPathAbsolute(NULL)` tratam `NULL` com segurança. Em busca na API do GitHub **não encontramos** issue sobre isso (busca por "GetFileExtension NULL"; não é prova de inexistência). *Não incluímos como teste automático* porque derrubaria o processo; é candidato a issue/PR futuro — **verifiquem antes de reportar**.

## 5.2 MVP

**O que será desenvolvido**

- `tests/rtest.h` — helper mínimo: `CHECK`, `CHECK_INT`, `CHECK_STR`, `KNOWN_BUG` (XFAIL/XPASS), `RUN`, `RTEST_SUMMARY` (código de saída ≠ 0 em falha).
- `tests/test_hash.c` — `ComputeSHA256/SHA1/MD5` em **14 comprimentos** (0, 1, 3, 55, 56, 57, 59, 60, 63, 64, 65, 119, 120, 121) com **oráculo independente** (.NET), `ComputeCRC32` (valor de verificação padrão `0xCBF43926`) e Base64 (vetores RFC 4648). **65 verificações.**
- `tests/test_paths.c` — `GetFileName`, `GetFileExtension`, `IsFileExtension`, `GetFileNameWithoutExt`, `IsPathAbsolute` (casos Windows/POSIX). **29 verificações.**
- `tests/test_utf8.c` — `GetCodepoint` (válidos, inválidos, truncados), `GetCodepointCount`, `CodepointToUTF8`; 3 `KNOWN_BUG` ligados ao PR #6348. **26 verificações.**
- `tests/Makefile` — `make test` (Windows/MSYS2, Linux, macOS).
- Workflow de CI `run_tests.yml` (cópia em `06-implementacao/ci/`), que compila a biblioteca e roda a suíte.

**Valor entregue:** um comando (`make test`) diz em segundos se funções críticas continuam corretas; regressões como #6143, #6312 e #6326 passam a ser detectadas automaticamente; bugs conhecidos ficam documentados no código.

**Fora do MVP (e por quê)**

| Item | Motivo |
|---|---|
| Testes de renderização/janela/áudio (US01, US02) | Exigem GPU/janela/dispositivo; são frágeis em CI e dependem de back-end. Próxima fase: usar `PLATFORM_MEMORY` + `rlsw` (renderizador por software) para testar sem GPU. |
| Matriz de CI multiplataforma (Windows/macOS/ARM) | Testamos no Windows/MSYS2 (local) e no Linux/Ubuntu (um workflow no GitHub Actions); **macOS e ARM não foram testados** (ver `06-implementacao/`). |
| Integração CMake/`ctest` (`add_test()` + `add_subdirectory(tests)`) | **Próximo passo natural**: o CI já executa `ctest`, então registrar os 3 executáveis acenderia um passo que hoje roda vazio. Ficou fora porque altera `CMakeLists.txt` do projeto e **não temos CMake instalado para verificar** — não afirmamos nada que não executamos. |
| Cobertura de código, *fuzzing*, *sanitizers* | Valiosos (ASan acharia #6348), mas ampliam o escopo; ficam como evolução. |
| Corrigir bugs encontrados (#6348, `GetFileExtension(NULL)`) | O MVP **não altera `src/`**; corrigir é outra mudança, com decisão do mantenedor. |
| Cobrir todas as funções de `rcore.c`/`rtext.c` | Priorizamos as com histórico de bugs (hash, caminhos, UTF-8). |

**Justificativa das escolhas:** (1) alvo *headless* garante execução em qualquer máquina e no CI; (2) helper próprio em C evita dependência (Unity, Check, CMocka), coerente com `README.md:37` e `CONTRIBUTING.md:18-19`; (3) oráculo externo impede "testar o código com ele mesmo"; (4) XFAIL permite entregar sem vermelho no CI e sem esconder o bug; (5) priorizar funções com histórico de regressão maximiza valor por linha de teste.

## 5.3 Análise de impacto

| Área | Impacto | Detalhe |
|---|---|---|
| **Classes / structs** | **Nenhum** | Nenhuma `struct` é criada/alterada. Os testes só chamam a API pública declarada em `raylib.h`. |
| **Pacotes / módulos** | **Nenhum em `src/`** (MVP) | Novo diretório de topo `tests/`. Indiretamente relacionados: `rcore.c` (arquivos, hash, Base64) e `rtext.c` (UTF-8). |
| **Componentes** | **Novo componente de teste** | `rtest.h` + 3 executáveis. Linkam com `libraylib.a`. |
| **Banco de dados** | **Não se aplica** | O raylib não usa banco de dados. |
| **APIs** | **Nenhuma alteração** | API pública e ABI inalteradas. *Risco colateral*: fixar comportamento por teste torna futuras mudanças de semântica explícitas (ex.: o `-WARNING-` do CHANGELOG deveria acompanhar atualização de teste). |
| **Interfaces** | **Nenhuma** | Sem interface gráfica; a "interface" nova é `make test` + saída em texto. |
| **Build / CI** | **Novo** | `tests/Makefile`; workflow `.github/workflows/run_tests.yml`. `CMakeLists.txt` e `src/Makefile` **não** são tocados no MVP. Complementa (não substitui) o `rexm test`, que valida exemplos, e o passo `ctest` hoje sem testes. |
| **Testes** | **Principal impacto** | De 0 para 120 verificações automatizadas (3 XFAIL documentados). |
| **Documentação** | **Pequeno** | `tests/README` (como rodar/escrever testes); sugerido: um parágrafo em `CONTRIBUTING.md` ("rode `make test` antes de abrir o PR"). |
| **Riscos** | Baixos | (a) Testes dependentes de plataforma (caminhos absolutos: tratados com `#ifdef _WIN32`); (b) `XFAIL` pode envelhecer — mitigado por `XPASS` avisando; (c) aceitação upstream: a filosofia "poucas funções, simples" pode resistir a infraestrutura nova → por isso o PR do trabalho é no **fork da dupla**, não no repositório oficial. |
