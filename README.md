# Brainfuck compiler
Compiles Brainfuck source code to executable file.

Brainfuck programming language is described at Wikipedia: https://en.wikipedia.org/wiki/Brainfuck

Current version of this compiler produces assembly source code,
which is targeted to x86 CPU and uses Linux system calls.
Assembly source code has to be compiled with NASM to create executable binary.

Install:
    
    make && sudo make install
    
Uninstall:

    sudo make uninstall

Usage (requires NASM):

    ./bfc [source filename] [assembler output filename]
    nasm -f elf32 [assembler output filename] -o [object output filename]
    ld -m elf_i386 -s [object output filename] -o [ELF output filename]

###### In the future, I will change the NASM to GAS for greater portability.
