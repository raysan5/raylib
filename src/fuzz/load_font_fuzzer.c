#include "raylib.h"
#include "stdio.h"
#include "sys/unistd.h"
#include "stdint.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
        char filename[256];
        sprintf(filename, "/tmp/libfuzzer.png");

        FILE *fp = fopen(filename, "wb");
        if (!fp)
                return 0;
        fwrite(data, size, 1, fp);
        fclose(fp);

        Font font = LoadFont(filename);

        unlink(filename);

        return 0;
}
