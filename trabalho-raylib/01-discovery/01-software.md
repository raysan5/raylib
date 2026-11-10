# 1.1 — O que é o raylib?

## Ficha do software

| Item | Descrição | Evidência |
|---|---|---|
| **Problema que resolve** | Aprender e fazer programação de jogos/gráficos em C sem a complexidade de engines e de toolchains com muitas dependências. Nasceu porque o autor, ao ensinar jogos a alunos sem experiência, achou as bibliotecas existentes (WinBGI) pouco claras e sem aceleração por hardware. | `[L]` `HISTORY.md:6-12` |
| **Público-alvo** | "Prototipagem, ferramentas, aplicações gráficas, sistemas embarcados e **educação**". | `[L]` `README.md:5` |
| **Principais usuários** | Estudantes e professores; desenvolvedores indie/protótipos; autores de *bindings* (+70 linguagens); contribuidores de portes de plataforma. | `[L]` `README.md:53`, `BINDINGS.md`, `CONTRIBUTING.md:5-11` |
| **Principais funcionalidades** | Janela + laço de renderização; entrada (teclado, mouse, gamepad, toque); formas 2D, texturas e texto; modelos 3D e animação; shaders; áudio (WAV, OGG, MP3, FLAC, QOA, XM, MOD); módulo matemático (`raymath`); câmeras; camada de abstração OpenGL (`rlgl`); renderizador por software (`rlsw`). | `[L]` `README.md:35-54` |
| **Tecnologias** | **C99** (PascalCase/camelCase), OpenGL 1.1/2.1/3.3/4.3/ES2/ES3 (ou software), back-ends de plataforma GLFW/SDL/RGFW/Win32/Web/DRM/Android/Memory, bibliotecas *single-header* embutidas (stb, miniaudio, cgltf…), Makefile + CMake + `build.zig`, GitHub Actions. | `[L]` `README.md:39-42`, `src/rcore.c:6-35`, `src/external/`, `.github/workflows/` |
| **Licença** | **zlib/libpng** (permissiva; permite uso comercial e *linking* estático com código fechado). | `[L]` `LICENSE`, `README.md:160`; `[GH]` API → `Zlib` |
| **Versão/release analisada** | Release **6.2** (15/10/2026) e snapshot local do `master` (`RAYLIB_VERSION "6.5-dev"`). Releases anteriores: 6.0 (23/04/2026), 5.5 (18/11/2024). | `[L]` `src/raylib.h:89-92`, `CHANGELOG:4`; `[GH]` releases |
| **Escala** | ~59,6 mil linhas de código próprio em `src/` (sem `external/`), ~266 mil linhas de bibliotecas externas embutidas, 228 arquivos `.c` em `examples/` (README diz "+220"). | `[M]` contagem PowerShell no snapshot |
| **Popularidade (31/10/2026)** | 35.517 estrelas, 3.365 forks, 21 itens abertos (issues + PRs). Criado em 24/10/2013. | `[GH]` `api.github.com/repos/raysan5/raylib` |


## Posicionamento: o que o raylib *não* faz 

`[L]` `README.md:56-65`: janela única com um contexto OpenGL; redimensionar/mover pausa o laço em algumas plataformas;
`RenderTexture` vem invertida; rasterização de fontes inferior ao Freetype; **sem RTL, ligaduras ou emojis** no desenho de texto.
Também é filosofia assumida: "sem interface, sem ajudantes visuais, sem botão de debug, só o código" (`README.md:7`).

## Personas e Jobs to Be Done (JTBD)


### Persona 1 — Estudante iniciante de programação de jogos

- **Quem é:** aluno com pouca ou nenhuma experiência, aprendendo C e lógica por meio de jogos.
- **Evidência:** a própria origem do projeto — alunos "que nunca tinham escrito uma linha de código" programando PONG
  (`HISTORY.md:8`); "raylib is designed to be learned using the examples" (`README.md:125`); método "Learn by reading code and examples" (`CONTRIBUTING.md:41-43`); 228 exemplos `[M]`.
- **JTBD:** *Quando* estou começando a programar e quero ver algo na tela logo na primeira aula, *eu quero* abrir uma janela
  e desenhar com poucas chamadas e sem instalar dependências, *para que* eu mantenha a motivação e entenda cada linha do que escrevi.
- **Como o software atende:** exemplo mínimo de 5 chamadas (`InitWindow`, `WindowShouldClose`, `BeginDrawing`/`EndDrawing`, `CloseWindow`) em `README.md:70-89`; "NO external dependencies" (`README.md:37`).

### Persona 2 — Desenvolvedor de protótipos / jogos indie

- **Quem é:** pessoa que já programa e quer prototipar rápido ou publicar um jogo pequeno em várias plataformas.
- **Evidência:** "prototyping, tooling, graphical applications, embedded systems" (`README.md:5`); suporte a Windows/Linux/macOS/RPI/Android/HTML5 (`README.md:38`); licença zlib permite uso comercial (`CONTRIBUTING.md:22`).
- **JTBD:** *Quando* tenho uma ideia de jogo e pouco tempo, *eu quero* uma biblioteca leve que cubra janela, entrada, 2D/3D e áudio
  e compile para várias plataformas, *para que* eu entregue um protótipo jogável sem aprender uma engine inteira.

### Persona 3 — Contribuidor de portes de plataforma / correção de bugs

- **Quem é:** desenvolvedor externo que melhora um back-end (ex.: Win32, GLFW, SDL, DRM) ou corrige bugs em funções do núcleo.
- **Evidência:** 6.2 tem **310 entradas** de changelog e **~130 autores distintos**; 77% referenciam um PR `[M]` (`CHANGELOG:7-332`); só o autor `@Ne0nWinds` aparece em **27 linhas** (muitas do back-end Win32, mas também correções do núcleo como `GetFPS()`). `CONTRIBUTING.md:59-64` pede PRs pequenos e "verificar que não quebra o build".
- **JTBD:** *Quando* corrijo um bug ou ajusto uma plataforma, *eu quero* um jeito rápido e automático de confirmar que não quebrei outras funções,
  *para que* meu PR pequeno seja aceito com confiança — hoje a única verificação exigida é "o build não quebra" (`CONTRIBUTING.md:63-64`).
- **Relação com a evolução proposta:** esta é a persona principal da proposta de testes (ver `05-evolucao/`).

### Persona 4 — Autor/mantenedor de *binding*

- **Quem é:** quem encapsula o raylib para outra linguagem (Python, Rust, Jai, Goose…).
- **Evidência:** `BINDINGS.md` lista bindings com a versão suportada (ex.: `raylib-py 6.2`, `raygoose 6.5-dev`) `[L]`; commits recentes do `master` atualizam essa lista (#6345, #6347, #6350) `[GH]`.
- **JTBD:** *Quando* sai uma nova versão do raylib, *eu quero* saber quais comportamentos da API mudaram (ex.: `-WARNING-` no CHANGELOG),
  *para que* eu atualize meu binding sem quebrar usuários.
- **Evidência de dor:** o CHANGELOG 6.2 marca 15 entradas com `-WARNING-` (mudanças de comportamento/API) `[M]`, p.ex. `File*()` com novos valores de retorno e `GetSplinePointBezierQuad()` renomeada.
