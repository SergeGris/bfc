CC=gcc
IDIR=
LDIR=
ODIR=obj
CFLAGS=-std=c11 -O2

_OBJ=main.o tokenizer.o compiler.o compile_asm_i386_linux.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))
DEPS=tokenizer.h compiler.h compile_asm_i386_linux.h
TARGET=bfc

all: main

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(IDIR) $(LDIR)

main: $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS) $(IDIR) $(LDIR)
	rm -rf $(ODIR)

