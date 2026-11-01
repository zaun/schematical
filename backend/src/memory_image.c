/*
 * memory_image: load files (raw binary, Intel HEX) to memory images
 * (plan-04-B section 4.3). Pure functions: bytes in, words out.
 */

#include "memory_image.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MemoryImageFormat memory_image_format_for(const char *file_name) {
    static const char *const HEX[] = {"hex", "ihx", "ihex"};
    const char *dot;
    size_t i;

    if (file_name == NULL || (dot = strrchr(file_name, '.')) == NULL) {
        return MEMORY_IMAGE_BINARY;
    }
    for (i = 0; i < sizeof HEX / sizeof HEX[0]; i++) {
        const char *a = dot + 1;
        const char *b = HEX[i];

        while (*a != '\0' && *b != '\0' && tolower((unsigned char)*a) == *b) {
            a++;
            b++;
        }
        if (*a == '\0' && *b == '\0') {
            return MEMORY_IMAGE_INTEL_HEX;
        }
    }
    return MEMORY_IMAGE_BINARY;
}

/* A malloc'd message in *error. Always returns -1. */
static int failure(char **error, const char *format, ...) {
    va_list args;
    int length;

    if (error == NULL) {
        return -1;
    }
    va_start(args, format);
    length = vsnprintf(NULL, 0, format, args);
    va_end(args);
    *error = malloc((size_t)length + 1);
    if (*error != NULL) {
        va_start(args, format);
        vsnprintf(*error, (size_t)length + 1, format, args);
        va_end(args);
    }
    return -1;
}

static int hex_digit(unsigned char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

static int is_space(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\f' || c == '\v';
}

static int parse_binary(const unsigned char *data, size_t size, uint32_t words, uint32_t *image,
                        char **error) {
    size_t i;

    if (size > words) {
        return failure(error, "the file is %zu bytes; the memory holds %u", size, (unsigned)words);
    }
    for (i = 0; i < size; i++) {
        image[i] = data[i];
    }
    return 0;
}

/* One record's bytes (after ':'), decoded. */
typedef struct {
    unsigned char bytes[256 + 5];
    size_t count;
} Record;

static int parse_hex(const unsigned char *data, size_t size, uint32_t words, uint32_t *image,
                     char **error) {
    Record record;
    uint64_t base = 0;
    int ended = 0;
    size_t line = 0;
    size_t pos = 0;

    while (pos < size) {
        size_t start = pos;
        size_t end;
        size_t first;
        size_t digits;
        size_t i;
        unsigned length, address, type, sum, expected;

        /* One line: up to LF, CR or CRLF. */
        while (pos < size && data[pos] != '\n' && data[pos] != '\r') {
            pos++;
        }
        end = pos;
        if (pos < size && data[pos] == '\r') {
            pos++;
            if (pos < size && data[pos] == '\n') {
                pos++; /* CRLF */
            }
        } else if (pos < size) {
            pos++; /* LF */
        }
        line++;

        first = start;
        while (first < end && is_space(data[first])) {
            first++;
        }
        while (end > first && is_space(data[end - 1])) {
            end--;
        }
        if (first == end) {
            continue; /* blank */
        }
        if (ended) {
            return failure(error, "line %zu: data after the end-of-file record", line);
        }
        if (data[first] != ':') {
            return failure(error, "line %zu: a record starts with ':'", line);
        }
        digits = end - first - 1;
        if (digits % 2 != 0 || digits / 2 > sizeof record.bytes) {
            return failure(error, "line %zu: not a hex record", line);
        }
        record.count = digits / 2;
        for (i = 0; i < record.count; i++) {
            int hi = hex_digit(data[first + 1 + 2 * i]);
            int lo = hex_digit(data[first + 2 + 2 * i]);

            if (hi < 0 || lo < 0) {
                return failure(error, "line %zu: not a hex record", line);
            }
            record.bytes[i] = (unsigned char)(hi * 16 + lo);
        }
        if (record.count < 5 || record.count != 5u + record.bytes[0]) {
            return failure(error, "line %zu: the record length does not match its data", line);
        }
        sum = 0;
        for (i = 0; i + 1 < record.count; i++) {
            sum += record.bytes[i];
        }
        expected = (0x100u - (sum & 0xFFu)) & 0xFFu;
        if (record.bytes[record.count - 1] != expected) {
            return failure(error, "line %zu: bad checksum 0x%02X (expected 0x%02X)", line,
                           (unsigned)record.bytes[record.count - 1], expected);
        }
        length = record.bytes[0];
        address = (unsigned)record.bytes[1] << 8 | record.bytes[2];
        type = record.bytes[3];
        switch (type) {
            case 0x00:
                for (i = 0; i < length; i++) {
                    uint64_t at = base + address + i;

                    if (at >= words) {
                        return failure(error, "line %zu: address 0x%llX is past the end of the memory (%u bytes)",
                                       line, (unsigned long long)at, (unsigned)words);
                    }
                    image[at] = record.bytes[4 + i];
                }
                break;
            case 0x01:
            case 0x02:
            case 0x04:
            case 0x03:
            case 0x05: {
                unsigned needed = type == 0x01 ? 0 : (type == 0x03 || type == 0x05) ? 4 : 2;

                if (length != needed) {
                    return failure(error, "line %zu: record type %02X needs %u data bytes", line, type, needed);
                }
                if (type == 0x01) {
                    ended = 1;
                } else if (type == 0x02) {
                    base = (uint64_t)((unsigned)record.bytes[4] << 8 | record.bytes[5]) * 16;
                } else if (type == 0x04) {
                    base = (uint64_t)((unsigned)record.bytes[4] << 8 | record.bytes[5]) << 16;
                }
                break;
            }
            default:
                return failure(error, "line %zu: unknown record type %02X", line, type);
        }
    }
    if (!ended) {
        return failure(error, "no end-of-file record");
    }
    return 0;
}

int memory_image_parse(const unsigned char *data, size_t size, MemoryImageFormat format,
                       uint32_t words, unsigned bits, uint32_t *values, char **error) {
    uint32_t *image;
    uint32_t i;
    int result;

    if (error != NULL) {
        *error = NULL;
    }
    if (bits != 8) {
        return failure(error, "only 8-bit memories can be loaded from a file");
    }
    image = malloc((words > 0 ? words : 1) * sizeof *image);
    if (image == NULL) {
        return failure(error, "out of memory");
    }
    for (i = 0; i < words; i++) {
        image[i] = 0xFF;
    }
    result = format == MEMORY_IMAGE_INTEL_HEX ? parse_hex(data, size, words, image, error)
                                              : parse_binary(data, size, words, image, error);
    if (result == 0) {
        memcpy(values, image, (size_t)words * sizeof *image);
    }
    free(image);
    return result;
}
