/*
 * The `ccsi` display driver (plan-56-D, TLC6983; docs/parts-format.md § leddriver, driver "ccsi"): a
 * 48 x 16 common-cathode RGB matrix LED driver on TI's continuous clock series interface.
 *
 * The interface (tlc6983.pdf p.24-28): SCLK never stops and SIN is sampled on both of its edges;
 * SIN idles high, a frame starts when it falls (START), carries a 16-bit command ID and then
 * N x 48 bits of data as 17-bit words (16 data bits, MSB first, and a check bit equal to NOT the
 * 16th bit), and ends with 18 consecutive highs (END). A write frame with data takes the data of
 * the last unit (3 words: bits 47-32 "B", 31-16 "G", 15-0 "R"); a read frame is answered on SOUT with
 * the command ID and the register's 48 bits in the same word format. The register file (FC0-FC4,
 * FC10, FC11 with the reset values of the register figures), the two-bank grayscale memory with the
 * write counters, VSYNC (bank swap) and Soft_Reset are kept here; the outputs are scanned by
 * display_out.c's timer: one LINEn pulled low in turn (open drain, the others released) and the
 * colour outputs Rn / Gn / Bn of that line's pixels driven high while their gray value is nonzero.
 */


#include <stdlib.h>

#include "converter_int.h"
#include "display_ccsi.h"
#include "logic_alloc.h"

enum { LINES = 16, PIXELS = 16, MAX_FRAME = 2048, TX_MAX = 128 };

enum { ID_FC0 = 0xAA00, ID_FC1, ID_FC2, ID_FC3, ID_FC4, ID_FC10 = 0xAA0A, ID_FC11,
       ID_INDEX = 0xAA10, ID_SRAM = 0xAA30, ID_SOFT = 0xAA80, ID_VSYNC = 0xAAF0, ID_READ = 0x60,
       ID_INDEX_READ = 0xAA70 };

typedef struct CcsiState {
    uint64_t fc[7];             /* FC0 FC1 FC2 FC3 FC4 FC10 FC11 */
    uint64_t fc01_shadow[2];    /* written FC0 / FC1, applied at VSYNC (p.50) */
    int has_shadow[2];
    int index;
    unsigned bank_sel, line_count, chan_count;
    uint64_t *sram;             /* [bank][32 lines][16 pixels] */
    int in_frame, ones;
    size_t nbits;
    uint8_t bits[MAX_FRAME];
    uint8_t tx[TX_MAX];
    size_t tx_len, tx_pos;
    LogicValue sclk;
} CcsiState;

static const uint64_t FC_MASK[7] = {
    (3ull << 46) | (0x1FFull << 35) | (0x1Full << 27) | (0x7ull << 21) | (0x1Full << 16) | (0xFull << 12) |
        (1ull << 8) | 0x1Full,
    (1ull << 47) - 1,
    ((1ull << 36) - 1) | (0xFull << 39),
    (1ull << 48) - 1,
    1ull | 8ull | (1ull << 14) | (0xFFFull << 16) | (0x1FFFull << 32),
    0x1F, 0x1F
};

static void reset_registers(CcsiState *c) {
    c->fc[0] = (1ull << 44) | (0x7ull << 28) | (0xFull << 16) | 0x107ull;
    c->fc[1] = (7ull << 37) | (9ull << 20) | (9ull << 15) | (9ull << 10);
    c->fc[2] = 0x666ull | (7ull << 39);
    c->fc[3] = (0x7Full << 8) | (0x7Full << 16) | (0x7Full << 24) | (3ull << 32) | (7ull << 35);
    c->fc[4] = (1ull << 14) | (1ull << 22) | (1ull << 24) | (1ull << 26) | (8ull << 40);
    c->fc[5] = c->fc[6] = 0;
    c->has_shadow[0] = c->has_shadow[1] = 0;
}

CcsiState *ccsi_new(void) {
    CcsiState *c = logic_calloc(1, sizeof *c);

    if (c == NULL) {
        return NULL;
    }
    c->sram = logic_calloc((size_t)2 * 32 * PIXELS, sizeof *c->sram);
    if (c->sram == NULL) {
        logic_free(c);
        return NULL;
    }
    reset_registers(c);
    c->sclk = LOGIC_X;
    return c;
}

void ccsi_free(CcsiState *c) {
    if (c != NULL) {
        logic_free(c->sram);
        logic_free(c);
    }
}

/* SCAN_NUM + 1 lines are scanned (FC0 bits 20-16), at most 32. */
unsigned ccsi_lines(const CcsiState *c) {
    return (unsigned)((c->fc[0] >> 16) & 0x1Fu) + 1u;
}

/* ---- Frames --------------------------------------------------------------------------------- */

static unsigned word_at(const uint8_t *bits, size_t at) {
    unsigned w = 0, i;

    for (i = 0; i < 16; i++) {
        w = (w << 1) | bits[at + i];
    }
    return w;
}

static uint64_t unit_at(const uint8_t *bits, size_t at) {
    return ((uint64_t)word_at(bits, at) << 32) | ((uint64_t)word_at(bits, at + 17) << 16) |
           (uint64_t)word_at(bits, at + 34);
}

static void tx_word(CcsiState *c, unsigned w) {
    int i;

    for (i = 15; i >= 0; i--) {
        c->tx[c->tx_len++] = (uint8_t)((w >> i) & 1u);
    }
    c->tx[c->tx_len++] = (uint8_t)(((w >> 0) & 1u) == 0);   /* the check bit: NOT the 16th (last) bit */
}

static int fc_slot(unsigned id) {
    switch (id) {
    case ID_FC0: case ID_FC1: case ID_FC2: case ID_FC3: case ID_FC4:
        return (int)(id - ID_FC0);
    case ID_FC10:
        return 5;
    case ID_FC11:
        return 6;
    default:
        return -1;
    }
}

static void write_sram(CcsiState *c, uint64_t unit) {
    unsigned bank = c->bank_sel & 1u;

    c->sram[((size_t)bank * 32 + c->line_count) * PIXELS + c->chan_count] = unit;
    if (++c->chan_count >= PIXELS) {
        c->chan_count = 0;
        if (++c->line_count >= ccsi_lines(c)) {
            c->line_count = 0;
        }
    }
}

/* A frame of n bits (the command and its data, without START and END); a bad length or check bit
   drops it: nothing changes. */
static void frame(CcsiState *c, size_t n) {
    unsigned id;
    size_t at, units;
    int slot;

    if (n < 17 || (n - 17) % 17 != 0) {
        return;
    }
    for (at = 0; at < n; at += 17) {
        if (c->bits[at + 16] != (uint8_t)!c->bits[at + 15]) {
                return;
        }
    }
    units = (n - 17) / 51;
    if ((n - 17) % 51 != 0) {
        return;
    }
    id = word_at(c->bits, 0);
    if ((slot = fc_slot(id)) >= 0 && units >= 1) {
        uint64_t v = unit_at(c->bits, 17 + (units - 1) * 51) & FC_MASK[slot];

        if (slot == 0 || slot == 1) {
            c->fc01_shadow[slot] = v;       /* FC0 and FC1 update after VSYNC (p.50) */
            c->has_shadow[slot] = 1;
        } else {
            c->fc[slot] = (c->fc[slot] & ~FC_MASK[slot]) | v;
        }
    } else if (id == ID_SRAM && units >= 1) {
        write_sram(c, unit_at(c->bits, 17 + (units - 1) * 51));
    } else if (id == ID_INDEX) {
        c->index = 0;
    } else if (id == ID_SOFT) {
        reset_registers(c);
    } else if (id == ID_VSYNC) {
        c->bank_sel ^= 1u;
        c->line_count = c->chan_count = 0;
        if (c->has_shadow[0]) {
            c->fc[0] = c->fc01_shadow[0] | (c->fc[0] & ~FC_MASK[0]);
        }
        if (c->has_shadow[1]) {
            c->fc[1] = c->fc01_shadow[1] | (c->fc[1] & ~FC_MASK[1]);
        }
        c->has_shadow[0] = c->has_shadow[1] = 0;
    } else if ((id & 0xFFF0u) == (0xAA00u | ID_READ) || id == ID_INDEX_READ) {
        /* read (§ 8.7, Table 8-5 Register Maps): FC0-FC4 (AA60-64), FC10-11 (AA6A-B), FC12-15 (AA6C-F, the
           LOD / LSD flags: none here, 0), chip index (AA70); AA65-69 are no register */
        uint64_t v = 0;
        unsigned low = id & 0x0Fu;

        if (id == ID_INDEX_READ) {
            v = (uint64_t)c->index;
        } else if (low <= 4) {
            v = c->fc[low];
        } else if (low == 0x0A || low == 0x0B) {
            v = c->fc[low - 5];
        } else if (low < 0x0C) {
            return;
        }
        c->tx_len = c->tx_pos = 0;
        c->tx[c->tx_len++] = 0;                 /* START */
        tx_word(c, id);
        tx_word(c, (unsigned)(v >> 32) & 0xFFFFu);
        tx_word(c, (unsigned)(v >> 16) & 0xFFFFu);
        tx_word(c, (unsigned)v & 0xFFFFu);
    }
}

/* One SCLK edge (either): SIN is sampled; SOUT shifts the pending reply. */
void ccsi_edge(CvState *s, const CcsiCfg *k, CcsiState *c) {
    LogicValue v = cv_level(s, cv_pin(s, k->sin), LOGIC_1);
    int b;

    if (v == LOGIC_X) {
        v = LOGIC_1;
    }
    b = v == LOGIC_1;
    if (c->tx_pos < c->tx_len) {
        cv_drive(s, cv_pin(s, k->sout), c->tx[c->tx_pos++] ? LOGIC_1 : LOGIC_0, 0);
    } else {
        cv_drive(s, cv_pin(s, k->sout), LOGIC_1, 0);
    }
    if (!c->in_frame) {
        if (!b) {
            c->in_frame = 1;
            c->nbits = 0;
            c->ones = 0;
        }
        return;
    }
    if (c->nbits < MAX_FRAME) {
        c->bits[c->nbits++] = (uint8_t)b;
    }
    /* END is 18 highs starting on a word boundary: a check bit of 1 (after a word ending in 0)
       next to the END's highs is not part of the run */
    c->ones = b ? c->ones + 1 : 0;
    if (c->nbits % 17 == 0 && c->ones != 17) {
        c->ones = 0;
    }
    if (c->ones >= 18) {
        c->in_frame = 0;
        c->ones = 0;
        frame(c, c->nbits >= 18 ? c->nbits - 18 : 0);
        c->nbits = 0;
    }
}

void ccsi_pin(CvState *s, const CcsiCfg *k, CcsiState *c, size_t pin) {
    LogicValue v;

    if (cv_pin(s, k->sclk) != pin) {
        return;
    }
    v = cv_level(s, pin, LOGIC_X);
    if ((v == LOGIC_0 || v == LOGIC_1) && v != c->sclk) {
        c->sclk = v;
        ccsi_edge(s, k, c);
    } else if (v != LOGIC_0 && v != LOGIC_1) {
        c->sclk = LOGIC_X;
    }
}

/* ---- Outputs -------------------------------------------------------------------------------- */

/* The slot's outputs: line `slot` low (when it is one of the 16 pins), the others released; each
   pixel's colour output high while its 16-bit gray is nonzero, in the displayed bank. */
void ccsi_apply(CvState *s, const CcsiCfg *k, const CcsiState *c, unsigned slot) {
    unsigned bank = (c->bank_sel & 1u) ^ 1u, l, p;

    for (l = 0; l < LINES; l++) {
        cv_drive(s, cv_pin(s, k->line[l]), l == slot ? LOGIC_0 : LOGIC_Z, 0);
    }
    for (p = 0; p < PIXELS; p++) {
        uint64_t u = slot < 32 ? c->sram[((size_t)bank * 32 + slot) * PIXELS + p] : 0;
        int on = slot < LINES;

        cv_drive(s, cv_pin(s, k->red[p]), on && (u & 0xFFFFu) ? LOGIC_1 : LOGIC_0, 0);
        cv_drive(s, cv_pin(s, k->green[p]), on && ((u >> 16) & 0xFFFFu) ? LOGIC_1 : LOGIC_0, 0);
        cv_drive(s, cv_pin(s, k->blue[p]), on && ((u >> 32) & 0xFFFFu) ? LOGIC_1 : LOGIC_0, 0);
    }
}
