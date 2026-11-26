/*
 * pio_cov_test (plan-68-S): the states of the pio models and the 65xx bus
 * that pio_test and pio_via_test do not drive: unknown (X) data written into
 * a register, an unknown register select, X on a control line, the PIA's
 * CA2 pulse mode, a VIA counter with unknown bits, and bus hold. The model
 * comments (pio_pia6521.c, pio_via6522.c, bus65.h) say what each one does;
 * the 65xx parts are the shipped W65C21N, W65C22N and W65C22S.
 */

#include "pio_bench.h"

enum { PIA_ORA = 0, PIA_CRA = 1, PIA_ORB = 2, PIA_CRB = 3 };
enum {
    V_ORB, V_ORA, V_DDRB, V_DDRA, V_T1CL, V_T1CH, V_T1LL, V_T1LH, V_T2CL, V_T2CH, V_SR, V_ACR,
    V_PCR, V_IFR, V_IER, V_ORA_NH
};

/* Destroys a bench, failing when the simulation stopped with an error. */
static void done(Bench *b) {
    CHECK(bench_error(b) == NULL);
    bench_destroy(b);
}

#define PASS(name) printf("%s %s\n", failures > before ? "FAIL" : "ok", name)

static void selects(Bench *b, int selected) {
    if (bench_has_pin(b, "CS0")) bench_set(b, "CS0", LOGIC_1);
    if (bench_has_pin(b, "CS1")) bench_set(b, "CS1", LOGIC_1);
    bench_set(b, "CS2B", selected ? LOGIC_0 : LOGIC_1);
}

/* bench_bus65_write's cycle with the data bits in `xmask` driven X. */
static void xwrite(Bench *b, unsigned reg, unsigned value, unsigned xmask) {
    int i;

    bench_run(b, 30 * NS);
    release_data(b);
    bench_set(b, "RWB", LOGIC_0);
    set_rs(b, reg);
    selects(b, 1);
    bench_run(b, 470 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 100 * NS);
    for (i = 0; i < 8; i++) {
        bench_set(b, DATA_NAMES[i], (xmask >> i & 1) ? LOGIC_X : bit_value(value, i));
    }
    bench_run(b, 400 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 0);
}

/* A selected read cycle with RS0 X: which register is read is unknown.
   Returns the data pins' unknown mask at +900 ns. */
static unsigned read_rs_x(Bench *b) {
    unsigned value;
    uint8_t x = 0;
    int i;

    bench_run(b, 30 * NS);
    release_data(b);
    bench_set(b, "RWB", LOGIC_1);
    set_rs(b, 0);
    bench_set(b, "RS0", LOGIC_X);
    selects(b, 1);
    bench_run(b, 470 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 400 * NS);
    for (i = 0; i < 8; i++) {
        LogicValue v = bench_get(b, DATA_NAMES[i]);

        if (v != LOGIC_0 && v != LOGIC_1) x |= (uint8_t)(1u << i);
    }
    (void)data_state(b, &value);
    bench_run(b, 100 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 0);
    set_rs(b, 0);
    return x;
}

static Bench *reset_bench(const char *part, int via) {
    Bench *b = bench_create(shipped_part(part));

    bench_set(b, "RESB", LOGIC_0);
    bench_set(b, "CA1", LOGIC_0);
    bench_set(b, "CB1", LOGIC_0);
    if (via) {
        bench_set(b, "CA2", LOGIC_1);
        bench_set(b, "CB2", LOGIC_1);
    }
    bench_bus65_begin(b);
    bench_bus65_idle(b, 2);
    bench_set(b, "RESB", LOGIC_1);
    bench_bus65_idle(b, 1);
    return b;
}

#define VIA_N "W65C22N6TPG-14"
#define VIA_S "W65C22S6TPG-14"
#define PIA_N "W65C21N6TPG-14"

/* ---- bus65 ------------------------------------------------------------------------ */

/* bus65.h: with bus_hold, a data pin left floating keeps its last level; without
   it, a floating pin is X. A write whose data floats before the fall (outside
   tdcw) stores the held byte, or nothing known. */
static void bus_hold_write(void) {
    Part part = test_part();
    int before = failures;
    int hold;

    for (hold = 0; hold < 2; hold++) {
        Bench *b = new_bench(&part);
        TestRegs *t = regs_of(b);

        t->bus.bus_hold = hold;
        bench_set(b, "RWB", LOGIC_0);
        set_rs(b, 3);
        bench_run(b, HALF);
        bench_set(b, "PHI2", LOGIC_1);
        bench_run(b, 100 * NS);
        set_data(b, 0x5A);
        bench_run(b, 100 * NS);
        release_data(b);                   /* 300 ns before the fall */
        bench_run(b, HALF - 200 * NS);
        bench_set(b, "PHI2", LOGIC_0);
        bench_run(b, 30 * NS);
        bench_set(b, "RWB", LOGIC_1);
        bench_run(b, NS);
        if (hold) {
            CHECK(t->writes == 1 && t->regs[3] == 0x5A && t->known[3] == 0xFF);
        } else {
            CHECK(t->writes == 1 && t->known[3] == 0x00);
        }
        done(b);
    }
    PASS("bus65: a floating data pin writes its held level (bus hold) or X");
}

/* bus65.h: PHI2 X writes every register unknown (write(reg, 0, 0)) once; Z
   reads X, the same level, so no second round; known again (high) is not a
   rise, so nothing is read until the next real cycle, which reads the
   register now unknown. */
static void phi2_x_then_z(void) {
    Part part = test_part();
    Bench *b = new_bench(&part);
    TestRegs *t = regs_of(b);
    int before = failures;
    unsigned value;

    t->regs[0] = 0x33;
    t->known[0] = 0xFF;
    bench_set(b, "PHI2", LOGIC_X);
    bench_run(b, 100 * NS);
    CHECK(t->writes == 16 && t->known[0] == 0);
    bench_set(b, "PHI2", LOGIC_Z);
    bench_run(b, 100 * NS);
    CHECK(t->writes == 16);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 200 * NS);
    CHECK(t->reads == 0 && data_state(b, &value) == 'Z');
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, HALF);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 200 * NS);
    CHECK(t->reads == 1 && data_state(b, &value) == 'X');
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, HALF);
    /* bus65.h: a timer id that is not the bus's is not handled. */
    CHECK(bus65_timer(&t->bus, NULL, NULL, 7) == 0);
    done(b);
    PASS("bus65: PHI2 X then Z is one unknown level; a foreign timer id returns 0");
}

/* ---- pia6521 ---------------------------------------------------------------------- */

/* Port B with DDRB bit 0 unknown: that bit reads unknown, the inputs their pins. */
static void pia_ddr_unknown(void) {
    Bench *b = reset_bench(PIA_N, 0);
    int before = failures;
    uint8_t x;
    int i;

    for (i = 0; i < 8; i++) bench_set(b, PB_NAMES[i], bit_value(0x5A, i));
    xwrite(b, PIA_ORB, 0x00, 0x01);                  /* DDRB: bit 0 X, others input */
    bench_bus65_write(b, PIA_CRB, 0x04);             /* ORB selected */
    CHECK((bench_bus65_read(b, PIA_ORB, &x) & 0xFE) == 0x5A && x == 0x01);
    CHECK(bench_error(b) == NULL);
    done(b);
    PASS("pia: DDRB bit X reads X; input bits read their pins");
}

/* CRA bit 2 unknown: a read of RS 0 is DDRA or ORA, the byte is X; a flag
   already set may have been cleared (X). */
static void pia_select_unknown(void) {
    Bench *b = reset_bench(PIA_N, 0);
    int before = failures;
    uint8_t x;

    bench_bus65_write(b, PIA_CRA, 0x02);             /* CA1 active rising */
    bench_set(b, "CA1", LOGIC_1);
    bench_bus65_idle(b, 1);
    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0x82 && x == 0);
    xwrite(b, PIA_CRA, 0x02, 0x04);                  /* bit 2 X */
    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0x82 && x == 0x04);
    bench_bus65_read(b, PIA_ORA, &x);
    CHECK(x == 0xFF);
    CHECK(bench_bus65_read(b, PIA_CRA, &x) == 0x02 && x == 0x84);   /* IRQA1 X */
    CHECK(bench_error(b) == NULL);
    done(b);
    PASS("pia: CR bit 2 X: the read is X and the IRQ1 flag becomes X");
}

/* A write of ORB with every data bit X in handshake mode: it may be a
   write of ORB (the CB2 strobe) or not: CB2 X. */
static void pia_cb2_unknown_write(void) {
    Bench *b = reset_bench(PIA_N, 0);
    int before = failures;

    bench_bus65_write(b, PIA_CRB, 0x24);             /* CB2 handshake, ORB */
    bench_bus65_idle(b, 1);
    CHECK(bench_get(b, "CB2") == LOGIC_1);
    xwrite(b, PIA_ORB, 0, 0xFF);
    bench_run(b, NS);
    CHECK(bench_get(b, "CB2") == LOGIC_X);
    done(b);
    PASS("pia: an unknown write in CB2 handshake mode makes CB2 X");
}

/* Table 3: CA2 pulse output: low after the read of ORA (tca2 after its fall),
   high again tca2h after the next fall. */
static void pia_ca2_pulse(void) {
    Bench *b = reset_bench(PIA_N, 0);
    int before = failures;

    bench_bus65_write(b, PIA_CRA, 0x2C);             /* CA2 pulse, ORA */
    bench_bus65_idle(b, 1);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    bench_bus65_read(b, PIA_ORA, NULL);
    bench_run(b, 25 * NS);                           /* tca2 20 ns */
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    bench_bus65_idle(b, 1);                          /* not selected: ends at the fall */
    bench_run(b, 10 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_0);           /* tca2h 20 ns */
    bench_run(b, 15 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    done(b);
    PASS("pia: CA2 pulse mode, low for one cycle after a read of ORA");
}

/* CA2 handshake, low after a read of ORA; CA1 going X may be the active edge
   that ends it: CA2 X (trs2). */
static void pia_ca1_x_handshake(void) {
    Bench *b = reset_bench(PIA_N, 0);
    int before = failures;

    bench_bus65_write(b, PIA_CRA, 0x24);             /* CA2 handshake, ORA */
    bench_bus65_read(b, PIA_ORA, NULL);
    bench_run(b, 100 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    bench_set(b, "CA1", LOGIC_X);
    bench_run(b, 100 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_X);
    done(b);
    PASS("pia: CA1 X in CA2 handshake mode makes CA2 X");
}

/* ---- via6522 ---------------------------------------------------------------------- */

/* PCR CA2 = 110 then 111: manual output low, then high. */
static void via_ca2_manual(void) {
    Bench *b = reset_bench(VIA_N, 1);
    int before = failures;

    bench_set(b, "CA2", LOGIC_Z);
    bench_bus65_write(b, V_PCR, 0x0C);
    bench_run(b, NS);
    CHECK(bench_get(b, "CA2") == LOGIC_0);
    bench_bus65_write(b, V_PCR, 0x0E);
    bench_run(b, NS);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    done(b);
    PASS("via: CA2 manual output low (110) and high (111)");
}

/* ACR bit 0 (PA latching) X: after a CA1 active edge the latch may or may
   not hold: ORA reads X. */
static void via_latch_unknown(void) {
    Bench *b = reset_bench(VIA_N, 1);
    int before = failures;
    uint8_t x;
    int i;

    for (i = 0; i < 8; i++) bench_set(b, PA_NAMES[i], bit_value(0x5A, i));
    bench_set(b, "CA1", LOGIC_1);
    bench_bus65_idle(b, 1);
    xwrite(b, V_ACR, 0x00, 0x01);
    bench_set(b, "CA1", LOGIC_0);                    /* the active (negative) edge */
    bench_bus65_idle(b, 1);
    bench_bus65_read(b, V_ORA_NH, &x);
    CHECK(x == 0xFF);
    done(b);
    PASS("via: PA latching X, ORA reads X after the CA1 edge");
}

/* ACR bit 7: PB7 is T1's output and reads back as ORB bit 7 (X when bit 7 is X). */
static void via_orb_pb7(void) {
    Bench *b = reset_bench(VIA_N, 1);
    int before = failures;
    uint8_t x;

    bench_bus65_write(b, V_ACR, 0x80);
    bench_bus65_write(b, V_DDRB, 0x7F);
    bench_bus65_write(b, V_ORB, 0x00);
    CHECK((bench_bus65_read(b, V_ORB, &x) & 0x80) == 0x80 && (x & 0x80) == 0);
    bench_bus65_write(b, V_T1CL, 0x40);
    bench_bus65_write(b, V_T1CH, 0x00);              /* one-shot: PB7 low */
    CHECK((bench_bus65_read(b, V_ORB, &x) & 0x80) == 0x00 && (x & 0x80) == 0);
    xwrite(b, V_ACR, 0x00, 0x80);
    CHECK((bench_bus65_read(b, V_ORB, &x), x & 0x80) == 0x80);
    done(b);
    PASS("via: ORB bit 7 reads PB7's T1 output (X with ACR7 X)");
}

/* A read with RS0 X in CA2 handshake mode: maybe ORA: CA2 X. */
static void via_read_unknown_ca2(void) {
    Bench *b = reset_bench(VIA_N, 1);
    int before = failures;

    bench_set(b, "CA2", LOGIC_Z);
    bench_bus65_write(b, V_PCR, 0x08);               /* CA2 read handshake */
    bench_bus65_idle(b, 1);
    CHECK(bench_get(b, "CA2") == LOGIC_1);
    CHECK(read_rs_x(b) == 0xFF);
    bench_run(b, 100 * NS);
    CHECK(bench_get(b, "CA2") == LOGIC_X);
    done(b);
    PASS("via: an unknown register read in CA2 handshake mode makes CA2 X");
}

/* T1 started from a latch with an unknown low bit: the time-out is unknown,
   IFR6 and PB7 (ACR7) X. */
static void via_t1_unknown_count(void) {
    Bench *b = reset_bench(VIA_N, 1);
    int before = failures;
    uint8_t x;

    bench_bus65_write(b, V_ACR, 0x80);
    xwrite(b, V_T1LL, 0x10, 0x01);
    bench_bus65_write(b, V_T1CH, 0x00);
    bench_bus65_idle(b, 3);
    bench_bus65_read(b, V_IFR, &x);
    CHECK((x & 0x40) != 0);
    CHECK(bench_get(b, "PB7") == LOGIC_X);
    bench_bus65_read(b, V_T1CL, &x);
    CHECK(x == 0xFF);
    done(b);
    PASS("via: T1 with unknown count bits: IFR6 and PB7 X");
}

/* ACR6 (free run) X at a T1 time-out: IFR6 and PB7 X, and the latch reloads. */
static void via_t1_free_run_unknown(void) {
    Bench *b = reset_bench(VIA_N, 1);
    int before = failures;
    uint8_t x;

    xwrite(b, V_ACR, 0x80, 0x40);
    bench_bus65_write(b, V_T1CL, 0x02);
    bench_bus65_write(b, V_T1CH, 0x00);
    bench_bus65_idle(b, 5);
    bench_bus65_read(b, V_IFR, &x);
    CHECK((x & 0x40) != 0);
    CHECK(bench_get(b, "PB7") == LOGIC_X);
    done(b);
    PASS("via: T1 time-out with ACR6 X: IFR6 and PB7 X");
}

/* T2 from a low latch with an unknown bit: IFR5 X as it counts; in pulse
   counting the same on a PB6 pulse; PB6 X makes a known count unknown. */
static void via_t2_unknown(void) {
    Bench *b = reset_bench(VIA_N, 1);
    int before = failures;
    uint8_t x;

    xwrite(b, V_T2CL, 0x10, 0x01);
    bench_bus65_write(b, V_T2CH, 0x00);
    bench_bus65_idle(b, 3);
    bench_bus65_read(b, V_IFR, &x);
    CHECK((x & 0x20) != 0);
    bench_bus65_read(b, V_T2CL, &x);
    CHECK(x == 0xFF);
    done(b);

    b = reset_bench(VIA_N, 1);
    bench_set(b, "PB6", LOGIC_1);
    bench_bus65_write(b, V_ACR, 0x20);               /* T2 counts PB6 pulses */
    xwrite(b, V_T2CL, 0x05, 0x01);
    bench_bus65_write(b, V_T2CH, 0x00);
    bench_bus65_idle(b, 3);
    bench_set(b, "PB6", LOGIC_0);
    bench_bus65_idle(b, 3);
    bench_bus65_read(b, V_IFR, &x);
    CHECK((x & 0x20) != 0);
    done(b);

    b = reset_bench(VIA_N, 1);
    bench_set(b, "PB6", LOGIC_1);
    bench_bus65_write(b, V_ACR, 0x20);
    bench_bus65_write(b, V_T2CL, 0x05);
    bench_bus65_write(b, V_T2CH, 0x00);
    bench_bus65_idle(b, 3);
    CHECK(bench_bus65_read(b, V_T2CL, &x) == 0x05 && x == 0);
    bench_set(b, "PB6", LOGIC_X);
    bench_bus65_idle(b, 3);
    bench_bus65_read(b, V_IFR, &x);
    CHECK((x & 0x20) != 0);
    bench_bus65_read(b, V_T2CL, &x);
    CHECK(x == 0xFF);
    done(b);
    PASS("via: T2 with unknown count bits or PB6 X: count and IFR5 X");
}

/* ACR 001 (shift in under T2): T2 times the shift clock and its time-out
   sets no IFR5 (pio_via6522_regs.c, v22_t2_rise). */
static void via_t2_shift_clock(void) {
    Bench *b = reset_bench(VIA_N, 1);
    int before = failures;
    uint8_t x;

    bench_set(b, "CB1", LOGIC_Z);                    /* CB1 is the shift clock output */
    bench_bus65_write(b, V_ACR, 0x04);
    bench_bus65_write(b, V_T2CL, 0x02);
    bench_bus65_write(b, V_T2CH, 0x00);
    bench_bus65_idle(b, 8);
    CHECK((bench_bus65_read(b, V_IFR, &x) & 0x20) == 0 && (x & 0x20) == 0);
    done(b);
    PASS("via: T2 timing the shift clock sets no IFR5");
}

/* Shift in under CB1: CB1 X is an unknown clock edge: SR X, and the next
   bit counted leaves IFR2 X; an edge less than tpcr before the fall the same. */
static void via_sr_external_unknown(void) {
    Bench *b = reset_bench(VIA_N, 1);
    int before = failures;
    uint8_t x;

    bench_set(b, "CB1", LOGIC_1);
    bench_bus65_write(b, V_ACR, 0x0C);               /* shift in under CB1 */
    bench_bus65_read(b, V_SR, &x);                   /* starts the shift */
    bench_set(b, "CB1", LOGIC_X);
    bench_bus65_idle(b, 1);
    bench_set(b, "CB1", LOGIC_0);
    bench_bus65_idle(b, 1);
    bench_set(b, "CB1", LOGIC_1);                    /* a rising edge: one bit in */
    bench_bus65_idle(b, 2);
    bench_bus65_read(b, V_IFR, &x);
    CHECK((x & 0x04) != 0);
    /* The bit shifted in after the unknown edge is CB2's level (high). */
    CHECK((bench_bus65_read(b, V_SR, &x) & 0x01) == 0x01 && x == 0xFE);
    done(b);

    /* Shift out under CB1 (111): a CB1 fall 5 ns before PHI2 falls. */
    b = reset_bench(VIA_N, 1);
    bench_set(b, "CB1", LOGIC_1);
    bench_bus65_write(b, V_ACR, 0x1C);
    bench_bus65_write(b, V_SR, 0xA5);
    bench_run(b, 500 * NS);
    bench_set(b, "PHI2", LOGIC_1);
    bench_run(b, 495 * NS);
    bench_set(b, "CB1", LOGIC_0);
    bench_run(b, 5 * NS);
    bench_set(b, "PHI2", LOGIC_0);
    bench_run(b, 100 * NS);
    CHECK(bench_get(b, "CB2") == LOGIC_X);
    bench_run(b, 900 * NS);
    bench_bus65_read(b, V_IFR, &x);
    CHECK((x & 0x04) != 0);
    done(b);
    PASS("via: shift register under CB1 with an unknown or late edge goes X");
}

int main(void) {
    bus_hold_write();
    phi2_x_then_z();
    pia_ddr_unknown();
    pia_select_unknown();
    pia_cb2_unknown_write();
    pia_ca2_pulse();
    pia_ca1_x_handshake();
    via_ca2_manual();
    via_latch_unknown();
    via_orb_pb7();
    via_read_unknown_ca2();
    via_t1_unknown_count();
    via_t1_free_run_unknown();
    via_t2_unknown();
    via_t2_shift_clock();
    via_sr_external_unknown();
    if (shipped_library != NULL) part_library_destroy(shipped_library);
    if (failures > 0) {
        printf("%d failures\n", failures);
        return 1;
    }
    return 0;
}
