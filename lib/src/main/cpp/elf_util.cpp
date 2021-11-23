#include <malloc.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "elf_util.h"
#include "logger.h"

using namespace BDFixer;

ElfImg::ElfImg(const char *elf) {
    this->elf = elf;
    //load elf
    int fd = open(elf, O_RDONLY);
    if (fd < 0) {
        LOGE("failed to open %s", elf);
        return;
    }

    size = lseek(fd, 0, SEEK_END);
    if (size <= 0) {
        LOGE("lseek() failed for %s", elf);
    }

    header = reinterpret_cast<Elf_Ehdr *>(mmap(0, size, PROT_READ, MAP_SHARED, fd, 0));

    close(fd);

    section_header = reinterpret_cast<Elf_Shdr *>(((size_t) header) + header->e_shoff);

    size_t shoff = reinterpret_cast<size_t>(section_header);
    char *section_str = reinterpret_cast<char *>(section_header[header->e_shstrndx].sh_offset +
        ((size_t) header));

    for (int i = 0; i < header->e_shnum; i++, shoff += header->e_shentsize) {
        Elf_Shdr *section_h = (Elf_Shdr *) shoff;
        char *sname = section_h->sh_name + section_str;
        Elf_Off entsize = section_h->sh_entsize;
        switch (section_h->sh_type) {
        case SHT_DYNSYM:
            if (bias == -4396) {
                dynsym = section_h;
                dynsym_offset = section_h->sh_offset;
                dynsym_size = section_h->sh_size;
                dynsym_count = dynsym_size / entsize;
                dynsym_start = reinterpret_cast<Elf_Sym *>(((size_t) header) + dynsym_offset);
            }
            break;
        case SHT_SYMTAB:
            if (strcmp(sname, ".symtab") == 0) {
                symtab = section_h;
                symtab_offset = section_h->sh_offset;
                symtab_size = section_h->sh_size;
                symtab_count = symtab_size / entsize;
                symtab_start = reinterpret_cast<Elf_Sym *>(((size_t) header) + symtab_offset);
            }
            break;
        case SHT_STRTAB:
            if (bias == -4396) {
                strtab = section_h;
                symstr_offset = section_h->sh_offset;
                strtab_start = reinterpret_cast<Elf_Sym *>(((size_t) header) + symstr_offset);
            }
            if (strcmp(sname, ".strtab") == 0) {
                symstr_offset_for_symtab = section_h->sh_offset;
            }
            break;
        case SHT_PROGBITS:
            if (strtab == nullptr || dynsym == nullptr)
                break;
            if (bias == -4396) {
                bias = (off_t) section_h->sh_addr - (off_t) section_h->sh_offset;
            }
            if (strcmp(sname, ".text") == 0) {
                progbittab = section_h;
            }
            break;
        }
    }

    if (!symtab_offset) {
        LOGD("can't find symtab from sections\n");
    }

    //load module base
    base = getModuleBase(elf);
}

ElfImg::~ElfImg() {
    //open elf file local
    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
    //use mmap
    if (header) {
        munmap(header, size);
    }
}

Elf_Addr ElfImg::getSymbOffset(const char *name) {
    Elf_Addr _offset = 0;

    //search dynmtab
    if (dynsym_start != nullptr && strtab_start != nullptr) {
        Elf_Sym *sym = dynsym_start;
        char *strings = (char *) strtab_start;
        int k;
        for (k = 0; k < dynsym_count; k++, sym++)
            if (strcmp(strings + sym->st_name, name) == 0) {
                _offset = sym->st_value;
                LOGD("find %s: %x\n", elf, _offset);
                return _offset;
            }
    }

    //search symtab
    if (symtab_start != nullptr && symstr_offset_for_symtab != 0) {
        for (int i = 0; i < symtab_count; i++) {
            unsigned int st_type = ELF_ST_TYPE(symtab_start[i].st_info);
            char *st_name = reinterpret_cast<char *>(((size_t) header) + symstr_offset_for_symtab +
                symtab_start[i].st_name);
            if (st_type == STT_FUNC && symtab_start[i].st_size) {
                if (strcmp(st_name, name) == 0) {
                    _offset = symtab_start[i].st_value;
                    LOGD("find %s: %x\n", elf, _offset);
                    return _offset;
                }
            }
        }
    }
    return 0;
}

Elf_Addr ElfImg::getSymbAddress(const char *name) {
    Elf_Addr offset = getSymbOffset(name);
    if (offset > 0 && base != nullptr) {
        return static_cast<Elf_Addr>((size_t) base + offset - bias);
    } else {
        return 0;
    }
}

Elf_Addr ElfImg::getInstructionAddress(const ASM_Instruction *target_code, size_t length) {
    if (progbittab == nullptr)
        return 0;
    Elf32_Off instr_offset = progbittab->sh_offset;
    LOGE("sh_offset add = %4x  offset = %4x size = %d   type = %d ",
         progbittab->sh_addr,
         instr_offset,
         progbittab->sh_size,
         progbittab->sh_type);

    char *section_str = reinterpret_cast<char *>(section_header[header->e_shstrndx].sh_offset +
        ((size_t) header));
    char *sname = progbittab->sh_name + section_str;
    LOGE(" sym name = %s ", sname);

    auto instr_start = reinterpret_cast<ASM_Instruction *>(((size_t) header) + instr_offset);
    size_t instr_code_size = sizeof(ASM_Instruction);
    size_t instruc_offset = 0;
    for (int i = 0; i < progbittab->sh_size; i += instr_code_size, instr_start++) {
        bool instruction_not_matched = false;
        for (int j = 0; j < length; j++) {
            if (*(instr_start + j) != *(target_code + j)) {
                instruction_not_matched = true;
                break;
            }
            LOGD("instr_founded value  %4x, num = %d", *(instr_start + j), j);
        }
        if (!instruction_not_matched) {
            instruc_offset = i;
            break;
        }
    }
    auto off = progbittab->sh_addr + instruc_offset;
    LOGE("111 target address base = %p       off = %4x  target_off = %d  bias=%d",
         base,
         off,
         instruc_offset,
         bias);
    return static_cast<Elf_Addr>((size_t) base + off - bias);
}

void *ElfImg::getBaseAddress() {
    return base;
}

void *ElfImg::getModuleBase(const char *name) {
    FILE *maps;
    char buff[256];
    off_t load_addr;
    maps = fopen("/proc/self/maps", "r");
    int result = 0;
    while (fgets(buff, sizeof(buff), maps)) {
        if ((strstr(buff, "r-xp") || strstr(buff, "r--p")) && strstr(buff, name)) {
            result = sscanf(buff, "%lx", &load_addr);
            LOGD("dlopen,  %s\n    result = %d", buff, result);
            break;
        }
    }
    fclose(maps);
    if (result != 1) {
        LOGE("failed to read load address for %s", name);
        return nullptr;
    }

    LOGD("get module base %s: %p", name, load_addr);
    return reinterpret_cast<void *>(load_addr);
}