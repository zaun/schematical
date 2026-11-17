/*
 * serialmem: serial (SPI) memory chips: the 25xxx-style EEPROMs (Microchip
 * AT25, onsemi CAT25, ST M95), F-RAMs (Infineon/Cypress FM25, CY15) and SPI
 * NOR flash (Adesto/Renesas AT25DF, AT25XE). The AT45DB DataFlash and the
 * M95P32 page EEPROM are in the library as facts only ("simulated": false).
 *
 * The host port is the SPI target front end of serial_target.h: modes 0 and 3
 * (the mode is read from SCK when CS# falls), bytes MSB first, SI sampled on
 * the rising edge, SO shifted on the falling edge, SO Z while CS# is high.
 * The X rules are the front end's: an X on CS# or SCK makes the rest of the
 * transaction X. This file adds the chip's own: an opcode with an unknown
 * bit is no known command (SO drives X for the rest of the transaction and
 * WEL becomes unknown at CS# rising); an address with unknown bits reads X
 * and makes every byte it could name unknown on a write or erase; unknown
 * data bits store as unknown bits; an unknown WEL, WP#, block-protect bit or
 * lock bit makes a write "maybe": it runs, and what it would change becomes
 * unknown. HOLD# is modeled (below). AC timing (tSU, tH, tCS ...) is not
 * checked: only the busy times and the SO delays (outputValid, outputDisable).
 *
 * Memory: every instance has its own array of 8-bit bytes (value and known
 * per bit). An EEPROM and a flash power up erased (FF); an F-RAM powers up
 * unknown (X: its datasheets give no content).
 *
 * eeprom / fram (opcodes 06 WREN, 04 WRDI, 05 RDSR, 01 WRSR, 03 READ, 02 WRITE;
 * with a8InOpcode, bit 3 of every opcode is address bit 8 and ignored by the
 * commands that take no address). Status: bit 0 RDY/WIP, bit 1 WEL, bits
 * 3:2 BP1:BP0 (upper quarter, half, all protected), bit 7 WPEN/SRWD where the
 * protection is "wpen". READ runs on, address + 1 wrapping at the array end.
 * A WRITE needs WEL = 1 and a start address outside the protected blocks, and
 * completes when CS# rises on a byte boundary after at least one data byte
 * (otherwise nothing is written and WEL stays). An EEPROM keeps the data in
 * a page buffer (pageSize bytes; the address wraps inside the page); CS#
 * rising starts the write cycle (writeCycle): RDSR shows RDY = 1, every other
 * command is ignored, the page is stored and WEL cleared when the cycle ends.
 * WRSR (data byte on a byte boundary) needs WEL = 1 and, per "protection",
 * "wpen": not (WPEN = 1 and WP# low) or "wp": WP# high; WP# falling between
 * CS# falling and rising counts as low. An EEPROM runs a write cycle for it
 * too; an F-RAM applies it at once. Only BP1, BP0 and WPEN are written. An
 * F-RAM writes each data byte as it arrives (a byte that completes in a
 * protected block stops the burst: later bytes are ignored), has no busy
 * state, and clears WEL when CS# rises after WRITE, WRSR or WRDI. F-RAM
 * extras by device key: RDID 9F ("jedecId" bytes, then SO Z), FSTRD 0B (a
 * dummy byte before the data), SLEEP B9 ("sleep": the next CS# falling
 * wakes the chip and that transaction, and the next "wake" ns, are
 * ignored). With protection "wp" (AT25010B) WP# low also blocks WREN and the
 * array write, and a write interrupted by WP# low is dropped.
 * Not modeled: the identification page (M95320-D, CAT25256 IPL/LIP), the
 * ECC re-write of 4-byte groups, status bits other than the above.
 *
 * flash (SPI NOR, 3 address bytes): 03 READ and 0B READ (a dummy byte) run on
 * and wrap; 02 PROGRAM (1 to pageSize bytes, wrapping inside the page, bits
 * only go 1 to 0: a byte takes old AND new; byteProgram time for one byte,
 * pageProgram for more); 81 PAGE ERASE (pageSize, if "pageErase" is given),
 * 20 (4 KB), 52 (32 KB), D8 (64 KB, or 32 KB when "blockErase64" is not
 * given), 60 and C7 CHIP ERASE; 06, 04; 05 RDSR (byte 1, then byte 2, then
 * byte 1 again ...); 01 WRSR; 31 WRSR byte 2 (RSTE); 9F JEDEC ID ("jedecId"
 * bytes, then SO Z); B9 DEEP POWER-DOWN (every command but AB is ignored);
 * AB RESUME; F0 RESET (F0 then D0, while RSTE = 1: clears WEL, the lock bit
 * and the sector protection registers to power-up, leaves deep power-down).
 * A program or erase needs WEL = 1, a complete command at a byte boundary
 * and an unprotected target; busy runs the datasheet time, RDSR is the only
 * command taken, the change is stored when the time ends, and WEL is
 * cleared when CS# rises (also when the command is dropped, once its opcode
 * is complete). Chip erase needs every sector unprotected. EPE is never set.
 * Protection "sector" (AT25DF021A, AT25XE021A, AT25XE041B): one protection
 * register per protectSize bytes, all protected at power-up. Status byte 1:
 * bit 7 SPRL, bit 4 WPP (WP# high), bits 3:2 SWP (00 none, 11 all, 01 some),
 * bit 1 WEL, bit 0 RDY. WRSR (WEL = 1, one data byte) follows the datasheet's
 * Table 4: with WP# low and SPRL = 1 nothing changes; with SPRL = 1 and WP#
 * high only SPRL can change; otherwise SPRL = bit 7 and data bits 5:2 =
 * 0000 unprotect every register, 1111 protect every register. 36 PROTECT
 * SECTOR, 39 UNPROTECT SECTOR (3 address bytes, WEL = 1, SPRL = 0) and 3C
 * READ SECTOR PROTECTION REGISTER (FF or 00 repeated) are modeled.
 * Protection "block" (AT25XE512C): status byte 1 bit 7 BPL, bit 4 WPP, bit
 * 2 BP0 (the whole array), WEL, RDY; WRSR (WEL = 1) is ignored while WP# is
 * low and BPL = 1, otherwise BPL = bit 7 and BP0 = bit 2; BP0 powers up 0.
 * Status byte 2: bit 4 RSTE, bit 0 RDY.
 * Not modeled: dual output/input reads (3B, A2), sequential program (AD, AF),
 * the OTP security register (9B, 77), active status interrupt (25), ultra
 * deep power-down (79), legacy opcodes (62, 15), erase/program suspend,
 * program/erase error reporting.
 *
 * HOLD# (when the device has the pin): while CS# is low and HOLD# is low
 * with SCK low the chip is held: SCK edges are ignored and SO is Z; the hold
 * ends when HOLD# is high with SCK low (a change while SCK is high takes
 * effect at the next SCK fall). A hold does not stop a busy time. An X on
 * HOLD# makes the open transaction X. WP#, HOLD# at Z read X, or high when
 * the device says "pullUp" (an internal pull-up).
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "serial_target.h"

#define SM_NONE SERIAL_NO_PIN
#define SM_MAX_SIZE ((uint64_t)1 << 24)
#define SM_MAX_ID 16

enum { TIMER_BUSY = 1 };

/* A three-valued condition: 0, 1 or unknown. */
enum { T0 = 0, T1 = 1, TX = 2 };

static int t_not(int a) { return a == TX ? TX : !a; }
static int t_and(int a, int b) {
    if (a == T0 || b == T0) {
        return T0;
    }
    return a == TX || b == TX ? TX : T1;
}
static int t_or(int a, int b) {
    if (a == T1 || b == T1) {
        return T1;
    }
    return a == TX || b == TX ? TX : T0;
}

typedef enum { SM_EEPROM, SM_FRAM, SM_FLASH, SM_DATAFLASH, SM_PAGEEEPROM } SmType;
static const char *const TYPE_NAMES[] = {"eeprom", "fram", "flash", "dataflash", "pageeeprom", NULL};

typedef enum { PROT_WPEN, PROT_WP, PROT_SECTOR, PROT_BLOCK } SmProt;
static const char *const PROT_NAMES[] = {"wpen", "wp", "sector", "block", NULL};

enum { SM_CS, SM_SCK, SM_SI, SM_SO, SM_WP, SM_HOLD, SM_RESET, SM_PINS };
static const char *const PIN_KEY_NAMES[SM_PINS] = {
    "chipSelect", "clock", "dataIn", "dataOut", "writeProtect", "hold", "reset"
};

static const char *const DEVICE_KEYS[] = {
    "type", "size", "addressBytes", "a8InOpcode", "pageSize", "protection", "protectSize",
    "chipSelect", "clock", "dataIn", "dataOut", "writeProtect", "hold", "reset",
    "writeCycle", "pageProgram", "byteProgram", "pageErase", "sectorErase", "blockErase32",
    "blockErase64", "chipErase", "writeStatus", "outputValid", "outputDisable", "wake",
    "fastRead", "sleep", "jedecId", "pullUp", "statusOnes", "busyOnes",
    "maxClock", "endurance", "retention", "vcc", "inputMax", NULL
};
static const char *const PART_KEYS[] = {"vcc", "inputMax", "maxClock", NULL};

/* ---- Device data ----------------------------------------------------------------------- */

typedef struct {
    SmType type;
    int simulated;
    uint32_t size;
    unsigned address_bytes;
    int a8;
    uint32_t page_size;
    SmProt prot;
    uint32_t protect_size;
    const char *pin_name[SM_PINS];
    SimTime t_write, t_page_prog, t_byte_prog, t_page_erase, t_sector, t_block32, t_block64,
        t_chip, t_wrsr, t_valid, t_disable, t_wake;
    int fast_read, sleep, pull_up;
    uint8_t status_ones, busy_ones;
    uint8_t jedec[SM_MAX_ID];
    unsigned jedec_len;
    PartSerialmem facts;
    const PartVoltage *vcc;
} SerialmemDevice;

typedef struct {
    size_t pin[SM_PINS];
    const SerialmemDevice *device;
} SerialmemPins;

/* ---- State ------------------------------------------------------------------------------- */

typedef enum {
    C_NONE, C_IGNORE, C_UNKNOWN, C_WREN, C_WRDI, C_RDSR, C_WRSR, C_WRSR2, C_READ, C_FREAD,
    C_WRITE, C_RDID, C_SLEEP, C_ERASE_PAGE, C_ERASE_4K, C_ERASE_32K, C_ERASE_64K,
    C_ERASE_CHIP, C_PROTECT, C_UNPROTECT, C_READ_SPR, C_DPD, C_RESUME, C_RESET
} Cmd;

typedef enum {
    P_NONE, P_PAGE, P_CORRUPT, P_ERASE, P_WRSR, P_WRSR_FLASH
} PendKind;

typedef struct {
    PendKind kind;
    uint32_t addr;
    uint32_t known;       /* P_CORRUPT: the known address bits */
    uint32_t length;      /* P_ERASE, P_CORRUPT: bytes */
    int maybe;            /* it may not have happened: what it changes is unknown */
    uint8_t value, value_known;
} Pending;

typedef struct {
    Cmd cmd;
    uint8_t op, op_known;
    unsigned naddr;
    unsigned dummy;
    unsigned addr_n;
    uint32_t addr, addr_known;
    int addr_done;
    unsigned data_n;
    uint8_t first, first_known;
    int stopped;          /* F-RAM: a burst reached a protected byte */
    int unknown_addr;     /* a read whose address has an unknown bit */
    uint32_t span;        /* P_CORRUPT span of a write */
} Tx;

typedef struct {
    const SerialmemDevice *dev;
    const SerialmemPins *pins;
    Sim *sim;
    SimInstance *instance;
    int passive;
    SerialSpiPins spi_pins;
    SerialSpi spi;
    SerialSpiEvents events;

    uint8_t *value;
    uint8_t *known;
    uint32_t mask;

    /* The status register. */
    uint8_t bp, bp_known;     /* BP1:BP0 as bits 1:0 */
    int lock;                 /* WPEN / SRWD (eeprom, fram), SPRL / BPL (flash) */
    int wel;
    int rste;
    uint8_t *sprot;           /* sector protection registers: 0, 1 or TX */
    uint32_t sprot_count;

    /* Busy. */
    int busy;
    Pending pend;
    SimTime busy_until;
    int asleep;               /* F-RAM sleep: the next CS# fall wakes it */
    SimTime wake_until;
    int dpd;                  /* flash deep power-down */

    /* The page buffer (EEPROM write, flash program). */
    uint8_t *buf_value;
    uint8_t *buf_known;
    uint8_t *buf_loaded;
    uint32_t page_base;
    uint32_t page_offset;

    Tx tx;
    int gate_ignore;          /* this transaction is ignored (waking, in sleep) */
    int wp_fell;              /* WP# fell during this transaction */
} SmState;

/* ---- Inputs ------------------------------------------------------------------------------- */

/* WP#, HOLD#: 1 high, 0 low, TX unknown; a pin the package leaves out is high. */
static int control(const SmState *s, int which) {
    size_t pin = s->pins->pin[which];
    LogicValue v;

    if (pin == SM_NONE) {
        return T1;
    }
    v = serial_level(s->sim, s->instance, pin, s->dev->pull_up ? LOGIC_1 : LOGIC_X);
    return v == LOGIC_1 ? T1 : v == LOGIC_0 ? T0 : TX;
}

static SimTime now_of(const SmState *s) {
    return sim_now(s->sim);
}

/* ---- Protection ---------------------------------------------------------------------------- */

static uint32_t quarter(const SmState *s) {
    return s->dev->size / 4;
}

/* Whether bytes [start, start + length) overlap the blocks the BP bits protect. */
static int bp_protected(const SmState *s, uint32_t start, uint32_t length) {
    uint32_t last = start + length - 1;
    int seen0 = 0, seen1 = 0;
    unsigned c;

    for (c = 0; c < 4; c++) {
        uint32_t zone;
        int p;

        if (((c ^ s->bp) & s->bp_known & 3u) != 0) {
            continue;
        }
        if (s->dev->prot == PROT_BLOCK) {
            p = (c & 1u) != 0;
        } else {
            zone = c == 0 ? s->dev->size : c == 1 ? s->dev->size - quarter(s)
                   : c == 2 ? s->dev->size / 2 : 0;
            p = c != 0 && last >= zone;
        }
        seen0 |= !p;
        seen1 |= p;
    }
    return seen0 && seen1 ? TX : seen1 ? T1 : T0;
}

static int range_protected(const SmState *s, uint32_t start, uint32_t length) {
    int result = T0;
    uint32_t sector, last;

    if (s->dev->prot != PROT_SECTOR) {
        return bp_protected(s, start, length);
    }
    last = (start + length - 1) / s->dev->protect_size;
    for (sector = start / s->dev->protect_size; sector <= last && sector < s->sprot_count; sector++) {
        result = t_or(result, s->sprot[sector]);
    }
    return result;
}

/* ---- Memory -------------------------------------------------------------------------------- */

static void put_byte(SmState *s, uint32_t address, uint8_t v, uint8_t k, int and_mode) {
    address &= s->mask;
    if (and_mode) {
        uint8_t ov = s->value[address], ok = s->known[address];
        uint8_t rk = (uint8_t)((ok & k) | (ok & (uint8_t)~ov) | (k & (uint8_t)~v));

        s->value[address] = (uint8_t)(ov & v & rk);
        s->known[address] = rk;
    } else {
        s->value[address] = (uint8_t)(v & k);
        s->known[address] = k;
    }
}

/* Every byte whose address matches the known bits (ignoring the low span bits) is unknown. */
static void corrupt_matching(SmState *s, uint32_t addr, uint32_t known, uint32_t span) {
    uint32_t w = 1;
    uint32_t mk;
    uint64_t a;

    while (w < span) {
        w <<= 1;
    }
    mk = known & ~(w - 1) & s->mask;
    for (a = 0; a <= s->mask; a++) {
        if ((((uint32_t)a ^ addr) & mk) == 0) {
            s->known[a] = 0;
        }
    }
}

static void erase_bytes(SmState *s, uint32_t start, uint32_t length, int maybe) {
    uint32_t i;

    for (i = 0; i < length; i++) {
        uint32_t a = (start + i) & s->mask;

        if (maybe) {
            s->known[a] = (uint8_t)(s->known[a] & s->value[a]);
        } else {
            s->value[a] = 0xFF;
            s->known[a] = 0xFF;
        }
    }
}

/* ---- Status -------------------------------------------------------------------------------- */

static void status_byte(const SmState *s, unsigned which, uint8_t *v, uint8_t *k) {
    const SerialmemDevice *d = s->dev;
    uint8_t value = 0;
    uint8_t known = 0xFF;
    int wp;

    if (d->type == SM_FLASH) {
        value = (uint8_t)(s->busy ? 1 : 0);
        if (which == 1) {
            if (s->rste) {
                value |= 0x10;
            }
            *v = value;
            *k = 0xFF;
            return;
        }
        if (s->wel == T1) {
            value |= 2;
        } else if (s->wel == TX) {
            known &= (uint8_t)~2;
        }
        if (s->lock == T1) {
            value |= 0x80;
        } else if (s->lock == TX) {
            known &= (uint8_t)~0x80;
        }
        wp = control(s, SM_WP);
        if (wp == T1) {
            value |= 0x10;
        } else if (wp == TX) {
            known &= (uint8_t)~0x10;
        }
        if (d->prot == PROT_SECTOR) {
            int any0 = 0, any1 = 0, anyx = 0;
            uint32_t i;

            for (i = 0; i < s->sprot_count; i++) {
                any0 |= s->sprot[i] == T0;
                any1 |= s->sprot[i] == T1;
                anyx |= s->sprot[i] == TX;
            }
            if (anyx) {
                known &= (uint8_t)~0x0C;
            } else if (any1 && !any0) {
                value |= 0x0C;
            } else if (any0 && any1) {
                value |= 0x04;
            }
        } else {
            if (s->bp & 1u) {
                value |= 4;
            }
            if (!(s->bp_known & 1u)) {
                known &= (uint8_t)~4;
            }
        }
        *v = value;
        *k = known;
        return;
    }
    if (s->busy) {
        value = (uint8_t)(1 | d->busy_ones);
    }
    value |= d->status_ones;
    if (s->wel == T1) {
        value |= 2;
    } else if (s->wel == TX) {
        known &= (uint8_t)~2;
    }
    value |= (uint8_t)((s->bp & 3u) << 2);
    known &= (uint8_t)~((~s->bp_known & 3u) << 2);
    if (d->prot == PROT_WPEN) {
        if (s->lock == T1) {
            value |= 0x80;
        } else if (s->lock == TX) {
            known &= (uint8_t)~0x80;
        }
    }
    *v = value;
    *k = known;
}

/* ---- Busy ---------------------------------------------------------------------------------- */

static void apply_pending(SmState *s) {
    Pending *p = &s->pend;
    uint32_t i;

    switch (p->kind) {
        case P_PAGE: {
            int and_mode = s->dev->type == SM_FLASH;

            for (i = 0; i < s->dev->page_size; i++) {
                if (s->buf_loaded[i]) {
                    put_byte(s, s->page_base + i, s->buf_value[i], p->maybe ? 0 : s->buf_known[i],
                             and_mode);
                }
            }
            break;
        }
        case P_CORRUPT:
            corrupt_matching(s, p->addr, p->known, p->length);
            break;
        case P_ERASE:
            erase_bytes(s, p->addr, p->length, p->maybe);
            break;
        case P_WRSR:
        case P_WRSR_FLASH:
            if (p->kind == P_WRSR_FLASH && s->dev->prot == PROT_BLOCK) {
                if (p->maybe) {
                    s->bp_known = 0;
                    s->lock = TX;
                } else {
                    s->bp = (uint8_t)((p->value >> 2) & 1u);
                    s->bp_known = (uint8_t)((p->value_known >> 2) & 1u);
                    s->lock = !(p->value_known & 0x80) ? TX : (p->value & 0x80) ? T1 : T0;
                }
            } else if (p->kind == P_WRSR) {
                if (p->maybe) {
                    s->bp_known = 0;
                    if (s->dev->prot == PROT_WPEN) {
                        s->lock = TX;
                    }
                } else {
                    s->bp = (uint8_t)((p->value >> 2) & 3u);
                    s->bp_known = (uint8_t)((p->value_known >> 2) & 3u);
                    if (s->dev->prot == PROT_WPEN) {
                        s->lock = !(p->value_known & 0x80) ? TX : (p->value & 0x80) ? T1 : T0;
                    }
                }
            }
            break;
        case P_NONE:
            break;
    }
    p->kind = P_NONE;
}

static void finish_busy(SmState *s) {
    apply_pending(s);
    s->busy = 0;
    if (s->dev->type == SM_EEPROM) {
        s->wel = T0;
    }
}

/* Starts an operation taking `duration`; 0 applies it at once. */
static void start_op(SmState *s, SimTime duration) {
    if (duration == 0) {
        apply_pending(s);
        if (s->dev->type == SM_EEPROM) {
            s->wel = T0;
        }
        return;
    }
    s->busy = 1;
    s->busy_until = now_of(s) + duration;
    sim_schedule_timer(s->sim, s->instance, TIMER_BUSY, duration);
}

/* ---- Commands ------------------------------------------------------------------------------ */

static Cmd lookup(const SmState *s, uint8_t op) {
    const SerialmemDevice *d = s->dev;

    if (d->type == SM_FLASH) {
        switch (op) {
            case 0x03: return C_READ;
            case 0x0B: return C_FREAD;
            case 0x02: return C_WRITE;
            case 0x81: return d->t_page_erase > 0 ? C_ERASE_PAGE : C_IGNORE;
            case 0x20: return C_ERASE_4K;
            case 0x52: return C_ERASE_32K;
            case 0xD8: return d->t_block64 > 0 ? C_ERASE_64K : C_ERASE_32K;
            case 0x60:
            case 0xC7: return C_ERASE_CHIP;
            case 0x06: return C_WREN;
            case 0x04: return C_WRDI;
            case 0x05: return C_RDSR;
            case 0x01: return C_WRSR;
            case 0x31: return C_WRSR2;
            case 0x9F: return C_RDID;
            case 0xB9: return C_DPD;
            case 0xAB: return C_RESUME;
            case 0xF0: return C_RESET;
            case 0x36: return d->prot == PROT_SECTOR ? C_PROTECT : C_IGNORE;
            case 0x39: return d->prot == PROT_SECTOR ? C_UNPROTECT : C_IGNORE;
            case 0x3C: return d->prot == PROT_SECTOR ? C_READ_SPR : C_IGNORE;
            default: return C_IGNORE;
        }
    }
    switch (op) {
        case 0x06: return C_WREN;
        case 0x04: return C_WRDI;
        case 0x05: return C_RDSR;
        case 0x01: return C_WRSR;
        case 0x03: return C_READ;
        case 0x02: return C_WRITE;
        case 0x9F: return d->jedec_len > 0 ? C_RDID : C_IGNORE;
        case 0x0B: return d->fast_read ? C_FREAD : C_IGNORE;
        case 0xB9: return d->sleep ? C_SLEEP : C_IGNORE;
        default: return C_IGNORE;
    }
}

static void decode(SmState *s, uint8_t op, uint8_t known) {
    const SerialmemDevice *d = s->dev;
    uint8_t need = d->a8 ? 0xF7 : 0xFF;
    Tx *t = &s->tx;
    Cmd c;

    t->op = op;
    t->op_known = known;
    if (s->gate_ignore) {
        t->cmd = C_IGNORE;
        return;
    }
    if ((known & need) != need) {
        t->cmd = C_UNKNOWN;
        return;
    }
    c = lookup(s, (uint8_t)(op & need));
    if (s->busy && c != C_RDSR) {
        c = C_IGNORE;
    }
    if (s->dpd && c != C_RESUME) {
        c = C_IGNORE;
    }
    t->cmd = c;
    switch (c) {
        case C_READ:
        case C_WRITE:
            t->naddr = d->address_bytes;
            break;
        case C_FREAD:
            t->naddr = d->address_bytes;
            t->dummy = 1;
            break;
        case C_ERASE_PAGE:
        case C_ERASE_4K:
        case C_ERASE_32K:
        case C_ERASE_64K:
        case C_PROTECT:
        case C_UNPROTECT:
        case C_READ_SPR:
            t->naddr = 3;
            break;
        default:
            break;
    }
}

static void address_done(SmState *s) {
    Tx *t = &s->tx;

    if (s->dev->a8) {
        t->addr |= (uint32_t)((t->op >> 3) & 1u) << 8;
        if ((t->op_known >> 3) & 1u) {
            t->addr_known |= (uint32_t)1 << 8;
        }
    }
    t->addr &= s->mask;
    t->addr_known &= s->mask;
    t->addr_done = 1;
    t->unknown_addr = t->addr_known != s->mask;
}

/* One byte of a WRITE or PROGRAM data phase. */
static void data_byte(SmState *s, uint8_t data, uint8_t known) {
    Tx *t = &s->tx;
    uint32_t page = s->dev->page_size;

    if (t->data_n == 0) {
        t->first = data;
        t->first_known = known;
    }
    if (s->dev->type == SM_FRAM) {
        int perm;

        if (t->stopped) {
            return;
        }
        if (t->unknown_addr) {
            if (s->wel != T0) {
                corrupt_matching(s, t->addr, t->addr_known, 1);
            }
            t->stopped = 1;
            return;
        }
        perm = t_and(s->wel, t_not(range_protected(s, t->addr, 1)));
        if (perm == T0) {
            t->stopped = 1;
            return;
        }
        if (perm == TX || s->spi.x) {
            known = 0;
        }
        put_byte(s, t->addr, data, known, 0);
        t->addr = (t->addr + 1) & s->mask;
        t->data_n++;
        return;
    }
    if (t->data_n == 0) {
        s->page_base = t->addr & ~(page - 1);
        s->page_offset = t->addr & (page - 1);
        memset(s->buf_loaded, 0, page);
    }
    {
        uint32_t index = (s->page_offset + t->data_n) & (page - 1);

        s->buf_value[index] = data;
        s->buf_known[index] = known;
        s->buf_loaded[index] = 1;
    }
    t->data_n++;
}

static void sm_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    SmState *s = context;
    Tx *t = &s->tx;

    if (index == 0) {
        decode(s, data, known);
        if (t->naddr == 0) {
            t->addr_done = 1;
        }
        return;
    }
    if (t->cmd == C_NONE || t->cmd == C_IGNORE || t->cmd == C_UNKNOWN) {
        return;
    }
    if (t->addr_n < t->naddr) {
        t->addr = (t->addr << 8) | data;
        t->addr_known = (t->addr_known << 8) | known;
        if (++t->addr_n == t->naddr) {
            address_done(s);
        }
        return;
    }
    if (t->dummy > 0 && index <= t->naddr + t->dummy) {
        return;
    }
    switch (t->cmd) {
        case C_WRITE:
            if (t->addr_done) {
                data_byte(s, data, known);
            }
            break;
        case C_WRSR:
        case C_WRSR2:
        case C_RESET:
            if (t->data_n == 0) {
                t->first = data;
                t->first_known = known;
            }
            t->data_n++;
            break;
        default:
            break;
    }
}

static SerialSpiByte sm_next(void *context, unsigned index) {
    SmState *s = context;
    Tx *t = &s->tx;
    SerialSpiByte out = {0, 0, 0};
    uint8_t v, k;

    if (index == 0) {
        return out;
    }
    if (t->cmd == C_UNKNOWN) {
        out.drive = 0xFF;
        return out;
    }
    switch (t->cmd) {
        case C_RDSR:
            status_byte(s, s->dev->type == SM_FLASH ? (index - 1) % 2 : 0, &v, &k);
            out.data = v;
            out.known = k;
            out.drive = 0xFF;
            break;
        case C_READ:
        case C_FREAD:
            if (index > t->naddr + t->dummy) {
                out.drive = 0xFF;
                if (t->unknown_addr || !t->addr_done) {
                    break;
                }
                out.data = s->value[t->addr];
                out.known = s->known[t->addr];
                t->addr = (t->addr + 1) & s->mask;
            }
            break;
        case C_RDID:
            if (index - 1 < s->dev->jedec_len) {
                out.data = s->dev->jedec[index - 1];
                out.known = 0xFF;
                out.drive = 0xFF;
            }
            break;
        case C_READ_SPR:
            if (index > t->naddr && t->addr_done) {
                int p = t->unknown_addr ? TX : s->sprot[(t->addr / s->dev->protect_size) % s->sprot_count];

                out.drive = 0xFF;
                out.data = p == T1 ? 0xFF : 0x00;
                out.known = p == TX ? 0x00 : 0xFF;
            }
            break;
        default:
            break;
    }
    return out;
}

static void sm_begin(void *context) {
    SmState *s = context;

    memset(&s->tx, 0, sizeof s->tx);
    s->tx.cmd = C_NONE;
    s->wp_fell = 0;
    s->gate_ignore = 0;
    if (s->asleep) {
        s->asleep = 0;
        s->wake_until = now_of(s) + s->dev->t_wake;
        s->gate_ignore = 1;
    } else if (now_of(s) < s->wake_until) {
        s->gate_ignore = 1;
    }
}

/* ---- End of a transaction ------------------------------------------------------------------- */

static void set_all_sectors(SmState *s, int value) {
    uint32_t i;

    for (i = 0; i < s->sprot_count; i++) {
        s->sprot[i] = (uint8_t)value;
    }
}

static void end_eeprom(SmState *s, unsigned bits) {
    const SerialmemDevice *d = s->dev;
    Tx *t = &s->tx;
    int x = s->spi.x;
    int wp = control(s, SM_WP);
    int bytes_ok = bits % 8 == 0 && bits >= 8;

    if (t->cmd == C_UNKNOWN) {
        s->wel = TX;
        return;
    }
    if (x && (t->cmd == C_WREN || t->cmd == C_WRDI || t->cmd == C_WRSR || t->cmd == C_SLEEP)) {
        s->wel = TX;   /* what an X transaction did is unknown */
        return;
    }
    switch (t->cmd) {
        case C_WREN:
            if (!bytes_ok) {
                break;
            }
            if (d->prot == PROT_WP && wp != T1) {
                if (wp == TX) {
                    s->wel = TX;
                }
                break;
            }
            s->wel = T1;
            break;
        case C_WRDI:
            if (bytes_ok) {
                s->wel = T0;
            }
            break;
        case C_SLEEP:
            if (bytes_ok) {
                s->asleep = 1;
            }
            break;
        case C_WRSR: {
            int writable;
            int perm;

            if (!bytes_ok || t->data_n < 1) {
                break;
            }
            if (s->wp_fell) {
                wp = T0;
            }
            writable = d->prot == PROT_WP ? wp : t_or(t_not(s->lock), wp);
            perm = t_and(s->wel, writable);
            if (perm == T0) {
                break;
            }
            s->pend.kind = P_WRSR;
            s->pend.value = t->first;
            s->pend.value_known = t->first_known;
            s->pend.maybe = perm == TX;
            if (d->type == SM_FRAM) {
                apply_pending(s);
                s->wel = T0;
            } else {
                start_op(s, d->t_write);
            }
            break;
        }
        case C_WRITE: {
            int perm;
            uint32_t span;

            if (!t->addr_done) {
                break;
            }
            if (d->type == SM_FRAM) {
                if (s->wel != T0) {
                    s->wel = T0;
                }
                break;
            }
            if (!bytes_ok || t->data_n < 1) {
                break;
            }
            if (s->wp_fell) {
                wp = T0;
            }
            perm = t_and(s->wel, t_not(t->unknown_addr ? T0 : range_protected(s, t->addr, 1)));
            if (d->prot == PROT_WP) {
                perm = t_and(perm, wp);
            }
            if (perm == T0) {
                break;
            }
            span = d->page_size;
            if (t->unknown_addr) {
                s->pend.kind = P_CORRUPT;
                s->pend.addr = t->addr;
                s->pend.known = t->addr_known;
                s->pend.length = span;
            } else {
                s->pend.kind = P_PAGE;
                s->pend.maybe = perm == TX || x;
            }
            start_op(s, d->t_write);
            break;
        }
        default:
            break;
    }
}

/* A flash command that stops WEL: the opcode was complete. */
static void end_flash(SmState *s, unsigned bits) {
    const SerialmemDevice *d = s->dev;
    Tx *t = &s->tx;
    int x = s->spi.x;
    int bytes_ok = bits % 8 == 0 && bits >= 8;
    int wel = s->wel;
    int wp = control(s, SM_WP);
    uint32_t length = 0;
    int perm;
    int prot;

    if (t->cmd == C_UNKNOWN) {
        s->wel = TX;
        return;
    }
    switch (t->cmd) {
        case C_WREN:
            if (bytes_ok) {
                s->wel = x ? TX : T1;
            }
            return;
        case C_WRDI:
            if (bytes_ok) {
                s->wel = x ? TX : T0;
            }
            return;
        case C_DPD:
            if (bytes_ok) {
                s->dpd = 1;
            }
            return;
        case C_RESUME:
            if (bytes_ok) {
                s->dpd = 0;
            }
            return;
        case C_RESET:
            if (bits == 16 && t->data_n == 1 && t->first_known == 0xFF && t->first == 0xD0 &&
                s->rste && !x) {
                s->wel = T0;
                s->lock = T0;
                s->dpd = 0;
                if (d->prot == PROT_SECTOR) {
                    set_all_sectors(s, T1);
                }
            }
            return;
        case C_WRSR2:
            if (wel == T0) {
                return;
            }
            s->wel = T0;
            /* An unknown RSTE write leaves RSTE as it was. */
            if (bytes_ok && t->data_n >= 1 && !x && wel == T1 && (t->first_known & 0x10)) {
                s->rste = (t->first >> 4) & 1u;
            }
            return;
        case C_WRSR: {
            if (wel == T0) {
                return;
            }
            s->wel = T0;
            if (!bytes_ok || t->data_n < 1) {
                return;
            }
            if (d->prot == PROT_SECTOR) {
                uint8_t need = 0xBC;   /* bit 7 and bits 5:2 */

                if (x || wel == TX || wp == TX || s->lock == TX || (t->first_known & need) != need) {
                    s->lock = TX;
                    set_all_sectors(s, TX);
                    return;
                }
                if (wp == T0 && s->lock == T1) {
                    return;   /* hard locked */
                }
                if (s->lock == T0) {
                    unsigned g = (t->first >> 2) & 0xFu;

                    if (g == 0) {
                        set_all_sectors(s, T0);
                    } else if (g == 0xF) {
                        set_all_sectors(s, T1);
                    }
                }
                s->lock = (t->first & 0x80) ? T1 : T0;
                return;
            }
            if (x || wel == TX || wp == TX || s->lock == TX) {
                s->pend.kind = P_WRSR_FLASH;
                s->pend.maybe = 1;
                start_op(s, d->t_wrsr);
                return;
            }
            if (wp == T0 && s->lock == T1) {
                return;
            }
            s->pend.kind = P_WRSR_FLASH;
            s->pend.value = t->first;
            s->pend.value_known = t->first_known;
            s->pend.maybe = 0;
            start_op(s, d->t_wrsr);
            return;
        }
        case C_PROTECT:
        case C_UNPROTECT:
            if (wel == T0) {
                return;
            }
            s->wel = T0;
            if (!bytes_ok || !t->addr_done) {
                return;
            }
            perm = t_and(wel, t_not(s->lock));
            if (perm == T0) {
                return;
            }
            if (t->unknown_addr || perm == TX || x) {
                set_all_sectors(s, TX);
                return;
            }
            s->sprot[(t->addr / d->protect_size) % s->sprot_count] =
                t->cmd == C_PROTECT ? T1 : T0;
            return;
        case C_WRITE:
        case C_ERASE_PAGE:
        case C_ERASE_4K:
        case C_ERASE_32K:
        case C_ERASE_64K:
        case C_ERASE_CHIP:
            break;
        default:
            return;
    }
    /* Program and erase. */
    if (wel == T0) {
        return;
    }
    s->wel = T0;
    if (!bytes_ok) {
        return;
    }
    {
        SimTime duration = 0;

        switch (t->cmd) {
            case C_WRITE:
                if (!t->addr_done || t->data_n < 1) {
                    return;
                }
                length = d->page_size;
                duration = t->data_n == 1 ? d->t_byte_prog : d->t_page_prog;
                break;
            case C_ERASE_PAGE:
                length = d->page_size;
                duration = d->t_page_erase;
                break;
            case C_ERASE_4K:
                length = 4096;
                duration = d->t_sector;
                break;
            case C_ERASE_32K:
                length = 32768;
                duration = d->t_block32;
                break;
            case C_ERASE_64K:
                length = 65536;
                duration = d->t_block64;
                break;
            default:
                length = d->size;
                duration = d->t_chip;
                break;
        }
        if (t->cmd != C_WRITE && t->cmd != C_ERASE_CHIP && !t->addr_done) {
            return;
        }
        if (length > d->size) {
            length = d->size;
        }
        if (t->cmd == C_ERASE_CHIP) {
            prot = range_protected(s, 0, d->size);
        } else if (t->unknown_addr) {
            prot = T0;
        } else {
            uint32_t start = t->addr & ~(length - 1);

            prot = t->cmd == C_WRITE ? range_protected(s, t->addr, 1) : range_protected(s, start, length);
        }
        perm = t_and(wel, t_not(prot));
        if (perm == T0) {
            return;
        }
        if (t->unknown_addr) {
            s->pend.kind = P_CORRUPT;
            s->pend.addr = t->addr;
            s->pend.known = t->addr_known;
            s->pend.length = length;
        } else if (t->cmd == C_WRITE) {
            s->pend.kind = P_PAGE;
            s->pend.maybe = perm == TX || x;
        } else {
            s->pend.kind = P_ERASE;
            s->pend.addr = t->cmd == C_ERASE_CHIP ? 0 : (t->addr & ~(length - 1));
            s->pend.length = length;
            s->pend.maybe = perm == TX || x;
        }
        start_op(s, duration);
    }
}

static void sm_end(void *context, unsigned bits) {
    SmState *s = context;
    Tx *t = &s->tx;

    if (t->cmd == C_IGNORE || t->cmd == C_NONE) {
        return;
    }
    if (s->dev->type == SM_FLASH) {
        end_flash(s, bits);
    } else {
        end_eeprom(s, bits);
    }
}

/* ---- Pins ---------------------------------------------------------------------------------- */

/* HOLD#: `release` before the front end sees an SCK edge, else after. */
static void hold_eval(SmState *s, int release) {
    int h;
    LogicValue sck;

    if (s->pins->pin[SM_HOLD] == SM_NONE || !s->spi.active) {
        return;
    }
    h = control(s, SM_HOLD);
    if (h == TX) {
        serial_spi_hold(&s->spi, s->sim, s->instance, 2, 0);
        return;
    }
    sck = serial_level(s->sim, s->instance, s->pins->pin[SM_SCK], LOGIC_X);
    if (sck != LOGIC_0) {
        return;
    }
    if (release && s->spi.held && h == T1) {
        serial_spi_hold(&s->spi, s->sim, s->instance, 0, s->dev->t_valid);
    } else if (!release && !s->spi.held && h == T0) {
        serial_spi_hold(&s->spi, s->sim, s->instance, 1, s->dev->t_disable);
    }
}

static void sm_start(Sim *sim, SimInstance *instance, void *raw) {
    SmState *s = raw;

    s->sim = sim;
    s->instance = instance;
    if (s->passive) {
        return;
    }
    serial_spi_start(&s->spi, sim, instance);
}

static void sm_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    SmState *s = raw;
    const size_t *p;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (s->passive) {
        return;
    }
    p = s->pins->pin;
    if (pin == p[SM_WP]) {
        int level = control(s, SM_WP);

        if (s->spi.active && level != T1) {
            s->wp_fell = 1;
        }
        return;
    }
    if (pin == p[SM_HOLD]) {
        hold_eval(s, 1);
        hold_eval(s, 0);
        return;
    }
    if (pin == p[SM_CS]) {
        if (!s->spi.active) {
            LogicValue sck = serial_level(sim, instance, p[SM_SCK], LOGIC_0);

            s->spi_pins.mode = sck == LOGIC_1 ? 3u : 0u;
        }
        serial_spi_pin(&s->spi, sim, instance, pin, &s->events);
        hold_eval(s, 0);
        return;
    }
    if (pin == p[SM_SCK]) {
        hold_eval(s, 1);
        serial_spi_pin(&s->spi, sim, instance, pin, &s->events);
        hold_eval(s, 0);
    }
}

static void sm_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    SmState *s = raw;

    (void)timer_id;
    s->sim = sim;
    s->instance = instance;
    if (s->passive || !s->busy) {
        return;
    }
    if (sim_now(sim) >= s->busy_until) {
        finish_busy(s);
    }
}

/* ---- Inspection hooks ---------------------------------------------------------------------- */

static int sm_memory_size(const void *raw, uint32_t *words, unsigned *bits) {
    const SmState *s = raw;

    if (s->passive) {
        return 0;
    }
    if (words != NULL) {
        *words = s->dev->size;
    }
    if (bits != NULL) {
        *bits = 8;
    }
    return 1;
}

static size_t sm_memory_read(const void *raw, uint32_t start, size_t count, uint32_t *values,
                             uint32_t *known) {
    const SmState *s = raw;
    size_t i;

    if (s->passive || start >= s->dev->size) {
        return 0;
    }
    if (count > s->dev->size - start) {
        count = s->dev->size - start;
    }
    for (i = 0; i < count; i++) {
        if (values != NULL) {
            values[i] = s->value[start + i];
        }
        if (known != NULL) {
            known[i] = s->known[start + i];
        }
    }
    return count;
}

static void sm_memory_load(void *raw, uint32_t start, const uint32_t *values, size_t count) {
    SmState *s = raw;
    size_t i;

    if (s->passive) {
        return;
    }
    for (i = 0; i < count; i++) {
        s->value[start + i] = (uint8_t)values[i];
        s->known[start + i] = 0xFF;
    }
}

/* ---- Create / destroy ------------------------------------------------------------------------ */

static size_t sm_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void sm_destroy(void *raw) {
    SmState *s = raw;

    if (s == NULL) {
        return;
    }
    free(s->value);
    free(s->known);
    free(s->sprot);
    free(s->buf_value);
    free(s->buf_known);
    free(s->buf_loaded);
    free(s);
}

static void power_up(SmState *s) {
    s->bp = 0;
    s->bp_known = 3;
    s->lock = T0;
    s->wel = T0;
    s->rste = 0;
    s->busy = 0;
    s->pend.kind = P_NONE;
    s->asleep = 0;
    s->wake_until = 0;
    s->dpd = 0;
    if (s->dev->prot == PROT_SECTOR) {
        set_all_sectors(s, T1);
    }
}

static void *sm_create(const void *config) {
    const Part *part = config;
    const PartInternal *internal = part_internal(part);
    const SerialmemPins *pins = internal->bound;
    const SerialmemDevice *d = pins->device;
    SmState *s = calloc(1, sizeof *s);

    if (s == NULL) {
        return NULL;
    }
    s->dev = d;
    s->pins = pins;
    if (!d->simulated) {
        s->passive = 1;
        return s;
    }
    s->mask = d->size - 1;
    s->value = calloc(d->size, 1);
    s->known = calloc(d->size, 1);
    s->buf_value = calloc(d->page_size, 1);
    s->buf_known = calloc(d->page_size, 1);
    s->buf_loaded = calloc(d->page_size, 1);
    if (d->prot == PROT_SECTOR) {
        s->sprot_count = d->size / d->protect_size;
        s->sprot = calloc(s->sprot_count, 1);
    }
    if (s->value == NULL || s->known == NULL || s->buf_value == NULL || s->buf_known == NULL ||
        s->buf_loaded == NULL || (d->prot == PROT_SECTOR && s->sprot == NULL)) {
        sm_destroy(s);
        return NULL;
    }
    if (d->type != SM_FRAM) {
        memset(s->value, 0xFF, d->size);
        memset(s->known, 0xFF, d->size);
    }
    s->spi_pins.cs = pins->pin[SM_CS];
    s->spi_pins.sclk = pins->pin[SM_SCK];
    s->spi_pins.mosi = pins->pin[SM_SI];
    s->spi_pins.miso = pins->pin[SM_SO];
    s->spi_pins.mode = 0;
    serial_spi_init(&s->spi, &s->spi_pins);
    s->spi.t_valid = d->t_valid;
    s->spi.t_out = d->t_valid;
    s->spi.t_float = d->t_disable;
    s->events.context = s;
    s->events.begin = sm_begin;
    s->events.next = sm_next;
    s->events.received = sm_received;
    s->events.end = sm_end;
    power_up(s);
    return s;
}

static const SimComponentType SERIALMEM_MODEL = {
    "serialmem", sm_pin_count, sm_create, sm_destroy, sm_start, sm_pin_changed, sm_timer,
    NULL, NULL, sm_memory_size, sm_memory_read, NULL, sm_memory_load
};

/* ---- Loader -------------------------------------------------------------------------------- */

static const cJSON *member(const cJSON *json, const char *key) {
    return cJSON_GetObjectItemCaseSensitive(json, key);
}

/* An optional time in ns (up to 1e11, more than kind_ns takes: a chip erase lasts seconds). */
static int load_time(KindContext *ctx, const cJSON *json, const char *key, SimTime *out) {
    const cJSON *item = member(json, key);

    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble >= 0) || item->valuedouble > 1e11) {
        return kind_error(ctx, "\"%s\" must be a time in ns from 0 to 1e11", key);
    }
    *out = (SimTime)llround(item->valuedouble * 1000.0);
    return 0;
}

static int load_uint(KindContext *ctx, const cJSON *json, const char *key, uint64_t max,
                     uint64_t *out) {
    if (member(json, key) == NULL) {
        return 0;
    }
    return kind_uint(ctx, json, key, max, out);
}

static int load_flag(KindContext *ctx, const cJSON *json, const char *key, int *out) {
    if (member(json, key) == NULL) {
        return 0;
    }
    return kind_bool(ctx, json, key, out);
}

static int power_of_two(uint64_t v) {
    return v != 0 && (v & (v - 1)) == 0;
}

/* A clock in MHz (a number above 0) as kHz. */
static int load_clock(KindContext *ctx, const cJSON *json, uint32_t *khz) {
    const cJSON *item = member(json, "maxClock");

    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble > 0) || item->valuedouble > 4000) {
        return kind_error(ctx, "\"maxClock\" must be a clock in MHz above 0 and at most 4000");
    }
    *khz = (uint32_t)llround(item->valuedouble * 1000.0);
    return 0;
}

static int hex_digit(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

static int load_jedec(KindContext *ctx, const cJSON *json, SerialmemDevice *d) {
    const cJSON *item = member(json, "jedecId");
    size_t length, i;

    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsString(item)) {
        return kind_error(ctx, "\"jedecId\" must be a string of hex digits");
    }
    length = strlen(item->valuestring);
    if (length == 0 || length % 2 != 0 || length > 2 * SM_MAX_ID) {
        return kind_error(ctx, "\"jedecId\" must be 1 to %d bytes of hex digits", SM_MAX_ID);
    }
    for (i = 0; i < length; i++) {
        if (hex_digit(item->valuestring[i]) < 0) {
            return kind_error(ctx, "\"jedecId\" \"%s\" is not hex digits", item->valuestring);
        }
    }
    for (i = 0; i < length / 2; i++) {
        d->jedec[i] = (uint8_t)(hex_digit(item->valuestring[2 * i]) * 16 +
                                hex_digit(item->valuestring[2 * i + 1]));
    }
    d->jedec_len = (unsigned)(length / 2);
    d->facts.jedec_id = kind_strdup(ctx, item->valuestring);
    return d->facts.jedec_id != NULL ? 0 : -1;
}

static int choice(KindContext *ctx, const cJSON *json, const char *key, const char *const *names,
                  int *out) {
    const char *text;
    int i;

    if (kind_string(ctx, json, key, &text) != 0) {
        return -1;
    }
    for (i = 0; names[i] != NULL; i++) {
        if (strcmp(names[i], text) == 0) {
            *out = i;
            return 0;
        }
    }
    return kind_error(ctx, "unknown %s \"%s\"", key, text);
}

static int check_simulated(KindContext *ctx, const SerialmemDevice *d) {
    int flash = d->type == SM_FLASH;

    if (!power_of_two(d->size) || d->size < 4 || d->size > SM_MAX_SIZE) {
        return kind_error(ctx, "\"size\" must be a power of two from 4 to %llu bytes",
                          (unsigned long long)SM_MAX_SIZE);
    }
    if (d->pin_name[SM_CS] == NULL || d->pin_name[SM_SCK] == NULL || d->pin_name[SM_SI] == NULL ||
        d->pin_name[SM_SO] == NULL) {
        return kind_error(ctx, "a simulated device needs \"chipSelect\", \"clock\", \"dataIn\" and "
                               "\"dataOut\"");
    }
    if (d->pin_name[SM_RESET] != NULL) {
        return kind_error(ctx, "\"reset\" is only for a device with \"simulated\": false (no "
                               "simulated chip has a RESET# pin)");
    }
    if (flash) {
        if (d->address_bytes != 3) {
            return kind_error(ctx, "a flash device takes 3 address bytes");
        }
        if (d->a8) {
            return kind_error(ctx, "\"a8InOpcode\" is for an eeprom or fram device");
        }
        if (d->t_page_prog == 0 || d->t_byte_prog == 0 || d->t_sector == 0 || d->t_block32 == 0 ||
            d->t_chip == 0) {
            return kind_error(ctx, "a flash device needs \"pageProgram\", \"byteProgram\", "
                                   "\"sectorErase\", \"blockErase32\" and \"chipErase\" (more than 0)");
        }
        if (d->size < 65536 && d->t_block64 > 0) {
            return kind_error(ctx, "\"blockErase64\" is more than the %u bytes of the array",
                              (unsigned)d->size);
        }
        if (d->prot != PROT_SECTOR && d->prot != PROT_BLOCK) {
            return kind_error(ctx, "a flash device's \"protection\" is \"sector\" or \"block\"");
        }
        if (d->jedec_len == 0) {
            return kind_error(ctx, "a flash device needs \"jedecId\"");
        }
        if (!power_of_two(d->page_size) || d->page_size > d->size || d->size < 4096) {
            return kind_error(ctx, "a flash device's \"pageSize\" is a power of two within the "
                                   "array, and the array is at least 4096 bytes");
        }
        if (d->prot == PROT_SECTOR &&
            (!power_of_two(d->protect_size) || d->protect_size < d->page_size ||
             d->protect_size > d->size)) {
            return kind_error(ctx, "\"protectSize\" must be a power of two from the page size to "
                                   "the array size");
        }
        if (d->sleep || d->fast_read || d->t_write > 0) {
            return kind_error(ctx, "\"sleep\", \"fastRead\" and \"writeCycle\" are not flash keys");
        }
        return 0;
    }
    if (d->prot != PROT_WPEN && d->prot != PROT_WP) {
        return kind_error(ctx, "an eeprom or fram device's \"protection\" is \"wpen\" or \"wp\"");
    }
    if (d->address_bytes < 1 || d->address_bytes > 3) {
        return kind_error(ctx, "\"addressBytes\" must be 1, 2 or 3");
    }
    if (d->a8 && d->address_bytes != 1) {
        return kind_error(ctx, "\"a8InOpcode\" needs \"addressBytes\" 1");
    }
    if ((uint64_t)d->size > ((uint64_t)1 << (8 * d->address_bytes + (d->a8 ? 1 : 0)))) {
        return kind_error(ctx, "\"size\" %u does not fit %u address byte%s%s", (unsigned)d->size,
                          d->address_bytes, d->address_bytes == 1 ? "" : "s",
                          d->a8 ? " and address bit 8 in the opcode" : "");
    }
    if (d->protect_size != 0 || d->t_page_prog || d->t_byte_prog || d->t_page_erase || d->t_sector ||
        d->t_block32 || d->t_block64 || d->t_chip || d->t_wrsr) {
        return kind_error(ctx, "the flash timing keys and \"protectSize\" are for a flash device");
    }
    if (d->type == SM_EEPROM) {
        if (d->t_write == 0) {
            return kind_error(ctx, "an eeprom device needs \"writeCycle\" (more than 0)");
        }
        if (!power_of_two(d->page_size) || d->page_size > d->size / 4) {
            return kind_error(ctx, "\"pageSize\" must be a power of two, at most a quarter of the "
                                   "array");
        }
    } else {
        if (d->t_write != 0) {
            return kind_error(ctx, "a fram device has no write cycle: leave out \"writeCycle\"");
        }
        if (d->page_size != 0) {
            return kind_error(ctx, "a fram device has no write page: leave out \"pageSize\"");
        }
    }
    if (d->type == SM_EEPROM && (d->sleep || d->fast_read)) {
        return kind_error(ctx, "\"sleep\" and \"fastRead\" are for a fram device");
    }
    if (d->sleep && d->t_wake == 0) {
        return kind_error(ctx, "\"sleep\" needs \"wake\" (more than 0)");
    }
    return 0;
}

static void *sm_load_device(KindContext *ctx, const cJSON *json) {
    SerialmemDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *simulated = member(json, "simulated");
    uint64_t v = 0;
    int type = 0, prot = 0;
    size_t i;

    if (d == NULL) {
        return NULL;
    }
    d->simulated = simulated == NULL || cJSON_IsTrue(simulated);
    if (choice(ctx, json, "type", TYPE_NAMES, &type) != 0) {
        return NULL;
    }
    d->type = (SmType)type;
    if (d->simulated && (d->type == SM_DATAFLASH || d->type == SM_PAGEEEPROM)) {
        kind_error(ctx, "a \"%s\" device has no simulation model: \"simulated\" must be false",
                   TYPE_NAMES[type]);
        return NULL;
    }
    if (kind_uint(ctx, json, "size", UINT32_MAX, &v) != 0) {
        return NULL;
    }
    if (v == 0) {
        kind_error(ctx, "\"size\" must be more than 0");
        return NULL;
    }
    d->size = (uint32_t)v;
    d->prot = d->type == SM_FLASH ? PROT_SECTOR : PROT_WPEN;
    if (member(json, "protection") != NULL) {
        if (choice(ctx, json, "protection", PROT_NAMES, &prot) != 0) {
            return NULL;
        }
        d->prot = (SmProt)prot;
    } else if (d->simulated && d->type == SM_FLASH) {
        kind_error(ctx, "missing \"protection\"");
        return NULL;
    }
    d->address_bytes = d->type == SM_FLASH ? 3 : 0;
    if (load_uint(ctx, json, "addressBytes", 3, &v) != 0) {
        return NULL;
    }
    if (member(json, "addressBytes") != NULL) {
        d->address_bytes = (unsigned)v;
    } else if (d->simulated && d->type != SM_FLASH) {
        kind_error(ctx, "missing \"addressBytes\"");
        return NULL;
    }
    d->page_size = d->type == SM_FLASH ? 256 : 0;
    if (load_uint(ctx, json, "pageSize", UINT32_MAX, &v) != 0) {
        return NULL;
    }
    if (member(json, "pageSize") != NULL) {
        d->page_size = (uint32_t)v;
        if (v == 0) {
            kind_error(ctx, "\"pageSize\" must be more than 0");
            return NULL;
        }
    } else if (d->simulated && d->type == SM_EEPROM) {
        kind_error(ctx, "missing \"pageSize\"");
        return NULL;
    }
    if (load_uint(ctx, json, "protectSize", UINT32_MAX, &v) != 0) {
        return NULL;
    }
    d->protect_size = member(json, "protectSize") != NULL ? (uint32_t)v : 0;
    if (d->simulated && d->type == SM_FLASH && d->prot == PROT_SECTOR && d->protect_size == 0) {
        kind_error(ctx, "missing \"protectSize\"");
        return NULL;
    }
    if (load_flag(ctx, json, "a8InOpcode", &d->a8) != 0 ||
        load_flag(ctx, json, "fastRead", &d->fast_read) != 0 ||
        load_flag(ctx, json, "sleep", &d->sleep) != 0 ||
        load_flag(ctx, json, "pullUp", &d->pull_up) != 0) {
        return NULL;
    }
    for (i = 0; i < SM_PINS; i++) {
        if (member(json, PIN_KEY_NAMES[i]) != NULL &&
            kind_string(ctx, json, PIN_KEY_NAMES[i], &d->pin_name[i]) != 0) {
            return NULL;
        }
    }
    if (load_time(ctx, json, "writeCycle", &d->t_write) != 0 ||
        load_time(ctx, json, "pageProgram", &d->t_page_prog) != 0 ||
        load_time(ctx, json, "byteProgram", &d->t_byte_prog) != 0 ||
        load_time(ctx, json, "pageErase", &d->t_page_erase) != 0 ||
        load_time(ctx, json, "sectorErase", &d->t_sector) != 0 ||
        load_time(ctx, json, "blockErase32", &d->t_block32) != 0 ||
        load_time(ctx, json, "blockErase64", &d->t_block64) != 0 ||
        load_time(ctx, json, "chipErase", &d->t_chip) != 0 ||
        load_time(ctx, json, "writeStatus", &d->t_wrsr) != 0 ||
        load_time(ctx, json, "outputValid", &d->t_valid) != 0 ||
        load_time(ctx, json, "outputDisable", &d->t_disable) != 0 ||
        load_time(ctx, json, "wake", &d->t_wake) != 0) {
        return NULL;
    }
    if (load_uint(ctx, json, "statusOnes", 255, &v) != 0) {
        return NULL;
    }
    d->status_ones = member(json, "statusOnes") != NULL ? (uint8_t)v : 0;
    if (load_uint(ctx, json, "busyOnes", 255, &v) != 0) {
        return NULL;
    }
    d->busy_ones = member(json, "busyOnes") != NULL ? (uint8_t)v : 0;
    d->facts.type = TYPE_NAMES[type];
    d->facts.size = d->size;
    d->facts.address_bytes = d->address_bytes;
    d->facts.page_bytes = d->page_size;
    d->facts.write_ps = d->type == SM_FLASH ? d->t_page_prog : d->t_write;
    d->facts.sector_erase_ps = d->t_sector;
    d->facts.chip_erase_ps = d->t_chip;
    if (load_jedec(ctx, json, d) != 0 || load_clock(ctx, json, &d->facts.max_clock_khz) != 0) {
        return NULL;
    }
    if (load_uint(ctx, json, "endurance", (uint64_t)1 << 53, &v) != 0) {
        return NULL;
    }
    d->facts.endurance = member(json, "endurance") != NULL ? v : 0;
    if (load_uint(ctx, json, "retention", 1000000, &v) != 0) {
        return NULL;
    }
    d->facts.retention_years = member(json, "retention") != NULL ? (uint32_t)v : 0;
    if (kind_voltage(ctx, json, NULL, &d->vcc) != 0) {
        return NULL;
    }
    if (d->simulated && check_simulated(ctx, d) != 0) {
        return NULL;
    }
    return d;
}

static int bind_one(KindContext *ctx, const PartPin *pins, size_t pin_count,
                    const SerialmemDevice *d, int which, size_t *out) {
    const char *name = d->pin_name[which];
    PinRole role;

    *out = SM_NONE;
    if (name == NULL) {
        return 0;
    }
    kind_push(ctx, "%s", PIN_KEY_NAMES[which]);
    if (kind_pin(ctx, pins, pin_count, name, out) != 0) {
        return -1;
    }
    role = pins[*out].role;
    if (which == SM_SO ? (role != PIN_OUTPUT && role != PIN_INOUT)
                       : (role != PIN_INPUT && role != PIN_INOUT)) {
        return kind_error(ctx, "pin \"%s\" must be %s", name,
                          which == SM_SO ? "an output or inout pin" : "an input or inout pin");
    }
    kind_pop(ctx);
    return 0;
}

static void *sm_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                            size_t pin_count) {
    const SerialmemDevice *d = raw;
    SerialmemPins *bound = kind_alloc(ctx, sizeof *bound);
    int i;

    if (bound == NULL) {
        return NULL;
    }
    for (i = 0; i < SM_PINS; i++) {
        if (bind_one(ctx, pins, pin_count, d, i, &bound->pin[i]) != 0) {
            return NULL;
        }
    }
    bound->device = d;
    return bound;
}

static int sm_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                        Part *out) {
    const SerialmemDevice *d = raw;
    PartSerialmem *facts = kind_alloc(ctx, sizeof *facts);

    (void)bound;
    if (facts == NULL) {
        return -1;
    }
    *facts = d->facts;
    if (load_clock(ctx, json, &facts->max_clock_khz) != 0) {
        return -1;
    }
    out->serialmem = facts;
    return kind_part_voltage(ctx, json, d->vcc, &out->vcc);
}

const PartKindOps SERIALMEM_KIND = {
    PART_SERIALMEM, "serialmem", &SERIALMEM_MODEL, DEVICE_KEYS, PART_KEYS,
    sm_load_device, sm_bind_pinout, sm_load_part
};
