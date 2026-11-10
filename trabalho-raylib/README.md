# Software Archaeology — raylib

Trabalho Prático de **CSI412 – Engenharia de Software I** (UFOP/ICEA/DECSI, 2026/2, Prof. Igor Muzetti Pereira).

| | |
|---|---|
| **Integrantes** | `Fabian Givanna Marcolino dos Santos` · `Luan Henrique Pais` |
| **Software analisado** | **raylib** — biblioteca C para programação de jogos |
| **Projeto original** | https://github.com/raysan5/raylib |
| **Licença do projeto** | zlib/libpng (`LICENSE`) |
| **Versão analisada** | Release estável **6.2** (tag `6.2`, 15/10/2026) + snapshot do `master` de 31/10/2026 (`RAYLIB_VERSION "6.5-dev"`, commit `2f4b7d8` ou posterior) |
| **Fork com a intervenção** | https://github.com/FaMarcolino/raylib (branch `feat/headless-tests`; commit base da análise `2f4b7d8e9f4aef3d5ed81a683b54594849e9d7a7`; após o rebase de 10/11/2026 a branch está sobre `0dedaf3066ffd0a63b5899385f59055a915f839c`) |
| **Pull Request ao projeto original** | https://github.com/raysan5/raylib/pull/6400 (aberto em 10/11/2026 e **fechado pelo mantenedor no mesmo dia, sem mesclar** — único comentário: "No, thanks."; revisão interna no fork: https://github.com/FaMarcolino/raylib/pull/1) |


## Objetivo do trabalho

Assumir o papel de uma equipe que entra em um projeto existente: compreender o software, seus usuários, requisitos,
código, processo de desenvolvimento e histórico — e então **justificar tecnicamente e implementar experimentalmente** uma pequena evolução.

## Resumo da proposta de evolução

**Problema (evidência):** o raylib não tem nenhum teste automatizado. O CI (12 workflows) só compila e roda análise estática
(clang-tidy, CodeQL); nenhum workflow executa testes. Em consequência, bugs em funções puras e simples chegam a usuários e são
corrigidos depois — por exemplo `ComputeSHA256()` retornando hash errado para mensagens de 56–59 bytes (issue #6143 / PR #6144) e
`GetFileExtension()` corrigida duas vezes em 3 dias (PRs #6312 e #6326).

**Proposta:** criar uma suíte de **testes de regressão headless** (sem janela/GPU) para o núcleo utilitário da biblioteca, executável
com `make test` e integrada ao CI.

**Intervenção experimental (pasta `06-implementacao/`):** 3 arquivos de teste, **120 verificações**, mais um mini-helper (`rtest.h`),
Makefile e workflow de CI. Foi verificado por *teste de mutação* que os testes detectam um `ComputeSHA256` quebrado.

## Como executar a intervenção

Pré-requisitos: compilador C (gcc/clang) e `make` (no Windows: MSYS2 `mingw32-make`).

```bash
# 1) Na raiz do fork do raylib, compile a biblioteca estática
cd src
make PLATFORM=PLATFORM_DESKTOP          # Windows/MSYS2: mingw32-make PLATFORM=PLATFORM_DESKTOP

# 2) Rode os testes
cd ../tests
make test                               # Windows/MSYS2: mingw32-make test
```

Detalhes em [`06-implementacao/README.md`](06-implementacao/README.md).

## Estrutura do repositório

| Pasta | Conteúdo | Seção do enunciado |
|---|---|---|
| [`01-discovery/`](01-discovery/01-software.md) | O que é o software, personas e JTBD | 1.1 |
| [`02-processo/`](02-processo/02-evolucao-e-processo.md) | Evolução, práticas e processos de software | 2.2 |
| [`03-requisitos/`](03-requisitos/03-requisitos.md) | Requisitos, histórias de usuário e cenários | 3 |
| [`04-uml/`](04-uml/04-uml.md) | 5 diagramas UML + correspondências com o código | 4 |
| [`05-evolucao/`](05-evolucao/05-evolucao.md) | Proposta, MVP e análise de impacto | 5 |
| [`06-implementacao/`](06-implementacao/README.md) | Testes, CI, descrição do PR e revisão | 6 |
| [`07-etica/`](07-etica/07-etica.md) | Questões éticas | 7 |
| [`08-ia/`](08-ia/ia.md) | Uso de IA (`ia.md`) | 8 |


## Convenção de evidências

Cada afirmação traz sua origem:

- **`[L]`** = verificado no código/arquivos locais do snapshot (`arquivo:linha`).
- **`[GH]`** = verificado no GitHub (issue/PR/release/commit, com URL ou número).
- **`[M]`** = medido/executado por nós (compilação, testes, contagens).
