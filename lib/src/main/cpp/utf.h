//
// Created by windysha on 2021/11/19.
//

#pragma once

#include "stdint.h"

namespace BDFixer {

void ConvertUtf16ToModifiedUtf8(char *utf8_out, const uint16_t *utf16_in, size_t char_count);
void ConvertModifiedUtf8ToUtf16(uint16_t *utf16_data_out, const char *utf8_data_in);
size_t CountUtf8Bytes(const uint16_t *chars, size_t char_count);
size_t CountModifiedUtf8Chars(const char *utf8);
}