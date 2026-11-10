# ia.md — Registro de uso de Inteligência Artificial


## 1. Ferramenta utilizada

- **Claude Code** (extensão do VS Code), modelo **Claude Sonnet 5.5**, com acesso a leitura de arquivos, execução de comandos (PowerShell, gcc/MSYS2, Node) e consulta HTTP a `api.github.com`.
- Sessões de trabalho em **01/11/2026** (análise, relatório e testes) e **10/11/2026** (migração do relatório para o fork, atualização da branch, PR ao projeto original e checagem do que falta). `TODO: acrescentar outras ferramentas, se a dupla usar.`

## 2. Objetivos

| # | Objetivo | Seção do trabalho |
|---|---|---|
| 1 | Compreender a estrutura e o código do raylib (leitura de `rcore.c`, `rlgl.h`, `raylib.h`, platforms) | 3, 4, 5 |
| 2 | Pesquisar histórico no GitHub (releases, PRs, issues) | 3.2, 6 |
| 3 | Gerar rascunhos de requisitos, histórias, UML (Mermaid) e análise de impacto | 4, 5, 6 |
| 4 | Elaborar os testes da evolução e verificar compilação/execução | 7 |
| 5 | Revisão textual e organização do repositório | 10 |
| 6 | Passo a passo de comandos git/GitHub (migração para o fork, rebase, PR ao projeto original) e checagem do trabalho contra o enunciado | 7, 10, 11 |

## 3. Principais prompts

| Origem | Prompt (resumo fiel) |
|---|---|
| Dupla | "me ajude a fazer esse trabalho, baseando nessa pasta raylib-master" (com o PDF do enunciado anexado). |
| Dupla (resposta a pergunta da IA) | Evolução escolhida: **testes automatizados**. Destino: **nova pasta ao lado do raylib** (`trabalho-raylib/`). |
| IA → ferramentas | Leitura de `README.md`, `ROADMAP.md`, `CHANGELOG`, `CONTRIBUTING.md`, `CONVENTIONS.md`, `SECURITY.md`, `rcore.c`, `rtext.c`, `rlgl.h`, `raylib.h`, workflows; consultas à API do GitHub (`/releases`, `/commits`, `/issues`, `/pulls/NNNN`, `/search/issues`); compilação do raylib e dos testes; contagens de linhas. |
| Dupla (10/11) | "Preciso transferir tudo o que está na pasta do GitHub Trabalho-Raylib para a raylib (o fork), apagar o outro; o professor também pediu que a implementação seja enviada ao original para o dono aceitar — me explique como faz." |
| Dupla (10/11) | Colagens da saída do cmd, com perguntas como "deu tudo certo?", "o que aconteceu?" e "posso executar esses comandos antes de corrigir o erro?" (falha do workflow macOS). |
| Dupla (10/11) | "Faça o 5, o 6 e prepare o PR_DESCRIPTION.md atualizado, mas eu quero fazer os comandos do cmd." (a IA edita arquivos; a dupla executa o git). |
| Dupla (10/11) | "Verifique o que já está feito e o que falta" e, com o PDF do enunciado anexado: "pegue cada ponto e tópico e me indique onde está o trabalho, checando". |
| Dupla (10/11) | Perguntas de compreensão: "me explique os diagramas", "me explique o que é autor de binding", "onde encontro a licença do fork", "como fechar o PR #1", "o que é necessário na revisão", "sobre as evidências em implementação". |
| Dupla | `TODO: colar aqui os prompts adicionais que vocês escreverem (ex.: pedir ajustes, perguntar "por que X?").` |

## 4. Resultados utilizados (e como foram verificados)

| Resultado da IA | Como foi verificado (no próprio software) |
|---|---|
| Descrição das funcionalidades, licença, versão | Lidos no `README.md`, `LICENSE`, `raylib.h:89-92`, `CHANGELOG:4` — **citados com linha** nos documentos. |
| Estruturas e fluxos de UML | Cada elemento ligado a arquivo:linha (tabela de correspondências em `04-uml/`). **Sintaxe Mermaid validada** com o *parser* oficial (`mermaid@11` + `jsdom`: 5/5 blocos OK) — isso valida a sintaxe, **não** a fidelidade ao código, que foi conferida por leitura. |
| Contagens (310 entradas no changelog 6.2; ~130 autores; 70 do mantenedor; 15 `-WARNING-`; 59.590 vs 266.147 linhas; 228 exemplos) | Executadas por **scripts PowerShell** no snapshot local (`[M]`). A contagem de "autores distintos" inclui qualquer `@nome` na seção 6.2 (aproximação). |
| Dados do GitHub (datas, autores, merge) | Obtidos pela **API pública** via ferramenta de busca que **resume o JSON com um modelo menor** → sujeita a erro. Por isso conferimos individualmente #4869, #5264, #5266, #6348, #6326, #5664 e corrigimos divergências (item 5). **A dupla deve abrir cada link e conferir antes de apresentar.** |
| Código dos testes (`rtest.h`, `test_*.c`, `Makefile`) | **Compilado e executado** (gcc 14.2.0, MSYS2): 120 verificações, 0 falhas (3 XFAIL na 1ª execução; 0 após o merge do #6348, em 10/11). Vetores de hash vêm de **oráculo independente (.NET)**. **Teste de mutação** provou que detectam um `ComputeSHA256` quebrado (5 falhas, código de saída 1). |
| Achado: `GetFileExtension(NULL)` trava | **Reproduzido** com programa mínimo: código de saída `0xC0000005`. Causa lida em `rcore.c:2503-2504`. |
| Achado: `GetCodepoint` tinha o problema do PR #6348 | **Reproduzido** pelos 3 `XFAIL` (PR aberto na API em 01/11). O PR foi mesclado em 09/11 (commit `7e626d65`); em 10/11 os testes viraram `XPASS` e foram promovidos a `CHECK`. |
| Migração do relatório para `trabalho-raylib/` no fork `FaMarcolino/raylib` (branch `master`) | Conferida com git: 21 arquivos no fork = 21 no repositório antigo, sem diferença de nomes; o repositório antigo só foi apagado depois. |
| Rebase de `feat/headless-tests` sobre o `upstream/master` | `git rev-list --left-right --count` mostrou 38 commits de atraso e 7 de avanço; rebase sem conflito; `git ls-tree` confirmou que `tests/` e `run_tests.yml` não existiam no upstream; `make test` rodou pela dupla após o rebase: 120 verificações, `ALL TEST FILES PASSED`. |
| PR ao projeto original: **#6400** | Aberto pela dupla; conferido na página: 8 commits, título e descrição, base `raysan5/raylib:master`. |
| Diagnóstico da falha do workflow macOS no PR #1 | Log colado pela dupla: "The job was not acquired by Runner of type hosted" (falha de infraestrutura do GitHub); as execuções manuais #4 (`master`) e #5 (`feat/headless-tests`) passaram. |
| Linhas `arquivo:linha` citadas no relatório | Conferidas na `master` do fork (`git show origin/master:src/rcore.c` etc.): `rcore.c:586`, `:863`, `:2500`, `:3509`, `:3545`, `rtext.c:2437` e `rlgl.h:420` batem; no raylib atual elas já mudaram. |
| Checagem do trabalho contra o enunciado (cada seção → arquivo) | Feita pela IA lendo os arquivos do repositório; a dupla deve reconferir. |

## 5. Resultados descartados ou corrigidos durante o trabalho

| O que a IA afirmou/fez | Por que foi descartado | Correção |
|---|---|---|
| Primeira busca por testes concluiu que **não havia `enable_testing`/`ctest`** em nenhum build file. | A busca usou um filtro de *glob* que **não alcançou** os arquivos; uma segunda busca, mais ampla, achou `enable_testing()` (`CMakeLists.txt:57`, `src/CMakeLists.txt:194`) e `ctest` no CI (`build_cmake.yml:67,111`). | Reformulamos o problema: "o `ctest` roda, mas **não há nenhum `add_test()`**". Também descobrimos o `rexm test` (smoke test de exemplos) e a issue #5664, e **removemos a frase "nenhum teste automatizado"** de todos os documentos, trocando por "nenhum teste das funções da biblioteca". |
| A IA citou **"issue/PR #6144"** (sem distinguir) nos comentários dos testes e nos textos iniciais. | Conferindo na API, **#6144 é PR** e a issue original é **#6143**. | Corrigido nos documentos e nos comentários de `test_hash.c`. |
| Uma consulta de busca da API devolveu para o **#6326** a data de criação **2025**, e só depois se autocorrigiu. | A data real é **24/10/2026** (e é PR). Isso mostra o risco de depender do resumo automático. | Corrigido; reforça o checklist da seção 7. |
| Dedução de que as PRs #5264 e #5266 "adicionaram" `ComputeSHA256`. | Só o **#5264** foi mesclado; **#5266** foi fechado sem merge. | Texto reescrito com os fatos. |
| Frase "`GetFPS` é função pura". | `GetFPS()` depende de tempo/estado; não é pura e **não** está nos testes. | Removida da lista de funções puras. |
| A IA sinalizou como possível inconsistência "6.2 (CHANGELOG) vs 6.5-dev (`raylib.h`)". | Não é erro: o `master` avançou além da última release publicada (o snapshot contém o commit `2f4b7d8` de 31/10/2026). | Explicado em `01-discovery/`. |
| Ideia de **ligar os testes ao `ctest`** já no MVP. | Não há CMake neste ambiente para **verificar**. | Mantido como "próximo passo", sem afirmar que funciona. |
| A IA redigiu a descrição do PR ao raylib em inglês, longa e detalhada (evidências, checklist). | O GitHub avisou que o `CONTRIBUTING.md` ganhou a seção "About AI Usage": pede descrição curta, não gerada por IA, e que o contribuidor conheça o contexto. | Texto reescrito curto; a dupla deve adaptá-lo com as próprias palavras; o PR mantém a declaração de uso de IA. |
| A IA sugeriu sincronizar a `master` do fork com o `upstream` para evitar builds no PR #1. | Quebraria as referências `arquivo:linha` do relatório (as linhas mudaram no upstream). | Sugestão retirada: a `master` do fork permanece no snapshot de 31/10. |
| A IA sugeriu o comentário "Review done here" para fechar o PR #1. | Falso: a revisão pelo outro integrante ainda não foi feita. | Erro reconhecido; a dupla deve editar o comentário (ver `REVISAO.md`). |
| A IA indicou a revisão no PR #1. | O PR #1 tem base antiga: 45 commits e comentários do CodeQL sobre arquivos que a dupla não alterou. | Código revisado no PR #6400 (7 arquivos); registro da revisão no fork. |
| A IA passou ao revisor o comando do teste de mutação sem indicar onde gravar a biblioteca. | O Makefile do raylib grava `libraylib.a` em `../src/` (`RAYLIB_RELEASE_PATH ?= ../src`): a cópia `src_mut` ficou com a biblioteca original, os testes passaram e a mutação não foi exercitada; a biblioteca original foi sobrescrita pela versão mutada. | Reconstruir a biblioteca original e refazer a mutação com `RAYLIB_RELEASE_PATH=.`; roteiro do `REVISAO.md` corrigido. |

## 6. O que a IA **não** fez / limites

- **Não** criou o repositório no GitHub nem o *fork* (feitos pela dupla), **não** abriu o **PR** nem fez a **revisão pelo outro integrante**. A IA instalou o Git (a pedido da dupla), clonou o fork e fez os commits localmente com a identidade informada pela dupla; o `git push` e o login no GitHub foram feitos pela dupla.
- **Não** executou o CMake. O workflow `run_tests.yml` (CI Linux) rodou no fork: a **1ª execução falhou** no passo "Run tests" (`exit code 2`). A IA não conseguia ler o log (exige login) e levantou a **hipótese** de falta de `libglfw3-dev` (que os workflows oficiais instalam). A dupla colou o log real, que **confirmou**: `/usr/bin/ld: cannot find -lGL` (erro de link, não de teste). Após o ajuste (commit `b83e2d3`) a execução ficou **verde**.
- **Não** validou a escolha do software com o professor.
- Em **10/11** a IA **só editou arquivos** locais e leu informações (comandos `git` de leitura e páginas públicas do GitHub). Os comandos que alteram o GitHub — `commit`, `push`, rebase, apagar o repositório `Trabalho-Raylib`, abrir o PR #6400 e fechar o PR #1 — foram **executados pela dupla** no cmd e no navegador. A IA também **não** viu logs que exigem login: dependeu da dupla colá-los.
- Personas são **arquétipos derivados de documentos**, não entrevistas.
- Os testes foram executados no **Windows/MSYS2** (local) e no **Linux/Ubuntu** (CI do GitHub). **macOS não foi executado.**

## 7. Checklist de verificação que a dupla deve fazer ANTES da apresentação

- [ ] Abrir os links e conferir número/título/autor/data de: #4869, #5264, #5266, #6143, #6144, #6312, #6326, #6348, #5664, #6351, releases 5.5/6.0/6.2.
- [ ] Rodar `make test` por conta própria e ver os 120 checks.
- [ ] Rodar o teste de mutação do `REVISAO.md` (seção D) e ver falhar.
- [ ] Abrir 5 linhas citadas (ex.: `rcore.c:586`, `:3545`, `:2500`, `rtext.c:2437`, `rlgl.h:420`) e confirmar o conteúdo no commit do fork.
- [ ] Conseguir explicar sem consultar: o que `InitPlatform()` faz; por que `GetFileExtension(".gitignore")` é `NULL`; o que é XFAIL/XPASS; por que o teste é *headless*.
- [ ] Reler cada documento e **reescrever com suas palavras** o que não souberem defender (o enunciado permite perguntas individuais).
- [ ] Preencher todos os `TODO` (nomes, ferramentas e prompts da dupla).
- [ ] Abrir o PR #6400 e conferir título, 8 commits, 7 arquivos e a nota de uso de IA.
- [ ] Reler a seção "About AI Usage" do `CONTRIBUTING.md` do raylib e adaptar a descrição do PR com as próprias palavras.
- [ ] Conferir que o comentário de fechamento do PR #1 não afirma uma revisão que ainda não aconteceu.
- [ ] Abrir 2 ou 3 linhas `arquivo:linha` na `master` do **fork** (não no upstream atual) e conferir o conteúdo.
