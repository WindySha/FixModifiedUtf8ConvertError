//
// Created by windysha on 2021/11/19.
//

#include <memory>
#include <sys/mman.h>
#include <unistd.h>
#include "func_replace.h"
#include "logger.h"

void DoFuncReplacing(void *origin, void *replace) {
    if (origin == nullptr || replace == nullptr) {
        return;
    }
    void *address_page = (void *) ((size_t) origin & PAGE_MASK);
    if (mprotect(address_page, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return;
    }
#ifdef __LP64__
//    LDR X17, 0x08    // 51000058
//    BR X17           // 20021FD6
//    [TARGET_
//    ADDRESS] (64bit)
    unsigned char szLdrPCOpcodes[8] = {0x51, 0x00, 0x00, 0x58, 0x20, 0x02, 0x1f, 0xd6};
    memcpy(origin, szLdrPCOpcodes, 8);
    memcpy((void *) ((uintptr_t) origin + 8), &replace, 8);

    __builtin___clear_cache(reinterpret_cast<char *>(origin),
                            reinterpret_cast<char *>(origin) + 20);
#else
    bool is_arm_mode = ((uintptr_t) origin & 1) == 0;
    LOGD("DoFuncReplacing is_arm_mode :%d", is_arm_mode);
    if (is_arm_mode) {
        *(uint32_t *) origin = 0XE51FF004; // 04F01FE5    LDR PC, [PC, #-4]
        *(uint32_t *) ((uintptr_t) origin + 4) = (uintptr_t) replace;
    } else {
        int i = 0;
        uintptr_t thumb_addr = (uintptr_t) origin - 1;
        if (thumb_addr % 4 != 0) {
            ((uint16_t *) thumb_addr)[i++] = 0xBF00;  // NOP
        }
        ((uint16_t *) thumb_addr)[i++] = 0xF8DF;
        ((uint16_t *) thumb_addr)[i++] = 0xF000;    // LDR.W PC, [PC]   DFF800F0
        ((uint16_t *) thumb_addr)[i++] = (uintptr_t) (replace) & 0xFFFF;
        ((uint16_t *) thumb_addr)[i++] = (uintptr_t) (replace) >> 16;
    }
    cacheflush((long)address_page, (long)address_page + PAGE_SIZE, 0);
#endif
    mprotect(address_page, PAGE_SIZE, PROT_READ | PROT_EXEC);
}