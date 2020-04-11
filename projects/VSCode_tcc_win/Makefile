
MAIN_CFILE = main.c
SUPPORT_CFILES = 

#**************************************************************************************************
# raylib makefile for TCC
#**************************************************************************************************
CC = tcc
CFLAGS = -vv -std=c99 -Wall -lmsvcrt -lraylib -lopengl32 -lgdi32 -lkernel32 -lshell32 -luser32 -lwinmm -Wl,-subsystem=gui 
OUTPUT = $(outputExe)
all:
	$(CC) -o $(OUTPUT) $(MAIN_CFILE) $(SUPPORT_CFILES) $(CFLAGS)