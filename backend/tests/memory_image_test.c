/*
 * memory_image_test: the load-file parser (plan-04-B section 4.3) — format
 * by file name, raw binary, every Intel HEX rule and every message, and
 * output left untouched on failure.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_image.h"

static int failures;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* Appends one record, ":LLAAAATT<data>CC", with a correct checksum unless bad >= 0. */
static void record(char *out, unsigned type, unsigned address, const char *data_hex, int bad) {
    unsigned char bytes[300];
    size_t n = strlen(data_hex) / 2;
    unsigned sum;
    size_t i;
    char *end = out + strlen(out);

    for (i = 0; i < n; i++) {
        unsigned v;

        sscanf(data_hex + 2 * i, "%2x", &v);
        bytes[i] = (unsigned char)v;
    }
    sum = (unsigned)n + (address >> 8) + (address & 0xFF) + type;
    for (i = 0; i < n; i++) {
        sum += bytes[i];
    }
    end += sprintf(end, ":%02X%04X%02X%s", (unsigned)n, address, type, data_hex);
    sprintf(end, "%02X", bad >= 0 ? (unsigned)bad : (0x100u - (sum & 0xFF)) & 0xFF);
}

static const char EOF_RECORD[] = ":00000001FF";

/* Parses text (format HEX unless binary) into a 0x5A-prefilled image. */
static int parse(const char *text, size_t size, MemoryImageFormat format, uint32_t words,
                 uint32_t *image, char **error) {
    uint32_t i;

    for (i = 0; i < words; i++) {
        image[i] = 0x5A;
    }
    *error = NULL;
    return memory_image_parse((const unsigned char *)text, size, format, words, 8, image, error);
}

static int untouched(const uint32_t *image, uint32_t words) {
    uint32_t i;

    for (i = 0; i < words; i++) {
        if (image[i] != 0x5A) {
            return 0;
        }
    }
    return 1;
}

/* Expects failure with exactly this message, and the image untouched. */
static void expect_error(const char *text, uint32_t words, const char *message, const char *what) {
    uint32_t *image = malloc(words * sizeof *image);
    char *error = NULL;
    int result = parse(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, words, image, &error);
    int ok = result == -1 && error != NULL && strcmp(error, message) == 0 && untouched(image, words);

    if (!ok) {
        printf("  got %d \"%s\", wanted \"%s\"\n", result, error != NULL ? error : "(none)", message);
    }
    expect(ok, what);
    free(error);
    free(image);
}

/* Expects success; returns the image (caller frees). */
static uint32_t *expect_ok(const char *text, size_t size, MemoryImageFormat format, uint32_t words,
                           const char *what) {
    uint32_t *image = malloc(words * sizeof *image);
    char *error = NULL;
    int result = parse(text, size, format, words, image, &error);

    if (result != 0) {
        printf("  error: %s\n", error != NULL ? error : "(none)");
    }
    expect(result == 0 && error == NULL, what);
    free(error);
    return image;
}

static int all_ff_except(const uint32_t *image, uint32_t words, uint32_t from, uint32_t to) {
    uint32_t i;

    for (i = 0; i < words; i++) {
        if ((i < from || i >= to) && image[i] != 0xFF) {
            return 0;
        }
    }
    return 1;
}

static void check_format_for(void) {
    expect(memory_image_format_for("a.hex") == MEMORY_IMAGE_INTEL_HEX, "format: a.hex is Intel HEX");
    expect(memory_image_format_for("A.HEX") == MEMORY_IMAGE_INTEL_HEX, "format: A.HEX is Intel HEX");
    expect(memory_image_format_for("b.ihx") == MEMORY_IMAGE_INTEL_HEX, "format: b.ihx is Intel HEX");
    expect(memory_image_format_for("c.ihex") == MEMORY_IMAGE_INTEL_HEX, "format: c.ihex is Intel HEX");
    expect(memory_image_format_for("d.bin") == MEMORY_IMAGE_BINARY, "format: d.bin is binary");
    expect(memory_image_format_for("e") == MEMORY_IMAGE_BINARY, "format: e (no extension) is binary");
    expect(memory_image_format_for("f.hex.bin") == MEMORY_IMAGE_BINARY, "format: f.hex.bin is binary");
}

static void check_binary(void) {
    char bytes[257];
    uint32_t *image;
    uint32_t *fail_image = malloc(256 * sizeof *fail_image);
    char *error = NULL;
    int i;
    int ok = 1;

    for (i = 0; i < 257; i++) {
        bytes[i] = (char)(i ^ 0x33);
    }
    image = expect_ok("", 0, MEMORY_IMAGE_BINARY, 256, "binary: empty file loads");
    expect(all_ff_except(image, 256, 0, 0), "binary: empty file is all FF");
    free(image);

    image = expect_ok(bytes, 3, MEMORY_IMAGE_BINARY, 256, "binary: 3 bytes load");
    expect(image[0] == 0x33 && image[1] == 0x32 && image[2] == 0x31 && all_ff_except(image, 256, 0, 3),
           "binary: byte i is word i, the rest FF");
    free(image);

    image = expect_ok(bytes, 256, MEMORY_IMAGE_BINARY, 256, "binary: exactly the memory size loads");
    for (i = 0; i < 256; i++) {
        ok &= image[i] == (uint32_t)((i ^ 0x33) & 0xFF);
    }
    expect(ok, "binary: every word from the file");
    free(image);

    expect(parse(bytes, 257, MEMORY_IMAGE_BINARY, 256, fail_image, &error) == -1 && error != NULL &&
               strcmp(error, "the file is 257 bytes; the memory holds 256") == 0 && untouched(fail_image, 256),
           "binary: one byte too many is refused, image untouched");
    free(error);

    expect(memory_image_parse((const unsigned char *)bytes, 3, MEMORY_IMAGE_BINARY, 256, 16, fail_image,
                              &error) == -1 &&
               error != NULL && strcmp(error, "only 8-bit memories can be loaded from a file") == 0 &&
               untouched(fail_image, 256),
           "bits 16 is refused");
    free(error);
    free(fail_image);
}

static void check_hex_rules(void) {
    char text[2048];
    uint32_t *image;

    /* One data record. */
    text[0] = '\0';
    record(text, 0x00, 0x0010, "0102A0", -1);
    strcat(text, "\n");
    strcat(text, EOF_RECORD);
    strcat(text, "\n");
    image = expect_ok(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, 256, "hex: one data record loads");
    expect(image[0x10] == 0x01 && image[0x11] == 0x02 && image[0x12] == 0xA0 &&
               all_ff_except(image, 256, 0x10, 0x13),
           "hex: data at its address, FF elsewhere");
    free(image);

    /* CRLF, no trailing newline. */
    text[0] = '\0';
    record(text, 0x00, 0x0000, "11", -1);
    strcat(text, "\r\n");
    strcat(text, EOF_RECORD);
    image = expect_ok(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, 256, "hex: CRLF endings, no final newline");
    expect(image[0] == 0x11, "hex: CRLF data");
    free(image);

    /* CR endings. */
    text[0] = '\0';
    record(text, 0x00, 0x0001, "22", -1);
    strcat(text, "\r");
    strcat(text, EOF_RECORD);
    strcat(text, "\r");
    image = expect_ok(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, 256, "hex: CR endings");
    expect(image[1] == 0x22, "hex: CR data");
    free(image);

    /* Blank lines (only whitespace) are skipped. */
    text[0] = '\0';
    record(text, 0x00, 0x0002, "33", -1);
    strcat(text, "\n\n   \t\n");
    strcat(text, EOF_RECORD);
    strcat(text, "\n\n");
    image = expect_ok(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, 256, "hex: blank lines are skipped");
    expect(image[2] == 0x33, "hex: data before blank lines");
    free(image);

    /* Lowercase digits. */
    strcpy(text, ":02000300abcd83\n:00000001ff\n");
    image = expect_ok(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, 256, "hex: lowercase digits");
    expect(image[3] == 0xAB && image[4] == 0xCD, "hex: lowercase data");
    free(image);

    /* Type 02: base = value x 16. */
    text[0] = '\0';
    record(text, 0x02, 0x0000, "0010", -1);
    strcat(text, "\n");
    record(text, 0x00, 0x0005, "44", -1);
    strcat(text, "\n");
    strcat(text, EOF_RECORD);
    image = expect_ok(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, 512, "hex: type 02 segment base");
    expect(image[0x105] == 0x44 && all_ff_except(image, 512, 0x105, 0x106), "hex: 02 base 0x100 + 5 = 0x105");
    free(image);

    /* Type 04: base = value << 16. */
    text[0] = '\0';
    record(text, 0x04, 0x0000, "0001", -1);
    strcat(text, "\n");
    record(text, 0x00, 0x0004, "55", -1);
    strcat(text, "\n");
    strcat(text, EOF_RECORD);
    image = expect_ok(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, 0x10010, "hex: type 04 linear base");
    expect(image[0x10004] == 0x55 && all_ff_except(image, 0x10010, 0x10004, 0x10005),
           "hex: 04 base 0x10000 + 4 = 0x10004");
    free(image);

    /* 03 and 05 are ignored. */
    text[0] = '\0';
    record(text, 0x03, 0x0000, "12345678", -1);
    strcat(text, "\n");
    record(text, 0x05, 0x0000, "00000100", -1);
    strcat(text, "\n");
    record(text, 0x00, 0x0006, "66", -1);
    strcat(text, "\n");
    strcat(text, EOF_RECORD);
    image = expect_ok(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, 256, "hex: 03 and 05 start addresses ignored");
    expect(image[6] == 0x66 && all_ff_except(image, 256, 6, 7), "hex: only the data record lands");
    free(image);

    /* The same address twice: the later wins. */
    text[0] = '\0';
    record(text, 0x00, 0x0007, "7788", -1);
    strcat(text, "\n");
    record(text, 0x00, 0x0008, "99", -1);
    strcat(text, "\n");
    strcat(text, EOF_RECORD);
    image = expect_ok(text, strlen(text), MEMORY_IMAGE_INTEL_HEX, 256, "hex: overlapping records load");
    expect(image[7] == 0x77 && image[8] == 0x99, "hex: the later record wins");
    free(image);
}

static void check_hex_messages(void) {
    char text[1024];

    expect_error("0200000011", 256, "line 1: a record starts with ':'", "hex message: no ':'");
    expect_error(":0100000011E", 256, "line 1: not a hex record", "hex message: odd digit count");
    expect_error(":01000000G1EE\n:00000001FF", 256, "line 1: not a hex record", "hex message: non-hex digit");
    expect_error(":0200000011EE\n:00000001FF", 256, "line 1: the record length does not match its data",
                 "hex message: LL disagrees with the line");

    text[0] = '\0';
    record(text, 0x00, 0x0000, "11", 0x00);
    strcat(text, "\n:00000001FF");
    expect_error(text, 256, "line 1: bad checksum 0x00 (expected 0xEE)", "hex message: bad checksum");

    text[0] = '\0';
    record(text, 0x06, 0x0000, "", -1);
    expect_error(text, 256, "line 1: unknown record type 06", "hex message: unknown record type");

    text[0] = '\0';
    record(text, 0x01, 0x0000, "00", -1);
    expect_error(text, 256, "line 1: record type 01 needs 0 data bytes", "hex message: type 01 with data");
    text[0] = '\0';
    record(text, 0x02, 0x0000, "00", -1);
    expect_error(text, 256, "line 1: record type 02 needs 2 data bytes", "hex message: type 02 with 1 byte");
    text[0] = '\0';
    record(text, 0x03, 0x0000, "0000", -1);
    expect_error(text, 256, "line 1: record type 03 needs 4 data bytes", "hex message: type 03 with 2 bytes");
    text[0] = '\0';
    record(text, 0x04, 0x0000, "000000", -1);
    expect_error(text, 256, "line 1: record type 04 needs 2 data bytes", "hex message: type 04 with 3 bytes");
    text[0] = '\0';
    record(text, 0x05, 0x0000, "00", -1);
    expect_error(text, 256, "line 1: record type 05 needs 4 data bytes", "hex message: type 05 with 1 byte");

    text[0] = '\0';
    record(text, 0x00, 0x00FF, "0102", -1);
    strcat(text, "\n:00000001FF");
    expect_error(text, 256, "line 1: address 0x100 is past the end of the memory (256 bytes)",
                 "hex message: a byte past the end");

    text[0] = '\0';
    strcat(text, ":00000001FF\n\n");
    record(text, 0x00, 0x0000, "11", -1);
    expect_error(text, 256, "line 3: data after the end-of-file record",
                 "hex message: a record after 01 (line count includes the blank line)");

    text[0] = '\0';
    record(text, 0x00, 0x0000, "11", -1);
    expect_error(text, 256, "no end-of-file record", "hex message: no 01");
    expect_error("", 256, "no end-of-file record", "hex message: an empty HEX file");
}

int main(void) {
    check_format_for();
    check_binary();
    check_hex_rules();
    check_hex_messages();
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
