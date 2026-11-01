# tests/ — testes headless do raylib

Testes de regressão das funções puras de `rcore.c` e `rtext.c`. **Não abrem janela nem contexto OpenGL**; só linkam com `libraylib.a`.

```bash
cd ../src && make PLATFORM=PLATFORM_DESKTOP   # gera libraylib.a (Windows/MSYS2: mingw32-make)
cd ../tests && make test
```

## Escrevendo um teste novo

```c
#include "raylib.h"
#include "rtest.h"

static void test_minha_funcao(void)
{
    CHECK_STR(".png", GetFileExtension("a/b.png"));   // comparação de strings
    CHECK_INT(3, GetCodepointCount("a\xC3\xA9b"));    // comparação de inteiros
    CHECK(IsPathAbsolute("") == false);               // condição booleana

    // Bug conhecido e ainda aberto: não deixa o CI vermelho; vira XPASS quando for corrigido
    KNOWN_BUG(resultado_correto, "issue/PR #NNNN");
}

int main(void)
{
    SetTraceLogLevel(LOG_NONE);
    RUN(test_minha_funcao);
    return RTEST_SUMMARY();
}
```

Depois adicione o nome do arquivo (sem `.c`) à variável `TESTS` do `Makefile`.

## Regras

1. **Headless:** não chame `InitWindow()`, `LoadTexture()` etc. (precisam de GPU).
2. **Valores esperados independentes:** gere-os com outra ferramenta/RFC, não com o próprio raylib.
3. **Um teste que nunca falhou não prova nada:** faça uma mutação no código e confira que o teste falha.
4. **Não teste o que derruba o processo** (ex.: passar `NULL` a função que não trata `NULL`); documente como issue.
