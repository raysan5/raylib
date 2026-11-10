# 3 — Requisitos reconstruídos do raylib 

Os requisitos abaixo foram **reconstruídos a partir de artefatos reais** (código, configuração, README, CI). Cada um traz evidência rastreável.
Convenção: `[L]` arquivo local, `[GH]` GitHub, `[M]` medido/executado por nós.

## Requisitos funcionais (RF)

| ID | Requisito | Evidência |
|---|---|---|
| **RF01** | O sistema deve permitir **criar uma janela e executar o laço de renderização por quadro** (`InitWindow` → laço com `BeginDrawing`/`EndDrawing` → `CloseWindow`), consultando `WindowShouldClose`. | `[L]` `src/rcore.c:586` (`InitWindow`), `:863` (`BeginDrawing`), `:880` (`EndDrawing`), `:726` (`CloseWindow`); `src/platforms/rcore_desktop_glfw.c:173` (`WindowShouldClose`); exemplo `README.md:70-89`; `examples/core/core_basic_window.c` |
| **RF02** | O sistema deve **ler entradas do usuário** — teclado, mouse, gamepad e toque — e expor consultas de estado por quadro (ex.: pressionou *neste* quadro vs. mantido pressionado). | `[L]` estado em `CORE.Input.*` (`src/rcore.c:324-383`); `IsKeyPressed` (`:3882`: compara `previousKeyState`×`currentKeyState`), `IsKeyDown` (`:3908`), `IsGamepadButtonPressed` (`:4025`), `IsMouseButtonPressed` (`:4117`); `PollInputEvents` (`rcore_desktop_glfw.c:1291`) |
| **RF03** | O sistema deve **carregar imagens (CPU) e texturas (GPU) e desenhar texturas, formas e texto**. | `[L]` `LoadImage` (`src/rtextures.c:273`), `LoadTexture` (`:4467`), `DrawTexture` (`:4841`), `DrawText` (`src/rtext.c:1211`); structs `Image`/`Texture`/`Font` (`src/raylib.h:264-324`) |
| **RF04** | O sistema deve **carregar e reproduzir áudio** (efeitos curtos e música em *streaming*). | `[L]` `InitAudioDevice` (`src/raudio.c:465`), `LoadSound` (`:937`), `PlaySound` (`:1200`), `LoadMusicStream` (`:1355`), `UpdateMusicStream` (`:1900`); formatos no `README.md:50` |
| **RF05** | O sistema deve oferecer **utilitários de arquivo/texto/hash independentes de janela** (extensão e nome de arquivo, caminhos absolutos, UTF-8, Base64, CRC32/MD5/SHA-1/SHA-256). *(Base da evolução proposta.)* | `[L]` `GetFileExtension` (`src/rcore.c:2500`), `GetFileName` (`:2522`), `IsPathAbsolute` (`:2889`), `ComputeCRC32` (`:3235`), `ComputeMD5` (`:3281`), `ComputeSHA1` (`:3399`), `ComputeSHA256` (`:3509`), `EncodeDataBase64` (`:3111`); `GetCodepoint` (`src/rtext.c:2437`) |

## Requisitos não funcionais (RNF)

| ID | Categoria | Requisito | Evidência |
|---|---|---|---|
| **RNF01** | **Portabilidade** | A biblioteca deve compilar e funcionar em várias plataformas/back-ends (Windows, Linux, macOS, Web, Android, DRM/Raspberry Pi, *memory*), escolhidos em tempo de compilação. | `[L]` lista de plataformas `src/rcore.c:6-35`; seleção por macro `:518-533`; 10 arquivos em `src/platforms/`; workflows `build_windows/linux/macos/android/webassembly.yml` |
| **RNF02** | **Simplicidade / aprendizagem** | A API deve ser pequena e legível para iniciantes, **sem dependências externas** para o usuário; funções são "mini-tutoriais". | `[L]` `README.md:37` ("NO external dependencies"), `CONTRIBUTING.md:18-19`, `CONVENTIONS.md` (nomes `TitleCase`, comentários antes da linha); exemplo mínimo de 5 chamadas |
| **RNF03** | **Desempenho** | O desenho deve ser agrupado em lotes (*batching*) para reduzir chamadas de desenho, e o tempo por quadro deve ser controlável (FPS alvo). | `[L]` `rlRenderBatch`/`rlDrawCall` (`src/rlgl.h:407-428`); `RL_DEFAULT_BATCH_BUFFER_ELEMENTS 8192` (`:207`); `EndDrawing` → `rlDrawRenderBatchActive` (`src/rcore.c:882`); `SetTargetFPS` (`:1576`); espera parcialmente ocupada (`src/config.h:93-95`) |
| **RNF04** | **Configurabilidade / tamanho** | Módulos e recursos devem poder ser ligados/desligados na compilação para reduzir tamanho e adaptar a sistemas embarcados. | `[L]` `SUPPORT_MODULE_RSHAPES/RTEXTURES/RTEXT/RMODELS/RAUDIO` (`src/config.h:40-53`) e dezenas de `SUPPORT_*`; log dos módulos carregados em `InitWindow` (`src/rcore.c:612-639`) |
| **RNF05** | **Segurança / robustez** | Entradas externas (URLs, arquivos de eventos, caminhos) devem ser validadas; o código passa por análise estática periódica. | `[L]` CHANGELOG 6.2: auditoria ROS (`CHANGELOG:13`), `OpenURL()` `CLN-002` (`:31`), automação `CLN-005/006` (`:32`); `analyze_codeql.yml` com `cron: '0 0 * * 1'` (semanal); `clang_tidy.yml` |
| **RNF06** | **Licenciamento** | O projeto deve permitir uso comercial e distribuição, mantendo o aviso de copyright. | `[L]` `LICENSE` (zlib/libpng), `README.md:160` |

> Nota crítica: os RNF acima são verificados por **build (CI), análise estática e *smoke test* dos exemplos** (`tools/rexm`), mas **nenhum é coberto por testes
> automatizados das funções da biblioteca** (não há `add_test()` no repositório) `[L]`. Este é o elo com a evolução proposta (RNF07 abaixo).

### Requisito não funcional proposto pela evolução

| ID | Categoria | Requisito | Evidência de necessidade |
|---|---|---|---|
| **RNF07** | **Testabilidade / manutenibilidade** | As funções puras do núcleo (arquivo, texto, hash, codificação) devem ter testes de regressão **executáveis sem janela/GPU** em um comando e no CI. | Sem testes de funções no repo: `enable_testing()` existe (`CMakeLists.txt:57`, `src/CMakeLists.txt:194`) e o CI roda `ctest` (`build_cmake.yml:67,111`), mas **não há nenhum `add_test()`** `[L]`; bugs: #6143/#6144 (SHA-256), PRs #6312 e #6326 (`GetFileExtension`), #6348 (`GetCodepoint`) `[GH]` |

## Histórias de usuário

- **US01 e US02** mostram o que o raylib **já faz** (dá para conferir no código).
- **US03, US04 e US05** mostram o que a **nossa evolução** entrega (os testes automáticos).

### US01 — Primeira janela (Persona 1: estudante) · existente

**Como** estudante iniciante, **quero** abrir uma janela e escrever um texto nela com poucas linhas de código, **para** ver o resultado já na primeira aula, sem instalar nada a mais.

- **Cenário 1.1 — Funciona**\
  **Dado que** a biblioteca raylib já está pronta e ligada ao meu programa,\
  **Quando** eu chamo `InitWindow(800, 450, "…")` e, dentro de um laço `while (!WindowShouldClose())`, desenho o texto com `DrawText(...)` entre `BeginDrawing()` e `EndDrawing()`,\
  **Então** aparece uma janela de 800×450 com o texto, e o laço termina quando eu fecho a janela ou aperto ESC (`exitKey = KEY_ESCAPE`, `rcore.c:650`).
- **Cenário 1.2 — Fechar direito**\
  **Dado que** a janela está aberta,\
  **Quando** eu chamo `CloseWindow()`,\
  **Então** o raylib fecha a janela e limpa o estado interno (`CORE`), e eu posso abrir outra janela depois (`rcore.c:736-740`).
- **Cenário 1.3 — Falha ao iniciar** *(ligado à issue #6351, que está aberta)*\
  **Dado que** o computador não tem tela disponível,\
  **Quando** `InitWindow()` não consegue iniciar (`InitPlatform() != 0`),\
  **Então** ele escreve o aviso "Failed to initialize platform" e termina (`rcore.c:659-663`). Mas, segundo a issue #6351 `[GH]`, chamar `CloseWindow()` depois disso ainda trava o programa.

### US02 — Reagir a uma tecla (Persona 2: desenvolvedor de protótipos e aplicações gráficas) · existente

**Como** desenvolvedor de protótipos, **quero** saber que uma tecla foi apertada *neste exato quadro*, **para** fazer uma ação (pular, atirar) uma vez só por toque, e não várias vezes enquanto o dedo fica na tecla.

- **Cenário 2.1 — Apertar a tecla**\
  **Dado que** a tecla `SPACE` estava solta no quadro anterior,\
  **Quando** ela está apertada no quadro atual,\
  **Então** `IsKeyPressed(KEY_SPACE)` responde `true` (estava solta e agora está apertada: `rcore.c:3888`).
- **Cenário 2.2 — Segurar a tecla**\
  **Dado que** `SPACE` já estava apertada no quadro anterior,\
  **Quando** eu continuo segurando,\
  **Então** `IsKeyPressed(KEY_SPACE)` responde `false`, e `IsKeyDown(KEY_SPACE)` responde `true` (a tecla continua apertada).
- **Cenário 2.3 — Tecla inválida**\
  **Dado que** eu passo um número de tecla inválido (`0`, ou maior ou igual a `MAX_KEYBOARD_KEYS`, que é 512),\
  **Quando** eu chamo `IsKeyPressed(key)`,\
  **Então** ele responde `false`, sem dar erro nem travar (`rcore.c:3886`).

### US03 — Rodar os testes com um comando (Persona 3: contribuidor) · evolução

**Como** contribuidor, **quero** rodar todos os testes com um único comando, **para** saber em poucos segundos se minha mudança estragou alguma função, antes de abrir o PR.

- **Cenário 3.1 — Tudo certo**\
  **Dado que** eu já compilei a biblioteca (`libraylib.a`) e estou na pasta `tests/`,\
  **Quando** eu rodo `make test`,\
  **Então** os 3 programas de teste rodam, o resumo mostra `0 failed` e o comando termina com sucesso `[M]` (65 + 29 + 26 = 120 verificações).
- **Cenário 3.2 — Um erro é pego**\
  **Dado que** alguém estraga o cálculo de `ComputeSHA256()` (um erro de preenchimento, o "padding"),\
  **Quando** eu rodo `make test`,\
  **Então** o `test_hash` falha nos tamanhos 56, 57, 59, 120 e 121, e o `make` termina com erro `[M]` (foi o teste de mutação, ver `06-implementacao/`).

### US04 — Bug conhecido não deixa tudo vermelho (Persona 3 / mantenedor) · evolução

**Como** mantenedor, **quero** anotar no teste que um bug já é conhecido (com o número do PR ou da issue), **para** deixar o problema registrado sem pintar o CI de vermelho enquanto a correção não chega.

- **Cenário 4.1 — Falha esperada (XFAIL)**\
  **Dado que** o PR #6348 (que corrige o tamanho devolvido por `GetCodepoint` em texto cortado) ainda está aberto,\
  **Quando** eu rodo `test_utf8`,\
  **Então** aparecem 3 linhas `XFAIL … (known bug PR #6348)` e o programa termina com sucesso `[M]`.
- **Cenário 4.2 — A correção chega (XPASS)**\
  **Dado que** o PR #6348 já foi aceito e o bug não existe mais,\
  **Quando** eu rodo `test_utf8`,\
  **Então** as linhas passam a mostrar `XPASS … seems FIXED`, avisando que aquela verificação já pode virar um teste normal (`CHECK`), como o `rtest.h` foi feito para fazer.

### US05 — Fixar o comportamento de uma função (Persona 4: autor de binding) · evolução

**Como** autor de binding (quem adapta o raylib para outra linguagem), **quero** que o comportamento de funções como `GetFileExtension()` fique gravado em testes, **para** não ser pego de surpresa se ele mudar numa versão futura.

- **Cenário 5.1 — Arquivo oculto**\
  **Dado que** o arquivo se chama `.gitignore` (ou `projeto/.gitignore`),\
  **Quando** eu chamo `GetFileExtension(...)`,\
  **Então** ele devolve `NULL` (nenhuma extensão), e não `.gitignore`. Essa regra foi corrigida no PR #6326 `[M]`.
- **Cenário 5.2 — Ponto só no nome da pasta**\
  **Dado que** o caminho é `my.dir/README`,\
  **Quando** eu chamo `GetFileExtension(...)`,\
  **Então** ele devolve `NULL`, porque o ponto está no nome da pasta e não no do arquivo. Essa regra foi corrigida no PR #6312 `[M]`.
- **Cenário 5.3 — Extensão normal**\
  **Dado que** o caminho é `archive.tar.gz`,\
  **Quando** eu chamo `GetFileExtension(...)`,\
  **Então** ele devolve `.gz` (só a última extensão).

## Rastreabilidade história → teste

| História/cenário | Teste automatizado (pasta `06-implementacao/tests/`) |
|---|---|
| US03/3.1, 3.2 | `Makefile` (`make test`), `test_hash.c::test_sha256_boundaries` |
| US04/4.1 | `test_utf8.c::test_invalid_size_stays_inside_string` (`KNOWN_BUG`) |
| US05/5.1–5.3 | `test_paths.c::test_get_file_extension` |
| US01, US02 | **Não cobertos** pela evolução (exigem janela/entrada); permanecem fora do MVP — ver `05-evolucao/` |
