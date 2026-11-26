/* models/serialmem_int.h: what the pieces of models/serialmem.c share (plan-68-I/J split it). */
#ifndef MODELS_SERIALMEM_INT_H
#define MODELS_SERIALMEM_INT_H

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

typedef enum { SM_EEPROM, SM_FRAM, SM_FLASH, SM_DATAFLASH, SM_PAGEEEPROM } SmType;

typedef enum { PROT_WPEN, PROT_WP, PROT_SECTOR, PROT_BLOCK } SmProt;

enum { SM_CS, SM_SCK, SM_SI, SM_SO, SM_WP, SM_HOLD, SM_RESET, SM_PINS };

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

int smem_t_not(int a);
int smem_t_and(int a, int b);
int smem_t_or(int a, int b);
extern const char *const smem_TYPE_NAMES[6];
extern const char *const smem_PROT_NAMES[5];
extern const char *const smem_PIN_KEY_NAMES[SM_PINS];
extern const char *const smem_DEVICE_KEYS[38];
extern const char *const smem_PART_KEYS[4];
int smem_control(const SmState *s, int which);
SimTime smem_now_of(const SmState *s);
uint32_t smem_quarter(const SmState *s);
int smem_range_protected(const SmState *s, uint32_t start, uint32_t length);
void smem_put_byte(SmState *s, uint32_t address, uint8_t v, uint8_t k, int and_mode);
void smem_corrupt_matching(SmState *s, uint32_t addr, uint32_t known, uint32_t span);
void smem_status_byte(const SmState *s, unsigned which, uint8_t *v, uint8_t *k);
void smem_apply_pending(SmState *s);
void smem_start_op(SmState *s, SimTime duration);
extern const SimComponentType smem_SERIALMEM_MODEL;
void smem_sm_received(void *context, unsigned index, uint8_t data, uint8_t known);
SerialSpiByte smem_sm_next(void *context, unsigned index);
void smem_sm_begin(void *context);
void smem_set_all_sectors(SmState *s, int value);
void smem_sm_end(void *context, unsigned bits);

#endif
