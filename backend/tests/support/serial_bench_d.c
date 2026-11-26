/* serial_bench_d (split from the serial tests, plan-68-G). */
#include "serial_bench.h"
#include "serial_bench_d.h"

/* ---- Bench configurations ----------------------------------------------------------- */

const BenchConfig TI550_BENCH = {
    "RD1", "WR1", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS0=1", "CS1=1", "CS2=0", "ADS=0", "RD2=0", "WR2=0", "SIN=1", "CTS=1", "DSR=1", "DCD=1",
     "RI=1", NULL},
    "MR", 1, "XIN", "BAUDOUT", "RCLK", {{NULL}}, 0
};

/* TL16C550D bga24 (ZQS): no CS0/CS1/ADS/RD2/WR2 (tied inside), no RCLK
   (tied to BAUDOUT inside), no DSR/RI/DCD. */
const BenchConfig TI550_BGA_BENCH = {
    "RD1", "WR1", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS2=0", "SIN=1", "CTS=1", NULL},
    "MR", 1, "XIN", NULL, NULL, {{NULL}}, 0
};

const BenchConfig TI450_BENCH = {
    "DISTRBAR", "DOSTRBAR", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS0=1", "CS1=1", "CS2=0", "ADS=0", "DISTR=0", "DOSTR=0", "SIN=1", "CTS=1", "DSR=1", "DCD=1",
     "RI=1", NULL},
    "MR", 1, "XTAL1", "BAUDOUT", "RCLK", {{NULL}}, 0
};

const BenchConfig QUAD_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "DCDA=1", "DCDB=1", "DCDC=1", "DCDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     "INTN=0", NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4
};

const BenchConfig QUAD_CD_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "CDA=1", "CDB=1", "CDC=1", "CDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     "INTSEL=0", "CLKSEL=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4, 400
};

const BenchConfig QUAD_C_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "RXC=1", "RXD=1", "CTSA=1", "CTSB=1", "CTSC=1", "CTSD=1", "DSRA=1", "DSRB=1",
     "DSRC=1", "DSRD=1", "CDA=1", "CDB=1", "CDC=1", "CDD=1", "RIA=1", "RIB=1", "RIC=1", "RID=1",
     NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}, {"CSC=0"}, {"CSD=0"}}, 4
};

const BenchConfig DUAL_SYNC_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "CTSA=1", "CTSB=1", "DSRA=1", "DSRB=1", "CDA=1", "CDB=1", "RIA=1", "RIB=1",
     NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}}, 2, 400
};

const BenchConfig DUAL_CD_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"RXA=1", "RXB=1", "CTSA=1", "CTSB=1", "DSRA=1", "DSRB=1", "CDA=1", "CDB=1", "RIA=1", "RIB=1",
     NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CSA=0"}, {"CSB=0"}}, 2
};

const BenchConfig CHSEL_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "RXA=1", "RXB=1", "CTSA=1", "CTSB=1", "DSRA=1", "DSRB=1", "CDA=1", "CDB=1", "RIA=1",
     "RIB=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL,
    {{"CHSEL=1"}, {"CHSEL=0"}}, 2
};

const BenchConfig NS_BENCH = {
    "RD", "WR", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "SIN1=1", "SIN2=1", "CTS1=1", "CTS2=1", "DSR1=1", "DSR2=1", "DCD1=1", "DCD2=1",
     "RI1=1", "RI2=1", NULL},
    "MR", 1, "XIN", NULL, NULL,
    {{"CHSL=1"}, {"CHSL=0"}}, 2
};

const BenchConfig TI45X_BENCH = {
    "IOR", "IOW", {"DB0", "DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7"}, {"A0", "A1", "A2"}, 3,
    {"SIN0=1", "SIN1=1", "CTS0=1", "CTS1=1", "DSR0=1", "DSR1=1", "RLSD0=1", "RLSD1=1", "RI0=1",
     "RI1=1", "LPTOE=0", "ACK=1", "BUSY=0", "PE=0", "SLCT=1", "ERROR=1", NULL},
    "RESET", 0, "CLK", NULL, NULL,
    {{"CS0=0"}, {"CS1=0"}, {"CS2=0"}}, 3
};

const BenchConfig TI451_BENCH = {
    "IOR", "IOW", {"DB0", "DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7"}, {"A0", "A1", "A2"}, 3,
    {"SIN0=1", "CTS0=1", "DSR0=1", "RLSD0=1", "RI0=1", "LPTOE=0", "ACK=1", "BUSY=0", "PE=0",
     "SLCT=1", "ERROR=1", NULL},
    "RESET", 0, "CLK", NULL, NULL,
    {{"CS0=0"}, {"CS2=0"}}, 2
};

const BenchConfig TI552_BENCH = {
    "IOR", "IOW", {"DB0", "DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7"}, {"A0", "A1", "A2"}, 3,
    {"SIN0=1", "SIN1=1", "CTS0=1", "CTS1=1", "DSR0=1", "DSR1=1", "DCD0=1", "DCD1=1", "RI0=1",
     "RI1=1", "PEMD=0", "ENIRQ=0", "TRI=0", "ACK=1", "BUSY=0", "PE=0", "SLCT=1", "ERR=1", NULL},
    "RESET", 0, "CLK", NULL, NULL,
    {{"CS0=0"}, {"CS1=0"}, {"CS2=0"}}, 3
};

const BenchConfig COM92_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"SIN=1", "CTS=1", "DSR=1", "DCD=1", "RI=1", "ACK=1", "BUSY=0", "PE=0", "SLCT=1", "ERROR=1",
     NULL},
    "RESET", 1, "CLK", NULL, NULL,
    {{"CE1=0"}, {"CE0=0"}}, 2
};

const BenchConfig NXP650_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "RX=1", "CTS=1", "DSR=1", NULL},
    "RESET", 1, "XTAL1", "BAUDOUT", "RCLK", {{NULL}}, 0
};

const BenchConfig TI750E_BENCH = {
    "IOR", "IOW", {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}, {"A0", "A1", "A2"}, 3,
    {"CS=0", "MODE=1", "RX=1", "CTS=1", "DSR=1", "CD=1", "RI=1", NULL},
    "RESET", 1, "XTAL1", NULL, NULL, {{NULL}}, 0
};

/* Programs the selected unit: divisor and LCR. */
void program_unit(Bench *b, unsigned unit, unsigned divisor, unsigned lcr) {
    select_unit(b, unit);
    program(b, divisor, lcr);
}

/* Drives bits ('0'/'1') on a named pin, one per bit time. */
void drive_bits_on(Bench *b, const char *pin, const char *bits, SimTime bt) {
    size_t i;

    for (i = 0; bits[i] != '\0'; i++) {
        set(b, pin, bits[i] == '1' ? LOGIC_1 : LOGIC_0);
        run(b, bt);
    }
}

/* Drives a byte on a named RX pin at the bit time. */
void drive_named(Bench *b, const char *rx, unsigned v, SimTime bt) {
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

int part_exists(const char *name) {
    return part_library_find(library, name) != NULL;
}

/* Waits up to `limit` for a pin to reach a level; 1 when it did. */
int wait_level(Bench *b, const char *pin, LogicValue level, SimTime limit) {
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
