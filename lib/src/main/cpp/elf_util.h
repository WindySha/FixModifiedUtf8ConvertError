#pragma once

#include <linux/elf.h>
#include <sys/types.h>

#if defined(__LP64__)
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Addr Elf_Addr;
typedef Elf64_Dyn Elf_Dyn;
typedef Elf64_Rela Elf_Rela;
typedef Elf64_Sym Elf_Sym;
typedef Elf64_Off Elf_Off;

typedef Elf32_Addr ASM_Instruction;

typedef Elf64_Word Elf_Thumb_Size;

#define ELF_R_SYM(i) ELF64_R_SYM(i)
#else
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Addr Elf_Addr;
typedef Elf32_Dyn Elf_Dyn;
typedef Elf32_Rel Elf_Rela;
typedef Elf32_Sym Elf_Sym;
typedef Elf32_Off Elf_Off;

typedef Elf32_Half ASM_Instruction;

#define ELF_R_SYM(i) ELF32_R_SYM(i)
#endif

typedef struct instruction_code {
    __u8 code1;
    __u8 code2;
//    __u8 code3;
//    __u8 code4;
} Instruction_Code;

namespace BDFixer {

class ElfImg {
public:

    ElfImg(const char *elf);

    Elf_Addr getSymbOffset(const char *name);

    void *getModuleBase(const char *name);

    void *getBaseAddress();

    Elf_Addr getSymbAddress(const char *name);

    Elf_Addr getInstructionAddress(const ASM_Instruction *target_code, size_t length);

    ~ElfImg();

private:
    const char *elf = nullptr;
    void *base = nullptr;
    char *buffer = nullptr;
    off_t size = 0;
    off_t bias = -4396;
    Elf_Ehdr *header = nullptr;
    Elf_Shdr *section_header = nullptr;
    Elf_Shdr *symtab = nullptr;
    Elf_Shdr *strtab = nullptr;
    Elf_Shdr *dynsym = nullptr;
    Elf_Off dynsym_count = 0;
    Elf_Sym *symtab_start = nullptr;
    Elf_Sym *dynsym_start = nullptr;
    Elf_Sym *strtab_start = nullptr;
    Elf_Off symtab_count = 0;
    Elf_Off symstr_offset = 0;
    Elf_Off symstr_offset_for_symtab = 0;
    Elf_Off symtab_offset = 0;
    Elf_Off dynsym_offset = 0;
    Elf_Off symtab_size = 0;
    Elf_Off dynsym_size = 0;
    Elf_Shdr *progbittab = nullptr;
};
}