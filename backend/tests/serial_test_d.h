/*
 * serial_test: plan-12-D cases D1-D24, on the shipped TI/National/Intersil/
 * SMSC/NXP device objects (tests/fixtures/serial/ns16550.json). Included by
 * serial_test.c after its bench and helpers.
 */

/* ---- Bench configurations ----------------------------------------------------------- */

static const BenchConfig TI550_BENCH = {
    "RD1", "WR1", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS0=1", "CS1=1", "CS2=0", "ADS=0", "RD2=0", "WR2=0", "SIN=1", "CTS=1", "DSR=1", "DCD=1",
     "RI=1", NULL},
    "MR", 1, "XIN", "BAUDOUT", "RCLK", {{NULL}}, 0
};

static const BenchConfig TI450_BENCH = {
    "DISTRBAR", "DOSTRBAR", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS0=1", "CS1=1", "CS2=0", "ADS=0", "DISTR=0", "DOSTR=0", "SIN=1", "CTS=1", "DSR=1", "DCD=1",
     "RI=1", NULL},
    "MR", 1, "XTAL1", "BAUDOUT", "RCLK", {{NULL}}, 0
};

static const BenchConfig QUAD_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "DCDA=1", "DCDB=1", "DCDC=1", "DCDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     "INTN=0", NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4
};

static const BenchConfig QUAD_CD_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "CDA=1", "CDB=1", "CDC=1", "CDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     "INTSEL=0", "CLKSEL=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4, 400
};

static const BenchConfig QUAD_C_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "CDA=1", "CDB=1", "CDC=1", "CDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4
};

static const BenchConfig DUAL_SYNC_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "CTSA=1", "CTSB=1", "DSRA=1", "DSRB=1", "CDA=1", "CDB=1", "RIA=1", "RIB=1",
     NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}}, 2, 400
};

static const BenchConfig DUAL_CD_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "CTSA=1", "CTSB=1", "DSRA=1", "DSRB=1", "CDA=1", "CDB=1", "RIA=1", "RIB=1",
     NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}}, 2
};

static const BenchConfig CHSEL_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "RXA=1", "RXB=1", "CTSA=1", "CTSB=1", "DSRA=1", "DSRB=1", "CDA=1", "CDB=1", "RIA=1",
     "RIB=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CHSEL=1"}, {"CHSEL=0"}}, 2
};

static const BenchConfig NS_BENCH = {
    "RD", "WR", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "SIN1=1", "SIN2=1", "CTS1=1", "CTS2=1", "DSR1=1", "DSR2=1", "DCD1=1", "DCD2=1",
     "RI1=1", "RI2=1", NULL},
    "MR", 1, "XIN", NULL, NULL,
    {{"CHSL=1"}, {"CHSL=0"}}, 2
};

static const BenchConfig TI45X_BENCH = {
    "IOR", "IOW", {"DB0", "DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7"}, {"A0", "A1", "A2"}, 3,
    {"SIN0=1", "SIN1=1", "CTS0=1", "CTS1=1", "DSR0=1", "DSR1=1", "RLSD0=1", "RLSD1=1", "RI0=1",
     "RI1=1", "LPTOE=0", "ACK=1", "BUSY=0", "PE=0", "SLCT=1", "ERROR=1", NULL},
    "RESET", 0, "CLK", NULL, NULL,
    {{"CS0=0"}, {"CS1=0"}, {"CS2=0"}}, 3
};

static const BenchConfig TI451_BENCH = {
    "IOR", "IOW", {"DB0", "DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7"}, {"A0", "A1", "A2"}, 3,
    {"SIN0=1", "CTS0=1", "DSR0=1", "RLSD0=1", "RI0=1", "LPTOE=0", "ACK=1", "BUSY=0", "PE=0",
     "SLCT=1", "ERROR=1", NULL},
    "RESET", 0, "CLK", NULL, NULL,
    {{"CS0=0"}, {"CS2=0"}}, 2
};

static const BenchConfig TI552_BENCH = {
    "IOR", "IOW", {"DB0", "DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7"}, {"A0", "A1", "A2"}, 3,
    {"SIN0=1", "SIN1=1", "CTS0=1", "CTS1=1", "DSR0=1", "DSR1=1", "DCD0=1", "DCD1=1", "RI0=1",
     "RI1=1", "PEMD=0", "ENIRQ=0", "TRI=0", "ACK=1", "BUSY=0", "PE=0", "SLCT=1", "ERR=1", NULL},
    "RESET", 0, "CLK", NULL, NULL,
    {{"CS0=0"}, {"CS1=0"}, {"CS2=0"}}, 3
};

static const BenchConfig COM92_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"SIN=1", "CTS=1", "DSR=1", "DCD=1", "RI=1", "ACK=1", "BUSY=0", "PE=0", "SLCT=1", "ERROR=1",
     NULL},
    "RESET", 1, "CLK", NULL, NULL,
    {{"CE1=0"}, {"CE0=0"}}, 2
};

static const BenchConfig NXP650_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "RX=1", "CTS=1", "DSR=1", "CD=1", "RI=1", NULL},
    "RESET", 1, "XTAL1", "BAUDOUT", "RCLK", {{NULL}}, 0
};

static const BenchConfig TI750E_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "RX=1", "CTS=1", "DSR=1", "CD=1", "RI=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{NULL}}, 0
};

/* Programs the selected unit: divisor and LCR. */
static void program_unit(Bench *b, unsigned unit, unsigned divisor, unsigned lcr) {
    select_unit(b, unit);
    program(b, divisor, lcr);
}

/* Drives bits ('0'/'1') on a named pin, one per bit time. */
static void drive_bits_on(Bench *b, const char *pin, const char *bits, SimTime bt) {
    size_t i;

    for (i = 0; bits[i] != '\0'; i++) {
        set(b, pin, bits[i] == '1' ? LOGIC_1 : LOGIC_0);
        run(b, bt);
    }
}

/* Drives a byte on a named RX pin at the bit time. */
static void drive_named(Bench *b, const char *rx, unsigned v, SimTime bt) {
    unsigned i;

    set(b, rx, LOGIC_0);
    run(b, bt);
    for (i = 0; i < 8; i++) {
        set(b, rx, (v >> i) & 1u ? LOGIC_1 : LOGIC_0);
        run(b, bt);
    }
    set(b, rx, LOGIC_1);
    run(b, bt);
}

static int part_exists(const char *name) {
    return part_library_find(library, name) != NULL;
}

/* Waits up to `limit` for a pin to reach a level; 1 when it did. */
static int wait_level(Bench *b, const char *pin, LogicValue level, SimTime limit) {
    SimTime spent = 0;

    while (spent < limit) {
        if (get(b, pin) == level) {
            return 1;
        }
        run(b, NS(200));
        spent += NS(200);
    }
    return 0;
}

/* ---- D1 TL16C550C: auto flow (AFE) ------------------------------------------------------- */

static void case_d1(void) {
    Bench *b = bench_open_cfg("SFX-TL16C550C", HALF_18432, &TI550_BENCH);
    SimTime bt;
    unsigned i;
    char text[9];

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x41);          /* FIFO, trigger 4 */
    bus_write(b, 4, 0x22);          /* AFE + RTS: auto-RTS and auto-CTS */
    run(b, NS(500));
    expect_pin(b, "RTS", LOGIC_0, "auto-RTS asserts RTS while below the trigger");
    for (i = 0; i < 3; i++) {
        drive_byte(b, 0x41 + i, bt, 1);
    }
    run(b, bt);
    expect_pin(b, "RTS", LOGIC_0, "3 bytes, trigger 4: RTS still active");
    drive_byte(b, 0x44, bt, 1);
    run(b, bt);
    expect_pin(b, "RTS", LOGIC_1, "the 4th byte reaches the trigger: RTS inactive");
    for (i = 0; i < 3; i++) {
        bus_read(b, 0);
    }
    expect_pin(b, "RTS", LOGIC_1, "RTS stays inactive until the FIFO is empty");
    bus_read(b, 0);
    run(b, bt);
    expect_pin(b, "RTS", LOGIC_0, "emptied: RTS active again");
    /* Auto-CTS: CTS inactive holds the transmitter; no ΔCTS meanwhile. */
    bus_read(b, 6);
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    bus_write(b, 0, 0x55);
    check(!wait_start(b, bt * 12), "CTS inactive: nothing is sent");
    set(b, "CTS", LOGIC_0);
    check(wait_start(b, bt * 30), "CTS active: the byte goes out");
    bus_read_text(b, 6, text);
    check(text[7] == '0', "no ΔCTS while auto-CTS is on (decisions.md)");
    case_end("D1 TL16C550C afe: auto-RTS at the trigger, auto-CTS holds TX, no ΔCTS");
    bench_close(b);
}

/* ---- D2 TL16C550D: 9-RCLK timeout; BGA without modem inputs ------------------------------- */

static void case_d2(void) {
    Bench *b = bench_open_cfg("SFX-TL16C550D", HALF_18432, &TI550_BENCH);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x41);
    bus_write(b, 1, 0x01);
    drive_byte(b, 0x61, bt, 1);
    /* The byte lands 3 RCLKs after its stop sample (mid stop bit), and the
       timeout comes 4 characters (40 bits) + 9 RCLKs later: 39.9 bit times
       after drive_byte returns (it ends half a bit after the stop sample). */
    run(b, bt * 39);
    expect_pin(b, "INTRPT", LOGIC_0, "39 bit times after the frame: no timeout yet");
    run(b, bt * 2);
    expect_pin(b, "INTRPT", LOGIC_1, "timeout after 4 characters + 9 RCLKs");
    bench_close(b);
    /* The ZQS (bga24) package has no DSR, RI, DCD: MSR[5:7] read X, deltas 0. */
    check(part_exists("SFX-TL16C550D-bga24"), "the bga24 package fixture (device data)");
    if (part_exists("SFX-TL16C550D-bga24")) {
        b = bench_open_cfg("SFX-TL16C550D-bga24", HALF_18432, &TI550_BENCH);
        expect_reg(b, 6, "XXX00000", "bga24: MSR DSR/RI/DCD X, CTS inactive, deltas 0");
        bench_close(b);
    }
    case_end("D2 TL16C550D: timeout delay 9 RCLKs; bga24 without modem inputs");
}

/* ---- D3 TL16C750: 16/64-byte FIFO, rxTrigger64, sleep -------------------------------------- */

static void case_d3(void) {
    Bench *b = bench_open_cfg("SFX-TL16C750", HALF_18432, &TI550_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x41);                 /* 16-byte, trigger 4 */
    expect_reg(b, 2, "11000001", "16-byte mode: IIR[5] = 0");
    bus_write(b, 3, 0x83);
    bus_write(b, 2, 0x61);                 /* FCR[5] with LCR[7] = 1: 64-byte, trigger 16 */
    bus_write(b, 3, 0x03);
    expect_reg(b, 2, "11100001", "64-byte mode: IIR[5] = 1");
    bus_write(b, 1, 0x01);
    for (i = 0; i < 15; i++) {
        drive_byte(b, 0x30 + i, bt, 1);
    }
    run(b, bt);
    expect_pin(b, "INTRPT", LOGIC_0, "64-byte mode trigger 01 = 16: 15 bytes no interrupt");
    drive_byte(b, 0x3F, bt, 1);
    run(b, bt);
    expect_pin(b, "INTRPT", LOGIC_1, "16 bytes: interrupt (rxTrigger64)");
    for (i = 0; i < 16; i++) {
        bus_read(b, 0);
    }
    for (i = 0; i < 17; i++) {
        drive_byte(b, 0x40 + i, bt, 1);
    }
    run(b, bt);
    expect_reg(b, 5, "01100001", "64-byte FIFO: 17 bytes, no overrun");
    for (i = 0; i < 17; i++) {
        bus_read(b, 0);
    }
    /* Sleep (IER[4]): idle, the baud clock stops (BAUDOUT X); a THR write wakes it. */
    bus_write(b, 1, 0x10);
    run(b, bt * 2);
    expect_pin(b, "BAUDOUT", LOGIC_X, "asleep: BAUDOUT X (decisions.md)");
    bus_write(b, 0, 0x5A);
    check(wait_start(b, bt * 4), "a THR write wakes it and the byte goes out");
    case_end("D3 TL16C750: 16/64-byte FIFO by FCR[5], rxTrigger64, IER[4] sleep");
    bench_close(b);
}

/* ---- D4 TL16C750E: LCR 0x1D, DLF, TLR x8 --------------------------------------------------- */

static void case_d4(void) {
    Bench *b = bench_open_cfg("SFX-TL16C750E", NS(50), &TI750E_BENCH);
    SimTime t0;
    SimTime t1;
    char message[96];

    case_begin();
    expect_reg(b, 3, "00011101", "LCR resets to 0x1D");
    /* EFR[4] on, then DLL = 1 with DLF = 0x20 (1 + 32/64 = 1.5), 8N1. */
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x10);
    bus_write(b, 3, 0x83);
    bus_write(b, 0, 0x01);
    bus_write(b, 1, 0x00);
    bus_write(b, 7, 0x20);
    expect_reg(b, 7, "00100000", "DLF reads back");
    bus_write(b, 3, 0x03);
    bus_write(b, 0, 0x00);   /* 0x00: start + 8 zeros, then the stop's rising edge */
    check(wait_level(b, "TX", LOGIC_0, NS(100000)), "the byte starts");
    t0 = sim_now(b->sim);
    while (get(b, "TX") == LOGIC_0) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    /* 9 bits of 1.5 x 16 clocks (100 ns) = 21.6 us. */
    snprintf(message, sizeof message, "9 bits with DLF 32/64: %llu ns, want about 21600",
             (unsigned long long)((t1 - t0) / 1000));
    check(t1 - t0 > NS(21000) && t1 - t0 < NS(22200), message);
    /* TLR x 8: RX nibble 1 -> trigger 8. */
    bus_write(b, 4, 0x40);                 /* MCR[6]: TCR/TLR */
    bus_write(b, 7, 0x10);
    expect_reg(b, 7, "00010000", "TLR reads back at 7 with MCR[6]");
    case_end("D4 TL16C750E: LCR 0x1D, DLF fraction n/64, TLR access");
    bench_close(b);
}

/* ---- D5 TL16C754B: 752 bank, TCR auto-RTS, Xon/Xoff, INTSEL, CLKSEL --------------------- */

static void case_d5(void) {
    Bench *b = bench_open_cfg("SFX-TL16C754B", NS(50), &QUAD_CD_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    expect_reg(b, 4, "00000000", "CLKSEL = 1: MCR[7] = 0 after reset");
    expect_pin(b, "INTA", LOGIC_Z, "INTSEL = 0, MCR[3] = 0: INTA 3-state");
    set(b, "INTSEL", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "INTA", LOGIC_0, "INTSEL = 1: INTA driven");
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x50);                 /* EFR: enhanced + auto-RTS */
    bus_write(b, 4, 0x01);                 /* Xon1 = 0x01 (bank address 4) */
    bus_write(b, 6, 0x13);                 /* Xoff1 = 0x13 */
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x42);                 /* MCR[6] (TCR/TLR) + RTS */
    bus_write(b, 6, 0x12);                 /* TCR: halt 2 x 4 = 8, resume 1 x 4 = 4 */
    bus_write(b, 4, 0x02);
    bus_write(b, 3, 0x83);
    bus_write(b, 0, 0x01);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 2, 0x01);
    bt = bit_time(b, 1);
    run(b, NS(500));
    expect_pin(b, "RTSA", LOGIC_0, "auto-RTS active below the halt level");
    for (i = 0; i < 8; i++) {
        drive_named(b, "RXA", 0x30 + i, bt);
    }
    run(b, bt * 2);
    expect_pin(b, "RTSA", LOGIC_1, "8 bytes reach TCR halt 8: RTS inactive");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    run(b, bt);
    expect_pin(b, "RTSA", LOGIC_0, "down to TCR resume 4: RTS active again");
    for (i = 0; i < 4; i++) {
        bus_read(b, 0);
    }
    /* Xoff1 received halts TX (EFR[1:0] = 10); Xon1 resumes. */
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x12);
    bus_write(b, 3, 0x03);
    drive_named(b, "RXA", 0x13, bt);
    run(b, bt);
    bus_write(b, 0, 0x77);
    check(!wait_level(b, "TXA", LOGIC_0, bt * 12), "after Xoff1 nothing is sent");
    expect_reg(b, 5, "00000000", "the Xoff is not stored (DR 0); the byte waits (THRE, TEMT 0)");
    drive_named(b, "RXA", 0x01, bt);
    check(wait_level(b, "TXA", LOGIC_0, bt * 12), "Xon1 resumes the transmitter");
    case_end("D5 TL16C754B: TCR auto-RTS, Xon/Xoff, INTSEL, CLKSEL, 752 bank");
    bench_close(b);
}

/* ---- D6 TL16C754C: trigger table [1,4,56,60]; Xon power-up values ------------------------ */

static void case_d6(void) {
    Bench *b = bench_open_cfg("SFX-TL16C754C", NS(50), &QUAD_C_BENCH);
    SimTime bt;

    case_begin();
    bus_write(b, 3, 0xBF);
    expect_reg(b, 4, "11111111", "Xon1 powers up 0xFF");
    expect_reg(b, 5, "11101111", "Xon2 powers up 0xEF");
    expect_reg(b, 7, "11111111", "Xoff2 powers up 0xFF");
    bus_write(b, 3, 0x83);
    bus_write(b, 0, 0x01);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    expect_reg(b, 7, "11111111", "SPR powers up 0xFF");
    bus_write(b, 2, 0x41);                 /* trigger 01 = 4 */
    bus_write(b, 1, 0x01);
    bus_write(b, 4, 0x08);                 /* MCR[3]: INTA enabled */
    bt = bit_time(b, 1);
    drive_named(b, "RXA", 1, bt);
    drive_named(b, "RXA", 2, bt);
    drive_named(b, "RXA", 3, bt);
    run(b, bt);
    expect_pin(b, "INTA", LOGIC_0, "3 bytes, trigger 4: no interrupt");
    drive_named(b, "RXA", 4, bt);
    run(b, bt);
    expect_pin(b, "INTA", LOGIC_1, "4 bytes: RX data interrupt");
    case_end("D6 TL16C754C: rxTrigger [1,4,56,60]; Xon/SPR power-up values");
    bench_close(b);
}

/* ---- D7 TL16C752C: AFR CONC, IrDA pulses, RS-485 DTR ---------------------------------------- */

static void case_d7(void) {
    Bench *b = bench_open_cfg("SFX-TL16C752C", NS(50), &DUAL_CD_BENCH);
    SimTime bt;
    SimTime t0;
    SimTime t1;
    char message[96];

    case_begin();
    /* CONC: a write to channel A reaches B. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x01);                 /* LCR[7:5] = 100: AFR, CONC */
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x01);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 7, 0x5A);
    select_unit(b, 1);
    expect_reg(b, 7, "01011010", "CONC: channel B got the SPR write");
    expect_reg(b, 3, "00000011", "and the LCR write");
    select_unit(b, 0);
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x00);
    bus_write(b, 3, 0x03);
    bt = bit_time(b, 1);
    /* IrDA (AFR[1]): a 0 bit is a 3/16 pulse; idle low. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x12);                 /* RCVEN + IREN */
    bus_write(b, 3, 0x03);
    expect_pin(b, "TXA", LOGIC_0, "IrDA: TX idles low");
    bus_write(b, 0, 0xFE);                 /* start 0, bit0 0 */
    check(wait_level(b, "TXA", LOGIC_1, bt * 4), "a light pulse for the start bit");
    t0 = sim_now(b->sim);
    while (get(b, "TXA") == LOGIC_1) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    snprintf(message, sizeof message, "pulse %llu ns, want 3/16 bit (%llu)",
             (unsigned long long)((t1 - t0) / 1000), (unsigned long long)(bt * 3 / 16 / 1000));
    check(t1 - t0 + NS(250) >= bt * 3 / 16 && t1 - t0 <= bt * 3 / 16 + NS(250), message);
    run(b, bt * 12);
    /* RS-485 (AFR[2]) with DLY 3: DTR high while sending and 3 bits after. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x74);                 /* DLY 3, RCVEN, 485EN */
    bus_write(b, 3, 0x03);
    expect_pin(b, "DTRA", LOGIC_0, "RS-485 idle: DTR low");
    bus_write(b, 0, 0x00);
    check(wait_level(b, "DTRA", LOGIC_1, bt * 2), "DTR rises before the start bit");
    check(wait_level(b, "TXA", LOGIC_0, bt * 3), "then the start bit");
    t0 = sim_now(b->sim);
    check(wait_level(b, "DTRA", LOGIC_0, bt * 20), "DTR falls after the frame");
    t1 = sim_now(b->sim);
    snprintf(message, sizeof message, "DTR low %llu bit times after the start, want 10 + 3",
             (unsigned long long)((t1 - t0) / bt));
    check(t1 - t0 >= bt * 12 && t1 - t0 <= bt * 14, message);
    case_end("D7 TL16C752C: AFR CONC, IrDA 3/16 pulses, RS-485 DTR with DLY 3");
    bench_close(b);
}

/* ---- D8 TL16C554: four channels, shared AND DMA pins, INTN --------------------------------- */

static void case_d8_d9(const char *part, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, &QUAD_BENCH);
    unsigned u;
    char text[9];

    case_begin();
    for (u = 0; u < 4; u++) {
        select_unit(b, u);
        bus_write(b, 7, 0x10 + u);
    }
    for (u = 0; u < 4; u++) {
        select_unit(b, u);
        bus_read_text(b, 7, text);
        check(strtol(text, NULL, 2) == (long)(0x10 + u), "each channel's own SPR");
    }
    expect_pin(b, "TXRDY", LOGIC_0, "shared TXRDY: low (all empty)");
    select_unit(b, 2);
    bus_write(b, 0, 0x55);
    expect_pin(b, "TXRDY", LOGIC_0, "one channel loaded: the AND stays low (others empty)");
    /* INTN low: MCR[3] gates; INTN high: always driven. */
    expect_pin(b, "INTA", LOGIC_Z, "INTN low, MCR[3] = 0: INTA 3-state");
    set(b, "INTN", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "INTA", LOGIC_0, "INTN high: INTA driven");
    case_end(name);
    bench_close(b);
}

/* ---- D10/D11 TL16C552(A): two channels + printer port --------------------------------------- */

static void case_d10_d11(const char *part, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, &TI552_BENCH);
    char text[9];

    case_begin();
    expect_reg(b, 5, "01100000", "RESET is active low: released, LSR 0x60");
    select_unit(b, 2);                     /* the printer port */
    bus_write(b, 0, 0xA5);
    expect_pin(b, "PD0", LOGIC_1, "PEMD low: PD drives the latch");
    expect_pin(b, "PD1", LOGIC_0, "PD1");
    bus_write(b, 2, 0x01);                 /* STB asserted */
    expect_pin(b, "STB", LOGIC_0, "control bit 0 asserts STB low");
    bus_read_text(b, 1, text);
    check(text[0] == '1' && text[5] == '1' && text[6] == '1' && text[7] == '1',
          "status: BSY = NOT BUSY, PRINT = 1, b1:0 = 1");
    bus_write(b, 2, 0x10);                 /* INT2 EN, ENIRQ low: INT2 = ACK */
    expect_pin(b, "INT2", LOGIC_1, "INT2 follows ACK (high)");
    set(b, "ACK", LOGIC_0);
    run(b, NS(300));
    expect_pin(b, "INT2", LOGIC_0, "INT2 follows ACK (low)");
    set(b, "ACK", LOGIC_1);
    /* TRI floats every output. */
    set(b, "TRI", LOGIC_1);
    run(b, NS(300));
    expect_pin(b, "SOUT0", LOGIC_Z, "TRI high: SOUT0 floats");
    expect_pin(b, "PD0", LOGIC_Z, "TRI high: PD floats");
    set(b, "TRI", LOGIC_0);
    run(b, NS(300));
    expect_pin(b, "SOUT0", LOGIC_1, "TRI low: SOUT0 driven again");
    case_end(name);
    bench_close(b);
}

/* ---- D12/D13 TL16C451/452: 16450 channels, printer port, single CLK, OUT2 gating -------------- */

static void case_d12_d13(const char *part, const BenchConfig *cfg, const char *name) {
    Bench *b = bench_open_cfg(part, HALF_18432, cfg);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    expect_reg(b, 2, "00000001", "16450 channel: IIR 01");
    bus_write(b, 1, 0x02);                 /* ETBEI: THRE interrupt */
    expect_pin(b, "INT0", LOGIC_Z, "MCR[3] = 0: INT0 3-state");
    bus_write(b, 4, 0x08);
    run(b, NS(500));
    expect_pin(b, "INT0", LOGIC_1, "MCR[3] = 1: INT0 driven high (THRE)");
    bus_write(b, 0, 0x41);
    check(wait_level(b, "SOUT0", LOGIC_0, bt * 3), "the single CLK clocks the transmitter");
    /* Printer port: LPTOE low drives PD; high makes them inputs. */
    select_unit(b, cfg->units - 1);
    bus_write(b, 0, 0x3C);
    expect_pin(b, "PD2", LOGIC_1, "LPTOE low: PD drives the latch");
    set(b, "LPTOE", LOGIC_1);
    run(b, NS(100));
    expect_pin(b, "PD2", LOGIC_Z, "LPTOE high: the PD pins float");
    set(b, "PD2", LOGIC_0);
    run(b, NS(300));
    check((bus_read(b, 0) & 0x04) == 0, "LPTOE high: the data register reads the PD pins");
    case_end(name);
    bench_close(b);
}

/* ---- D14 TL16C450, D20 82C50A ----------------------------------------------------------------- */

static void case_d14(void) {
    Bench *b = bench_open_cfg("SFX-TL16C450", NS(50), &TI450_BENCH);
    SimTime t0;
    LogicValue last;

    case_begin();
    expect_pin(b, "INTRPT", LOGIC_0, "INTRPT driven low after MR (no OUT2 gating)");
    expect_pin(b, "CSOUT", LOGIC_1, "CSOUT high while selected");
    set(b, "CS0", LOGIC_0);
    run(b, NS(200));
    expect_pin(b, "CSOUT", LOGIC_0, "CSOUT low when not selected");
    set(b, "CS0", LOGIC_1);
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x05);
    bus_write(b, 1, 0x00);
    /* BAUDOUT low for 2 clocks, high for 3 (divisor 5). */
    last = get(b, "BAUDOUT");
    while (!(last == LOGIC_1 && get(b, "BAUDOUT") == LOGIC_0)) {
        last = get(b, "BAUDOUT");
        run(b, NS(5));
    }
    t0 = sim_now(b->sim);
    while (get(b, "BAUDOUT") == LOGIC_0) {
        run(b, NS(5));
    }
    check(sim_now(b->sim) - t0 >= NS(195) && sim_now(b->sim) - t0 <= NS(210),
          "BAUDOUT low for 2 XTAL1 cycles");
    expect_reg(b, 2, "00000001", "16450: no FIFO bits");
    case_end("D14 TL16C450: driven INTRPT, CSOUT, BAUDOUT low for 2 clocks, no FIFO");
    bench_close(b);
}

static void case_d20(void) {
    Bench *b = bench_open_cfg("SFX-82C50A", HALF_18432, &TI450_BENCH);
    SimTime bt;
    char bits[16];

    case_begin();
    program(b, 1, 0x07);                   /* 8 bits, 2 stops */
    bt = bit_time(b, 1);
    /* Second stop bit 0: a framing error on the 82C50A (both stops checked). */
    strcpy(bits, "0100000001");
    drive_bits(b, bits, bt);
    set(b, "SIN", LOGIC_0);
    run(b, bt);
    set(b, "SIN", LOGIC_1);
    run(b, bt * 3);
    expect_reg(b, 5, "01101001", "82C50A: a 0 second stop bit is a framing error");
    bus_read(b, 0);
    expect_pin(b, "INTRPT", LOGIC_0, "INTRPT driven (no OUT2 gating)");
    case_end("D20 82C50A: both stop bits checked, deferred status, driven INTRPT");
    bench_close(b);
}

/* ---- D15 TL16C550B: DMA class B ---------------------------------------------------------------- */

static void case_d15(void) {
    Bench *b = bench_open_cfg("SFX-TL16C550B", HALF_18432, &TI550_BENCH);
    unsigned i;
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x09);                 /* FIFO + DMA mode 1 */
    /* The first byte moves straight to the shifter: 17 writes fill the FIFO. */
    for (i = 0; i < 17; i++) {
        bus_write(b, 0, 0x30 + i);
    }
    run(b, NS(300));
    expect_pin(b, "TXRDY", LOGIC_1, "mode 1: TXRDY high when the FIFO is full");
    check(wait_level(b, "TXRDY", LOGIC_0, bt * 12), "TXRDY low again once one slot frees");
    case_end("D15 TL16C550B: DMA class B (TXRDY back as soon as a slot is free)");
    bench_close(b);
}

/* ---- D16 TL16C2550: per-channel chip selects --------------------------------------------------- */

static void case_d16(void) {
    Bench *b = bench_open_cfg("SFX-TL16C2550", HALF_18432, &DUAL_CD_BENCH);
    char text[9];

    case_begin();
    bus_write(b, 7, 0xA1);
    select_unit(b, 1);
    bus_write(b, 7, 0xB2);
    bus_read_text(b, 7, text);
    check(strcmp(text, "10110010") == 0, "channel B's SPR");
    select_unit(b, 0);
    bus_read_text(b, 7, text);
    check(strcmp(text, "10100001") == 0, "channel A's SPR");
    /* Both CS low: no broadcast on this part: a read is X. */
    set(b, "CSA", LOGIC_0);
    set(b, "CSB", LOGIC_0);
    run(b, NS(100));
    bus_read_text(b, 7, text);
    check(strcmp(text, "XXXXXXXX") == 0, "both CS low: the read is X (P2)");
    case_end("D16 TL16C2550: per-channel chip selects");
    bench_close(b);
}

/* ---- D17 TL16C2552, D18 NS16C2552, D19 NS16C2752: channel pin, AFR, MF -------------------------- */

static void case_d17(void) {
    Bench *b = bench_open_cfg("SFX-TL16C2552", HALF_18432, &CHSEL_BENCH);
    char text[9];

    case_begin();
    expect_reg(b, 4, "00001000", "MCR resets to 0x08 (decisions.md): INT driven");
    bus_write(b, 7, 0x11);
    select_unit(b, 1);
    bus_write(b, 7, 0x22);
    select_unit(b, 0);
    bus_read_text(b, 7, text);
    check(strcmp(text, "00010001") == 0, "CHSEL = 1 is channel A");
    /* AFR (DLAB = 1, address 2): MF select 10 = RXRDY on MFA. */
    bus_write(b, 3, 0x80);
    bus_write(b, 2, 0x04);
    bus_write(b, 3, 0x03);
    run(b, NS(300));
    expect_pin(b, "MFA", LOGIC_1, "MFA = RXRDY (inactive, empty)");
    case_end("D17 TL16C2552: channelSelect, AFR MF select, MCR reset 0x08");
    bench_close(b);
}

static void case_d18_d19(const char *part, const char *name, unsigned trigger_index,
                         unsigned rts_off, unsigned drev) {
    Bench *b = bench_open_cfg(part, HALF_18432, &NS_BENCH);
    SimTime bt;
    unsigned i;
    char text[9];

    case_begin();
    expect_reg(b, 7, "11111111", "SCR resets to 0xFF");
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    bus_write(b, 1, 0x00);
    bus_read_text(b, 0, text);
    check(strtol(text, NULL, 2) == (long)drev, "DREV reads with DLL = DLM = 0");
    bus_write(b, 0, 0x01);
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x50);                 /* EFR: enhanced + auto-RTS */
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x02);                 /* MCR[1]: needed for auto-RTS */
    bus_write(b, 2, 0x01 | (trigger_index << 6));
    bt = bit_time(b, 1);
    run(b, NS(300));
    expect_pin(b, "RTS1", LOGIC_0, "auto-RTS active");
    for (i = 0; i + 1 < rts_off; i++) {
        drive_named(b, "SIN1", 0x40, bt);
    }
    run(b, bt);
    expect_pin(b, "RTS1", LOGIC_0, "one below the table's off level: still active");
    drive_named(b, "SIN1", 0x41, bt);
    run(b, bt);
    expect_pin(b, "RTS1", LOGIC_1, "the table's off level: RTS inactive");
    /* lsrClearOnGood: a framing error, the byte read out (the LSR not read),
       then a good byte loading clears FE without an LSR read. */
    bus_write(b, 2, 0x06);                 /* clear the FIFOs, then FIFOs off */
    bus_write(b, 2, 0x00);
    bus_read(b, 5);
    drive_bits_on(b, "SIN1", "0110000000", bt);   /* 0x03, stop 0 */
    set(b, "SIN1", LOGIC_1);
    run(b, bt * 3);
    bus_read(b, 0);
    drive_named(b, "SIN1", 0x55, bt);
    run(b, bt);
    check((bus_read(b, 5) & 0x1E) == 0, "a good byte loading cleared FE (NS rule)");
    case_end(name);
    bench_close(b);
}

/* ---- D21 COM92C451 ------------------------------------------------------------------------------- */

static void case_d21(void) {
    Bench *b = bench_open_cfg("SFX-COM92C451", NS(50), &COM92_BENCH);
    SimTime t0;
    SimTime t1;

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x00);
    bus_write(b, 1, 0x00);
    bus_write(b, 3, 0x03);
    bus_write(b, 0, 0x00);
    check(wait_start(b, NS(100000)), "divisor 0 still clocks (as 3)");
    t0 = sim_now(b->sim);
    while (get(b, "SOUT") == LOGIC_0) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    check(t1 - t0 > NS(43000) && t1 - t0 < NS(43500), "9 bits x 16 x 3 clocks of 100 ns = 43.2 us");
    /* PIA: control bit 4 enables IRQ0; ACK falling latches it; a status read clears. */
    select_unit(b, 1);
    expect_pin(b, "IRQ0", LOGIC_Z, "IRQ0 3-state while disabled");
    bus_write(b, 2, 0x10);
    expect_pin(b, "IRQ0", LOGIC_0, "enabled, no ACK yet");
    set(b, "ACK", LOGIC_0);
    run(b, NS(300));
    set(b, "ACK", LOGIC_1);
    run(b, NS(300));
    expect_pin(b, "IRQ0", LOGIC_1, "ACK falling latches IRQ0");
    bus_read(b, 1);
    run(b, NS(300));
    expect_pin(b, "IRQ0", LOGIC_0, "a status read clears IRQ0");
    expect_pin(b, "INIT", LOGIC_0, "control bit 2 = 0: INIT asserted low (reset state)");
    case_end("D21 COM92C451: divisor 0 = 3, PIA IRQ0, INIT after reset");
    bench_close(b);
}

/* ---- D22 SC16C650B, D23 SC16C752B ---------------------------------------------------------------- */

static void case_d22(void) {
    Bench *b = bench_open_cfg("SFX-SC16C650B", HALF_18432, &NXP650_BENCH);
    SimTime bt;
    unsigned i;

    case_begin();
    expect_reg(b, 7, "11111111", "SPR powers up 0xFF");
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x10);                 /* EFR[4] */
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x20);                 /* MCR[5]: INT open source */
    bus_write(b, 1, 0x01);
    run(b, NS(300));
    expect_pin(b, "INT", LOGIC_Z, "open source, no interrupt: INT floats");
    bus_write(b, 2, 0x01);                 /* FIFO, trigger 8 */
    for (i = 0; i < 8; i++) {
        drive_named(b, "RX", 0x30, bt);
    }
    run(b, bt);
    expect_pin(b, "INT", LOGIC_1, "trigger 8: INT driven high");
    /* LSR[7] clears on any LSR read (lsr7Clear: lsrRead). */
    drive_bits_on(b, "RX", "0100011000", bt);   /* 0x31 with stop 0 */
    set(b, "RX", LOGIC_1);
    run(b, bt * 3);
    check((bus_read(b, 5) & 0x80) != 0, "an errored byte: LSR[7]");
    check((bus_read(b, 5) & 0x80) == 0, "the LSR read cleared LSR[7]");
    /* DLAB hides the upper registers. */
    bus_write(b, 3, 0x83);
    expect_reg(b, 7, "XXXXXXXX", "LCR[7] = 1: SPR not reachable (X)");
    bus_write(b, 3, 0x03);
    case_end("D22 SC16C650B: 650 bank, open-source INT, LSR[7] on read, DLAB hides");
    bench_close(b);
}

static void case_d23(void) {
    Bench *b = bench_open_cfg("SFX-SC16C752B", NS(50), &DUAL_SYNC_BENCH);
    SimTime bt;

    case_begin();
    expect_reg(b, 3, "00011101", "LCR resets to 0x1D");
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 3, 0xBF);
    bus_write(b, 2, 0x10);
    bus_write(b, 3, 0x03);
    bus_write(b, 4, 0x44);                 /* MCR[6] TCR/TLR, MCR[2] FIFO Rdy */
    expect_reg(b, 7, "XXXXXXXX", "TLR and FIFO Rdy both qualify: X (decisions.md)");
    bus_write(b, 4, 0x04);
    bus_write(b, 2, 0x01);
    expect_reg(b, 7, "00000011", "FIFO Rdy: both TX FIFOs ready, RX empty");
    /* IrDA (MCR[6] is also TLR access; use MCR = 0x40 only with EFR[4]). */
    bus_write(b, 4, 0x40);
    run(b, NS(300));
    expect_pin(b, "TXA", LOGIC_0, "IrDA on: TX idles low");
    bus_write(b, 4, 0x00);
    (void)bt;
    case_end("D23 SC16C752B: LCR 0x1D, 752 decode X rule, FIFO Rdy, IrDA");
    bench_close(b);
}

/* ---- D24 PC16550D: C's cases on the shipped device ------------------------------------------------ */

static void run_d_cases(void) {
    case_d1();
    case_d2();
    case_d3();
    case_d4();
    case_d5();
    case_d6();
    case_d7();
    case_d8_d9("SFX-TL16C554", "D8 TL16C554: four channels, shared AND TXRDY, INTN both levels");
    case_d8_d9("SFX-TL16C554A", "D9 TL16C554A: the same on the AFE part");
    case_d10_d11("SFX-TL16C552", "D10 TL16C552: printer port (PEMD, STB, status, INT2), TRI");
    case_d10_d11("SFX-TL16C552A", "D11 TL16C552A: the same");
    case_d12_d13("SFX-TL16C451", &TI451_BENCH,
                 "D12 TL16C451: 16450 channel, single CLK, OUT2-gated INT, printer port");
    case_d12_d13("SFX-TL16C452", &TI45X_BENCH,
                 "D13 TL16C452: two 16450 channels, printer port with LPTOE");
    case_d14();
    case_d15();
    case_d16();
    case_d17();
    case_d18_d19("SFX-NS16C2552", "D18 NS16C2552: channel pin, DREV 0x31, RTS table (trigger 4: off at 8)",
                 1, 8, 0x31);
    case_d18_d19("SFX-NS16C2752", "D19 NS16C2752: DREV 0x21, RTS table (trigger 16: off at 56)",
                 1, 56, 0x21);
    case_d20();
    case_d21();
    case_d22();
    case_d23();
}
