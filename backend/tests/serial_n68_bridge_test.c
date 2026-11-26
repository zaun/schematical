/*
 * serial_n68_bridge_test (plan-68-N): core paths of the ns16550 model
 * (models/ns16550_io.c) that only a bridge's host port reaches: the
 * XR20M1280 TX address bit (the 9th bit in place of the parity), and the
 * SC16IS750 software reset written X (the device unknown: reads X, writes
 * dropped). Expectations from planning/plan-12-data/specs/bridges.md.
 */

#include "serial_bench_bridge.h"
#include "serial_bench_n68.h"

/* An SPI register write (mode 0) whose data bits may be X ('0', '1', 'X',
   high bit first). */
static void spi_write_x(MB *b, unsigned reg, const char *bits) {
    unsigned i;

    mb_set(b, b->cs, LOGIC_0);
    mb_run(b, SPI_HALF);
    for (i = 0; i < 16; i++) {
        LogicValue v = i < 8 ? (((reg << 3) >> (7 - i)) & 1u ? LOGIC_1 : LOGIC_0)
                     : bits[i - 8] == '1' ? LOGIC_1 : bits[i - 8] == '0' ? LOGIC_0 : LOGIC_X;

        mb_set(b, b->mosi, v);
        mb_run(b, SPI_HALF);
        mb_set(b, b->sclk, LOGIC_1);
        mb_run(b, SPI_HALF);
        mb_set(b, b->sclk, LOGIC_0);
    }
    mb_run(b, SPI_HALF);
    mb_set(b, b->cs, LOGIC_1);
    mb_run(b, SPI_HALF * 4);
}

/* XR20M1280 SFR[7] (bridges.md XR20M1280 SFR): the TX address bit, the 9th
   bit = 1 for the next THR byte, self-clearing, with LCR[5:3] = 111 (forced
   0 parity): that byte's parity bit is 1, the next one's 0. */
static void case_xr_address_bit(void) {
    Br r = br_open("SFX-XR20M1280-qfn32", 1, 1, NULL);
    MB *b = r.b;
    char frame[16];

    case_begin();
    br_w(&r, 0, 3, 0xBF);
    br_w(&r, 0, 2, 0x10);
    br_w(&r, 0, 3, 0x80);
    br_w(&r, 0, 0, 0x01);
    br_w(&r, 0, 1, 0x00);
    br_w(&r, 0, 3, 0x3B);   /* 8 bits, forced parity 0 */
    br_w(&r, 0, 6, 0x80);   /* SFR[7]: TX address bit */
    br_w(&r, 0, 0, 0x55);
    br_frame(b, "TX", BIT1, frame, BIT1 * 20);
    check(strcmp(frame, "0101010101") == 0, "address byte 0x55: 9th (parity) bit 1");
    mb_run(b, BIT1 * 3);
    br_w(&r, 0, 0, 0x55);
    br_frame(b, "TX", BIT1, frame, BIT1 * 20);
    check(strcmp(frame, "0101010100") == 0, "the next byte: forced parity 0 again");
    case_end("B1 XR20M1280: SFR[7] TX address bit");
    mb_close(b);
}

/* SC16IS750 IOControl[3] (SRESET) written X: the device is unknown (P2) -
   registers read X and writes are dropped until the RESET pin; DLL/DLM are
   not reset by it, so a divisor written while unknown would still show. */
static void case_sreset_x(void) {
    Br r = br_open("SFX-SC16IS750", 1, 0, NULL);
    MB *b = r.b;

    case_begin();
    br_w(&r, 0, 3, 0x80);
    br_w(&r, 0, 0, 0x01);
    br_w(&r, 0, 3, 0x03);
    spi_write_x(b, 14, "0000X000");
    br_expect_text(&r, 0, 7, "XXXXXXXX", "SRESET X: SPR reads X");
    br_w(&r, 0, 3, 0x80);   /* dropped */
    br_w(&r, 0, 0, 0x05);   /* dropped */
    br_reset(&r);
    br_w(&r, 0, 3, 0x80);
    br_expect(&r, 0, 0, 0x01, "after RESET: DLL 0x01, the unknown-time write was dropped");
    case_end("B2 SC16IS750: SRESET X, reads X, writes dropped");
    mb_close(b);
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_xr_address_bit();
    case_sreset_x();
    return serial_bench_finish();
}
