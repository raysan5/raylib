# 7 — Ética e responsabilidade

## 1 — Segurança e responsabilidade sobre vulnerabilidades

### O problema

- **O raylib é base de outros produtos.** Jogos, ferramentas e +70 *bindings* embutem o código (`README.md:53`, `BINDINGS.md`); uma falha na biblioteca se propaga a todos os dependentes, que raramente têm como corrigi-la sozinhos.
- **Falhas de memória reais aparecem em funções "inocentes".** O PR **#6348** (aberto quando fizemos a análise; mesclado em 09/11/2026) descreve que `GetCodepoint()` pode devolver um tamanho que faz o chamador ler **além do `'\0'`** em UTF-8 truncado — a descrição do PR cita detecção por AddressSanitizer `[GH]`. Nós reproduzimos o sintoma no código de 31/10: 3 entradas truncadas devolviam tamanho > `strlen` (testes `XFAIL`); após o merge do PR, os mesmos testes passaram (`XPASS`) e foram promovidos a `CHECK` `[M]`. Também verificamos que `GetFileExtension(NULL)` derruba o processo (violação de acesso), enquanto funções vizinhas tratam `NULL` `[M]`.
- **Funções de integridade podem estar silenciosamente erradas.** `ComputeSHA256()` retornou hash incorreto para `tamanho mod 64 ∈ 56..59` (issue #6143 / PR #6144). Quem usou o hash para verificar integridade tinha uma garantia falsa.
- **O processo de divulgação é informal.** `SECURITY.md` manda reportar vulnerabilidades "diretamente em Issues e/ou Discussions" (públicas), e a tabela de versões suportadas lista `6.0.x` e `< 5.5` — **não menciona 6.2** (release de 15/10/2026), então está desatualizada `[L]`. Por outro lado, o projeto passou por **auditoria externa (ROS)** e tratou achados como `OpenURL()` (`CLN-002`) e eventos de automação (`CLN-005/006`) `[L]` `CHANGELOG:13,31-32`; roda CodeQL semanalmente (`analyze_codeql.yml`).

### Responsabilidade do engenheiro de software

1. **Tratar entrada externa como não confiável** (URLs, arquivos, strings UTF-8, caminhos) e validar limites — dever profissional, não só "boa prática".
2. **Divulgar com cuidado.** Um bug de leitura fora dos limites pode ser explorável; relatar publicamente sem avaliar o impacto pode expor usuários. O correto é seguir o canal que o projeto define **e** propor um canal privado (ex.: *GitHub Security Advisories*) quando houver risco de exploração. Antes de abrir qualquer issue sobre `GetFileExtension(NULL)`/`GetCodepoint`, a dupla deve **verificar** se já existe relato e avaliar se há risco real — **não afirmamos que haja vulnerabilidade explorável**, apenas comportamento incorreto reproduzido.
3. **Sinalizar mudanças que quebram contrato** (o `-WARNING-` do CHANGELOG é um bom exemplo) e **manter a política de suporte atualizada**.
4. **Verificar o que se publica.** Testes de regressão (nossa evolução) são a forma concreta de assumir responsabilidade: reduzem a chance de enviar defeitos conhecidos a terceiros.
5. **Honestidade sobre limites**: nossos testes foram executados no Windows/MSYS2 e no Linux (CI do GitHub, que falhou na 1ª execução e só ficou verde após um ajuste de dependências); **macOS não foi testado** — dizer isso é parte da responsabilidade.

---

## 2 — Licenciamento e reuso de código de terceiros

### O problema

- O raylib é **zlib/libpng** (permissiva) — permite uso comercial, mas exige: não alegar autoria, **marcar versões alteradas**, manter o aviso (`LICENSE`; `rcore.c:71-88`).
- Ele **embute** ~266 mil linhas de bibliotecas com licenças **diferentes** em `src/external/` `[M]`. Verificamos nos cabeçalhos `[L]`: `miniaudio.h` — domínio público **ou** MIT-0 (`:2`); `stb_image.h` — domínio público/2 licenças (`:1`, `:7950`); `cgltf.h`, `par_shapes.h`, `tinyobj_loader_c.h` — **MIT** (exige preservar o aviso); `jar_xm.h` — licença **WTFPL v2** (`:3-11`); GLFW — `external/glfw/LICENSE.md`. O README remete à Wiki "raylib dependencies" para o quadro completo (`README.md:162`).
- Os exemplos usam **recursos** (imagens, sons, fontes) que precisam ter licença permissiva (exigência do fluxo do `rexm`, `tools/rexm/README.md:15`).
- **Nosso trabalho** cria um *fork* modificado e usa IA generativa para auxiliar: precisamos respeitar a cláusula 2 da zlib (versão alterada deve ser claramente marcada) e ser transparentes sobre autoria.

### Responsabilidade do engenheiro de software

1. **Conhecer e cumprir as obrigações de cada licença** ao redistribuir binários (ex.: reproduzir avisos MIT de `cgltf`/`par_shapes`/`tinyobj`), e **não assumir que "permissiva" = "sem obrigações"**.
2. **Rastrear dependências** e suas licenças (inventário/SBOM), e reavaliar a cada atualização de biblioteca embutida.
3. **Marcar claramente** o *fork* como alterado (`README.md` da dupla) e **não usar a marca/nome de forma enganosa**.
4. **Atribuição e IA:** declarar o uso de IA (`08-ia/ia.md`) e conferir que trecho gerado não reproduz código de licença incompatível; o código dos testes é nosso e é oferecido sob zlib, coerente com o projeto (`TODO: confirmar com a dupla a licença do fork`).
5. **Respeitar a licença dos recursos** (assets) incluídos em exemplos e não copiar documentação do projeto como se fosse própria (critério 13 do enunciado).

---

## 3 — Acessibilidade e inclusão linguística (impacto social)

### O problema

- O raylib se apresenta como ferramenta de **educação** e é gratuito para escolas (`CONTRIBUTING.md:20`; `HISTORY.md`). Mas o próprio README declara a limitação: **o desenho de texto não suporta RTL, ligaduras nem emojis** (`README.md:65`) e a fonte padrão não consegue renderizar o caractere de substituição U+FFFD, por isso `GetCodepoint()` devolve `?` em erros (`src/rtext.c:2432-2436`) `[L]`.
- Efeito: textos em **árabe, hebraico** (direita-para-esquerda) e escritas que dependem de ligaduras/*shaping* **não são renderizados corretamente** pela biblioteca (inferência a partir da limitação declarada — não testamos esses idiomas); alunos e jogadores desses idiomas ficam à margem. O README também lista como limitação a rasterização de fontes inferior à de alternativas (`README.md:64`).
- Os jogos construídos herdam essas limitações; o projeto **não oferece APIs de acessibilidade** (leitor de tela, contraste, etc.) — coerente com a filosofia "sem interface, sem ajudantes visuais" (`README.md:7`), mas com impacto sobre pessoas com deficiência.

### Responsabilidade do engenheiro de software

1. **Ser transparente sobre limites** — o projeto faz isso no README; quem constrói produtos sobre ele deve repassar a informação aos usuários finais.
2. **Projetar para inclusão desde o início**: não tratar texto como "somente latino"; considerar internacionalização, contraste e alternativas de entrada em jogos e ferramentas educacionais.
3. **Priorizar de forma consciente**: o `ROADMAP.md` cita investigar o algoritmo *Slug* de renderização de fontes (`ROADMAP.md:27`); é uma oportunidade de melhoria com impacto social, e o engenheiro deve defender isso, com dados de usuários afetados, em vez de tratar como "luxo".
4. **Não prometer o que o sistema não entrega** (ex.: dizer que "suporta todos os idiomas").

