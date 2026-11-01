/* SRAM chip data, transcribed from the datasheets in datasheets/sram/. Included by sram.c. */

/* ---- AS6C6264: 8K x 8 low-power CMOS SRAM (datasheets/sram/AS6C6264.pdf) ---- */

/* AC characteristics, VCC = 2.7 V to 5.5 V (the only table, AS6C6264-55). */
static const SramTiming AS6C6264_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .toh = NS(10),
    .tclz = NS(10), .tolz = NS(5), .tchz = NS(20), .tohz = NS(20),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tas = NS(0), .twp = NS(45),
    .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20), .tow = NS(5),
};

/* 28-pin PDIP (600 mil) and SOP (330 mil). */
#define AS6C6264_PINS(X) \
    X(1, "NC", NC, 0) \
    X(2, "A12", A, 12) \
    X(3, "A7", A, 7) \
    X(4, "A6", A, 6) \
    X(5, "A5", A, 5) \
    X(6, "A4", A, 4) \
    X(7, "A3", A, 3) \
    X(8, "A2", A, 2) \
    X(9, "A1", A, 1) \
    X(10, "A0", A, 0) \
    X(11, "DQ0", DQ, 0) \
    X(12, "DQ1", DQ, 1) \
    X(13, "DQ2", DQ, 2) \
    X(14, "Vss", GND, 0) \
    X(15, "DQ3", DQ, 3) \
    X(16, "DQ4", DQ, 4) \
    X(17, "DQ5", DQ, 5) \
    X(18, "DQ6", DQ, 6) \
    X(19, "DQ7", DQ, 7) \
    X(20, "CE#", CE_N, 0) \
    X(21, "A10", A, 10) \
    X(22, "OE#", OE_N, 0) \
    X(23, "A11", A, 11) \
    X(24, "A9", A, 9) \
    X(25, "A8", A, 8) \
    X(26, "CE2", CE, 0) \
    X(27, "WE#", WE_N, 0) \
    X(28, "Vcc", VCC, 0)

#define AS6C6264_PARTS(X) \
    X("AS6C6264-55PCN", AS6C6264_55, PACKAGE_TYPE_DIP, 0) \
    X("AS6C6264-55PIN", AS6C6264_55, PACKAGE_TYPE_DIP, 0) \
    X("AS6C6264-55SCN", AS6C6264_55, PACKAGE_TYPE_SOP, 0) \
    X("AS6C6264-55SIN", AS6C6264_55, PACKAGE_TYPE_SOP, 0)

SRAM_DEFINE_CHIP(AS6C6264, "AS6C6264", "8 KB", "8K x 8 low-power CMOS SRAM",
                 13, 8, AS6C6264_PINS, AS6C6264_PARTS)

/* 28-pin sTSOP (8 x 13.4 mm). */
#define AS6C6264_STSOP_PINS(X) \
    X(1, "OE#", OE_N, 0) \
    X(2, "A11", A, 11) \
    X(3, "A9", A, 9) \
    X(4, "A8", A, 8) \
    X(5, "CE2", CE, 0) \
    X(6, "WE#", WE_N, 0) \
    X(7, "Vcc", VCC, 0) \
    X(8, "NC", NC, 0) \
    X(9, "A12", A, 12) \
    X(10, "A7", A, 7) \
    X(11, "A6", A, 6) \
    X(12, "A5", A, 5) \
    X(13, "A4", A, 4) \
    X(14, "A3", A, 3) \
    X(15, "A2", A, 2) \
    X(16, "A1", A, 1) \
    X(17, "A0", A, 0) \
    X(18, "DQ0", DQ, 0) \
    X(19, "DQ1", DQ, 1) \
    X(20, "DQ2", DQ, 2) \
    X(21, "Vss", GND, 0) \
    X(22, "DQ3", DQ, 3) \
    X(23, "DQ4", DQ, 4) \
    X(24, "DQ5", DQ, 5) \
    X(25, "DQ6", DQ, 6) \
    X(26, "DQ7", DQ, 7) \
    X(27, "CE#", CE_N, 0) \
    X(28, "A10", A, 10)

#define AS6C6264_STSOP_PARTS(X) \
    X("AS6C6264-55STCN", AS6C6264_55, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS6C6264-55STIN", AS6C6264_55, PACKAGE_TYPE_STSOP_I, 0)

SRAM_DEFINE_CHIP(AS6C6264_STSOP, "AS6C6264_STSOP", "8 KB", "8K x 8 low-power CMOS SRAM",
                 13, 8, AS6C6264_STSOP_PINS, AS6C6264_STSOP_PARTS)

/* ---- AS6C62256: 32K x 8 low-power CMOS SRAM (datasheets/sram/AS6C62256.pdf) ---- */

/* AC characteristics, VCC = 2.7 V to 5.5 V (the only table). */
static const SramTiming AS6C62256_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .toh = NS(10),
    .tclz = NS(10), .tolz = NS(5), .tchz = NS(20), .tohz = NS(20),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tas = NS(0), .twp = NS(45),
    .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20), .tow = NS(5),
};

/* 28-pin PDIP and SOP. */
#define AS6C62256_PINS(X) \
    X(1, "A14", A, 14) \
    X(2, "A12", A, 12) \
    X(3, "A7", A, 7) \
    X(4, "A6", A, 6) \
    X(5, "A5", A, 5) \
    X(6, "A4", A, 4) \
    X(7, "A3", A, 3) \
    X(8, "A2", A, 2) \
    X(9, "A1", A, 1) \
    X(10, "A0", A, 0) \
    X(11, "DQ0", DQ, 0) \
    X(12, "DQ1", DQ, 1) \
    X(13, "DQ2", DQ, 2) \
    X(14, "VSS", GND, 0) \
    X(15, "DQ3", DQ, 3) \
    X(16, "DQ4", DQ, 4) \
    X(17, "DQ5", DQ, 5) \
    X(18, "DQ6", DQ, 6) \
    X(19, "DQ7", DQ, 7) \
    X(20, "CE#", CE_N, 0) \
    X(21, "A10", A, 10) \
    X(22, "OE#", OE_N, 0) \
    X(23, "A11", A, 11) \
    X(24, "A9", A, 9) \
    X(25, "A8", A, 8) \
    X(26, "A13", A, 13) \
    X(27, "WE#", WE_N, 0) \
    X(28, "VCC", VCC, 0)

#define AS6C62256_PARTS(X) \
    X("AS6C62256-55PCN", AS6C62256_55, PACKAGE_TYPE_DIP, 0) \
    X("AS6C62256-55SCN", AS6C62256_55, PACKAGE_TYPE_SOP, 0) \
    X("AS6C62256-55SIN", AS6C62256_55, PACKAGE_TYPE_SOP, 0)

SRAM_DEFINE_CHIP(AS6C62256, "AS6C62256", "32 KB", "32K x 8 low-power CMOS SRAM",
                 15, 8, AS6C62256_PINS, AS6C62256_PARTS)

/* 28-pin sTSOP (8 x 13.4 mm). */
#define AS6C62256_STSOP_PINS(X) \
    X(1, "OE#", OE_N, 0) \
    X(2, "A11", A, 11) \
    X(3, "A9", A, 9) \
    X(4, "A8", A, 8) \
    X(5, "A13", A, 13) \
    X(6, "WE#", WE_N, 0) \
    X(7, "VCC", VCC, 0) \
    X(8, "A14", A, 14) \
    X(9, "A12", A, 12) \
    X(10, "A7", A, 7) \
    X(11, "A6", A, 6) \
    X(12, "A5", A, 5) \
    X(13, "A4", A, 4) \
    X(14, "A3", A, 3) \
    X(15, "A2", A, 2) \
    X(16, "A1", A, 1) \
    X(17, "A0", A, 0) \
    X(18, "DQ0", DQ, 0) \
    X(19, "DQ1", DQ, 1) \
    X(20, "DQ2", DQ, 2) \
    X(21, "VSS", GND, 0) \
    X(22, "DQ3", DQ, 3) \
    X(23, "DQ4", DQ, 4) \
    X(24, "DQ5", DQ, 5) \
    X(25, "DQ6", DQ, 6) \
    X(26, "DQ7", DQ, 7) \
    X(27, "CE#", CE_N, 0) \
    X(28, "A10", A, 10)

#define AS6C62256_STSOP_PARTS(X) \
    X("AS6C62256-55STCN", AS6C62256_55, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS6C62256-55STIN", AS6C62256_55, PACKAGE_TYPE_STSOP_I, 0)

SRAM_DEFINE_CHIP(AS6C62256_STSOP, "AS6C62256_STSOP", "32 KB", "32K x 8 low-power CMOS SRAM",
                 15, 8, AS6C62256_STSOP_PINS, AS6C62256_STSOP_PARTS)

/* ---- AS6C1008: 128K x 8 low-power CMOS SRAM (datasheets/sram/AS6C1008.pdf) ---- */

/* AC characteristics, VCC = 2.7 V to 5.5 V (the only table, AS6C1008-55). */
static const SramTiming AS6C1008_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .toh = NS(10),
    .tclz = NS(10), .tolz = NS(5), .tchz = NS(20), .tohz = NS(20),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tas = NS(0), .twp = NS(45),
    .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20), .tow = NS(5),
};

/* 32-pin SOP (450 mil) and P-DIP (600 mil). */
#define AS6C1008_PINS(X) \
    X(1, "NC", NC, 0) \
    X(2, "A16", A, 16) \
    X(3, "A14", A, 14) \
    X(4, "A12", A, 12) \
    X(5, "A7", A, 7) \
    X(6, "A6", A, 6) \
    X(7, "A5", A, 5) \
    X(8, "A4", A, 4) \
    X(9, "A3", A, 3) \
    X(10, "A2", A, 2) \
    X(11, "A1", A, 1) \
    X(12, "A0", A, 0) \
    X(13, "DQ0", DQ, 0) \
    X(14, "DQ1", DQ, 1) \
    X(15, "DQ2", DQ, 2) \
    X(16, "Vss", GND, 0) \
    X(17, "DQ3", DQ, 3) \
    X(18, "DQ4", DQ, 4) \
    X(19, "DQ5", DQ, 5) \
    X(20, "DQ6", DQ, 6) \
    X(21, "DQ7", DQ, 7) \
    X(22, "CE#", CE_N, 0) \
    X(23, "A10", A, 10) \
    X(24, "OE#", OE_N, 0) \
    X(25, "A11", A, 11) \
    X(26, "A9", A, 9) \
    X(27, "A8", A, 8) \
    X(28, "A13", A, 13) \
    X(29, "WE#", WE_N, 0) \
    X(30, "CE2", CE, 0) \
    X(31, "A15", A, 15) \
    X(32, "Vcc", VCC, 0)

#define AS6C1008_PARTS(X) \
    X("AS6C1008-55PCN", AS6C1008_55, PACKAGE_TYPE_DIP, 0) \
    X("AS6C1008-55PIN", AS6C1008_55, PACKAGE_TYPE_DIP, 0) \
    X("AS6C1008-55SIN", AS6C1008_55, PACKAGE_TYPE_SOP, 0)

SRAM_DEFINE_CHIP(AS6C1008, "AS6C1008", "128 KB", "128K x 8 low-power CMOS SRAM",
                 17, 8, AS6C1008_PINS, AS6C1008_PARTS)

/* 32-pin TSOP-I (8 x 20 mm) and sTSOP (8 x 13.4 mm): same pinout. */
#define AS6C1008_TSOP_PINS(X) \
    X(1, "A11", A, 11) \
    X(2, "A9", A, 9) \
    X(3, "A8", A, 8) \
    X(4, "A13", A, 13) \
    X(5, "WE#", WE_N, 0) \
    X(6, "CE2", CE, 0) \
    X(7, "A15", A, 15) \
    X(8, "Vcc", VCC, 0) \
    X(9, "NC", NC, 0) \
    X(10, "A16", A, 16) \
    X(11, "A14", A, 14) \
    X(12, "A12", A, 12) \
    X(13, "A7", A, 7) \
    X(14, "A6", A, 6) \
    X(15, "A5", A, 5) \
    X(16, "A4", A, 4) \
    X(17, "A3", A, 3) \
    X(18, "A2", A, 2) \
    X(19, "A1", A, 1) \
    X(20, "A0", A, 0) \
    X(21, "DQ0", DQ, 0) \
    X(22, "DQ1", DQ, 1) \
    X(23, "DQ2", DQ, 2) \
    X(24, "Vss", GND, 0) \
    X(25, "DQ3", DQ, 3) \
    X(26, "DQ4", DQ, 4) \
    X(27, "DQ5", DQ, 5) \
    X(28, "DQ6", DQ, 6) \
    X(29, "DQ7", DQ, 7) \
    X(30, "CE#", CE_N, 0) \
    X(31, "A10", A, 10) \
    X(32, "OE#", OE_N, 0)

#define AS6C1008_TSOP_PARTS(X) \
    X("AS6C1008-55TIN", AS6C1008_55, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS6C1008-55STIN", AS6C1008_55, PACKAGE_TYPE_STSOP_I, 0)

SRAM_DEFINE_CHIP(AS6C1008_TSOP, "AS6C1008_TSOP", "128 KB", "128K x 8 low-power CMOS SRAM",
                 17, 8, AS6C1008_TSOP_PINS, AS6C1008_TSOP_PARTS)

/* 36-ball TFBGA (6 x 8 mm), populated balls numbered row-major A1..H6. */
#define AS6C1008_BGA_PINS(X) \
    X(1, "A0", A, 0) /* A1 */ \
    X(2, "A1", A, 1) /* A2 */ \
    X(3, "CE2", CE, 0) /* A3 */ \
    X(4, "A3", A, 3) /* A4 */ \
    X(5, "A6", A, 6) /* A5 */ \
    X(6, "A8", A, 8) /* A6 */ \
    X(7, "DQ4", DQ, 4) /* B1 */ \
    X(8, "A2", A, 2) /* B2 */ \
    X(9, "WE#", WE_N, 0) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "A7", A, 7) /* B5 */ \
    X(12, "DQ0", DQ, 0) /* B6 */ \
    X(13, "DQ5", DQ, 5) /* C1 */ \
    X(14, "NC", NC, 0) /* C3 */ \
    X(15, "A5", A, 5) /* C4 */ \
    X(16, "DQ1", DQ, 1) /* C6 */ \
    X(17, "Vss", GND, 0) /* D1 */ \
    X(18, "Vcc", VCC, 0) /* D6 */ \
    X(19, "Vcc", VCC, 0) /* E1 */ \
    X(20, "Vss", GND, 0) /* E6 */ \
    X(21, "DQ6", DQ, 6) /* F1 */ \
    X(22, "NC", NC, 0) /* F3 */ \
    X(23, "NC", NC, 0) /* F4 */ \
    X(24, "DQ2", DQ, 2) /* F6 */ \
    X(25, "DQ7", DQ, 7) /* G1 */ \
    X(26, "OE#", OE_N, 0) /* G2 */ \
    X(27, "CE#", CE_N, 0) /* G3 */ \
    X(28, "A16", A, 16) /* G4 */ \
    X(29, "A15", A, 15) /* G5 */ \
    X(30, "DQ3", DQ, 3) /* G6 */ \
    X(31, "A9", A, 9) /* H1 */ \
    X(32, "A10", A, 10) /* H2 */ \
    X(33, "A11", A, 11) /* H3 */ \
    X(34, "A12", A, 12) /* H4 */ \
    X(35, "A13", A, 13) /* H5 */ \
    X(36, "A14", A, 14) /* H6 */

#define AS6C1008_BGA_PARTS(X) \
    X("AS6C1008-55BIN", AS6C1008_55, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS6C1008_BGA, "AS6C1008_BGA", "128 KB", "128K x 8 low-power CMOS SRAM",
                 17, 8, AS6C1008_BGA_PINS, AS6C1008_BGA_PARTS)

/* ---- AS6C4008: 512K x 8 low-power CMOS SRAM (datasheets/sram/AS6C4008.pdf) ---- */

/* AC characteristics, VCC = 2.7 V to 5.5 V (the only table, AS6C4008-55;
   test load CL = 30 pF + 1 TTL). */
static const SramTiming AS6C4008_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .toh = NS(10),
    .tclz = NS(10), .tolz = NS(5), .tchz = NS(20), .tohz = NS(20),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tas = NS(0), .twp = NS(45),
    .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20), .tow = NS(5),
};

/* 32-pin SOP (450 mil), P-DIP (600 mil) and TSOP-II (400 mil): identical pinout.
   Note: the TSOP-II drawing spells the supplies "VCC"/"VSS"; the SOP/P-DIP
   spelling "Vcc"/"Vss" is used here. */
#define AS6C4008_PINS(X) \
    X(1, "A18", A, 18) \
    X(2, "A16", A, 16) \
    X(3, "A14", A, 14) \
    X(4, "A12", A, 12) \
    X(5, "A7", A, 7) \
    X(6, "A6", A, 6) \
    X(7, "A5", A, 5) \
    X(8, "A4", A, 4) \
    X(9, "A3", A, 3) \
    X(10, "A2", A, 2) \
    X(11, "A1", A, 1) \
    X(12, "A0", A, 0) \
    X(13, "DQ0", DQ, 0) \
    X(14, "DQ1", DQ, 1) \
    X(15, "DQ2", DQ, 2) \
    X(16, "Vss", GND, 0) \
    X(17, "DQ3", DQ, 3) \
    X(18, "DQ4", DQ, 4) \
    X(19, "DQ5", DQ, 5) \
    X(20, "DQ6", DQ, 6) \
    X(21, "DQ7", DQ, 7) \
    X(22, "CE#", CE_N, 0) \
    X(23, "A10", A, 10) \
    X(24, "OE#", OE_N, 0) \
    X(25, "A11", A, 11) \
    X(26, "A9", A, 9) \
    X(27, "A8", A, 8) \
    X(28, "A13", A, 13) \
    X(29, "WE#", WE_N, 0) \
    X(30, "A17", A, 17) \
    X(31, "A15", A, 15) \
    X(32, "Vcc", VCC, 0)

#define AS6C4008_PARTS(X) \
    X("AS6C4008-55PCN", AS6C4008_55, PACKAGE_TYPE_DIP, 0) \
    X("AS6C4008-55SIN", AS6C4008_55, PACKAGE_TYPE_SOP, 0) \
    X("AS6C4008-55ZIN", AS6C4008_55, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS6C4008, "AS6C4008", "512 KB", "512K x 8 low-power CMOS SRAM",
                 19, 8, AS6C4008_PINS, AS6C4008_PARTS)

/* 32-pin TSOP-I (8 x 20 mm) and sTSOP (8 x 13.4 mm): same pinout. */
#define AS6C4008_TSOP_PINS(X) \
    X(1, "A11", A, 11) \
    X(2, "A9", A, 9) \
    X(3, "A8", A, 8) \
    X(4, "A13", A, 13) \
    X(5, "WE#", WE_N, 0) \
    X(6, "A17", A, 17) \
    X(7, "A15", A, 15) \
    X(8, "Vcc", VCC, 0) \
    X(9, "A18", A, 18) \
    X(10, "A16", A, 16) \
    X(11, "A14", A, 14) \
    X(12, "A12", A, 12) \
    X(13, "A7", A, 7) \
    X(14, "A6", A, 6) \
    X(15, "A5", A, 5) \
    X(16, "A4", A, 4) \
    X(17, "A3", A, 3) \
    X(18, "A2", A, 2) \
    X(19, "A1", A, 1) \
    X(20, "A0", A, 0) \
    X(21, "DQ0", DQ, 0) \
    X(22, "DQ1", DQ, 1) \
    X(23, "DQ2", DQ, 2) \
    X(24, "Vss", GND, 0) \
    X(25, "DQ3", DQ, 3) \
    X(26, "DQ4", DQ, 4) \
    X(27, "DQ5", DQ, 5) \
    X(28, "DQ6", DQ, 6) \
    X(29, "DQ7", DQ, 7) \
    X(30, "CE#", CE_N, 0) \
    X(31, "A10", A, 10) \
    X(32, "OE#", OE_N, 0)

#define AS6C4008_TSOP_PARTS(X) \
    X("AS6C4008-55TIN", AS6C4008_55, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS6C4008-55STIN", AS6C4008_55, PACKAGE_TYPE_STSOP_I, 0)

SRAM_DEFINE_CHIP(AS6C4008_TSOP, "AS6C4008_TSOP", "512 KB", "512K x 8 low-power CMOS SRAM",
                 19, 8, AS6C4008_TSOP_PINS, AS6C4008_TSOP_PARTS)

/* 36-ball TFBGA (6 x 8 mm), populated balls numbered row-major A1..H6. */
#define AS6C4008_BGA_PINS(X) \
    X(1, "A0", A, 0) /* A1 */ \
    X(2, "A1", A, 1) /* A2 */ \
    X(3, "NC", NC, 0) /* A3 */ \
    X(4, "A3", A, 3) /* A4 */ \
    X(5, "A6", A, 6) /* A5 */ \
    X(6, "A8", A, 8) /* A6 */ \
    X(7, "DQ4", DQ, 4) /* B1 */ \
    X(8, "A2", A, 2) /* B2 */ \
    X(9, "WE#", WE_N, 0) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "A7", A, 7) /* B5 */ \
    X(12, "DQ0", DQ, 0) /* B6 */ \
    X(13, "DQ5", DQ, 5) /* C1 */ \
    X(14, "NC", NC, 0) /* C3 */ \
    X(15, "A5", A, 5) /* C4 */ \
    X(16, "DQ1", DQ, 1) /* C6 */ \
    X(17, "Vss", GND, 0) /* D1 */ \
    X(18, "Vcc", VCC, 0) /* D6 */ \
    X(19, "Vcc", VCC, 0) /* E1 */ \
    X(20, "Vss", GND, 0) /* E6 */ \
    X(21, "DQ6", DQ, 6) /* F1 */ \
    X(22, "A18", A, 18) /* F3 */ \
    X(23, "A17", A, 17) /* F4 */ \
    X(24, "DQ2", DQ, 2) /* F6 */ \
    X(25, "DQ7", DQ, 7) /* G1 */ \
    X(26, "OE#", OE_N, 0) /* G2 */ \
    X(27, "CE#", CE_N, 0) /* G3 */ \
    X(28, "A16", A, 16) /* G4 */ \
    X(29, "A15", A, 15) /* G5 */ \
    X(30, "DQ3", DQ, 3) /* G6 */ \
    X(31, "A9", A, 9) /* H1 */ \
    X(32, "A10", A, 10) /* H2 */ \
    X(33, "A11", A, 11) /* H3 */ \
    X(34, "A12", A, 12) /* H4 */ \
    X(35, "A13", A, 13) /* H5 */ \
    X(36, "A14", A, 14) /* H6 */

#define AS6C4008_BGA_PARTS(X) \
    X("AS6C4008-55BIN", AS6C4008_55, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS6C4008_BGA, "AS6C4008_BGA", "512 KB", "512K x 8 low-power CMOS SRAM",
                 19, 8, AS6C4008_BGA_PINS, AS6C4008_BGA_PARTS)

/* ---- AS6C8008: 1M x 8 low-power CMOS SRAM (datasheets/sram/AS6C8008.pdf) ---- */

/* AC electrical characteristics, AS6C8008-55, VCC = 2.7 V to 5.5 V (the only table). */
static const SramTiming AS6C8008_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .toh = NS(10),
    .tclz = NS(10), .tolz = NS(5), .tchz = NS(20), .tohz = NS(20),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tas = NS(0), .twp = NS(45),
    .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20), .tow = NS(5),
};

/* 44-pin 400 mil TSOP-II. */
#define AS6C8008_TSOP2_PINS(X) \
    X(1, "A4", A, 4) \
    X(2, "A3", A, 3) \
    X(3, "A2", A, 2) \
    X(4, "A1", A, 1) \
    X(5, "A0", A, 0) \
    X(6, "CE#", CE_N, 0) \
    X(7, "NC", NC, 0) \
    X(8, "NC", NC, 0) \
    X(9, "DQ0", DQ, 0) \
    X(10, "DQ1", DQ, 1) \
    X(11, "Vcc", VCC, 0) \
    X(12, "Vss", GND, 0) \
    X(13, "DQ2", DQ, 2) \
    X(14, "DQ3", DQ, 3) \
    X(15, "NC", NC, 0) \
    X(16, "NC", NC, 0) \
    X(17, "WE#", WE_N, 0) \
    X(18, "A19", A, 19) \
    X(19, "A18", A, 18) \
    X(20, "A17", A, 17) \
    X(21, "A16", A, 16) \
    X(22, "A15", A, 15) \
    X(23, "A14", A, 14) \
    X(24, "A13", A, 13) \
    X(25, "A12", A, 12) \
    X(26, "A11", A, 11) \
    X(27, "A10", A, 10) \
    X(28, "A9", A, 9) \
    X(29, "NC", NC, 0) \
    X(30, "NC", NC, 0) \
    X(31, "DQ4", DQ, 4) \
    X(32, "DQ5", DQ, 5) \
    X(33, "Vcc", VCC, 0) \
    X(34, "Vss", GND, 0) \
    X(35, "DQ6", DQ, 6) \
    X(36, "DQ7", DQ, 7) \
    X(37, "NC", NC, 0) \
    X(38, "NC", NC, 0) \
    X(39, "A8", A, 8) \
    X(40, "CE2", CE, 0) \
    X(41, "OE#", OE_N, 0) \
    X(42, "A7", A, 7) \
    X(43, "A6", A, 6) \
    X(44, "A5", A, 5)

/* Note: the part numbering system mentions a "TR" (tape and reel) packing
 * suffix, but the ordering table lists only the tray codes, so only those
 * are included. */
#define AS6C8008_TSOP2_PARTS(X) \
    X("AS6C8008-55ZIN", AS6C8008_55, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS6C8008_TSOP2, "AS6C8008_TSOP2", "1024 KB", "1M x 8 low-power CMOS SRAM",
                 20, 8, AS6C8008_TSOP2_PINS, AS6C8008_TSOP2_PARTS)

/* 48-ball 6 x 8 mm TFBGA (all 48 balls populated), numbered row-major A1..H6. */
#define AS6C8008_BGA_PINS(X) \
    X(1, "NC", NC, 0) /* A1 */ \
    X(2, "OE#", OE_N, 0) /* A2 */ \
    X(3, "A0", A, 0) /* A3 */ \
    X(4, "A1", A, 1) /* A4 */ \
    X(5, "A2", A, 2) /* A5 */ \
    X(6, "CE2", CE, 0) /* A6 */ \
    X(7, "NC", NC, 0) /* B1 */ \
    X(8, "NC", NC, 0) /* B2 */ \
    X(9, "A3", A, 3) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "CE#", CE_N, 0) /* B5 */ \
    X(12, "NC", NC, 0) /* B6 */ \
    X(13, "DQ0", DQ, 0) /* C1 */ \
    X(14, "NC", NC, 0) /* C2 */ \
    X(15, "A5", A, 5) /* C3 */ \
    X(16, "A6", A, 6) /* C4 */ \
    X(17, "NC", NC, 0) /* C5 */ \
    X(18, "DQ4", DQ, 4) /* C6 */ \
    X(19, "Vss", GND, 0) /* D1 */ \
    X(20, "DQ1", DQ, 1) /* D2 */ \
    X(21, "A17", A, 17) /* D3 */ \
    X(22, "A7", A, 7) /* D4 */ \
    X(23, "DQ5", DQ, 5) /* D5 */ \
    X(24, "Vcc", VCC, 0) /* D6 */ \
    X(25, "Vcc", VCC, 0) /* E1 */ \
    X(26, "DQ2", DQ, 2) /* E2 */ \
    X(27, "NC", NC, 0) /* E3 */ \
    X(28, "A16", A, 16) /* E4 */ \
    X(29, "DQ6", DQ, 6) /* E5 */ \
    X(30, "Vss", GND, 0) /* E6 */ \
    X(31, "DQ3", DQ, 3) /* F1 */ \
    X(32, "NC", NC, 0) /* F2 */ \
    X(33, "A14", A, 14) /* F3 */ \
    X(34, "A15", A, 15) /* F4 */ \
    X(35, "NC", NC, 0) /* F5 */ \
    X(36, "DQ7", DQ, 7) /* F6 */ \
    X(37, "NC", NC, 0) /* G1 */ \
    X(38, "NC", NC, 0) /* G2 */ \
    X(39, "A12", A, 12) /* G3 */ \
    X(40, "A13", A, 13) /* G4 */ \
    X(41, "WE#", WE_N, 0) /* G5 */ \
    X(42, "NC", NC, 0) /* G6 */ \
    X(43, "A18", A, 18) /* H1 */ \
    X(44, "A8", A, 8) /* H2 */ \
    X(45, "A9", A, 9) /* H3 */ \
    X(46, "A10", A, 10) /* H4 */ \
    X(47, "A11", A, 11) /* H5 */ \
    X(48, "A19", A, 19) /* H6 */

#define AS6C8008_BGA_PARTS(X) \
    X("AS6C8008-55BIN", AS6C8008_55, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS6C8008_BGA, "AS6C8008_BGA", "1024 KB", "1M x 8 low-power CMOS SRAM",
                 20, 8, AS6C8008_BGA_PINS, AS6C8008_BGA_PARTS)

/* ---- AS6C1608B: 2M x 8 low-power CMOS SRAM (datasheets/sram/AS6C1608B.pdf) ---- */

/* AC electrical characteristics, AS6C1608B-45 column, VCC = 2.7 V to 3.6 V
 * (the only VCC range).
 * Note: the table also has a "55ns Specification" column, but no 55 ns part
 * appears in the ordering information, so only the -45 grade is defined. */
static const SramTiming AS6C1608B_45 = {
    .trc = NS(45), .taa = NS(45), .tace = NS(45), .toe = NS(25), .toh = NS(10),
    .tclz = NS(10), .tolz = NS(5), .tchz = NS(15), .tohz = NS(15),
    .twc = NS(45), .taw = NS(40), .tcw = NS(40), .tas = NS(0), .twp = NS(35),
    .twr = NS(0), .tdw = NS(20), .tdh = NS(0), .twhz = NS(15), .tow = NS(5),
};

/* 44-pin 400 mil TSOP-II. */
#define AS6C1608B_TSOP2_PINS(X) \
    X(1, "A4", A, 4) \
    X(2, "A3", A, 3) \
    X(3, "A2", A, 2) \
    X(4, "A1", A, 1) \
    X(5, "A0", A, 0) \
    X(6, "CE#", CE_N, 0) \
    X(7, "NC", NC, 0) \
    X(8, "NC", NC, 0) \
    X(9, "DQ0", DQ, 0) \
    X(10, "DQ1", DQ, 1) \
    X(11, "Vcc", VCC, 0) \
    X(12, "Vss", GND, 0) \
    X(13, "DQ2", DQ, 2) \
    X(14, "DQ3", DQ, 3) \
    X(15, "NC", NC, 0) \
    X(16, "A20", A, 20) \
    X(17, "WE#", WE_N, 0) \
    X(18, "A19", A, 19) \
    X(19, "A18", A, 18) \
    X(20, "A17", A, 17) \
    X(21, "A16", A, 16) \
    X(22, "A15", A, 15) \
    X(23, "A14", A, 14) \
    X(24, "A13", A, 13) \
    X(25, "A12", A, 12) \
    X(26, "A11", A, 11) \
    X(27, "A10", A, 10) \
    X(28, "A9", A, 9) \
    X(29, "NC", NC, 0) \
    X(30, "NC", NC, 0) \
    X(31, "DQ4", DQ, 4) \
    X(32, "DQ5", DQ, 5) \
    X(33, "Vcc", VCC, 0) \
    X(34, "Vss", GND, 0) \
    X(35, "DQ6", DQ, 6) \
    X(36, "DQ7", DQ, 7) \
    X(37, "NC", NC, 0) \
    X(38, "NC", NC, 0) \
    X(39, "A8", A, 8) \
    X(40, "CE2", CE, 0) \
    X(41, "OE#", OE_N, 0) \
    X(42, "A7", A, 7) \
    X(43, "A6", A, 6) \
    X(44, "A5", A, 5)

#define AS6C1608B_TSOP2_PARTS(X) \
    X("AS6C1608B-45TIN", AS6C1608B_45, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS6C1608B-45TINTR", AS6C1608B_45, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS6C1608B_TSOP2, "AS6C1608B_TSOP2", "2048 KB", "2M x 8 low-power CMOS SRAM",
                 21, 8, AS6C1608B_TSOP2_PINS, AS6C1608B_TSOP2_PARTS)

/* 48-ball 6 x 8 mm TFBGA (all 48 balls populated), numbered row-major A1..H6. */
#define AS6C1608B_BGA_PINS(X) \
    X(1, "NC", NC, 0) /* A1 */ \
    X(2, "OE#", OE_N, 0) /* A2 */ \
    X(3, "A0", A, 0) /* A3 */ \
    X(4, "A1", A, 1) /* A4 */ \
    X(5, "A2", A, 2) /* A5 */ \
    X(6, "CE2", CE, 0) /* A6 */ \
    X(7, "NC", NC, 0) /* B1 */ \
    X(8, "NC", NC, 0) /* B2 */ \
    X(9, "A3", A, 3) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "CE#", CE_N, 0) /* B5 */ \
    X(12, "NC", NC, 0) /* B6 */ \
    X(13, "DQ0", DQ, 0) /* C1 */ \
    X(14, "NC", NC, 0) /* C2 */ \
    X(15, "A5", A, 5) /* C3 */ \
    X(16, "A6", A, 6) /* C4 */ \
    X(17, "NC", NC, 0) /* C5 */ \
    X(18, "DQ4", DQ, 4) /* C6 */ \
    X(19, "Vss", GND, 0) /* D1 */ \
    X(20, "DQ1", DQ, 1) /* D2 */ \
    X(21, "A17", A, 17) /* D3 */ \
    X(22, "A7", A, 7) /* D4 */ \
    X(23, "DQ5", DQ, 5) /* D5 */ \
    X(24, "Vcc", VCC, 0) /* D6 */ \
    X(25, "Vcc", VCC, 0) /* E1 */ \
    X(26, "DQ2", DQ, 2) /* E2 */ \
    X(27, "NC", NC, 0) /* E3 */ \
    X(28, "A16", A, 16) /* E4 */ \
    X(29, "DQ6", DQ, 6) /* E5 */ \
    X(30, "Vss", GND, 0) /* E6 */ \
    X(31, "DQ3", DQ, 3) /* F1 */ \
    X(32, "NC", NC, 0) /* F2 */ \
    X(33, "A14", A, 14) /* F3 */ \
    X(34, "A15", A, 15) /* F4 */ \
    X(35, "NC", NC, 0) /* F5 */ \
    X(36, "DQ7", DQ, 7) /* F6 */ \
    X(37, "NC", NC, 0) /* G1 */ \
    X(38, "A20", A, 20) /* G2 */ \
    X(39, "A12", A, 12) /* G3 */ \
    X(40, "A13", A, 13) /* G4 */ \
    X(41, "WE#", WE_N, 0) /* G5 */ \
    X(42, "NC", NC, 0) /* G6 */ \
    X(43, "A18", A, 18) /* H1 */ \
    X(44, "A8", A, 8) /* H2 */ \
    X(45, "A9", A, 9) /* H3 */ \
    X(46, "A10", A, 10) /* H4 */ \
    X(47, "A11", A, 11) /* H5 */ \
    X(48, "A19", A, 19) /* H6 */

#define AS6C1608B_BGA_PARTS(X) \
    X("AS6C1608B-45BIN", AS6C1608B_45, PACKAGE_TYPE_TFBGA, 0) \
    X("AS6C1608B-45BINTR", AS6C1608B_45, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS6C1608B_BGA, "AS6C1608B_BGA", "2048 KB", "2M x 8 low-power CMOS SRAM",
                 21, 8, AS6C1608B_BGA_PINS, AS6C1608B_BGA_PARTS)

/* ---- AS6C2016: 128K x 16 low-power CMOS SRAM (datasheets/sram/AS6C2016.pdf) ---- */

/* AC electrical characteristics, AS6C2016-55, VCC = 2.7 V to 5.5 V (the only table). */
static const SramTiming AS6C2016_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .tba = NS(55), .toh = NS(10),
    .tclz = NS(10), .tolz = NS(5), .tblz = NS(10), .tchz = NS(20), .tohz = NS(20), .tbhz = NS(25),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tbw = NS(50), .tas = NS(0), .twp = NS(45),
    .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20), .tow = NS(5),
};

/* 44-pin 400 mil TSOP-II. */
#define AS6C2016_TSOP2_PINS(X) \
    X(1, "A4", A, 4) \
    X(2, "A3", A, 3) \
    X(3, "A2", A, 2) \
    X(4, "A1", A, 1) \
    X(5, "A0", A, 0) \
    X(6, "CE#", CE_N, 0) \
    X(7, "DQ0", DQ, 0) \
    X(8, "DQ1", DQ, 1) \
    X(9, "DQ2", DQ, 2) \
    X(10, "DQ3", DQ, 3) \
    X(11, "Vcc", VCC, 0) \
    X(12, "Vss", GND, 0) \
    X(13, "DQ4", DQ, 4) \
    X(14, "DQ5", DQ, 5) \
    X(15, "DQ6", DQ, 6) \
    X(16, "DQ7", DQ, 7) \
    X(17, "WE#", WE_N, 0) \
    X(18, "A16", A, 16) \
    X(19, "A15", A, 15) \
    X(20, "A14", A, 14) \
    X(21, "A13", A, 13) \
    X(22, "A12", A, 12) \
    X(23, "NC", NC, 0) \
    X(24, "A11", A, 11) \
    X(25, "A10", A, 10) \
    X(26, "A9", A, 9) \
    X(27, "A8", A, 8) \
    X(28, "NC", NC, 0) \
    X(29, "DQ8", DQ, 8) \
    X(30, "DQ9", DQ, 9) \
    X(31, "DQ10", DQ, 10) \
    X(32, "DQ11", DQ, 11) \
    X(33, "Vcc", VCC, 0) \
    X(34, "Vss", GND, 0) \
    X(35, "DQ12", DQ, 12) \
    X(36, "DQ13", DQ, 13) \
    X(37, "DQ14", DQ, 14) \
    X(38, "DQ15", DQ, 15) \
    X(39, "LB#", BE_N, 0) \
    X(40, "UB#", BE_N, 1) \
    X(41, "OE#", OE_N, 0) \
    X(42, "A7", A, 7) \
    X(43, "A6", A, 6) \
    X(44, "A5", A, 5)

#define AS6C2016_TSOP2_PARTS(X) \
    X("AS6C2016-55ZIN", AS6C2016_55, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS6C2016_TSOP2, "AS6C2016_TSOP2", "256 KB", "128K x 16 low-power CMOS SRAM",
                 17, 16, AS6C2016_TSOP2_PINS, AS6C2016_TSOP2_PARTS)

/* 48-ball 6 x 8 mm TFBGA (all 48 balls populated), numbered row-major A1..H6. */
#define AS6C2016_BGA_PINS(X) \
    X(1, "LB#", BE_N, 0) /* A1 */ \
    X(2, "OE#", OE_N, 0) /* A2 */ \
    X(3, "A0", A, 0) /* A3 */ \
    X(4, "A1", A, 1) /* A4 */ \
    X(5, "A2", A, 2) /* A5 */ \
    X(6, "NC", NC, 0) /* A6 */ \
    X(7, "DQ8", DQ, 8) /* B1 */ \
    X(8, "UB#", BE_N, 1) /* B2 */ \
    X(9, "A3", A, 3) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "CE#", CE_N, 0) /* B5 */ \
    X(12, "DQ0", DQ, 0) /* B6 */ \
    X(13, "DQ9", DQ, 9) /* C1 */ \
    X(14, "DQ10", DQ, 10) /* C2 */ \
    X(15, "A5", A, 5) /* C3 */ \
    X(16, "A6", A, 6) /* C4 */ \
    X(17, "DQ1", DQ, 1) /* C5 */ \
    X(18, "DQ2", DQ, 2) /* C6 */ \
    X(19, "Vss", GND, 0) /* D1 */ \
    X(20, "DQ11", DQ, 11) /* D2 */ \
    X(21, "NC", NC, 0) /* D3 */ \
    X(22, "A7", A, 7) /* D4 */ \
    X(23, "DQ3", DQ, 3) /* D5 */ \
    X(24, "Vcc", VCC, 0) /* D6 */ \
    X(25, "Vcc", VCC, 0) /* E1 */ \
    X(26, "DQ12", DQ, 12) /* E2 */ \
    X(27, "NC", NC, 0) /* E3 */ \
    X(28, "A16", A, 16) /* E4 */ \
    X(29, "DQ4", DQ, 4) /* E5 */ \
    X(30, "Vss", GND, 0) /* E6 */ \
    X(31, "DQ14", DQ, 14) /* F1 */ \
    X(32, "DQ13", DQ, 13) /* F2 */ \
    X(33, "A14", A, 14) /* F3 */ \
    X(34, "A15", A, 15) /* F4 */ \
    X(35, "DQ5", DQ, 5) /* F5 */ \
    X(36, "DQ6", DQ, 6) /* F6 */ \
    X(37, "DQ15", DQ, 15) /* G1 */ \
    X(38, "NC", NC, 0) /* G2 */ \
    X(39, "A12", A, 12) /* G3 */ \
    X(40, "A13", A, 13) /* G4 */ \
    X(41, "WE#", WE_N, 0) /* G5 */ \
    X(42, "DQ7", DQ, 7) /* G6 */ \
    X(43, "NC", NC, 0) /* H1 */ \
    X(44, "A8", A, 8) /* H2 */ \
    X(45, "A9", A, 9) /* H3 */ \
    X(46, "A10", A, 10) /* H4 */ \
    X(47, "A11", A, 11) /* H5 */ \
    X(48, "NC", NC, 0) /* H6 */

#define AS6C2016_BGA_PARTS(X) \
    X("AS6C2016-55BIN", AS6C2016_55, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS6C2016_BGA, "AS6C2016_BGA", "256 KB", "128K x 16 low-power CMOS SRAM",
                 17, 16, AS6C2016_BGA_PINS, AS6C2016_BGA_PARTS)

/* ---- AS6C4016: 256K x 16 low-power CMOS SRAM (datasheets/sram/AS6C4016.pdf) ---- */

/* AC characteristics, AS6C4016-55, VCC = 2.7 V to 5.5 V (the only table). */
static const SramTiming AS6C4016_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .tba = NS(55),
    .toh = NS(10), .tclz = NS(10), .tolz = NS(5), .tblz = NS(10),
    .tchz = NS(20), .tohz = NS(20), .tbhz = NS(25),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tbw = NS(45), .tas = NS(0),
    .twp = NS(45), .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20),
    .tow = NS(5),
};

/* 44-pin TSOP-II (400 mil). */
#define AS6C4016_TSOP2_PINS(X) \
    X(1, "A4", A, 4) \
    X(2, "A3", A, 3) \
    X(3, "A2", A, 2) \
    X(4, "A1", A, 1) \
    X(5, "A0", A, 0) \
    X(6, "CE#", CE_N, 0) \
    X(7, "DQ0", DQ, 0) \
    X(8, "DQ1", DQ, 1) \
    X(9, "DQ2", DQ, 2) \
    X(10, "DQ3", DQ, 3) \
    X(11, "Vcc", VCC, 0) \
    X(12, "Vss", GND, 0) \
    X(13, "DQ4", DQ, 4) \
    X(14, "DQ5", DQ, 5) \
    X(15, "DQ6", DQ, 6) \
    X(16, "DQ7", DQ, 7) \
    X(17, "WE#", WE_N, 0) \
    X(18, "A17", A, 17) \
    X(19, "A16", A, 16) \
    X(20, "A15", A, 15) \
    X(21, "A14", A, 14) \
    X(22, "A13", A, 13) \
    X(23, "A12", A, 12) \
    X(24, "A11", A, 11) \
    X(25, "A10", A, 10) \
    X(26, "A9", A, 9) \
    X(27, "A8", A, 8) \
    X(28, "NC", NC, 0) \
    X(29, "DQ8", DQ, 8) \
    X(30, "DQ9", DQ, 9) \
    X(31, "DQ10", DQ, 10) \
    X(32, "DQ11", DQ, 11) \
    X(33, "Vcc", VCC, 0) \
    X(34, "Vss", GND, 0) \
    X(35, "DQ12", DQ, 12) \
    X(36, "DQ13", DQ, 13) \
    X(37, "DQ14", DQ, 14) \
    X(38, "DQ15", DQ, 15) \
    X(39, "LB#", BE_N, 0) \
    X(40, "UB#", BE_N, 1) \
    X(41, "OE#", OE_N, 0) \
    X(42, "A7", A, 7) \
    X(43, "A6", A, 6) \
    X(44, "A5", A, 5)

#define AS6C4016_TSOP2_PARTS(X) \
    X("AS6C4016-55ZIN", AS6C4016_55, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS6C4016_TSOP2, "AS6C4016_TSOP2", "512 KB", "256K x 16 low-power CMOS SRAM",
                 18, 16, AS6C4016_TSOP2_PINS, AS6C4016_TSOP2_PARTS)

/* 48-ball TFBGA (6 x 8 mm). */
/* Note: the PDF draws the ball map in a substituted font ("DC n" for DQn,
   "RB#" for UB#, "LAA"/"LWW" for Vss/Vcc); read here as the standard 48-ball
   x16 layout (D1/E6 = Vss, D6/E1 = Vcc), the same map AS6C8016 prints legibly. */
#define AS6C4016_BGA_PINS(X) \
    X(1, "LB#", BE_N, 0) /* A1 */ \
    X(2, "OE#", OE_N, 0) /* A2 */ \
    X(3, "A0", A, 0) /* A3 */ \
    X(4, "A1", A, 1) /* A4 */ \
    X(5, "A2", A, 2) /* A5 */ \
    X(6, "NC", NC, 0) /* A6 */ \
    X(7, "DQ8", DQ, 8) /* B1 */ \
    X(8, "UB#", BE_N, 1) /* B2 */ \
    X(9, "A3", A, 3) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "CE#", CE_N, 0) /* B5 */ \
    X(12, "DQ0", DQ, 0) /* B6 */ \
    X(13, "DQ9", DQ, 9) /* C1 */ \
    X(14, "DQ10", DQ, 10) /* C2 */ \
    X(15, "A5", A, 5) /* C3 */ \
    X(16, "A6", A, 6) /* C4 */ \
    X(17, "DQ1", DQ, 1) /* C5 */ \
    X(18, "DQ2", DQ, 2) /* C6 */ \
    X(19, "Vss", GND, 0) /* D1 */ \
    X(20, "DQ11", DQ, 11) /* D2 */ \
    X(21, "A17", A, 17) /* D3 */ \
    X(22, "A7", A, 7) /* D4 */ \
    X(23, "DQ3", DQ, 3) /* D5 */ \
    X(24, "Vcc", VCC, 0) /* D6 */ \
    X(25, "Vcc", VCC, 0) /* E1 */ \
    X(26, "DQ12", DQ, 12) /* E2 */ \
    X(27, "NC", NC, 0) /* E3 */ \
    X(28, "A16", A, 16) /* E4 */ \
    X(29, "DQ4", DQ, 4) /* E5 */ \
    X(30, "Vss", GND, 0) /* E6 */ \
    X(31, "DQ14", DQ, 14) /* F1 */ \
    X(32, "DQ13", DQ, 13) /* F2 */ \
    X(33, "A14", A, 14) /* F3 */ \
    X(34, "A15", A, 15) /* F4 */ \
    X(35, "DQ5", DQ, 5) /* F5 */ \
    X(36, "DQ6", DQ, 6) /* F6 */ \
    X(37, "DQ15", DQ, 15) /* G1 */ \
    X(38, "NC", NC, 0) /* G2 */ \
    X(39, "A12", A, 12) /* G3 */ \
    X(40, "A13", A, 13) /* G4 */ \
    X(41, "WE#", WE_N, 0) /* G5 */ \
    X(42, "DQ7", DQ, 7) /* G6 */ \
    X(43, "NC", NC, 0) /* H1 */ \
    X(44, "A8", A, 8) /* H2 */ \
    X(45, "A9", A, 9) /* H3 */ \
    X(46, "A10", A, 10) /* H4 */ \
    X(47, "A11", A, 11) /* H5 */ \
    X(48, "NC", NC, 0) /* H6 */

#define AS6C4016_BGA_PARTS(X) \
    X("AS6C4016-55BIN", AS6C4016_55, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS6C4016_BGA, "AS6C4016_BGA", "512 KB", "256K x 16 low-power CMOS SRAM",
                 18, 16, AS6C4016_BGA_PINS, AS6C4016_BGA_PARTS)

/* ---- AS6C8016: 512K x 16 low-power CMOS SRAM (datasheets/sram/AS6C8016.pdf) ---- */

/* AC characteristics, AS6C8016-55, VCC = 2.7 V to 5.5 V (the only table). */
static const SramTiming AS6C8016_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .tba = NS(55),
    .toh = NS(10), .tclz = NS(10), .tolz = NS(5), .tblz = NS(10),
    .tchz = NS(20), .tohz = NS(20), .tbhz = NS(25),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tbw = NS(45), .tas = NS(0),
    .twp = NS(45), .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20),
    .tow = NS(5),
};

/* 44-pin TSOP-II (400 mil). */
#define AS6C8016_TSOP2_PINS(X) \
    X(1, "A4", A, 4) \
    X(2, "A3", A, 3) \
    X(3, "A2", A, 2) \
    X(4, "A1", A, 1) \
    X(5, "A0", A, 0) \
    X(6, "CE#", CE_N, 0) \
    X(7, "DQ0", DQ, 0) \
    X(8, "DQ1", DQ, 1) \
    X(9, "DQ2", DQ, 2) \
    X(10, "DQ3", DQ, 3) \
    X(11, "Vcc", VCC, 0) \
    X(12, "Vss", GND, 0) \
    X(13, "DQ4", DQ, 4) \
    X(14, "DQ5", DQ, 5) \
    X(15, "DQ6", DQ, 6) \
    X(16, "DQ7", DQ, 7) \
    X(17, "WE#", WE_N, 0) \
    X(18, "A18", A, 18) \
    X(19, "A17", A, 17) \
    X(20, "A16", A, 16) \
    X(21, "A15", A, 15) \
    X(22, "A14", A, 14) \
    X(23, "A13", A, 13) \
    X(24, "A12", A, 12) \
    X(25, "A11", A, 11) \
    X(26, "A10", A, 10) \
    X(27, "A9", A, 9) \
    X(28, "A8", A, 8) \
    X(29, "DQ8", DQ, 8) \
    X(30, "DQ9", DQ, 9) \
    X(31, "DQ10", DQ, 10) \
    X(32, "DQ11", DQ, 11) \
    X(33, "Vcc", VCC, 0) \
    X(34, "Vss", GND, 0) \
    X(35, "DQ12", DQ, 12) \
    X(36, "DQ13", DQ, 13) \
    X(37, "DQ14", DQ, 14) \
    X(38, "DQ15", DQ, 15) \
    X(39, "LB#", BE_N, 0) \
    X(40, "UB#", BE_N, 1) \
    X(41, "OE#", OE_N, 0) \
    X(42, "A7", A, 7) \
    X(43, "A6", A, 6) \
    X(44, "A5", A, 5)

#define AS6C8016_TSOP2_PARTS(X) \
    X("AS6C8016-55ZIN", AS6C8016_55, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS6C8016_TSOP2, "AS6C8016_TSOP2", "1024 KB", "512K x 16 low-power CMOS SRAM",
                 19, 16, AS6C8016_TSOP2_PINS, AS6C8016_TSOP2_PARTS)

/* 48-ball TFBGA (6 x 8 mm). */
#define AS6C8016_BGA_PINS(X) \
    X(1, "LB#", BE_N, 0) /* A1 */ \
    X(2, "OE#", OE_N, 0) /* A2 */ \
    X(3, "A0", A, 0) /* A3 */ \
    X(4, "A1", A, 1) /* A4 */ \
    X(5, "A2", A, 2) /* A5 */ \
    X(6, "NC", NC, 0) /* A6 */ \
    X(7, "DQ8", DQ, 8) /* B1 */ \
    X(8, "UB#", BE_N, 1) /* B2 */ \
    X(9, "A3", A, 3) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "CE#", CE_N, 0) /* B5 */ \
    X(12, "DQ0", DQ, 0) /* B6 */ \
    X(13, "DQ9", DQ, 9) /* C1 */ \
    X(14, "DQ10", DQ, 10) /* C2 */ \
    X(15, "A5", A, 5) /* C3 */ \
    X(16, "A6", A, 6) /* C4 */ \
    X(17, "DQ1", DQ, 1) /* C5 */ \
    X(18, "DQ2", DQ, 2) /* C6 */ \
    X(19, "Vss", GND, 0) /* D1 */ \
    X(20, "DQ11", DQ, 11) /* D2 */ \
    X(21, "A17", A, 17) /* D3 */ \
    X(22, "A7", A, 7) /* D4 */ \
    X(23, "DQ3", DQ, 3) /* D5 */ \
    X(24, "Vcc", VCC, 0) /* D6 */ \
    X(25, "Vcc", VCC, 0) /* E1 */ \
    X(26, "DQ12", DQ, 12) /* E2 */ \
    X(27, "NC", NC, 0) /* E3 */ \
    X(28, "A16", A, 16) /* E4 */ \
    X(29, "DQ4", DQ, 4) /* E5 */ \
    X(30, "Vss", GND, 0) /* E6 */ \
    X(31, "DQ14", DQ, 14) /* F1 */ \
    X(32, "DQ13", DQ, 13) /* F2 */ \
    X(33, "A14", A, 14) /* F3 */ \
    X(34, "A15", A, 15) /* F4 */ \
    X(35, "DQ5", DQ, 5) /* F5 */ \
    X(36, "DQ6", DQ, 6) /* F6 */ \
    X(37, "DQ15", DQ, 15) /* G1 */ \
    X(38, "NC", NC, 0) /* G2 */ \
    X(39, "A12", A, 12) /* G3 */ \
    X(40, "A13", A, 13) /* G4 */ \
    X(41, "WE#", WE_N, 0) /* G5 */ \
    X(42, "DQ7", DQ, 7) /* G6 */ \
    X(43, "A18", A, 18) /* H1 */ \
    X(44, "A8", A, 8) /* H2 */ \
    X(45, "A9", A, 9) /* H3 */ \
    X(46, "A10", A, 10) /* H4 */ \
    X(47, "A11", A, 11) /* H5 */ \
    X(48, "NC", NC, 0) /* H6 */

#define AS6C8016_BGA_PARTS(X) \
    X("AS6C8016-55BIN", AS6C8016_55, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS6C8016_BGA, "AS6C8016_BGA", "1024 KB", "512K x 16 low-power CMOS SRAM",
                 19, 16, AS6C8016_BGA_PINS, AS6C8016_BGA_PARTS)

/* ---- AS6C1616: 1M x 16 low-power CMOS SRAM (datasheets/sram/AS6C1616.pdf) ---- */

/* AC electrical characteristics, AS6C1616-55, VCC = 2.7 V to 3.6 V (the only table). */
static const SramTiming AS6C1616_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .tba = NS(55), .toh = NS(10),
    .tclz = NS(10), .tolz = NS(5), .tblz = NS(10), .tchz = NS(20), .tohz = NS(20), .tbhz = NS(25),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tbw = NS(45), .tas = NS(0), .twp = NS(45),
    .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20), .tow = NS(5),
};

/* 48-pin TSOP-I (12 x 20 mm). */
#define AS6C1616_TSOP_PINS(X) \
    X(1, "A15", A, 15) \
    X(2, "A14", A, 14) \
    X(3, "A13", A, 13) \
    X(4, "A12", A, 12) \
    X(5, "A11", A, 11) \
    X(6, "A10", A, 10) \
    X(7, "A9", A, 9) \
    X(8, "A8", A, 8) \
    X(9, "A19", A, 19) \
    X(10, "NC", NC, 0) \
    X(11, "WE#", WE_N, 0) \
    X(12, "CE2", CE, 0) \
    X(13, "NC", NC, 0) \
    X(14, "UB#", BE_N, 1) \
    X(15, "LB#", BE_N, 0) \
    X(16, "A18", A, 18) \
    X(17, "A17", A, 17) \
    X(18, "A7", A, 7) \
    X(19, "A6", A, 6) \
    X(20, "A5", A, 5) \
    X(21, "A4", A, 4) \
    X(22, "A3", A, 3) \
    X(23, "A2", A, 2) \
    X(24, "A1", A, 1) \
    X(25, "A0", A, 0) \
    X(26, "CE#", CE_N, 0) \
    X(27, "Vss", GND, 0) \
    X(28, "OE#", OE_N, 0) \
    X(29, "DQ0", DQ, 0) \
    X(30, "DQ8", DQ, 8) \
    X(31, "DQ1", DQ, 1) \
    X(32, "DQ9", DQ, 9) \
    X(33, "DQ2", DQ, 2) \
    X(34, "DQ10", DQ, 10) \
    X(35, "DQ3", DQ, 3) \
    X(36, "DQ11", DQ, 11) \
    X(37, "Vcc", VCC, 0) \
    X(38, "DQ4", DQ, 4) \
    X(39, "DQ12", DQ, 12) \
    X(40, "DQ5", DQ, 5) \
    X(41, "DQ13", DQ, 13) \
    X(42, "DQ6", DQ, 6) \
    X(43, "DQ14", DQ, 14) \
    X(44, "DQ7", DQ, 7) \
    X(45, "DQ15", DQ, 15) \
    X(46, "Vss", GND, 0) \
    X(47, "NC", NC, 0) \
    X(48, "A16", A, 16)

#define AS6C1616_TSOP_PARTS(X) \
    X("AS6C1616-55TIN", AS6C1616_55, PACKAGE_TYPE_TSOP_I, 0)

SRAM_DEFINE_CHIP(AS6C1616_TSOP, "AS6C1616_TSOP", "2048 KB", "1M x 16 low-power CMOS SRAM",
                 20, 16, AS6C1616_TSOP_PINS, AS6C1616_TSOP_PARTS)

/* ---- AS6C6416: 4M x 16 low-power CMOS SRAM (datasheets/sram/AS6C6416-55BIN.pdf) ---- */

/* AC characteristics, AS6C6416-55BIN, VCC = 2.7 V to 3.6 V (the only table). */
static const SramTiming AS6C6416_55 = {
    .trc = NS(55), .taa = NS(55), .tace = NS(55), .toe = NS(30), .tba = NS(55),
    .toh = NS(10), .tclz = NS(10), .tolz = NS(5), .tblz = NS(10),
    .tchz = NS(20), .tohz = NS(20), .tbhz = NS(20),
    .twc = NS(55), .taw = NS(50), .tcw = NS(50), .tbw = NS(50), .tas = NS(0),
    .twp = NS(45), .twr = NS(0), .tdw = NS(25), .tdh = NS(0), .twhz = NS(20),
    .tow = NS(5),
};

/* 48-ball TFBGA (8 x 10 mm), the only package. */
#define AS6C6416_PINS(X) \
    X(1, "LB#", BE_N, 0) /* A1 */ \
    X(2, "OE#", OE_N, 0) /* A2 */ \
    X(3, "A0", A, 0) /* A3 */ \
    X(4, "A1", A, 1) /* A4 */ \
    X(5, "A2", A, 2) /* A5 */ \
    X(6, "CE2", CE, 0) /* A6 */ \
    X(7, "DQ8", DQ, 8) /* B1 */ \
    X(8, "UB#", BE_N, 1) /* B2 */ \
    X(9, "A3", A, 3) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "CE#", CE_N, 0) /* B5 */ \
    X(12, "DQ0", DQ, 0) /* B6 */ \
    X(13, "DQ9", DQ, 9) /* C1 */ \
    X(14, "DQ10", DQ, 10) /* C2 */ \
    X(15, "A5", A, 5) /* C3 */ \
    X(16, "A6", A, 6) /* C4 */ \
    X(17, "DQ1", DQ, 1) /* C5 */ \
    X(18, "DQ2", DQ, 2) /* C6 */ \
    X(19, "Vss", GND, 0) /* D1 */ \
    X(20, "DQ11", DQ, 11) /* D2 */ \
    X(21, "A17", A, 17) /* D3 */ \
    X(22, "A7", A, 7) /* D4 */ \
    X(23, "DQ3", DQ, 3) /* D5 */ \
    X(24, "Vcc", VCC, 0) /* D6 */ \
    X(25, "Vcc", VCC, 0) /* E1 */ \
    X(26, "DQ12", DQ, 12) /* E2 */ \
    X(27, "A21", A, 21) /* E3 */ \
    X(28, "A16", A, 16) /* E4 */ \
    X(29, "DQ4", DQ, 4) /* E5 */ \
    X(30, "Vss", GND, 0) /* E6 */ \
    X(31, "DQ14", DQ, 14) /* F1 */ \
    X(32, "DQ13", DQ, 13) /* F2 */ \
    X(33, "A14", A, 14) /* F3 */ \
    X(34, "A15", A, 15) /* F4 */ \
    X(35, "DQ5", DQ, 5) /* F5 */ \
    X(36, "DQ6", DQ, 6) /* F6 */ \
    X(37, "DQ15", DQ, 15) /* G1 */ \
    X(38, "A19", A, 19) /* G2 */ \
    X(39, "A12", A, 12) /* G3 */ \
    X(40, "A13", A, 13) /* G4 */ \
    X(41, "WE#", WE_N, 0) /* G5 */ \
    X(42, "DQ7", DQ, 7) /* G6 */ \
    X(43, "A18", A, 18) /* H1 */ \
    X(44, "A8", A, 8) /* H2 */ \
    X(45, "A9", A, 9) /* H3 */ \
    X(46, "A10", A, 10) /* H4 */ \
    X(47, "A11", A, 11) /* H5 */ \
    X(48, "A20", A, 20) /* H6 */

/* Note: the ordering page gives the code format with a packing suffix
   (none = tray, TR = reel) rather than a list of codes; both are listed. */
#define AS6C6416_PARTS(X) \
    X("AS6C6416-55BIN", AS6C6416_55, PACKAGE_TYPE_TFBGA, 0) \
    X("AS6C6416-55BINTR", AS6C6416_55, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS6C6416, "AS6C6416", "8192 KB", "4M x 16 low-power CMOS SRAM",
                 22, 16, AS6C6416_PINS, AS6C6416_PARTS)

/* ---- AS7C256A: 32K x 8 CMOS SRAM (datasheets/sram/as7c256av.pdf) ---- */

/* Note: despite its file name, datasheets/sram/as7c256av.pdf is the AS7C256A
   datasheet (5 V, 4.5-5.5 V, Sept 2004 v1.2), not the 3.3 V AS7C256A. The C
   identifier is AS7C256A as requested, but every value, pin and order code below
   is the AS7C256A's as printed. Order codes: the table lists leaded codes; a
   footnote says to add suffix 'N' for lead-free, so both are listed.
   tWR and tAH (address hold from end of write) are both 0. */

/* -10: read/write cycle tables (over the operating range, VCC = 4.5-5.5 V). */
static const SramTiming AS7C256A_10 = {
    .trc = NS(10), .taa = NS(10), .tace = NS(10), .toe = NS(5), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tchz = NS(3), .tohz = NS(3), .twc = NS(10),
    .taw = NS(8), .tcw = NS(8), .tas = NS(0), .twp = NS(7), .twr = NS(0), .tdw = NS(5),
    .tdh = NS(0), .twhz = NS(5), .tow = NS(3),
};

/* -12: read/write cycle tables (over the operating range, VCC = 4.5-5.5 V). */
static const SramTiming AS7C256A_12 = {
    .trc = NS(12), .taa = NS(12), .tace = NS(12), .toe = NS(6), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tchz = NS(3), .tohz = NS(3), .twc = NS(12),
    .taw = NS(8), .tcw = NS(8), .tas = NS(0), .twp = NS(8), .twr = NS(0), .tdw = NS(6),
    .tdh = NS(0), .twhz = NS(6), .tow = NS(3),
};

/* -15: read/write cycle tables (over the operating range, VCC = 4.5-5.5 V). */
static const SramTiming AS7C256A_15 = {
    .trc = NS(15), .taa = NS(15), .tace = NS(15), .toe = NS(7), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tchz = NS(4), .tohz = NS(4), .twc = NS(15),
    .taw = NS(10), .tcw = NS(10), .tas = NS(0), .twp = NS(9), .twr = NS(0),
    .tdw = NS(8), .tdh = NS(0), .twhz = NS(7), .tow = NS(3),
};

/* -20: read/write cycle tables (over the operating range, VCC = 4.5-5.5 V). */
static const SramTiming AS7C256A_20 = {
    .trc = NS(20), .taa = NS(20), .tace = NS(20), .toe = NS(8), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tchz = NS(5), .tohz = NS(5), .twc = NS(20),
    .taw = NS(12), .tcw = NS(12), .tas = NS(0), .twp = NS(12), .twr = NS(0),
    .tdw = NS(10), .tdh = NS(0), .twhz = NS(8), .tow = NS(3),
};

/* 28-pin SOJ (300 mil). */
#define AS7C256A_PINS(X) \
    X(1, "A14", A, 14) \
    X(2, "A12", A, 12) \
    X(3, "A7", A, 7) \
    X(4, "A6", A, 6) \
    X(5, "A5", A, 5) \
    X(6, "A4", A, 4) \
    X(7, "A3", A, 3) \
    X(8, "A2", A, 2) \
    X(9, "A1", A, 1) \
    X(10, "A0", A, 0) \
    X(11, "I/O0", DQ, 0) \
    X(12, "I/O1", DQ, 1) \
    X(13, "I/O2", DQ, 2) \
    X(14, "GND", GND, 0) \
    X(15, "I/O3", DQ, 3) \
    X(16, "I/O4", DQ, 4) \
    X(17, "I/O5", DQ, 5) \
    X(18, "I/O6", DQ, 6) \
    X(19, "I/O7", DQ, 7) \
    X(20, "CE#", CE_N, 0) \
    X(21, "A10", A, 10) \
    X(22, "OE#", OE_N, 0) \
    X(23, "A11", A, 11) \
    X(24, "A9", A, 9) \
    X(25, "A8", A, 8) \
    X(26, "A13", A, 13) \
    X(27, "WE#", WE_N, 0) \
    X(28, "VCC", VCC, 0)

#define AS7C256A_PARTS(X) \
    X("AS7C256A-10JC", AS7C256A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-10JCN", AS7C256A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-12JC", AS7C256A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-12JCN", AS7C256A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-15JC", AS7C256A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-15JCN", AS7C256A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-20JC", AS7C256A_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-20JCN", AS7C256A_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-10JI", AS7C256A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-10JIN", AS7C256A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-12JI", AS7C256A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-12JIN", AS7C256A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-15JI", AS7C256A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-15JIN", AS7C256A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-20JI", AS7C256A_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C256A-20JIN", AS7C256A_20, PACKAGE_TYPE_SOJ, 0)

SRAM_DEFINE_CHIP(AS7C256A, "AS7C256A", "32 KB", "32K x 8 CMOS SRAM, 5 V",
                 15, 8, AS7C256A_PINS, AS7C256A_PARTS)

/* 28-pin TSOP 1 (8 x 13.4 mm). */
#define AS7C256A_STSOP_PINS(X) \
    X(1, "OE#", OE_N, 0) \
    X(2, "A11", A, 11) \
    X(3, "A9", A, 9) \
    X(4, "A8", A, 8) \
    X(5, "A13", A, 13) \
    X(6, "WE#", WE_N, 0) \
    X(7, "VCC", VCC, 0) \
    X(8, "A14", A, 14) \
    X(9, "A12", A, 12) \
    X(10, "A7", A, 7) \
    X(11, "A6", A, 6) \
    X(12, "A5", A, 5) \
    X(13, "A4", A, 4) \
    X(14, "A3", A, 3) \
    X(15, "A2", A, 2) \
    X(16, "A1", A, 1) \
    X(17, "A0", A, 0) \
    X(18, "I/O0", DQ, 0) \
    X(19, "I/O1", DQ, 1) \
    X(20, "I/O2", DQ, 2) \
    X(21, "GND", GND, 0) \
    X(22, "I/O3", DQ, 3) \
    X(23, "I/O4", DQ, 4) \
    X(24, "I/O5", DQ, 5) \
    X(25, "I/O6", DQ, 6) \
    X(26, "I/O7", DQ, 7) \
    X(27, "CE#", CE_N, 0) \
    X(28, "A10", A, 10)

#define AS7C256A_STSOP_PARTS(X) \
    X("AS7C256A-10TC", AS7C256A_10, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-10TCN", AS7C256A_10, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-12TC", AS7C256A_12, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-12TCN", AS7C256A_12, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-15TC", AS7C256A_15, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-15TCN", AS7C256A_15, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-20TC", AS7C256A_20, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-20TCN", AS7C256A_20, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-10TI", AS7C256A_10, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-10TIN", AS7C256A_10, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-12TI", AS7C256A_12, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-12TIN", AS7C256A_12, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-15TI", AS7C256A_15, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-15TIN", AS7C256A_15, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-20TI", AS7C256A_20, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C256A-20TIN", AS7C256A_20, PACKAGE_TYPE_STSOP_I, 0)

SRAM_DEFINE_CHIP(AS7C256A_STSOP, "AS7C256A_STSOP", "32 KB", "32K x 8 CMOS SRAM, 5 V",
                 15, 8, AS7C256A_STSOP_PINS, AS7C256A_STSOP_PARTS)

/* ---- AS7C31024B: 128K x 8 3.3 V CMOS SRAM (datasheets/sram/AS7C31024B.pdf) ---- */

/* Note: the ordering table lists only the leaded codes; a footnote says to add
   suffix 'N' for lead-free parts, so both the base codes and the -N codes are listed.
   tCW/tACE/tCLZ/tCHZ are given separately for CE1# and CE2 but are identical
   (note 12); tWR and tAH (address hold from end of write) are both 0. */

/* -10: read/write cycle tables (over the operating range, VCC = 3.0-3.6 V). */
static const SramTiming AS7C31024B_10 = {
    .trc = NS(10), .taa = NS(10), .tace = NS(10), .toe = NS(5), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tchz = NS(3), .tohz = NS(5), .twc = NS(10),
    .taw = NS(8), .tcw = NS(8), .tas = NS(0), .twp = NS(7), .twr = NS(0), .tdw = NS(5),
    .tdh = NS(0), .twhz = NS(5), .tow = NS(1),
};

/* -12: read/write cycle tables (over the operating range, VCC = 3.0-3.6 V). */
static const SramTiming AS7C31024B_12 = {
    .trc = NS(12), .taa = NS(12), .tace = NS(12), .toe = NS(6), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tchz = NS(3), .tohz = NS(6), .twc = NS(12),
    .taw = NS(9), .tcw = NS(9), .tas = NS(0), .twp = NS(8), .twr = NS(0), .tdw = NS(6),
    .tdh = NS(0), .twhz = NS(6), .tow = NS(1),
};

/* -15: read/write cycle tables (over the operating range, VCC = 3.0-3.6 V). */
static const SramTiming AS7C31024B_15 = {
    .trc = NS(15), .taa = NS(15), .tace = NS(15), .toe = NS(7), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tchz = NS(4), .tohz = NS(7), .twc = NS(15),
    .taw = NS(10), .tcw = NS(10), .tas = NS(0), .twp = NS(9), .twr = NS(0),
    .tdw = NS(8), .tdh = NS(0), .twhz = NS(7), .tow = NS(1),
};

/* -20: read/write cycle tables (over the operating range, VCC = 3.0-3.6 V). */
static const SramTiming AS7C31024B_20 = {
    .trc = NS(20), .taa = NS(20), .tace = NS(20), .toe = NS(8), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tchz = NS(5), .tohz = NS(8), .twc = NS(20),
    .taw = NS(12), .tcw = NS(12), .tas = NS(0), .twp = NS(12), .twr = NS(0),
    .tdw = NS(10), .tdh = NS(0), .twhz = NS(8), .tow = NS(1),
};

/* 32-pin SOJ, 300 mil (TJ) and 400 mil (J). */
#define AS7C31024B_PINS(X) \
    X(1, "NC", NC, 0) \
    X(2, "A16", A, 16) \
    X(3, "A14", A, 14) \
    X(4, "A12", A, 12) \
    X(5, "A7", A, 7) \
    X(6, "A6", A, 6) \
    X(7, "A5", A, 5) \
    X(8, "A4", A, 4) \
    X(9, "A3", A, 3) \
    X(10, "A2", A, 2) \
    X(11, "A1", A, 1) \
    X(12, "A0", A, 0) \
    X(13, "I/O0", DQ, 0) \
    X(14, "I/O1", DQ, 1) \
    X(15, "I/O2", DQ, 2) \
    X(16, "GND", GND, 0) \
    X(17, "I/O3", DQ, 3) \
    X(18, "I/O4", DQ, 4) \
    X(19, "I/O5", DQ, 5) \
    X(20, "I/O6", DQ, 6) \
    X(21, "I/O7", DQ, 7) \
    X(22, "CE1#", CE_N, 0) \
    X(23, "A10", A, 10) \
    X(24, "OE#", OE_N, 0) \
    X(25, "A11", A, 11) \
    X(26, "A9", A, 9) \
    X(27, "A8", A, 8) \
    X(28, "A13", A, 13) \
    X(29, "WE#", WE_N, 0) \
    X(30, "CE2", CE, 0) \
    X(31, "A15", A, 15) \
    X(32, "VCC", VCC, 0)

#define AS7C31024B_PARTS(X) \
    X("AS7C31024B-10TJC", AS7C31024B_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-10TJCN", AS7C31024B_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-12TJC", AS7C31024B_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-12TJCN", AS7C31024B_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-15TJC", AS7C31024B_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-15TJCN", AS7C31024B_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-20TJC", AS7C31024B_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-20TJCN", AS7C31024B_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-10JC", AS7C31024B_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-10JCN", AS7C31024B_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-12JC", AS7C31024B_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-12JCN", AS7C31024B_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-15JC", AS7C31024B_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-15JCN", AS7C31024B_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-20JC", AS7C31024B_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31024B-20JCN", AS7C31024B_20, PACKAGE_TYPE_SOJ, 0)

SRAM_DEFINE_CHIP(AS7C31024B, "AS7C31024B", "128 KB", "128K x 8 CMOS SRAM, 3.3 V",
                 17, 8, AS7C31024B_PINS, AS7C31024B_PARTS)

/* 32-pin TSOP I (8 x 20 mm) and sTSOP I (8 x 13.4 mm): identical pinout. */
#define AS7C31024B_TSOP_PINS(X) \
    X(1, "A11", A, 11) \
    X(2, "A9", A, 9) \
    X(3, "A8", A, 8) \
    X(4, "A13", A, 13) \
    X(5, "WE#", WE_N, 0) \
    X(6, "CE2", CE, 0) \
    X(7, "A15", A, 15) \
    X(8, "VCC", VCC, 0) \
    X(9, "NC", NC, 0) \
    X(10, "A16", A, 16) \
    X(11, "A14", A, 14) \
    X(12, "A12", A, 12) \
    X(13, "A7", A, 7) \
    X(14, "A6", A, 6) \
    X(15, "A5", A, 5) \
    X(16, "A4", A, 4) \
    X(17, "A3", A, 3) \
    X(18, "A2", A, 2) \
    X(19, "A1", A, 1) \
    X(20, "A0", A, 0) \
    X(21, "I/O0", DQ, 0) \
    X(22, "I/O1", DQ, 1) \
    X(23, "I/O2", DQ, 2) \
    X(24, "GND", GND, 0) \
    X(25, "I/O3", DQ, 3) \
    X(26, "I/O4", DQ, 4) \
    X(27, "I/O5", DQ, 5) \
    X(28, "I/O6", DQ, 6) \
    X(29, "I/O7", DQ, 7) \
    X(30, "CE1#", CE_N, 0) \
    X(31, "A10", A, 10) \
    X(32, "OE#", OE_N, 0)

#define AS7C31024B_TSOP_PARTS(X) \
    X("AS7C31024B-10TC", AS7C31024B_10, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS7C31024B-10TCN", AS7C31024B_10, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS7C31024B-12TC", AS7C31024B_12, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS7C31024B-12TCN", AS7C31024B_12, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS7C31024B-15TC", AS7C31024B_15, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS7C31024B-15TCN", AS7C31024B_15, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS7C31024B-20TC", AS7C31024B_20, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS7C31024B-20TCN", AS7C31024B_20, PACKAGE_TYPE_TSOP_I, 0) \
    X("AS7C31024B-10STC", AS7C31024B_10, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C31024B-10STCN", AS7C31024B_10, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C31024B-12STC", AS7C31024B_12, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C31024B-12STCN", AS7C31024B_12, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C31024B-15STC", AS7C31024B_15, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C31024B-15STCN", AS7C31024B_15, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C31024B-20STC", AS7C31024B_20, PACKAGE_TYPE_STSOP_I, 0) \
    X("AS7C31024B-20STCN", AS7C31024B_20, PACKAGE_TYPE_STSOP_I, 0)

SRAM_DEFINE_CHIP(AS7C31024B_TSOP, "AS7C31024B_TSOP", "128 KB", "128K x 8 CMOS SRAM, 3.3 V",
                 17, 8, AS7C31024B_TSOP_PINS, AS7C31024B_TSOP_PARTS)

/* ---- AS7C4096A: 512K x 8 5.0 V CMOS fast SRAM (datasheets/sram/AS7C4096A.pdf) ---- */

/* Read/write cycle tables, -10 column, VCC = 4.5 V to 5.5 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ (max) = twhz; tWR and tAH both 0 */
static const SramTiming AS7C4096A_10 = {
    .trc = NS(10), .taa = NS(10), .tace = NS(10), .toe = NS(5), .toh = NS(3), .tclz = NS(3),
    .tolz = NS(0), .tchz = NS(5), .tohz = NS(5), .twc = NS(10), .taw = NS(7), .tcw = NS(7),
    .tas = NS(0), .twp = NS(7), .twp_oe_low = NS(10), .twr = NS(0), .tdw = NS(5),
    .tdh = NS(0), .twhz = NS(5), .tow = NS(3),
};

/* Read/write cycle tables, -12 column, VCC = 4.5 V to 5.5 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ (max) = twhz; tWR and tAH both 0 */
static const SramTiming AS7C4096A_12 = {
    .trc = NS(12), .taa = NS(12), .tace = NS(12), .toe = NS(6), .toh = NS(3), .tclz = NS(3),
    .tolz = NS(0), .tchz = NS(6), .tohz = NS(6), .twc = NS(12), .taw = NS(8), .tcw = NS(8),
    .tas = NS(0), .twp = NS(8), .twp_oe_low = NS(12), .twr = NS(0), .tdw = NS(6),
    .tdh = NS(0), .twhz = NS(6), .tow = NS(3),
};

/* Read/write cycle tables, -15 column, VCC = 4.5 V to 5.5 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ (max) = twhz; tWR and tAH both 0 */
static const SramTiming AS7C4096A_15 = {
    .trc = NS(15), .taa = NS(15), .tace = NS(15), .toe = NS(6), .toh = NS(3), .tclz = NS(3),
    .tolz = NS(0), .tchz = NS(7), .tohz = NS(7), .twc = NS(15), .taw = NS(10),
    .tcw = NS(10), .tas = NS(0), .twp = NS(10), .twp_oe_low = NS(15), .twr = NS(0),
    .tdw = NS(7), .tdh = NS(0), .twhz = NS(7), .tow = NS(3),
};

/* Read/write cycle tables, -20 column, VCC = 4.5 V to 5.5 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ (max) = twhz; tWR and tAH both 0 */
static const SramTiming AS7C4096A_20 = {
    .trc = NS(20), .taa = NS(20), .tace = NS(20), .toe = NS(6), .toh = NS(3), .tclz = NS(3),
    .tolz = NS(0), .tchz = NS(9), .tohz = NS(9), .twc = NS(20), .taw = NS(12),
    .tcw = NS(12), .tas = NS(0), .twp = NS(12), .twp_oe_low = NS(20), .twr = NS(0),
    .tdw = NS(9), .tdh = NS(0), .twhz = NS(9), .tow = NS(3),
};

/* 36-pin 400-mil SOJ. Data pins are I/O1..I/O8 -> DQ 0..7. */
#define AS7C4096A_PINS(X) \
    X(1, "A0", A, 0) \
    X(2, "A1", A, 1) \
    X(3, "A2", A, 2) \
    X(4, "A3", A, 3) \
    X(5, "A4", A, 4) \
    X(6, "CE#", CE_N, 0) \
    X(7, "I/O1", DQ, 0) \
    X(8, "I/O2", DQ, 1) \
    X(9, "VCC", VCC, 0) \
    X(10, "GND", GND, 0) \
    X(11, "I/O3", DQ, 2) \
    X(12, "I/O4", DQ, 3) \
    X(13, "WE#", WE_N, 0) \
    X(14, "A5", A, 5) \
    X(15, "A6", A, 6) \
    X(16, "A7", A, 7) \
    X(17, "A8", A, 8) \
    X(18, "A9", A, 9) \
    X(19, "NC", NC, 0) \
    X(20, "A10", A, 10) \
    X(21, "A11", A, 11) \
    X(22, "A12", A, 12) \
    X(23, "A13", A, 13) \
    X(24, "A14", A, 14) \
    X(25, "I/O5", DQ, 4) \
    X(26, "I/O6", DQ, 5) \
    X(27, "VCC", VCC, 0) \
    X(28, "GND", GND, 0) \
    X(29, "I/O7", DQ, 6) \
    X(30, "I/O8", DQ, 7) \
    X(31, "OE#", OE_N, 0) \
    X(32, "A15", A, 15) \
    X(33, "A16", A, 16) \
    X(34, "A17", A, 17) \
    X(35, "A18", A, 18) \
    X(36, "NC", NC, 0)

#define AS7C4096A_PARTS(X) \
    X("AS7C4096A-10JCN", AS7C4096A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4096A-10JIN", AS7C4096A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4096A-12JCN", AS7C4096A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4096A-12JIN", AS7C4096A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4096A-15JCN", AS7C4096A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4096A-15JIN", AS7C4096A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4096A-20JCN", AS7C4096A_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4096A-20JIN", AS7C4096A_20, PACKAGE_TYPE_SOJ, 0)

SRAM_DEFINE_CHIP(AS7C4096A, "AS7C4096A", "512 KB", "512K x 8 CMOS fast SRAM, 5 V",
                 19, 8, AS7C4096A_PINS, AS7C4096A_PARTS)

/* 44-pin TSOP 2: the SOJ pinout with NC pins 1, 2, 21-25 and 42-44 added. */
#define AS7C4096A_TSOP2_PINS(X) \
    X(1, "NC", NC, 0) \
    X(2, "NC", NC, 0) \
    X(3, "A0", A, 0) \
    X(4, "A1", A, 1) \
    X(5, "A2", A, 2) \
    X(6, "A3", A, 3) \
    X(7, "A4", A, 4) \
    X(8, "CE#", CE_N, 0) \
    X(9, "I/O1", DQ, 0) \
    X(10, "I/O2", DQ, 1) \
    X(11, "VCC", VCC, 0) \
    X(12, "GND", GND, 0) \
    X(13, "I/O3", DQ, 2) \
    X(14, "I/O4", DQ, 3) \
    X(15, "WE#", WE_N, 0) \
    X(16, "A5", A, 5) \
    X(17, "A6", A, 6) \
    X(18, "A7", A, 7) \
    X(19, "A8", A, 8) \
    X(20, "A9", A, 9) \
    X(21, "NC", NC, 0) \
    X(22, "NC", NC, 0) \
    X(23, "NC", NC, 0) \
    X(24, "NC", NC, 0) \
    X(25, "NC", NC, 0) \
    X(26, "A10", A, 10) \
    X(27, "A11", A, 11) \
    X(28, "A12", A, 12) \
    X(29, "A13", A, 13) \
    X(30, "A14", A, 14) \
    X(31, "I/O5", DQ, 4) \
    X(32, "I/O6", DQ, 5) \
    X(33, "VCC", VCC, 0) \
    X(34, "GND", GND, 0) \
    X(35, "I/O7", DQ, 6) \
    X(36, "I/O8", DQ, 7) \
    X(37, "OE#", OE_N, 0) \
    X(38, "A15", A, 15) \
    X(39, "A16", A, 16) \
    X(40, "A17", A, 17) \
    X(41, "A18", A, 18) \
    X(42, "NC", NC, 0) \
    X(43, "NC", NC, 0) \
    X(44, "NC", NC, 0)

#define AS7C4096A_TSOP2_PARTS(X) \
    X("AS7C4096A-10TCN", AS7C4096A_10, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4096A-10TIN", AS7C4096A_10, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4096A-12TCN", AS7C4096A_12, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4096A-12TIN", AS7C4096A_12, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4096A-15TCN", AS7C4096A_15, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4096A-15TIN", AS7C4096A_15, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4096A-20TCN", AS7C4096A_20, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4096A-20TIN", AS7C4096A_20, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS7C4096A_TSOP2, "AS7C4096A_TSOP2", "512 KB", "512K x 8 CMOS fast SRAM, 5 V",
                 19, 8, AS7C4096A_TSOP2_PINS, AS7C4096A_TSOP2_PARTS)

/* ---- AS7C31026B: 64K x 16 3.3 V fast asynchronous SRAM (datasheets/sram/AS7C31026B.pdf) ---- */

/* Note: values transcribed as printed even where they look odd: -15 tBHZ = 6 ns
   (less than tOHZ = 7), -20 tBW = 9 ns (less than tWP = 12), -20 tOW = 2 ns.
   tWR and tAH (address hold from end of write) are both 0. */

/* -10: read/write cycle tables (over the operating range, VCC = 3.0-3.6 V). */
static const SramTiming AS7C31026B_10 = {
    .trc = NS(10), .taa = NS(10), .tace = NS(10), .toe = NS(5), .tba = NS(5),
    .toh = NS(3), .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(3),
    .tohz = NS(5), .tbhz = NS(5), .twc = NS(10), .taw = NS(8), .tcw = NS(8),
    .tbw = NS(7), .tas = NS(0), .twp = NS(7), .twr = NS(0), .tdw = NS(5), .tdh = NS(0),
    .twhz = NS(5), .tow = NS(1),
};

/* -12: read/write cycle tables (over the operating range, VCC = 3.0-3.6 V). */
static const SramTiming AS7C31026B_12 = {
    .trc = NS(12), .taa = NS(12), .tace = NS(12), .toe = NS(6), .tba = NS(6),
    .toh = NS(3), .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(3),
    .tohz = NS(6), .tbhz = NS(6), .twc = NS(12), .taw = NS(9), .tcw = NS(9),
    .tbw = NS(8), .tas = NS(0), .twp = NS(8), .twr = NS(0), .tdw = NS(6), .tdh = NS(0),
    .twhz = NS(6), .tow = NS(1),
};

/* -15: read/write cycle tables (over the operating range, VCC = 3.0-3.6 V). */
static const SramTiming AS7C31026B_15 = {
    .trc = NS(15), .taa = NS(15), .tace = NS(15), .toe = NS(7), .tba = NS(7),
    .toh = NS(3), .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(4),
    .tohz = NS(7), .tbhz = NS(6), .twc = NS(15), .taw = NS(10), .tcw = NS(10),
    .tbw = NS(9), .tas = NS(0), .twp = NS(9), .twr = NS(0), .tdw = NS(8), .tdh = NS(0),
    .twhz = NS(7), .tow = NS(1),
};

/* -20: read/write cycle tables (over the operating range, VCC = 3.0-3.6 V). */
static const SramTiming AS7C31026B_20 = {
    .trc = NS(20), .taa = NS(20), .tace = NS(20), .toe = NS(8), .tba = NS(8),
    .toh = NS(3), .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(5),
    .tohz = NS(8), .tbhz = NS(8), .twc = NS(20), .taw = NS(12), .tcw = NS(12),
    .tbw = NS(9), .tas = NS(0), .twp = NS(12), .twr = NS(0), .tdw = NS(10),
    .tdh = NS(0), .twhz = NS(8), .tow = NS(2),
};

/* 44-pin SOJ (400 mil) and TSOP II: identical pinout.
   Note: -10TCN is still in the ordering table, but revision 1.1 says the 10 ns
   TSOP II is not offered, so it is marked obsolete. There is no -10TIN; the
   12/15/20 ns TIN codes were reintroduced in revision 3.0. Packing suffix TR
   (reel) is mentioned in the part numbering key but no TR codes are listed. */
#define AS7C31026B_PINS(X) \
    X(1, "A4", A, 4) \
    X(2, "A3", A, 3) \
    X(3, "A2", A, 2) \
    X(4, "A1", A, 1) \
    X(5, "A0", A, 0) \
    X(6, "CE#", CE_N, 0) \
    X(7, "I/O0", DQ, 0) \
    X(8, "I/O1", DQ, 1) \
    X(9, "I/O2", DQ, 2) \
    X(10, "I/O3", DQ, 3) \
    X(11, "VCC", VCC, 0) \
    X(12, "GND", GND, 0) \
    X(13, "I/O4", DQ, 4) \
    X(14, "I/O5", DQ, 5) \
    X(15, "I/O6", DQ, 6) \
    X(16, "I/O7", DQ, 7) \
    X(17, "WE#", WE_N, 0) \
    X(18, "A15", A, 15) \
    X(19, "A14", A, 14) \
    X(20, "A13", A, 13) \
    X(21, "A12", A, 12) \
    X(22, "NC", NC, 0) \
    X(23, "NC", NC, 0) \
    X(24, "A11", A, 11) \
    X(25, "A10", A, 10) \
    X(26, "A9", A, 9) \
    X(27, "A8", A, 8) \
    X(28, "NC", NC, 0) \
    X(29, "I/O8", DQ, 8) \
    X(30, "I/O9", DQ, 9) \
    X(31, "I/O10", DQ, 10) \
    X(32, "I/O11", DQ, 11) \
    X(33, "VCC", VCC, 0) \
    X(34, "GND", GND, 0) \
    X(35, "I/O12", DQ, 12) \
    X(36, "I/O13", DQ, 13) \
    X(37, "I/O14", DQ, 14) \
    X(38, "I/O15", DQ, 15) \
    X(39, "LB#", BE_N, 0) \
    X(40, "UB#", BE_N, 1) \
    X(41, "OE#", OE_N, 0) \
    X(42, "A7", A, 7) \
    X(43, "A6", A, 6) \
    X(44, "A5", A, 5)

#define AS7C31026B_PARTS(X) \
    X("AS7C31026B-10JCN", AS7C31026B_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31026B-12JCN", AS7C31026B_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31026B-15JCN", AS7C31026B_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31026B-20JCN", AS7C31026B_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31026B-10JIN", AS7C31026B_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31026B-12JIN", AS7C31026B_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31026B-15JIN", AS7C31026B_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31026B-20JIN", AS7C31026B_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C31026B-10TCN", AS7C31026B_10, PACKAGE_TYPE_TSOP_II, PART_FLAG_OBSOLETE) \
    X("AS7C31026B-12TCN", AS7C31026B_12, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C31026B-15TCN", AS7C31026B_15, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C31026B-20TCN", AS7C31026B_20, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C31026B-12TIN", AS7C31026B_12, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C31026B-15TIN", AS7C31026B_15, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C31026B-20TIN", AS7C31026B_20, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS7C31026B, "AS7C31026B", "128 KB", "64K x 16 fast asynchronous SRAM, 3.3 V",
                 16, 16, AS7C31026B_PINS, AS7C31026B_PARTS)

/* ---- AS7C34098A: 256K x 16 3.3 V CMOS fast SRAM (datasheets/sram/AS7C34098A.pdf) ---- */

/* Read/write cycle tables, -10 column, VCC = 3.0 V to 3.6 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ = twhz; tWR and tAH both 0 */
static const SramTiming AS7C34098A_10 = {
    .trc = NS(10), .taa = NS(10), .tace = NS(10), .toe = NS(4), .tba = NS(5), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(5), .tohz = NS(5),
    .tbhz = NS(5), .twc = NS(10), .taw = NS(7), .tcw = NS(7), .tbw = NS(7), .tas = NS(0),
    .twp = NS(7), .twp_oe_low = NS(10), .twr = NS(0), .tdw = NS(5), .tdh = NS(0),
    .twhz = NS(5), .tow = NS(3),
};

/* Read/write cycle tables, -12 column, VCC = 3.0 V to 3.6 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ = twhz; tWR and tAH both 0 */
static const SramTiming AS7C34098A_12 = {
    .trc = NS(12), .taa = NS(12), .tace = NS(12), .toe = NS(5), .tba = NS(6), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(6), .tohz = NS(6),
    .tbhz = NS(6), .twc = NS(12), .taw = NS(8), .tcw = NS(8), .tbw = NS(8), .tas = NS(0),
    .twp = NS(8), .twp_oe_low = NS(12), .twr = NS(0), .tdw = NS(6), .tdh = NS(0),
    .twhz = NS(6), .tow = NS(3),
};

/* Read/write cycle tables, -15 column, VCC = 3.0 V to 3.6 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ = twhz; tWR and tAH both 0 */
static const SramTiming AS7C34098A_15 = {
    .trc = NS(15), .taa = NS(15), .tace = NS(15), .toe = NS(6), .tba = NS(7), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(7), .tohz = NS(7),
    .tbhz = NS(7), .twc = NS(15), .taw = NS(10), .tcw = NS(10), .tbw = NS(10), .tas = NS(0),
    .twp = NS(10), .twp_oe_low = NS(15), .twr = NS(0), .tdw = NS(7), .tdh = NS(0),
    .twhz = NS(7), .tow = NS(3),
};

/* Read/write cycle tables, -20 column, VCC = 3.0 V to 3.6 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ = twhz; tWR and tAH both 0 */
static const SramTiming AS7C34098A_20 = {
    .trc = NS(20), .taa = NS(20), .tace = NS(20), .toe = NS(7), .tba = NS(8), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(9), .tohz = NS(9),
    .tbhz = NS(9), .twc = NS(20), .taw = NS(12), .tcw = NS(12), .tbw = NS(12), .tas = NS(0),
    .twp = NS(12), .twp_oe_low = NS(20), .twr = NS(0), .tdw = NS(9), .tdh = NS(0),
    .twhz = NS(9), .tow = NS(3),
};

/* 44-pin 400-mil SOJ and 44-pin TSOP 2 (one shared diagram). Data pins are I/O1..I/O16 -> DQ 0..15.
   LB# controls I/O1-I/O8, UB# controls I/O9-I/O16. */
#define AS7C34098A_PINS(X) \
    X(1, "A0", A, 0) \
    X(2, "A1", A, 1) \
    X(3, "A2", A, 2) \
    X(4, "A3", A, 3) \
    X(5, "A4", A, 4) \
    X(6, "CE#", CE_N, 0) \
    X(7, "I/O1", DQ, 0) \
    X(8, "I/O2", DQ, 1) \
    X(9, "I/O3", DQ, 2) \
    X(10, "I/O4", DQ, 3) \
    X(11, "VCC", VCC, 0) \
    X(12, "GND", GND, 0) \
    X(13, "I/O5", DQ, 4) \
    X(14, "I/O6", DQ, 5) \
    X(15, "I/O7", DQ, 6) \
    X(16, "I/O8", DQ, 7) \
    X(17, "WE#", WE_N, 0) \
    X(18, "A5", A, 5) \
    X(19, "A6", A, 6) \
    X(20, "A7", A, 7) \
    X(21, "A8", A, 8) \
    X(22, "A9", A, 9) \
    X(23, "A10", A, 10) \
    X(24, "A11", A, 11) \
    X(25, "A12", A, 12) \
    X(26, "A13", A, 13) \
    X(27, "A14", A, 14) \
    X(28, "NC", NC, 0) \
    X(29, "I/O9", DQ, 8) \
    X(30, "I/O10", DQ, 9) \
    X(31, "I/O11", DQ, 10) \
    X(32, "I/O12", DQ, 11) \
    X(33, "VCC", VCC, 0) \
    X(34, "GND", GND, 0) \
    X(35, "I/O13", DQ, 12) \
    X(36, "I/O14", DQ, 13) \
    X(37, "I/O15", DQ, 14) \
    X(38, "I/O16", DQ, 15) \
    X(39, "LB#", BE_N, 0) \
    X(40, "UB#", BE_N, 1) \
    X(41, "OE#", OE_N, 0) \
    X(42, "A15", A, 15) \
    X(43, "A16", A, 16) \
    X(44, "A17", A, 17)

/* Note: the ordering table prints "AS7C3498A-12JCN"; corrected to AS7C34098A-12JCN. */
#define AS7C34098A_PARTS(X) \
    X("AS7C34098A-10JCN", AS7C34098A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C34098A-10JIN", AS7C34098A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C34098A-12JCN", AS7C34098A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C34098A-12JIN", AS7C34098A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C34098A-15JCN", AS7C34098A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C34098A-15JIN", AS7C34098A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C34098A-20JCN", AS7C34098A_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C34098A-20JIN", AS7C34098A_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C34098A-10TCN", AS7C34098A_10, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C34098A-10TIN", AS7C34098A_10, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C34098A-12TCN", AS7C34098A_12, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C34098A-12TIN", AS7C34098A_12, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C34098A-15TCN", AS7C34098A_15, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C34098A-15TIN", AS7C34098A_15, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C34098A-20TCN", AS7C34098A_20, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C34098A-20TIN", AS7C34098A_20, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS7C34098A, "AS7C34098A", "512 KB", "256K x 16 CMOS fast SRAM, 3.3 V",
                 18, 16, AS7C34098A_PINS, AS7C34098A_PARTS)

/* 48-ball mini BGA (bottom-view diagram, rows A-H, columns 1-6, all 48 balls populated).
   The BGA diagram names data balls I/O0..I/O15 (unlike the SOJ's I/O1..I/O16) and power VDD. */
#define AS7C34098A_BGA_PINS(X) \
    X(1, "LB#", BE_N, 0) /* A1 */ \
    X(2, "OE#", OE_N, 0) /* A2 */ \
    X(3, "A0", A, 0) /* A3 */ \
    X(4, "A1", A, 1) /* A4 */ \
    X(5, "A2", A, 2) /* A5 */ \
    X(6, "N/C", NC, 0) /* A6 */ \
    X(7, "I/O8", DQ, 8) /* B1 */ \
    X(8, "UB#", BE_N, 1) /* B2 */ \
    X(9, "A3", A, 3) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "CE#", CE_N, 0) /* B5 */ \
    X(12, "I/O0", DQ, 0) /* B6 */ \
    X(13, "I/O9", DQ, 9) /* C1 */ \
    X(14, "I/O10", DQ, 10) /* C2 */ \
    X(15, "A5", A, 5) /* C3 */ \
    X(16, "A6", A, 6) /* C4 */ \
    X(17, "I/O1", DQ, 1) /* C5 */ \
    X(18, "I/O2", DQ, 2) /* C6 */ \
    X(19, "GND", GND, 0) /* D1 */ \
    X(20, "I/O11", DQ, 11) /* D2 */ \
    X(21, "A17", A, 17) /* D3 */ \
    X(22, "A7", A, 7) /* D4 */ \
    X(23, "I/O3", DQ, 3) /* D5 */ \
    X(24, "VDD", VCC, 0) /* D6 */ \
    X(25, "VDD", VCC, 0) /* E1 */ \
    X(26, "I/O12", DQ, 12) /* E2 */ \
    X(27, "NC", NC, 0) /* E3 */ \
    X(28, "A16", A, 16) /* E4 */ \
    X(29, "I/O4", DQ, 4) /* E5 */ \
    X(30, "GND", GND, 0) /* E6 */ \
    X(31, "I/O14", DQ, 14) /* F1 */ \
    X(32, "I/O13", DQ, 13) /* F2 */ \
    X(33, "A14", A, 14) /* F3 */ \
    X(34, "A15", A, 15) /* F4 */ \
    X(35, "I/O5", DQ, 5) /* F5 */ \
    X(36, "I/O6", DQ, 6) /* F6 */ \
    X(37, "I/O15", DQ, 15) /* G1 */ \
    X(38, "NC", NC, 0) /* G2 */ \
    X(39, "A12", A, 12) /* G3 */ \
    X(40, "A13", A, 13) /* G4 */ \
    X(41, "WE#", WE_N, 0) /* G5 */ \
    X(42, "I/O7", DQ, 7) /* G6 */ \
    X(43, "NC", NC, 0) /* H1 */ \
    X(44, "A8", A, 8) /* H2 */ \
    X(45, "A9", A, 9) /* H3 */ \
    X(46, "A10", A, 10) /* H4 */ \
    X(47, "A11", A, 11) /* H5 */ \
    X(48, "NC", NC, 0) /* H6 */

/* Note: the revision history mentions "AS7C34098A-10BIN/TR" but the ordering table lists only
   AS7C34098A-10BIN, so no separate tape-and-reel code is listed. */
#define AS7C34098A_BGA_PARTS(X) \
    X("AS7C34098A-10BIN", AS7C34098A_10, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS7C34098A_BGA, "AS7C34098A_BGA", "512 KB", "256K x 16 CMOS fast SRAM, 3.3 V",
                 18, 16, AS7C34098A_BGA_PINS, AS7C34098A_BGA_PARTS)

/* ---- AS7C4098A: 256K x 16 5.0 V CMOS fast SRAM (datasheets/sram/AS7C4098A.pdf) ---- */

/* Read/write cycle tables, -10 column, VCC = 4.5 V to 5.5 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ (max) = twhz; tWR and tAH both 0 */
static const SramTiming AS7C4098A_10 = {
    .trc = NS(10), .taa = NS(10), .tace = NS(10), .toe = NS(5), .tba = NS(5), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(5), .tohz = NS(5),
    .tbhz = NS(5), .twc = NS(10), .taw = NS(7), .tcw = NS(7), .tbw = NS(7), .tas = NS(0),
    .twp = NS(7), .twp_oe_low = NS(10), .twr = NS(0), .tdw = NS(5), .tdh = NS(0),
    .twhz = NS(5), .tow = NS(3),
};

/* Read/write cycle tables, -12 column, VCC = 4.5 V to 5.5 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ (max) = twhz; tWR and tAH both 0 */
static const SramTiming AS7C4098A_12 = {
    .trc = NS(12), .taa = NS(12), .tace = NS(12), .toe = NS(6), .tba = NS(6), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(6), .tohz = NS(6),
    .tbhz = NS(6), .twc = NS(12), .taw = NS(8), .tcw = NS(8), .tbw = NS(8), .tas = NS(0),
    .twp = NS(8), .twp_oe_low = NS(12), .twr = NS(0), .tdw = NS(6), .tdh = NS(0),
    .twhz = NS(6), .tow = NS(3),
};

/* Read/write cycle tables, -15 column, VCC = 4.5 V to 5.5 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ (max) = twhz; tWR and tAH both 0 */
static const SramTiming AS7C4098A_15 = {
    .trc = NS(15), .taa = NS(15), .tace = NS(15), .toe = NS(6), .tba = NS(7), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(7), .tohz = NS(7),
    .tbhz = NS(7), .twc = NS(15), .taw = NS(10), .tcw = NS(10), .tbw = NS(10), .tas = NS(0),
    .twp = NS(10), .twp_oe_low = NS(15), .twr = NS(0), .tdw = NS(7), .tdh = NS(0),
    .twhz = NS(7), .tow = NS(3),
};

/* Read/write cycle tables, -20 column, VCC = 4.5 V to 5.5 V. tWP1 (OE# high) = twp, tWP2 (OE# low) = twp_oe_low; tWZ (max) = twhz; tWR and tAH both 0 */
static const SramTiming AS7C4098A_20 = {
    .trc = NS(20), .taa = NS(20), .tace = NS(20), .toe = NS(6), .tba = NS(8), .toh = NS(3),
    .tclz = NS(3), .tolz = NS(0), .tblz = NS(0), .tchz = NS(9), .tohz = NS(9),
    .tbhz = NS(9), .twc = NS(20), .taw = NS(12), .tcw = NS(12), .tbw = NS(12), .tas = NS(0),
    .twp = NS(12), .twp_oe_low = NS(20), .twr = NS(0), .tdw = NS(9), .tdh = NS(0),
    .twhz = NS(9), .tow = NS(3),
};

/* 44-pin 400-mil SOJ and 44-pin TSOP 2 (one shared diagram). Data pins are I/O1..I/O16 -> DQ 0..15.
   LB# controls I/O1-I/O8, UB# controls I/O9-I/O16. */
#define AS7C4098A_PINS(X) \
    X(1, "A0", A, 0) \
    X(2, "A1", A, 1) \
    X(3, "A2", A, 2) \
    X(4, "A3", A, 3) \
    X(5, "A4", A, 4) \
    X(6, "CE#", CE_N, 0) \
    X(7, "I/O1", DQ, 0) \
    X(8, "I/O2", DQ, 1) \
    X(9, "I/O3", DQ, 2) \
    X(10, "I/O4", DQ, 3) \
    X(11, "VCC", VCC, 0) \
    X(12, "GND", GND, 0) \
    X(13, "I/O5", DQ, 4) \
    X(14, "I/O6", DQ, 5) \
    X(15, "I/O7", DQ, 6) \
    X(16, "I/O8", DQ, 7) \
    X(17, "WE#", WE_N, 0) \
    X(18, "A5", A, 5) \
    X(19, "A6", A, 6) \
    X(20, "A7", A, 7) \
    X(21, "A8", A, 8) \
    X(22, "A9", A, 9) \
    X(23, "A10", A, 10) \
    X(24, "A11", A, 11) \
    X(25, "A12", A, 12) \
    X(26, "A13", A, 13) \
    X(27, "A14", A, 14) \
    X(28, "NC", NC, 0) \
    X(29, "I/O9", DQ, 8) \
    X(30, "I/O10", DQ, 9) \
    X(31, "I/O11", DQ, 10) \
    X(32, "I/O12", DQ, 11) \
    X(33, "VCC", VCC, 0) \
    X(34, "GND", GND, 0) \
    X(35, "I/O13", DQ, 12) \
    X(36, "I/O14", DQ, 13) \
    X(37, "I/O15", DQ, 14) \
    X(38, "I/O16", DQ, 15) \
    X(39, "LB#", BE_N, 0) \
    X(40, "UB#", BE_N, 1) \
    X(41, "OE#", OE_N, 0) \
    X(42, "A15", A, 15) \
    X(43, "A16", A, 16) \
    X(44, "A17", A, 17)

#define AS7C4098A_PARTS(X) \
    X("AS7C4098A-10JCN", AS7C4098A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4098A-10JIN", AS7C4098A_10, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4098A-12JCN", AS7C4098A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4098A-12JIN", AS7C4098A_12, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4098A-15JCN", AS7C4098A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4098A-15JIN", AS7C4098A_15, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4098A-20JCN", AS7C4098A_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4098A-20JIN", AS7C4098A_20, PACKAGE_TYPE_SOJ, 0) \
    X("AS7C4098A-10TCN", AS7C4098A_10, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4098A-10TIN", AS7C4098A_10, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4098A-12TCN", AS7C4098A_12, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4098A-12TIN", AS7C4098A_12, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4098A-15TCN", AS7C4098A_15, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4098A-15TIN", AS7C4098A_15, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4098A-20TCN", AS7C4098A_20, PACKAGE_TYPE_TSOP_II, 0) \
    X("AS7C4098A-20TIN", AS7C4098A_20, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS7C4098A, "AS7C4098A", "512 KB", "256K x 16 CMOS fast SRAM, 5 V",
                 18, 16, AS7C4098A_PINS, AS7C4098A_PARTS)

/* ---- AS7C38098A: 512K x 16 bit high speed CMOS SRAM, 3.3 V (datasheets/sram/AS7C38098A.pdf) ---- */

/* AC electrical characteristics (read and write cycle tables), AS7C38098A-10, VCC = 2.7 V to 3.6 V (the only grade) */
static const SramTiming AS7C38098A_10 = {
    .trc = NS(10), .taa = NS(10), .tace = NS(10), .toe = NS(4.5), .tba = NS(4.5),
    .toh = NS(2), .tclz = NS(2), .tolz = NS(0), .tblz = NS(0), .tchz = NS(4), .tohz = NS(4),
    .tbhz = NS(4), .twc = NS(10), .taw = NS(8), .tcw = NS(8), .tbw = NS(8), .tas = NS(0),
    .twp = NS(8), .twr = NS(0), .tdw = NS(6), .tdh = NS(0), .twhz = NS(4), .tow = NS(2),
};

/* 44-pin 400-mil TSOP II. */
#define AS7C38098A_PINS(X) \
    X(1, "A4", A, 4) \
    X(2, "A3", A, 3) \
    X(3, "A2", A, 2) \
    X(4, "A1", A, 1) \
    X(5, "A0", A, 0) \
    X(6, "CE#", CE_N, 0) \
    X(7, "DQ0", DQ, 0) \
    X(8, "DQ1", DQ, 1) \
    X(9, "DQ2", DQ, 2) \
    X(10, "DQ3", DQ, 3) \
    X(11, "Vcc", VCC, 0) \
    X(12, "Vss", GND, 0) \
    X(13, "DQ4", DQ, 4) \
    X(14, "DQ5", DQ, 5) \
    X(15, "DQ6", DQ, 6) \
    X(16, "DQ7", DQ, 7) \
    X(17, "WE#", WE_N, 0) \
    X(18, "A18", A, 18) \
    X(19, "A17", A, 17) \
    X(20, "A16", A, 16) \
    X(21, "A15", A, 15) \
    X(22, "A14", A, 14) \
    X(23, "A13", A, 13) \
    X(24, "A12", A, 12) \
    X(25, "A11", A, 11) \
    X(26, "A10", A, 10) \
    X(27, "A9", A, 9) \
    X(28, "A8", A, 8) \
    X(29, "DQ8", DQ, 8) \
    X(30, "DQ9", DQ, 9) \
    X(31, "DQ10", DQ, 10) \
    X(32, "DQ11", DQ, 11) \
    X(33, "Vcc", VCC, 0) \
    X(34, "Vss", GND, 0) \
    X(35, "DQ12", DQ, 12) \
    X(36, "DQ13", DQ, 13) \
    X(37, "DQ14", DQ, 14) \
    X(38, "DQ15", DQ, 15) \
    X(39, "LB#", BE_N, 0) \
    X(40, "UB#", BE_N, 1) \
    X(41, "OE#", OE_N, 0) \
    X(42, "A7", A, 7) \
    X(43, "A6", A, 6) \
    X(44, "A5", A, 5)

#define AS7C38098A_PARTS(X) \
    X("AS7C38098A-10TIN", AS7C38098A_10, PACKAGE_TYPE_TSOP_II, 0)

SRAM_DEFINE_CHIP(AS7C38098A, "AS7C38098A", "1024 KB", "512K x 16 high speed CMOS SRAM, 3.3 V",
                 19, 16, AS7C38098A_PINS, AS7C38098A_PARTS)

/* 48-ball 6 x 8 mm TFBGA (rows A-H, columns 1-6, all 48 balls populated). */
#define AS7C38098A_BGA_PINS(X) \
    X(1, "LB#", BE_N, 0) /* A1 */ \
    X(2, "OE#", OE_N, 0) /* A2 */ \
    X(3, "A0", A, 0) /* A3 */ \
    X(4, "A1", A, 1) /* A4 */ \
    X(5, "A2", A, 2) /* A5 */ \
    X(6, "NC", NC, 0) /* A6 */ \
    X(7, "DQ8", DQ, 8) /* B1 */ \
    X(8, "UB#", BE_N, 1) /* B2 */ \
    X(9, "A3", A, 3) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "CE#", CE_N, 0) /* B5 */ \
    X(12, "DQ0", DQ, 0) /* B6 */ \
    X(13, "DQ9", DQ, 9) /* C1 */ \
    X(14, "DQ10", DQ, 10) /* C2 */ \
    X(15, "A5", A, 5) /* C3 */ \
    X(16, "A6", A, 6) /* C4 */ \
    X(17, "DQ1", DQ, 1) /* C5 */ \
    X(18, "DQ2", DQ, 2) /* C6 */ \
    X(19, "Vss", GND, 0) /* D1 */ \
    X(20, "DQ11", DQ, 11) /* D2 */ \
    X(21, "A17", A, 17) /* D3 */ \
    X(22, "A7", A, 7) /* D4 */ \
    X(23, "DQ3", DQ, 3) /* D5 */ \
    X(24, "Vcc", VCC, 0) /* D6 */ \
    X(25, "Vcc", VCC, 0) /* E1 */ \
    X(26, "DQ12", DQ, 12) /* E2 */ \
    X(27, "NC", NC, 0) /* E3 */ \
    X(28, "A16", A, 16) /* E4 */ \
    X(29, "DQ4", DQ, 4) /* E5 */ \
    X(30, "Vss", GND, 0) /* E6 */ \
    X(31, "DQ14", DQ, 14) /* F1 */ \
    X(32, "DQ13", DQ, 13) /* F2 */ \
    X(33, "A14", A, 14) /* F3 */ \
    X(34, "A15", A, 15) /* F4 */ \
    X(35, "DQ5", DQ, 5) /* F5 */ \
    X(36, "DQ6", DQ, 6) /* F6 */ \
    X(37, "DQ15", DQ, 15) /* G1 */ \
    X(38, "NC", NC, 0) /* G2 */ \
    X(39, "A12", A, 12) /* G3 */ \
    X(40, "A13", A, 13) /* G4 */ \
    X(41, "WE#", WE_N, 0) /* G5 */ \
    X(42, "DQ7", DQ, 7) /* G6 */ \
    X(43, "A18", A, 18) /* H1 */ \
    X(44, "A8", A, 8) /* H2 */ \
    X(45, "A9", A, 9) /* H3 */ \
    X(46, "A10", A, 10) /* H4 */ \
    X(47, "A11", A, 11) /* H5 */ \
    X(48, "NC", NC, 0) /* H6 */

#define AS7C38098A_BGA_PARTS(X) \
    X("AS7C38098A-10BIN", AS7C38098A_10, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS7C38098A_BGA, "AS7C38098A_BGA", "1024 KB", "512K x 16 high speed CMOS SRAM, 3.3 V",
                 19, 16, AS7C38098A_BGA_PINS, AS7C38098A_BGA_PARTS)

/* ---- AS7C316098A: 1024K x 16 high-speed CMOS SRAM (datasheets/sram/AS7C316098A.pdf) ---- */

/* -10: AC electrical characteristics, read and write cycle tables (VCC = 2.7-3.6 V, the only table). */
static const SramTiming AS7C316098A_10 = {
    .trc = NS(10), .taa = NS(10), .tace = NS(10), .toe = NS(4.5), .tba = NS(4.5),
    .toh = NS(2), .tclz = NS(2), .tolz = NS(0), .tblz = NS(0), .tchz = NS(4),
    .tohz = NS(4), .tbhz = NS(4), .twc = NS(10), .taw = NS(8), .tcw = NS(8),
    .tbw = NS(8), .tas = NS(0), .twp = NS(8), .twr = NS(0), .tdw = NS(6), .tdh = NS(0),
    .twhz = NS(4), .tow = NS(2),
};

/* 48-pin TSOP I (12 x 20 mm). */
#define AS7C316098A_TSOP_PINS(X) \
    X(1, "A4", A, 4) \
    X(2, "A3", A, 3) \
    X(3, "A2", A, 2) \
    X(4, "A1", A, 1) \
    X(5, "A0", A, 0) \
    X(6, "NC", NC, 0) \
    X(7, "CE#", CE_N, 0) \
    X(8, "DQ0", DQ, 0) \
    X(9, "DQ1", DQ, 1) \
    X(10, "DQ2", DQ, 2) \
    X(11, "DQ3", DQ, 3) \
    X(12, "VCC", VCC, 0) \
    X(13, "VSS", GND, 0) \
    X(14, "DQ4", DQ, 4) \
    X(15, "DQ5", DQ, 5) \
    X(16, "DQ6", DQ, 6) \
    X(17, "DQ7", DQ, 7) \
    X(18, "WE#", WE_N, 0) \
    X(19, "NC", NC, 0) \
    X(20, "A18", A, 18) \
    X(21, "A17", A, 17) \
    X(22, "A16", A, 16) \
    X(23, "A15", A, 15) \
    X(24, "A14", A, 14) \
    X(25, "A13", A, 13) \
    X(26, "A12", A, 12) \
    X(27, "A11", A, 11) \
    X(28, "A10", A, 10) \
    X(29, "A9", A, 9) \
    X(30, "A8", A, 8) \
    X(31, "NC", NC, 0) \
    X(32, "DQ8", DQ, 8) \
    X(33, "DQ9", DQ, 9) \
    X(34, "DQ10", DQ, 10) \
    X(35, "DQ11", DQ, 11) \
    X(36, "VCC", VCC, 0) \
    X(37, "VSS", GND, 0) \
    X(38, "DQ12", DQ, 12) \
    X(39, "DQ13", DQ, 13) \
    X(40, "DQ14", DQ, 14) \
    X(41, "DQ15", DQ, 15) \
    X(42, "LB#", BE_N, 0) \
    X(43, "UB#", BE_N, 1) \
    X(44, "OE#", OE_N, 0) \
    X(45, "A19", A, 19) \
    X(46, "A7", A, 7) \
    X(47, "A6", A, 6) \
    X(48, "A5", A, 5)

#define AS7C316098A_TSOP_PARTS(X) \
    X("AS7C316098A-10TIN", AS7C316098A_10, PACKAGE_TYPE_TSOP_I, 0)

SRAM_DEFINE_CHIP(AS7C316098A_TSOP, "AS7C316098A_TSOP", "2048 KB", "1024K x 16 high-speed CMOS SRAM, 3.3 V",
                 20, 16, AS7C316098A_TSOP_PINS, AS7C316098A_TSOP_PARTS)

/* 48-ball TFBGA (6 x 8 mm), all 48 balls populated, numbered row-major A1..H6. */
#define AS7C316098A_BGA_PINS(X) \
    X(1, "LB#", BE_N, 0) /* A1 */ \
    X(2, "OE#", OE_N, 0) /* A2 */ \
    X(3, "A0", A, 0) /* A3 */ \
    X(4, "A1", A, 1) /* A4 */ \
    X(5, "A2", A, 2) /* A5 */ \
    X(6, "NC", NC, 0) /* A6 */ \
    X(7, "DQ8", DQ, 8) /* B1 */ \
    X(8, "UB#", BE_N, 1) /* B2 */ \
    X(9, "A3", A, 3) /* B3 */ \
    X(10, "A4", A, 4) /* B4 */ \
    X(11, "CE#", CE_N, 0) /* B5 */ \
    X(12, "DQ0", DQ, 0) /* B6 */ \
    X(13, "DQ9", DQ, 9) /* C1 */ \
    X(14, "DQ10", DQ, 10) /* C2 */ \
    X(15, "A5", A, 5) /* C3 */ \
    X(16, "A6", A, 6) /* C4 */ \
    X(17, "DQ1", DQ, 1) /* C5 */ \
    X(18, "DQ2", DQ, 2) /* C6 */ \
    X(19, "VSS", GND, 0) /* D1 */ \
    X(20, "DQ11", DQ, 11) /* D2 */ \
    X(21, "A17", A, 17) /* D3 */ \
    X(22, "A7", A, 7) /* D4 */ \
    X(23, "DQ3", DQ, 3) /* D5 */ \
    X(24, "VCC", VCC, 0) /* D6 */ \
    X(25, "VCC", VCC, 0) /* E1 */ \
    X(26, "DQ12", DQ, 12) /* E2 */ \
    X(27, "NC", NC, 0) /* E3 */ \
    X(28, "A16", A, 16) /* E4 */ \
    X(29, "DQ4", DQ, 4) /* E5 */ \
    X(30, "VSS", GND, 0) /* E6 */ \
    X(31, "DQ14", DQ, 14) /* F1 */ \
    X(32, "DQ13", DQ, 13) /* F2 */ \
    X(33, "A14", A, 14) /* F3 */ \
    X(34, "A15", A, 15) /* F4 */ \
    X(35, "DQ5", DQ, 5) /* F5 */ \
    X(36, "DQ6", DQ, 6) /* F6 */ \
    X(37, "DQ15", DQ, 15) /* G1 */ \
    X(38, "A19", A, 19) /* G2 */ \
    X(39, "A12", A, 12) /* G3 */ \
    X(40, "A13", A, 13) /* G4 */ \
    X(41, "WE#", WE_N, 0) /* G5 */ \
    X(42, "DQ7", DQ, 7) /* G6 */ \
    X(43, "A18", A, 18) /* H1 */ \
    X(44, "A8", A, 8) /* H2 */ \
    X(45, "A9", A, 9) /* H3 */ \
    X(46, "A10", A, 10) /* H4 */ \
    X(47, "A11", A, 11) /* H5 */ \
    X(48, "NC", NC, 0) /* H6 */

#define AS7C316098A_BGA_PARTS(X) \
    X("AS7C316098A-10BIN", AS7C316098A_10, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS7C316098A_BGA, "AS7C316098A_BGA", "2048 KB", "1024K x 16 high-speed CMOS SRAM, 3.3 V",
                 20, 16, AS7C316098A_BGA_PINS, AS7C316098A_BGA_PARTS)

/* ---- AS7C351232: 512K x 32 high-speed CMOS SRAM (datasheets/sram/AS7C351232-10BIN.pdf) ---- */

/* AC characteristics, AS7C351232-10BIN, VCC = 2.7 V to 3.6 V (the only table). */
static const SramTiming AS7C351232_10 = {
    .trc = NS(10), .taa = NS(10), .tace = NS(10), .toe = NS(4.5), .tba = NS(4.5),
    .toh = NS(2), .tclz = NS(2), .tolz = NS(0), .tblz = NS(0),
    .tchz = NS(4), .tohz = NS(4), .tbhz = NS(4),
    .twc = NS(10), .taw = NS(8), .tcw = NS(8), .tbw = NS(8), .tas = NS(0),
    .twp = NS(8), .twr = NS(0), .tdw = NS(6), .tdh = NS(0), .twhz = NS(4),
    .tow = NS(2),
};

/* 90-ball TFBGA (8 x 13 mm), the only package. Columns 1-3 and 7-9 are
   populated; rows A-H, J-N, P, R. */
/* Note: the truth table header labels the byte-enable columns
   "B0# B1# B0# B1#"; the last two are B2#/B3# (the feature list gives
   B0# DQ0-7, B1# DQ8-15, B2# DQ16-23, B3# DQ24-31, and the table's rows
   agree with that). */
#define AS7C351232_PINS(X) \
    X(1, "DQ1", DQ, 1) /* A1 */ \
    X(2, "DQ0", DQ, 0) /* A2 */ \
    X(3, "Vss", GND, 0) /* A3 */ \
    X(4, "Vcc", VCC, 0) /* A7 */ \
    X(5, "DQ31", DQ, 31) /* A8 */ \
    X(6, "DQ30", DQ, 30) /* A9 */ \
    X(7, "DQ2", DQ, 2) /* B1 */ \
    X(8, "Vcc", VCC, 0) /* B2 */ \
    X(9, "Vss", GND, 0) /* B3 */ \
    X(10, "Vcc", VCC, 0) /* B7 */ \
    X(11, "Vss", GND, 0) /* B8 */ \
    X(12, "DQ29", DQ, 29) /* B9 */ \
    X(13, "Vss", GND, 0) /* C1 */ \
    X(14, "DQ3", DQ, 3) /* C2 */ \
    X(15, "DQ4", DQ, 4) /* C3 */ \
    X(16, "DQ27", DQ, 27) /* C7 */ \
    X(17, "DQ28", DQ, 28) /* C8 */ \
    X(18, "Vcc", VCC, 0) /* C9 */ \
    X(19, "Vss", GND, 0) /* D1 */ \
    X(20, "DQ6", DQ, 6) /* D2 */ \
    X(21, "DQ5", DQ, 5) /* D3 */ \
    X(22, "DQ26", DQ, 26) /* D7 */ \
    X(23, "DQ25", DQ, 25) /* D8 */ \
    X(24, "Vcc", VCC, 0) /* D9 */ \
    X(25, "Vcc", VCC, 0) /* E1 */ \
    X(26, "DQ7", DQ, 7) /* E2 */ \
    X(27, "NC", NC, 0) /* E3 */ \
    X(28, "NC", NC, 0) /* E7 */ \
    X(29, "DQ24", DQ, 24) /* E8 */ \
    X(30, "Vss", GND, 0) /* E9 */ \
    X(31, "Vss", GND, 0) /* F1 */ \
    X(32, "B0#", BE_N, 0) /* F2 */ \
    X(33, "A3", A, 3) /* F3 */ \
    X(34, "A4", A, 4) /* F7 */ \
    X(35, "B3#", BE_N, 3) /* F8 */ \
    X(36, "Vcc", VCC, 0) /* F9 */ \
    X(37, "A0", A, 0) /* G1 */ \
    X(38, "A1", A, 1) /* G2 */ \
    X(39, "A2", A, 2) /* G3 */ \
    X(40, "A10", A, 10) /* G7 */ \
    X(41, "A5", A, 5) /* G8 */ \
    X(42, "A6", A, 6) /* G9 */ \
    X(43, "A15", A, 15) /* H1 */ \
    X(44, "A14", A, 14) /* H2 */ \
    X(45, "A13", A, 13) /* H3 */ \
    X(46, "A8", A, 8) /* H7 */ \
    X(47, "A7", A, 7) /* H8 */ \
    X(48, "A11", A, 11) /* H9 */ \
    X(49, "CE2", CE, 0) /* J1 */ \
    X(50, "A17", A, 17) /* J2 */ \
    X(51, "A16", A, 16) /* J3 */ \
    X(52, "A9", A, 9) /* J7 */ \
    X(53, "A12", A, 12) /* J8 */ \
    X(54, "CE#", CE_N, 0) /* J9 */ \
    X(55, "B1#", BE_N, 1) /* K1 */ \
    X(56, "NC", NC, 0) /* K2 */ \
    X(57, "A18", A, 18) /* K3 */ \
    X(58, "OE#", OE_N, 0) /* K7 */ \
    X(59, "WE#", WE_N, 0) /* K8 */ \
    X(60, "B2#", BE_N, 2) /* K9 */ \
    X(61, "Vcc", VCC, 0) /* L1 */ \
    X(62, "DQ8", DQ, 8) /* L2 */ \
    X(63, "Vss", GND, 0) /* L3 */ \
    X(64, "Vcc", VCC, 0) /* L7 */ \
    X(65, "DQ23", DQ, 23) /* L8 */ \
    X(66, "Vss", GND, 0) /* L9 */ \
    X(67, "Vss", GND, 0) /* M1 */ \
    X(68, "DQ9", DQ, 9) /* M2 */ \
    X(69, "DQ10", DQ, 10) /* M3 */ \
    X(70, "DQ21", DQ, 21) /* M7 */ \
    X(71, "DQ22", DQ, 22) /* M8 */ \
    X(72, "Vcc", VCC, 0) /* M9 */ \
    X(73, "Vss", GND, 0) /* N1 */ \
    X(74, "DQ12", DQ, 12) /* N2 */ \
    X(75, "DQ11", DQ, 11) /* N3 */ \
    X(76, "DQ20", DQ, 20) /* N7 */ \
    X(77, "DQ19", DQ, 19) /* N8 */ \
    X(78, "Vcc", VCC, 0) /* N9 */ \
    X(79, "DQ13", DQ, 13) /* P1 */ \
    X(80, "Vcc", VCC, 0) /* P2 */ \
    X(81, "Vss", GND, 0) /* P3 */ \
    X(82, "Vcc", VCC, 0) /* P7 */ \
    X(83, "Vss", GND, 0) /* P8 */ \
    X(84, "DQ18", DQ, 18) /* P9 */ \
    X(85, "DQ14", DQ, 14) /* R1 */ \
    X(86, "DQ15", DQ, 15) /* R2 */ \
    X(87, "Vss", GND, 0) /* R3 */ \
    X(88, "Vcc", VCC, 0) /* R7 */ \
    X(89, "DQ16", DQ, 16) /* R8 */ \
    X(90, "DQ17", DQ, 17) /* R9 */

#define AS7C351232_PARTS(X) \
    X("AS7C351232-10BIN", AS7C351232_10, PACKAGE_TYPE_TFBGA, 0) \
    X("AS7C351232-10BINTR", AS7C351232_10, PACKAGE_TYPE_TFBGA, 0)

SRAM_DEFINE_CHIP(AS7C351232, "AS7C351232", "2048 KB", "512K x 32 high-speed CMOS SRAM",
                 19, 32, AS7C351232_PINS, AS7C351232_PARTS)
