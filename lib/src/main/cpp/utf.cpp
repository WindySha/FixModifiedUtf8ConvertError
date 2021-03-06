//
// Created by windysha on 2021/11/19.
//

#include "utf.h"
#include <jni.h>

namespace BDFixer {

inline uint16_t GetTrailingUtf16Char(uint32_t maybe_pair) {
    return static_cast<uint16_t>(maybe_pair >> 16);
}

inline uint16_t GetLeadingUtf16Char(uint32_t maybe_pair) {
    return static_cast<uint16_t>(maybe_pair & 0x0000FFFF);
}

inline uint32_t GetUtf16FromUtf8(const char **utf8_data_in) {
    const uint8_t one = *(*utf8_data_in)++;
    if ((one & 0x80) == 0) {
        // one-byte encoding
        return one;
    }

    const uint8_t two = *(*utf8_data_in)++;
    if ((one & 0x20) == 0) {
        // two-byte encoding
        return ((one & 0x1f) << 6) | (two & 0x3f);
    }

    const uint8_t three = *(*utf8_data_in)++;
    if ((one & 0x10) == 0) {
        return ((one & 0x0f) << 12) | ((two & 0x3f) << 6) | (three & 0x3f);
    }

    // Four byte encodings need special handling. We'll have
    // to convert them into a surrogate pair.
    const uint8_t four = *(*utf8_data_in)++;

    // Since this is a 4 byte UTF-8 sequence, it will lie between
    // U+10000 and U+1FFFFF.
    //
    // TODO: What do we do about values in (U+10FFFF, U+1FFFFF) ? The
    // spec says they're invalid but nobody appears to check for them.
    const uint32_t code_point = ((one & 0x0f) << 18) | ((two & 0x3f) << 12)
        | ((three & 0x3f) << 6) | (four & 0x3f);

    uint32_t surrogate_pair = 0;
    // Step two: Write out the high (leading) surrogate to the bottom 16 bits
    // of the of the 32 bit type.
    surrogate_pair |= ((code_point >> 10) + 0xd7c0) & 0xffff;
    // Step three : Write out the low (trailing) surrogate to the top 16 bits.
    surrogate_pair |= ((code_point & 0x03ff) + 0xdc00) << 16;

    return surrogate_pair;
}

void ConvertModifiedUtf8ToUtf16(uint16_t *utf16_data_out, const char *utf8_data_in) {
    while (*utf8_data_in != '\0') {
        const uint32_t ch = GetUtf16FromUtf8(&utf8_data_in);
        const uint16_t leading = GetLeadingUtf16Char(ch);
        const uint16_t trailing = GetTrailingUtf16Char(ch);

        *utf16_data_out++ = leading;
        if (trailing != 0) {
            *utf16_data_out++ = trailing;
        }
    }
}

void ConvertUtf16ToModifiedUtf8(char *utf8_out, const uint16_t *utf16_in, size_t char_count) {
    while (char_count--) {
        const uint16_t ch = *utf16_in++;
        if (ch > 0 && ch <= 0x7f) {
            *utf8_out++ = ch;
        } else {
            // char_count == 0 here implies we've encountered an unpaired
            // surrogate and we have no choice but to encode it as 3-byte UTF
            // sequence. Note that unpaired surrogates can occur as a part of
            // "normal" operation.
            if ((ch >= 0xd800 && ch <= 0xdbff) && (char_count > 0)) {
                const uint16_t ch2 = *utf16_in;

                // Check if the other half of the pair is within the expected
                // range. If it isn't, we will have to emit both "halves" as
                // separate 3 byte sequences.
                if (ch2 >= 0xdc00 && ch2 <= 0xdfff) {
                    utf16_in++;
                    char_count--;
                    const uint32_t code_point = (ch << 10) + ch2 - 0x035fdc00;
                    *utf8_out++ = (code_point >> 18) | 0xf0;
                    *utf8_out++ = ((code_point >> 12) & 0x3f) | 0x80;
                    *utf8_out++ = ((code_point >> 6) & 0x3f) | 0x80;
                    *utf8_out++ = (code_point & 0x3f) | 0x80;
                    continue;
                }
            }

            if (ch > 0x07ff) {
                // Three byte encoding.
                *utf8_out++ = (ch >> 12) | 0xe0;
                *utf8_out++ = ((ch >> 6) & 0x3f) | 0x80;
                *utf8_out++ = (ch & 0x3f) | 0x80;
            } else /*(ch > 0x7f || ch == 0)*/ {
                // Two byte encoding.
                *utf8_out++ = (ch >> 6) | 0xc0;
                *utf8_out++ = (ch & 0x3f) | 0x80;
            }
        }
    }
}

void ConvertUtf16ToModifiedUtf8(char *utf8_out, size_t byte_count,
                                const uint16_t *utf16_in, size_t char_count) {
//    if (LIKELY(byte_count == char_count)) {
//        // Common case where all characters are ASCII.
//        const uint16_t *utf16_end = utf16_in + char_count;
//        for (const uint16_t *p = utf16_in; p < utf16_end;) {
//            *utf8_out++ = dchecked_integral_cast<char>(*p++);
//        }
//        return;
//    }

    // String contains non-ASCII characters.
    while (char_count--) {
        const uint16_t ch = *utf16_in++;
        if (ch > 0 && ch <= 0x7f) {
            *utf8_out++ = ch;
        } else {
            // Char_count == 0 here implies we've encountered an unpaired
            // surrogate and we have no choice but to encode it as 3-byte UTF
            // sequence. Note that unpaired surrogates can occur as a part of
            // "normal" operation.
            if ((ch >= 0xd800 && ch <= 0xdbff) && (char_count > 0)) {
                const uint16_t ch2 = *utf16_in;

                // Check if the other half of the pair is within the expected
                // range. If it isn't, we will have to emit both "halves" as
                // separate 3 byte sequences.
                if (ch2 >= 0xdc00 && ch2 <= 0xdfff) {
                    utf16_in++;
                    char_count--;
                    const uint32_t code_point = (ch << 10) + ch2 - 0x035fdc00;
                    *utf8_out++ = (code_point >> 18) | 0xf0;
                    *utf8_out++ = ((code_point >> 12) & 0x3f) | 0x80;
                    *utf8_out++ = ((code_point >> 6) & 0x3f) | 0x80;
                    *utf8_out++ = (code_point & 0x3f) | 0x80;
                    continue;
                }
            }

            if (ch > 0x07ff) {
                // Three byte encoding.
                *utf8_out++ = (ch >> 12) | 0xe0;
                *utf8_out++ = ((ch >> 6) & 0x3f) | 0x80;
                *utf8_out++ = (ch & 0x3f) | 0x80;
            } else /*(ch > 0x7f || ch == 0)*/ {
                // Two byte encoding.
                *utf8_out++ = (ch >> 6) | 0xc0;
                *utf8_out++ = (ch & 0x3f) | 0x80;
            }
        }
    }
}

size_t CountUtf8Bytes(const uint16_t *chars, size_t char_count) {
    size_t result = 0;
    while (char_count--) {
        const uint16_t ch = *chars++;
        if (ch > 0 && ch <= 0x7f) {
            ++result;
        } else if (ch >= 0xd800 && ch <= 0xdbff) {
            if (char_count > 0) {
                const uint16_t ch2 = *chars;
                // If we find a properly paired surrogate, we emit it as a 4 byte
                // UTF sequence. If we find an unpaired leading or trailing surrogate,
                // we emit it as a 3 byte sequence like would have done earlier.
                if (ch2 >= 0xdc00 && ch2 <= 0xdfff) {
                    chars++;
                    char_count--;

                    result += 4;
                } else {
                    result += 3;
                }
            } else {
                // This implies we found an unpaired trailing surrogate at the end
                // of a string.
                result += 3;
            }
        } else if (ch > 0x7ff) {
            result += 3;
        } else {
            result += 2;
        }
    }
    return result;
}

size_t CountModifiedUtf8Chars(const char *utf8) {
    size_t len = 0;
    int ic;
    while ((ic = *utf8++) != '\0') {
        len++;
        if ((ic & 0x80) == 0) {
            // one-byte encoding
            continue;
        }
        // two- or three-byte encoding
        utf8++;
        if ((ic & 0x20) == 0) {
            // two-byte encoding
            continue;
        }
        utf8++;
        if ((ic & 0x10) == 0) {
            // three-byte encoding
            continue;
        }

        // four-byte encoding: needs to be converted into a surrogate
        // pair.
        utf8++;
        len++;
    }
    return len;
}
}