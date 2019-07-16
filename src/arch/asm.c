
#if defined(__x86_64__) || defined(_AMD64_) || defined(__i386__)
# include "x86-asm.c"
#elif defined(__arm__)
# include "arm-asm.c"
#elif defined(__aarch64__)
# include "arm64-asm.c"
#endif
