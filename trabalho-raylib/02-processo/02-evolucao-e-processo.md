# 2.2 — Como o raylib evoluiu e como é desenvolvido

## Linha do tempo resumida

| Data | Marco | Fonte |
|---|---|---|
| 24/10/2013 | Repositório criado | `[GH]` API do repositório |
| nov/2013 | raylib 1.0 ("começou como projeto de fim de semana", 3 meses de trabalho) | `[L]` `HISTORY.md:16` |
| abr/2014 | 1.1 — redesenho interno; nasce o módulo `rlgl` (OpenGL 1.1/3.3/ES2) | `[L]` `HISTORY.md:23-27` |
| set/2014 | 1.2 — redesenho do `core` para Android e Raspberry Pi | `[L]` `HISTORY.md:36-38` |
| 2.0 | Remoção das dependências externas (GLFW3 e OpenAL) | `[L]` `ROADMAP.md:67-68` |
| 18/11/2024 | Release 5.5 | `[GH]` releases |
| 31/03/2025 → 31/08/2025 | PR #4869: back-end nativo Win32 | `[GH]` |
| out/2025 | PRs do renderizador por software `rlsw` | `[GH]` |
| 23/04/2026 | Release 6.0 ("a maior até agora") | `[GH]` releases |
| 15/10/2026 | Release 6.2 | `[GH]` releases |

## Acontecimentos relevantes 

### A1 — Mudança arquitetural: de um `core` monolítico para back-ends de plataforma

- **O que mudou:** o `rcore.c` deixou de conter todo o código de janela/entrada por plataforma. A meta "Split core module into separate platforms"
  aparece como concluída em 5.0 (`ROADMAP.md:40`). Hoje `rcore.c` seleciona o back-end com `#include "platforms/..."` por macro
  (`src/rcore.c:518-533`) e existem 10 arquivos em `src/platforms/` (GLFW, SDL, RGFW, Win32, Web, Emscripten, DRM, Android, Memory + template).
- **Contrato entre núcleo e plataforma:** `InitPlatform()`/`ClosePlatform()` declarados como `extern` em `rcore.c:491-492`;
  `rcore_template.c` documenta o esqueleto para novos back-ends (`rcore_template.c:1-69`).
- **Por que importa:** permitiu adicionar plataformas (Win32, Memory, Emscripten) sem reescrever o núcleo — base dos acontecimentos A2 e A3.

### A2 — Nova funcionalidade grande via contribuição externa: back-end nativo Win32 (PR #4869)

- **Evidência `[GH]`:** PR #4869 "[rcore][win32] Adding native win32 backend", autor `marler8997`, aberto em **31/03/2025**, mesclado em **31/08/2025** por `raysan5`;
  14 commits, +2.194 linhas, 0 removidas, 8 comentários. O autor justificava com executáveis menores (~0,83× o do GLFW) e menos comportamentos surpreendentes.
- **Evolução posterior (incremental):** correções logo depois — #5297 (reverter mudança indesejada, 23/10/2025), #5299 (mingw), #5311 (Makefile), #5365 (MSVC, 20/11/2025) `[GH]`.
  No 6.2, há dezenas de linhas `[rcore][Win32]` (MSAA, clipboard, `IsKeyPressedRepeat`…) `[L]` `CHANGELOG:71-88`, e o `ROADMAP.md:18` ainda lista "Improve new Windows platform backend" para a 7.0.
- **Leitura crítica:** uma funcionalidade de 2 mil linhas ficou 5 meses em revisão e chegou "incompleta mas funcionando" — ou seja, entrega **incremental** com estabilização posterior pela comunidade.

### A3 — Reversão de decisão de roadmap: renderizador por software (`rlsw`)

- **Antes:** no roadmap da 4.0, "Software rendering backend (avoiding OpenGL) (#1370) — _DISCARDED_" (`ROADMAP.md:52`).
- **Depois:** na 6.0 vira meta cumprida: "`rlsw`: New Software Renderer backend, pseudo-OpenGL 1.1 implementation" (`ROADMAP.md:33`), e a release 6.0 o descreve como "CPU-only software renderer" `[GH]`.
- **Ritmo:** série de PRs de `Bigfoot71` em out/2025, muitos mesclados em horas: #5304 aberto 25/10/2025 22:47 UTC e mesclado 22:58; #5300 (25/10) e #5312 (28/10) mesclados no mesmo dia em que foram abertos; #5342 aberto 07/11 e mesclado 08/11/2025 `[GH]`.
  No 6.2: "Double buffering support… Pico 2", "ESP32 optimizations" `[L]` `CHANGELOG:95-114`.
- **Leitura crítica:** o projeto **muda de ideia com base em necessidade e em contribuição disponível** (aderente ao valor ágil "responder a mudanças").

### A4 — Qualidade e segurança: auditoria externa e correção rápida de bug (release 6.2)

- **Auditoria:** "Multiple security concerns reported by ROS (Radically Open Security)" (`CHANGELOG:13`); ex.: `OpenURL()` com checagens contra URLs maliciosas — `CLN-002` (`CHANGELOG:31`), `LoadAutomationEventList()` — `CLN-005/006` (`CHANGELOG:32`).
- **Bug curto-ciclo — `ComputeSHA256()`:** a função entrou pelo PR **#5264** (`JohnnyCena123`, aberto 13/10/2025, **mesclado 15/10/2025** por `raysan5`);
  uma implementação concorrente, PR **#5266** (`manuel5975p`, "alternative implementation"), foi **fechada sem merge** em 02/11/2025 `[GH]` — exemplo de escolha entre alternativas na revisão.
  Em **12/09/2026 05:15 UTC** foi aberta a issue **#6143** (hash errado se `tamanho mod 64` ∈ 56..59) e **05:16** o PR **#6144** com a correção, fechado 08:24 do mesmo dia `[GH]`; entra no CHANGELOG 6.2 com `-WARNING-` (`CHANGELOG:42`).
  Confirmamos no código atual: `src/rcore.c:3545-3546` reserva 8 bytes do comprimento antes de arredondar para 64 `[L]`.
- **Leitura crítica:** a correção foi rápida, mas o bug foi **descoberto por usuário depois da função publicada** — não por teste do projeto.

### A5 — Padrão de regressões na mesma função: `GetFileExtension()`

- PR **#6326** "Fix `GetFileExtension()` returning the name of hidden files…" (aberto 24/10/2026, fechado/mesclado 25/10/2026) e, três dias antes, PR **#6312** "Fix `GetFileExtension()` returning a dot from a directory name" (aberto 21/10/2026) `[GH]`.
- O código atual trata os dois casos (`src/rcore.c:2500-2509`): usa `GetFileName()` para ignorar diretórios e rejeita `dot == name` para arquivos ocultos `[L]`.
- **Observação:** duas correções no mesmo helper em 3 dias sugerem ausência de uma suíte que fixe o comportamento esperado — motivação direta da evolução proposta (`05-evolucao/`).

## Como o projeto é desenvolvido (práticas observadas)

| Prática | Evidência |
|---|---|
| **Mantenedor central + comunidade grande** | `raysan5` mescla PRs (ex.: #4869) e escreve ~23% das entradas do 6.2 (70/310) `[M]`; ~130 autores distintos no 6.2 `[M]`. |
| **Contribuição por PR pequeno** | `CONTRIBUTING.md:61-64`: "Don't send big pull requests… send small pull requests, one at a time". |
| **Issues com tag de módulo** | `CONTRIBUTING.md:55-56`: prefixar o título com `[rcore]`, `[rtext]`…; visto em todos os títulos recentes `[GH]`. |
| **Padrão de código documentado** | `CONVENTIONS.md` (tabela de nomes, 4 espaços, sem TAB, comentários antes da linha). |
| **Integração contínua** | 12 workflows em `.github/workflows/`: builds Windows/Linux/macOS/Android/WebAssembly/CMake e dos exemplos; `clang_tidy.yml`, `analyze_codeql.yml` (cron semanal) e `parse_api.yml` (gera `raylib_api.*`). `build_cmake.yml` tem um passo "Test" com `ctest` (`:62-67`, `:106-111`), mas **não há nenhum `add_test()`** no repositório (busca em `.txt/.cmake/.zig/.yml`, fora de `src/external`) — logo não há testes registrados para o `ctest` rodar `[L]` *(inferência: não executamos o CMake)*. |
| **Teste de exemplos (smoke test)** | `tools/rexm` (`rexm test`, `testlog`, `validate`) compila e roda cada exemplo e confere: avisos de compilação/log, inicialização, fechamento, carga de assets, `rlgl`, plataforma, fonte e timer; gera relatórios por plataforma em `tools/rexm/reports/examples_testing_{windows,linux,macos,web,drm}.md` `[L]`. A comunidade complementa com a issue #5664 (22 comentários) pedindo relatos "`[exemplo] - Plataforma`" `[GH]`. **Verifica a biblioteca de ponta a ponta, mas não o resultado das funções** (um exemplo passa mesmo que `ComputeSHA256` retorne hash errado). |
| **Releases com changelog detalhado** | `CHANGELOG` por módulo e tag `[rcore][GLFW]`…, autor e nº do PR em cada linha. |
| **Planejamento aberto** | `ROADMAP.md` + "wishlist" em *Discussions* por versão (7.0: discussão #5710) `[L]`. |
| **Reuso massivo** | `src/external/` com ~266 mil linhas de bibliotecas *single-header* (stb, miniaudio, cgltf, GLFW…) frente a ~59,6 mil linhas próprias `[M]`. |
| **Documentação por exemplos** | 228 exemplos compiláveis; sem documentação formal de API (`README.md:125`). |

## Relação com processos de software

> Análise crítica: o raylib **não segue** um modelo prescritivo. Abaixo, o que **se parece** e o que **não se parece** com cada um.

| Conceito | Aderência | Evidência e crítica |
|---|---|---|
| **Desenvolvimento incremental** | **Alta** | Entregas contínuas em PRs pequenos; releases com changelog (5.5 → 6.0 → 6.2); recursos grandes (Win32, `rlsw`) entram "incompletos mas funcionando" e são refinados depois (A2, A3). **Crítica:** o intervalo entre 5.5 e 6.0 foi de ~17 meses (18/11/2024 → 23/04/2026), longo para um modelo incremental. |
| **Reuso** | **Alta** | `src/external/` embute bibliotecas (stb, miniaudio, cgltf, par_shapes, GLFW…) em vez de reimplementar; `rlgl` e `raymath` são reutilizáveis de forma isolada (`README.md:41,49`); `rcore_template.c` é um gabarito reutilizável para novas plataformas. **Crítica:** reuso custa governança de licenças heterogêneas (ver `07-etica/`) e de atualizações. |
| **Manifesto Ágil / XP (parcial)** | **Média** | *Software funcionando acima de documentação* (exemplos no lugar de docs de API); *responder a mudanças* (A3); *indivíduos e interações* (Discord, Reddit, Discussions); XP: padrão de código (`CONVENTIONS.md`), integração contínua, *small releases*, propriedade coletiva. XP também valoriza testes; o projeto tem *smoke tests* de exemplos (`rexm test`) e testes manuais da comunidade (#5664), um traço de "teste de aceitação" informal. **Crítica — não há testes automatizados das funções da biblioteca (nem TDD):** é o furo mais visível frente ao XP, e o que sustenta a evolução proposta. |
| **Scrum** | **Baixa** | Não há sprints, papéis (PO/SM), *backlog* priorizado nem cerimônias; issues quase sem rótulos (ex.: #6351, #6349, #6342 sem rótulo) `[GH]`. O que existe é um *roadmap* de desejos, não um backlog. |
| **Cascata** | **Nula** | Não há fases sequenciais nem especificação prévia; `ROADMAP.md` é lista de desejos revisável. Único traço "planejado": metas por versão major. |
| **RUP** | **Baixa** | Sem fases formais (concepção/elaboração/construção/transição) nem artefatos UML mantidos; porém há **desenvolvimento iterativo** e foco em arquitetura (`rlgl`, platforms) lembrando o princípio de "arquitetura primeiro". |
| **Modelo Spotify** | **Nula** | Sem squads/tribos; um mantenedor principal e contribuidores voluntários. As tags de módulo (`[rcore]`, `[rtext]`) lembram áreas de responsabilidade, mas sem times fixos. |

**Conclusão:** o processo real é um **incremental/ágil informal centrado em um mantenedor**, com forte reuso, CI de *build*/análise estática e *smoke tests* de exemplos, mas **sem testes automatizados das funções da biblioteca** —
o que torna a verificação de regressões em funções puras dependente de revisão humana e de relatos de usuários.
