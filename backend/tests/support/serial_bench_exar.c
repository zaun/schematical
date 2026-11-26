/* serial_bench_exar (split from the serial tests, plan-68-G). */
#include "serial_bench.h"
#include "serial_bench_exar.h"

/* ==== plan-12-E: the Exar/MaxLinear options (cases E1-E28) ===========================
 *
 * The 28 Exar devices on their fixture objects (tests/fixtures/serial/keys/ merged
 * into devices/ by merge.py), on the bench above. Motorola cycles: CS is the strobe
 * and the Intel write pin is R/W (m_write / m_read).
 */

const BenchConfig E_ST450_BENCH = {
    "IORBAR", "IOWBAR", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS0=1", "CS1=1", "CS2=0", "AS=0", "IOR=0", "IOW=0", "RX=1", "CTS=1", "DSR=1", "CD=1", "RI=1",
     NULL},
    "RESET", 1, "XTAL1", "BAUDOUT", "RCLK", {{NULL}}, 0
};

const BenchConfig E_ONE_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "RX=1", "CTS=1", "DSR=1", "CD=1", "RI=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{NULL}}, 0
};

const BenchConfig E_L580_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "RX=1", "CTS=1", "DSR=1", "CD=1", "RI=1", "16/68=1", "PWRSAVE=0", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{NULL}}, 0
};

const BenchConfig E_DUAL_MOTO_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "CTSA=1", "CTSB=1", "DSRA=1", "DSRB=1", "CDA=1", "CDB=1", "RIA=1", "RIB=1",
     "16/68=1", "PWRSAVE=0", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{"CSA=0"}, {"CSB=0"}}, 2
};

const BenchConfig E_DUAL_68_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "CTSA=1", "CTSB=1", "DSRA=1", "DSRB=1", "CDA=1", "CDB=1", "RIA=1", "RIB=1",
     "16/68=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{"CSA=0"}, {"CSB=0"}}, 2
};

const BenchConfig E_DUAL_CLKSEL_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "CTSA=1", "CTSB=1", "DSRA=1", "DSRB=1", "CDA=1", "CDB=1", "RIA=1", "RIB=1",
     "16/68=1", "PWRSAVE=0", "CLKSEL=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{"CSA=0"}, {"CSB=0"}}, 2
};

const BenchConfig E_DUAL_QFN_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "CTSA=1", "CTSB=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{"CSA=0"}, {"CSB=0"}}, 2
};

const BenchConfig E_QUAD_V_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "CDA=1", "CDB=1", "CDC=1", "CDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     "16/68=1", "INTSEL=0", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4
};

const BenchConfig E_QUAD_CLKSEL_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "CDA=1", "CDB=1", "CDC=1", "CDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     "CLKSEL=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4
};

const BenchConfig E_C854_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "CDA=1", "CDB=1", "CDC=1", "CDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     "16/68=1", "INTSEL=0", "CLKSEL=1", "FSRS=1", "CHCCLK=0", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4
};

const BenchConfig E_C864_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "CDA=1", "CDB=1", "CDC=1", "CDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     "16/68=1", "INTSEL=0", "CLKSEL=1", "FSRS=1", "CHCCLK=0", "DACKA=1", "DACKB=1", "DACKC=1",
     "DACKD=1", "AEN=0", "TC=0", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4
};

const BenchConfig E_V798_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"},
    {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"}, 8,
    {"CS=0", "16/68=1", "TMRCK=0", "ENIR=0", "RX0=1", "RX1=1", "RX2=1", "RX3=1", "RX4=1", "RX5=1",
     "RX6=1", "RX7=1", "CTS0=1", "CTS1=1", "CTS2=1", "CTS3=1", "CTS4=1", "CTS5=1", "CTS6=1",
     "CTS7=1", "DSR0=1", "DSR1=1", "DSR2=1", "DSR3=1", NULL},
    "RST", 0, "XTAL1", NULL, NULL, {{NULL}}, 0
};

const BenchConfig E_L784_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"},
    {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"}, 8,
    {"CS=0", "16/68=1", "TMRCK=0", "ENIR=0", "RX0=1", "RX1=1", "RX2=1", "RX3=1", "CTS0=1",
     "CTS1=1", "CTS2=1", "CTS3=1", "DSR0=1", "DSR1=1", "DSR2=1", "DSR3=1", "CD0=1", "CD1=1",
     "CD2=1", "CD3=1", "RI0=1", "RI1=1", "RI2=1", "RI3=1", NULL},
    "RST", 0, "XTAL1", NULL, NULL, {{NULL}}, 0
};

/* The enhanced bank (LCR = 0xBF) for one EFR write, then back to 8N1. */
void e_efr(Bench *b, unsigned efr) {
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, efr);
    bus_write(b, 3, 0x03);
}

/* Runs to `before` ps ahead of the next XTAL1 rising edge (the bench clock
   rises at (2n + 1) half periods). */
void e_before_rise(Bench *b, SimTime before) {
    SimTime now = sim_now(b->sim);
    SimTime next = (now / b->period) * b->period + b->half;

    while (next <= now + before) {
        next += b->period;
    }
    run(b, next - before - now);
}

/* Motorola cycles: R/W on IOW (1 = read), CS the strobe, data latched at CS rising. */
void m_write(Bench *b, const char *cs, unsigned a, unsigned v) {
    set_address(b, a);
    set_data(b, 1, v);
    set(b, "IOW", LOGIC_0);
    run(b, NS(40));
    set(b, cs, LOGIC_0);
    run(b, NS(120));
    set(b, cs, LOGIC_1);
    run(b, NS(40));
    set_data(b, 0, 0);
    set(b, "IOW", LOGIC_1);
    run(b, NS(160));
}

unsigned m_read_text(Bench *b, const char *cs, unsigned a, char text[9]) {
    unsigned v = 0;
    unsigned i;

    set_address(b, a);
    set(b, "IOW", LOGIC_1);
    run(b, NS(40));
    set(b, cs, LOGIC_0);
    run(b, NS(150));
    for (i = 0; i < 8; i++) {
        LogicValue l = get(b, b->cfg->data[i]);

        text[7 - i] = "01XZ"[l];
        if (l == LOGIC_1) {
            v |= 1u << i;
        }
    }
    text[8] = '\0';
    set(b, cs, LOGIC_1);
    run(b, NS(160));
    return v;
}

void m_expect(Bench *b, const char *cs, unsigned a, const char *want, const char *what) {
    char text[9];
    char message[160];

    m_read_text(b, cs, a, text);
    snprintf(message, sizeof message, "%s: read %s, want %s", what, text, want);
    check(strcmp(text, want) == 0, message);
}

/* Bit time of a TX pin's start bit: the low time from its falling edge, sampled
   every `step`; 0 when no start bit within `limit`. */
SimTime e_low_time(Bench *b, const char *pin, SimTime limit, SimTime step) {
    SimTime t0;
    SimTime spent = 0;

    while (get(b, pin) != LOGIC_0) {
        if (spent >= limit) {
            return 0;
        }
        run(b, step);
        spent += step;
    }
    t0 = sim_now(b->sim);
    while (get(b, pin) == LOGIC_0 && sim_now(b->sim) - t0 < limit) {
        run(b, step);
    }
    return sim_now(b->sim) - t0;
}

void e_motorola_dual(Bench *b) {
    set(b, "RESET", LOGIC_1);              /* Motorola: RESET is active low */
    set(b, "16/68", LOGIC_0);
    set(b, "CSA", LOGIC_1);
    set(b, "CSB", LOGIC_0);
    set(b, "IOR", LOGIC_1);
    set(b, "IOW", LOGIC_1);
    run(b, NS(500));
    m_write(b, "CSA", 7, 0xA5);            /* A3 = 0: channel A */
    set(b, "CSB", LOGIC_1);
    m_write(b, "CSA", 7, 0x5A);            /* A3 = 1: channel B */
    m_expect(b, "CSA", 7, "01011010", "Motorola, A3 = 1: channel B's SPR");
    set(b, "CSB", LOGIC_0);
    m_expect(b, "CSA", 7, "10100101", "Motorola, A3 = 0: channel A's SPR");
    expect_pin(b, "INTA", LOGIC_Z, "IRQ released (no interrupt)");
    expect_pin(b, "INTB", strncmp(b->part->name, "SFX-XR16V", 9) == 0 ? LOGIC_Z : LOGIC_0,
               "INTB in Motorola mode: driven low (XR16L/M), NC (XR16V)");
    set(b, "CSB", LOGIC_1);
    m_write(b, "CSA", 1, 0x02);            /* channel B: THRE interrupt, MCR[3] = 0 */
    run(b, NS(300));
    expect_pin(b, "INTA", LOGIC_0, "channel B's interrupt pulls IRQ low, MCR[3] = 0 (not gated)");
    m_write(b, "CSA", 1, 0x00);
    run(b, NS(300));
    expect_pin(b, "INTA", LOGIC_Z, "released again");
    set(b, "RESET", LOGIC_0);
    run(b, NS(500));
    set(b, "RESET", LOGIC_1);
    set(b, "CSB", LOGIC_0);
    run(b, NS(500));
    m_expect(b, "CSA", 7, "11111111", "RESET low resets in Motorola mode (SPR 0xFF)");
    set(b, "16/68", LOGIC_1);
    set(b, "RESET", LOGIC_0);
    set(b, "CSA", LOGIC_1);
    run(b, NS(500));
}
void e_powersave(Bench *b, int indicator) {
    SimTime bt;
    char text[9];
    unsigned u;

    program_unit(b, 1, 1, 0x03);
    program_unit(b, 0, 1, 0x03);
    bt = bit_time(b, 1);
    for (u = 0; u < 2; u++) {
        select_unit(b, u);
        e_efr(b, 0x10);
        bus_write(b, 7, 0x66);
        bus_write(b, 4, 0x08);
        bus_write(b, 1, 0x10);             /* IER[4] sleep (both channels needed) */
    }
    select_unit(b, 0);
    set(b, "PWRSAVE", LOGIC_1);
    run(b, bt * 4);
    bus_read_text(b, 7, text);
    check(strcmp(text, "ZZZZZZZZ") == 0, "asleep with PWRSAVE high: the bus is isolated (D7-D0 Z)");
    bus_write(b, 7, 0x77);                 /* ignored */
    drive_named(b, "RXA", 0x41, bt);       /* the start bit wakes it; the byte stays awake */
    run(b, bt * 2);
    if (indicator) {
        expect_pin(b, "INTA", LOGIC_1, "woken: the wake-up indicator asserts INT");
        expect_reg(b, 2, "00000001", "the indicator reads as ISR 0x01");
        run(b, NS(300));
        expect_pin(b, "INTA", LOGIC_0, "and the ISR read clears it");
        bus_write(b, 1, 0x00);             /* stay awake */
    } else {
        bus_write(b, 1, 0x00);
    }
    expect_reg(b, 7, "01100110", "awake: the bus works; the isolated write was dropped");
    expect_reg(b, 0, "01000001", "and the waking byte arrived");
    set(b, "PWRSAVE", LOGIC_0);
}
