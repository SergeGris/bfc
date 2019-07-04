ODIR = obj
COMPILER=../bft
ASSEMBLER=nasm -f elf32
LINKER=ld -m elf_i386 -s -o
OUTFILES=hello-world print-a cat infinite-loop

all: $(OUTFILES)

hello-world: hello-world.o
	$(LINKER) $@ $(ODIR)/$<

print-a: print-a.o
	$(LINKER) $@ $(ODIR)/$<

cat: cat.o
	$(LINKER) $@ $(ODIR)/$<

infinite-loop: infinite-loop.o
	$(LINKER) $@ $(ODIR)/$<

%.o: $(ODIR)/%.asm
	$(ASSEMBLER) $<

$(ODIR)/%.asm: %.bf $(COMPILER)
	mkdir -p $(ODIR)
	$(COMPILER) $< $@

.PHONY: clean all

.PRECIOUS: $(ODIR)/%.asm

clean:
	rm -rf $(ODIR)
	rm -f $(OUTFILES)
