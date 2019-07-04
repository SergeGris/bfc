IDIR =-I/usr/include -I..
LDIR =-I/usr/lib
ODIR =obj
FLAGS =-std=c11 -O3 -Wall -lm -lcmocka

_OBJ = tests.o test_tokenizer.o ../tokenizer.o ../compiler.o ../compile_asm_i386_linux.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
DEPS = ../tokenizer.h ../compiler.h ../compile_asm_i386_linux.h

all: tests

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(ODIR)
	gcc -c -o $@ $< $(IDIR) $(LDIR) $(FLAGS)

tests: $(OBJ)
	gcc -o tests $(OBJ) $(IDIR) $(LDIR) $(FLAGS)

.PHONY: clean tests

clean:
	rm -rf $(ODIR)
	rm -f tests
