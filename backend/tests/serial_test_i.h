/*
 * plan-12-I cases I1-I9: the I2C/SPI-to-UART bridges on the ns16550 model
 * and the sc18is604 SPI-to-I2C bridge, on the shipped device objects
 * (tests/fixtures/serial/bridges.json, merge.py). Included by serial_test.c
 * after the plan-12-J block (it uses its MB bench, SPI and I2C masters and
 * the JF test target).
 *
 * The bridges run on a 10 MHz XTAL1 (bit time 1.6 us x divisor). I2C is the
 * bench master at 400 kHz with weak pull-ups on SDA, SCL and IRQ; SPI is
 * bit-banged in mode 0 (mode 3 for the SC18IS604).
 */

#define XT_HALF NS(50)
#define BIT1 NS(1600)

typedef struct {
    MB *b;
    int spi;
    unsigned addr;        /* 7-bit I2C address */
    const char *a0, *a1;
} Br;

static const char *const BR_PULLS_SC[] = {"SCL/SCLK", "SDA", "IRQ", NULL};
static const char *const BR_PULLS_XR[] = {"SCL", "SDA", "IRQ", NULL};

static void br_reset(Br *r) {
    MB *b = r->b;

    mb_set(b, "RESET", LOGIC_0);
    mb_run(b, NS(4000));
    mb_set(b, "RESET", LOGIC_1);
    mb_run(b, NS(4000));
}

/* A bridge on a bench. xr: the MaxLinear pin names. ties as mb_open. */
static Br br_open(const char *part, int spi, int xr, const char *const *ties) {
    Br r;
    MB *b;
    size_t k;
    int a0_tied = 0;
    int a1_tied = 0;

    r.b = b = mb_open(part, "XTAL1", XT_HALF, ties, xr ? BR_PULLS_XR : BR_PULLS_SC);
    r.spi = spi;
    r.addr = xr ? 0x30 : 0x48;
    r.a0 = xr ? "A0/CS" : "CS/A0";
    r.a1 = xr ? "A1/SI" : "SI/A1";
    b->cs = r.a0;
    b->mosi = r.a1;
    b->sclk = xr ? "SCL" : "SCL/SCLK";
    b->scl = b->sclk;
    b->miso = "SO";
    b->sda = "SDA";
    for (k = 0; ties != NULL && ties[k] != NULL; k += 2) {
        a0_tied |= strcmp(ties[k], r.a0) == 0;
        a1_tied |= strcmp(ties[k], r.a1) == 0;
    }
    mb_set(b, "I2C/SPI", spi ? LOGIC_0 : LOGIC_1);
    for (k = 0; k < b->part->pin_count; k++) {
        const char *name = b->part->pins[k].name;

        /* UART inputs idle: RX mark, CTS inactive. */
        if (strncmp(name, "RX", 2) == 0 || strncmp(name, "CTS", 3) == 0) {
            mb_set(b, name, LOGIC_1);
        }
    }
    if (spi) {
        mb_set(b, b->cs, LOGIC_1);
        mb_set(b, b->sclk, LOGIC_0);
        mb_set(b, b->mosi, LOGIC_0);
    } else {
        if (!a0_tied) {
            mb_set(b, r.a0, LOGIC_1);
        }
        if (!a1_tied) {
            mb_set(b, r.a1, LOGIC_1);
        }
        mb_set(b, b->scl, LOGIC_Z);
        mb_set(b, "SDA", LOGIC_Z);
    }
    br_reset(&r);
    return r;
}

static unsigned br_sub(unsigned reg, unsigned ch) {
    return ((reg & 15u) << 3) | ((ch & 3u) << 1);
}

/* A register write: the I2C ACK of the data byte (0 = ACK) or 0 for SPI. */
static LogicValue br_w(Br *r, unsigned ch, unsigned reg, unsigned v) {
    MB *b = r->b;
    LogicValue ack;

    if (r->spi) {
        mb_spi(b, (br_sub(reg, ch) << 8) | (v & 0xFFu), 16, NULL, 0);
        return LOGIC_0;
    }
    i2c_start(b);
    i2c_write(b, r->addr << 1);
    i2c_write(b, br_sub(reg, ch));
    ack = i2c_write(b, v);
    i2c_stop(b);
    return ack;
}

/* A register read; text gets the 8 bits as "01XZ". */
static unsigned br_r(Br *r, unsigned ch, unsigned reg, char *text) {
    MB *b = r->b;
    char all[20];
    unsigned v;

    if (r->spi) {
        v = mb_spi(b, (0x80u | br_sub(reg, ch)) << 8, 16, all, 0) & 0xFFu;
        if (text != NULL) {
            memcpy(text, all + 8, 9);
        }
        return v;
    }
    i2c_start(b);
    i2c_write(b, r->addr << 1);
    i2c_write(b, br_sub(reg, ch));
    i2c_restart(b);
    i2c_write(b, (r->addr << 1) | 1u);
    v = i2c_read(b, 0, text);
    i2c_stop(b);
    return v;
}

static void br_expect(Br *r, unsigned ch, unsigned reg, unsigned want, const char *what) {
    mb_expect_u(br_r(r, ch, reg, NULL), want, what);
}

static void br_expect_text(Br *r, unsigned ch, unsigned reg, const char *want, const char *what) {
    char text[16];
    char message[160];

    br_r(r, ch, reg, text);
    snprintf(message, sizeof message, "%s: read %s, want %s", what, text, want);
    check(strcmp(text, want) == 0, message);
}

/* 8N1 with divisor d. */
static void br_program(Br *r, unsigned ch, unsigned d) {
    br_w(r, ch, 3, 0x80);
    br_w(r, ch, 0, d & 0xFFu);
    br_w(r, ch, 1, d >> 8);
    br_w(r, ch, 3, 0x03);
}

/* SPI mode 0, one frame of n bytes; MISO bytes into in (may be NULL). */
static void br_spi_frame(MB *b, const uint8_t *out, unsigned n, uint8_t *in, char *text) {
    unsigned i;
    unsigned bit;

    mb_set(b, b->cs, LOGIC_0);
    mb_run(b, SPI_HALF);
    for (i = 0; i < n; i++) {
        uint8_t v = 0;

        for (bit = 0; bit < 8; bit++) {
            LogicValue l;

            mb_set(b, b->mosi, (out[i] >> (7 - bit)) & 1u ? LOGIC_1 : LOGIC_0);
            mb_run(b, SPI_HALF);
            l = mb_get(b, b->miso);
            if (text != NULL) {
                text[i * 8 + bit] = "01XZ"[l];
            }
            v = (uint8_t)((v << 1) | (l == LOGIC_1 ? 1u : 0u));
            mb_set(b, b->sclk, LOGIC_1);
            mb_run(b, SPI_HALF);
            mb_set(b, b->sclk, LOGIC_0);
        }
        if (in != NULL) {
            in[i] = v;
        }
    }
    if (text != NULL) {
        text[n * 8] = '\0';
    }
    mb_run(b, SPI_HALF);
    mb_set(b, b->cs, LOGIC_1);
    mb_run(b, SPI_HALF);
}

/* The time (ps) TX stays at `level` from now (it must be there already). */
static long long br_width(MB *b, const char *pin, LogicValue level, SimTime limit) {
    SimTime spent = 0;
    SimTime step = NS(20);

    while (spent <= limit && mb_get(b, pin) == level) {
        mb_run(b, step);
        spent += step;
    }
    return (long long)spent;
}

static int near(long long got, long long want, long long tol) {
    return got >= want - tol && got <= want + tol;
}

static void expect_near(long long got, long long want, long long tol, const char *what) {
    char message[200];

    snprintf(message, sizeof message, "%s: %lld ns, want %lld ns", what, got / 1000, want / 1000);
    check(near(got, want, tol), message);
}

/* One bit time of a 0x55 frame on `pin`: the width of data bit 0 (a 1
   after the start bit), measured from its edges. */
static long long br_bit_time(MB *b, const char *pin) {
    if (mb_wait(b, pin, LOGIC_0, NS(400000), NS(20)) < 0 ||
        mb_wait(b, pin, LOGIC_1, NS(400000), NS(20)) < 0) {
        return -1;
    }
    return br_width(b, pin, LOGIC_1, NS(400000));
}

/* Waits for a start bit on `pin` and returns the frame "start d0..d7 stop". */
static void br_frame(MB *b, const char *pin, SimTime bt, char *out, SimTime limit) {
    if (mb_wait(b, pin, LOGIC_0, limit, NS(20)) < 0) {
        strcpy(out, "(none)");
        return;
    }
    mb_frame(b, pin, bt, 9, out);
}

/* The address of strap combination (a1 tie, a0 tie) per the datasheet table. */
static unsigned nxp_address(int a1, int a0) {
    static const unsigned BASE[4] = {0x4C, 0x48, 0x50, 0x54};   /* VSS, VDD, SCL, SDA */
    static const unsigned OFF[4] = {1, 0, 2, 3};

    return BASE[a1] + OFF[a0];
}

static unsigned xr_address(int a1, int a0) {
    static const unsigned BASE[4] = {0x34, 0x30, 0x30, 0x34};   /* GND/SDA 0x34, VCC/SCL 0x30 */
    static const unsigned OFF[4] = {1, 0, 2, 3};

    return BASE[a1] + OFF[a0];
}

/* Every A1/A0 tie (low, high, SCL, SDA): the table's address ACKs, the one
   above it NACKs. */
static void address_table(const char *part, int xr) {
    static const char *const TIE_NAME[4] = {"VSS", "VDD", "SCL", "SDA"};
    int a1;
    int a0;
    char message[160];

    for (a1 = 0; a1 < 4; a1++) {
        for (a0 = 0; a0 < 4; a0++) {
            const char *ties[5] = {NULL, NULL, NULL, NULL, NULL};
            const char *scl = xr ? "SCL" : "SCL/SCLK";
            const char *a0n = xr ? "A0/CS" : "CS/A0";
            const char *a1n = xr ? "A1/SI" : "SI/A1";
            unsigned n = 0;
            unsigned want = xr ? xr_address(a1, a0) : nxp_address(a1, a0);
            Br r;
            LogicValue ack;
            LogicValue other;

            if (a0 >= 2) {
                ties[n++] = a0n;
                ties[n++] = a0 == 2 ? scl : "SDA";
            }
            if (a1 >= 2) {
                ties[n++] = a1n;
                ties[n++] = a1 == 2 ? scl : "SDA";
            }
            r = br_open(part, 0, xr, ties);
            if (a0 < 2) {
                mb_set(r.b, a0n, a0 ? LOGIC_1 : LOGIC_0);
            }
            if (a1 < 2) {
                mb_set(r.b, a1n, a1 ? LOGIC_1 : LOGIC_0);
            }
            mb_run(r.b, NS(2000));
            i2c_start(r.b);
            ack = i2c_write(r.b, want << 1);
            i2c_stop(r.b);
            i2c_start(r.b);
            other = i2c_write(r.b, (want ^ 0x08u) << 1);
            i2c_stop(r.b);
            snprintf(message, sizeof message, "%s: A1 %s, A0 %s -> 0x%02X ACK, 0x%02X NACK", part,
                     TIE_NAME[a1], TIE_NAME[a0], want, want ^ 0x08u);
            check(ack == LOGIC_0 && other == LOGIC_1, message);
            mb_close(r.b);
        }
    }
}

/* ---- I1 SC16IS750 over I2C ------------------------------------------------------------- */

static void case_i1(void) {
    Br r;
    MB *b;
    char frame[16];
    uint16_t frames[8];
    size_t n;

    case_begin();
    address_table("SFX-SC16IS750", 0);
    r = br_open("SFX-SC16IS750", 0, 0, NULL);
    b = r.b;
    br_expect(&r, 0, 3, 0x1D, "LCR after reset (lcrReset 0x1D)");
    br_expect(&r, 0, 8, 0x40, "TXLVL after reset");
    br_expect(&r, 0, 9, 0x00, "RXLVL after reset");
    br_expect(&r, 0, 2, 0x01, "IIR after reset");
    br_expect(&r, 0, 5, 0x60, "LSR after reset");
    mb_expect_pin(b, "IRQ", LOGIC_1, "IRQ released (pulled up)");
    /* LCR, DLL, THR: a TX frame. */
    br_program(&r, 0, 16);
    check(br_w(&r, 0, 0, 0x55) == LOGIC_0, "THR write ACKed");
    br_frame(b, "TX", 16 * BIT1, frame, NS(200000));
    check(strcmp(frame, "0101010101") == 0, "TX frame of 0x55 at divisor 16 (start, LSB first, stop)");
    mb_run(b, NS(400000));
    n = mb_sent(b, 0, frames, 8);
    check(n == 1 && frames[0] == 0x55, "one frame sent: 0x55");
    /* RX: a frame in, RXLVL, the RX interrupt, RHR. */
    br_w(&r, 0, 1, 0x01);
    mb_drive_byte(b, "RX", 0xA5, 16 * BIT1, 1);
    mb_run(b, 16 * BIT1);
    br_expect(&r, 0, 9, 0x01, "RXLVL after a frame");
    mb_expect_pin(b, "IRQ", LOGIC_0, "IRQ low: RX data");
    br_expect(&r, 0, 2, 0x04, "IIR: RHR data (FIFO off)");
    br_expect(&r, 0, 0, 0xA5, "RHR");
    mb_run(b, NS(2000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "IRQ released after the RHR read");
    /* A wrong address: NACK. */
    i2c_start(b);
    check(i2c_write(b, 0x49 << 1) == LOGIC_1, "address 0x49 (not this chip's 0x48): NACK");
    i2c_stop(b);
    /* SRESET: the byte is NACKed and the chip resets (SPR is not reset). */
    br_w(&r, 0, 7, 0x5A);
    check(br_w(&r, 0, 14, 0x08) == LOGIC_1, "IOControl[3] (SRESET) write: NACK");
    br_expect(&r, 0, 3, 0x1D, "LCR back at 0x1D after the software reset");
    br_expect(&r, 0, 7, 0x5A, "SPR kept by the software reset");
    br_expect(&r, 0, 1, 0x00, "IER reset");
    mb_close(b);
    /* The 25 ms SDA-low bus timeout: the target holds SDA low (bit 7 of a
       read) while the controller stops clocking; it lets go after 25 ms. */
    r = br_open("SFX-SC16IS750", 0, 0, NULL);
    b = r.b;
    br_w(&r, 0, 7, 0x00);
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(7, 0));
    i2c_restart(b);
    i2c_write(b, (r.addr << 1) | 1u);
    mb_run(b, NS(2000));
    mb_expect_pin(b, "SDA", LOGIC_0, "the target drives bit 7 (0) of the read");
    mb_run(b, (SimTime)24000000000ull);
    mb_expect_pin(b, "SDA", LOGIC_0, "still held at 24 ms");
    mb_run(b, (SimTime)2000000000ull);
    mb_expect_pin(b, "SDA", LOGIC_1, "released after 25 ms (bus interface reset)");
    i2c_stop(b);
    br_expect(&r, 0, 3, 0x1D, "the port answers again");
    mb_close(b);
    case_end("I1 SC16IS750 I2C: 16-entry address table, LCR/DLL/THR -> TX frame, RX -> RHR, IRQ, "
             "NACK for a wrong address and for SRESET, 25 ms SDA timeout");
}

/* ---- I2 SC16IS750 over SPI ------------------------------------------------------------- */

static void case_i2(void) {
    Br r = br_open("SFX-SC16IS750", 1, 0, NULL);
    MB *b = r.b;
    char text[80];
    uint8_t out[8];
    uint8_t in[8];
    uint16_t frames[8];
    size_t n;

    case_begin();
    mb_expect_pin(b, "SO", LOGIC_Z, "SO 3-state while CS is high");
    mb_spi(b, (0x80u | br_sub(3, 0)) << 8, 16, text, 0);
    check(strcmp(text, "ZZZZZZZZ00011101") == 0, "LCR read: SO Z during the command byte, then 0x1D");
    mb_expect_pin(b, "SO", LOGIC_Z, "SO 3-state again after CS rises");
    /* Command byte: bit 7 R/W, [6:3] register, [2:1] channel. */
    br_w(&r, 0, 7, 0x5A);
    br_expect(&r, 0, 7, 0x5A, "SPR written and read back");
    br_w(&r, 1, 7, 0x33);
    br_expect(&r, 0, 7, 0x5A, "a write with channel bits 01 (reserved on one channel) is ignored");
    br_expect_text(&r, 1, 7, "XXXXXXXX", "a read with channel bits 01 drives X");
    mb_spi(b, (br_sub(7, 0) | 0x01u) << 8 | 0x44u, 16, NULL, 0);
    br_expect(&r, 0, 7, 0x44, "command bit 0 is don't care");
    /* Burst FIFO write in one CS frame (TX held by EFCR[2]). */
    br_program(&r, 0, 1);
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x04);
    out[0] = (uint8_t)br_sub(0, 0);
    out[1] = 0x11;
    out[2] = 0x22;
    out[3] = 0x33;
    out[4] = 0x44;
    br_spi_frame(b, out, 5, NULL, NULL);
    br_expect(&r, 0, 8, 60, "TXLVL after a 4-byte burst (TXDISABLE holds them)");
    mb_run(b, 30 * BIT1);
    n = mb_sent(b, 0, frames, 8);
    check(n == 0, "EFCR[2] TXDISABLE: nothing sent");
    br_w(&r, 0, 15, 0x00);
    mb_run(b, 50 * BIT1);
    n = mb_sent(b, 0, frames, 8);
    check(n == 4 && frames[0] == 0x11 && frames[1] == 0x22 && frames[2] == 0x33 && frames[3] == 0x44,
          "the burst goes out in order once TX is enabled");
    /* Burst FIFO read in one CS frame. */
    mb_drive_byte(b, "RX", 0x61, BIT1, 1);
    mb_drive_byte(b, "RX", 0x62, BIT1, 1);
    mb_drive_byte(b, "RX", 0x63, BIT1, 1);
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 9, 3, "RXLVL 3");
    out[0] = (uint8_t)(0x80u | br_sub(0, 0));
    out[1] = out[2] = out[3] = 0;
    br_spi_frame(b, out, 4, in, text);
    check(in[1] == 0x61 && in[2] == 0x62 && in[3] == 0x63, "one CS frame reads RHR three times in order");
    br_expect(&r, 0, 9, 0, "RXLVL 0 after the burst read");
    /* RXDISABLE: frames are dropped. */
    br_w(&r, 0, 15, 0x02);
    mb_drive_byte(b, "RX", 0x70, BIT1, 1);
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 9, 0, "EFCR[1] RXDISABLE: nothing received");
    br_w(&r, 0, 15, 0x00);
    /* RS-485: RTSCON drives RTS low while sending, RTSINVER flips it. */
    mb_expect_pin(b, "RTS", LOGIC_1, "RTS inactive (MCR[1] = 0)");
    br_w(&r, 0, 15, 0x10);
    br_w(&r, 0, 0, 0x7E);
    mb_run(b, NS(400));
    mb_expect_pin(b, "RTS", LOGIC_0, "EFCR[4]: RTS low from the THR write");
    mb_run(b, 12 * BIT1);
    mb_expect_pin(b, "RTS", LOGIC_1, "EFCR[4]: RTS high after the last bit");
    br_w(&r, 0, 15, 0x30);
    mb_expect_pin(b, "RTS", LOGIC_0, "EFCR[5] RTSINVER: RTS low while idle");
    br_w(&r, 0, 0, 0x7E);
    mb_run(b, NS(400));
    mb_expect_pin(b, "RTS", LOGIC_1, "EFCR[5]: RTS high while sending");
    mb_run(b, 12 * BIT1);
    br_w(&r, 0, 15, 0x00);
    /* 9-bit multidrop: with LCR[5:3] = 111 an address byte (9th bit 1)
       arrives with PE; data bytes wait for EFCR[1] = 0. */
    br_w(&r, 0, 3, 0x3B);
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x03);
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* data 0x30, 9th bit 0 */
    mb_drive_bits(b, "RX", "1000000011", BIT1, LOGIC_1);   /* address 0x01, 9th bit 1 */
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 9, 1, "9-bit normal mode, receiver off: only the address byte is kept");
    br_expect(&r, 0, 5, 0xE5, "LSR: data, PE (the address flag), FIFO error");
    br_expect(&r, 0, 0, 0x01, "RHR: the address");
    /* Auto address (EFR[5] = 1, Xoff2 = this address). */
    br_w(&r, 0, 3, 0xBF);
    br_w(&r, 0, 2, 0x30);
    br_w(&r, 0, 7, 0x05);
    br_w(&r, 0, 3, 0x3B);
    br_w(&r, 0, 15, 0x01);
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* data before any address: dropped */
    mb_drive_bits(b, "RX", "1100000011", BIT1, LOGIC_1);   /* address 0x03: not ours, dropped */
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* dropped */
    mb_drive_bits(b, "RX", "1010000011", BIT1, LOGIC_1);   /* address 0x05: ours, kept */
    mb_drive_bits(b, "RX", "0100110001", BIT1, LOGIC_1);   /* data 0x32: kept */
    mb_drive_bits(b, "RX", "1100000011", BIT1, LOGIC_1);   /* address 0x03: receiver off, dropped */
    mb_drive_bits(b, "RX", "0000110001", BIT1, LOGIC_1);   /* dropped */
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 9, 2, "auto address: our address and the data after it");
    br_expect(&r, 0, 0, 0x05, "first: the matching address");
    br_expect(&r, 0, 0, 0x32, "then its data");
    mb_close(b);
    case_end("I2 SC16IS750 SPI: command byte R/W, register, channel bits; SO 3-state; burst FIFO "
             "write and read; EFCR TX/RX disable, RS-485 RTSCON/RTSINVER, 9-bit multidrop and auto address");
}

/* ---- I3 SC16IS752 ------------------------------------------------------------------------ */

static void case_i3(void) {
    Br r = br_open("SFX-SC16IS752", 1, 0, NULL);
    MB *b = r.b;
    uint16_t frames[4];

    case_begin();
    br_w(&r, 1, 7, 0x42);
    br_w(&r, 0, 7, 0x24);
    br_expect(&r, 1, 7, 0x42, "channel B SPR");
    br_expect(&r, 0, 7, 0x24, "channel A SPR");
    br_program(&r, 1, 1);
    br_expect(&r, 1, 3, 0x03, "channel B LCR");
    br_expect(&r, 0, 3, 0x1D, "channel A LCR untouched");
    br_w(&r, 1, 0, 0x5A);
    mb_run(b, 12 * BIT1);
    check(mb_sent(b, 1, frames, 4) == 1 && frames[0] == 0x5A, "channel B sends on TXB");
    check(mb_sent(b, 0, frames, 4) == 0, "channel A sent nothing");
    /* GPIO input change: the interrupt shows in channel A's IIR only. */
    br_w(&r, 0, 12, 0x01);
    br_r(&r, 0, 11, NULL);
    mb_expect_pin(b, "IRQ", LOGIC_1, "no GPIO interrupt yet");
    mb_set(b, "GPIO0/DSRB", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "GPIO0 changed: IRQ low");
    br_expect(&r, 0, 2, 0x30, "channel A IIR: I/O pins (0x30)");
    br_expect(&r, 1, 2, 0x01, "channel B IIR: none (planner decision)");
    mb_expect_u(br_r(&r, 0, 11, NULL) & 0x0Fu, 0x0E, "IOState: GPIO0 low, GPIO1-3 pulled up");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "the IOState read clears it");
    /* IOLATCH = 0: a pulse back to the old level before the read clears it. */
    mb_set(b, "GPIO0/DSRB", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "GPIO0 high: IRQ low");
    mb_set(b, "GPIO0/DSRB", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "IOLATCH 0: back to the read level, the interrupt clears");
    /* IOLATCH = 1: the change is latched until IOState is read. */
    br_w(&r, 0, 14, 0x01);
    mb_set(b, "GPIO0/DSRB", LOGIC_1);
    mb_run(b, NS(1000));
    mb_set(b, "GPIO0/DSRB", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "IOLATCH 1: still pending after the pin returns");
    mb_expect_u(br_r(&r, 0, 11, NULL) & 0x01u, 0x01, "IOState shows the latched 1");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "the IOState read clears the latch");
    /* Writing IODir clears a pending GPIO interrupt. */
    mb_set(b, "GPIO0/DSRB", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "pending again");
    br_w(&r, 0, 10, 0x00);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "an IODir write clears it");
    br_w(&r, 0, 14, 0x00);
    /* GPIO outputs. */
    br_w(&r, 0, 10, 0x08);
    br_w(&r, 0, 11, 0x08);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO3/RIB", LOGIC_1, "GPIO3 output high");
    br_w(&r, 0, 11, 0x00);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO3/RIB", LOGIC_0, "GPIO3 output low");
    br_w(&r, 0, 10, 0x00);
    /* IOControl[1]: GPIO7..4 are channel A's modem pins; IODir has no effect on them. */
    br_w(&r, 0, 10, 0xF0);
    br_w(&r, 0, 11, 0x00);
    br_w(&r, 0, 14, 0x02);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO5/DTRA", LOGIC_1, "DTRA inactive (MCR[0] = 0)");
    br_w(&r, 0, 4, 0x01);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "GPIO5/DTRA", LOGIC_0, "MCR[0] = 1: DTRA low");
    mb_expect_pin(b, "GPIO1/DTRB", LOGIC_Z, "channel B's DTR is still a GPIO input");
    br_r(&r, 0, 6, NULL);
    mb_set(b, "GPIO4/DSRA", LOGIC_0);
    mb_run(b, NS(1000));
    mb_expect_u(br_r(&r, 0, 6, NULL) & 0x22u, 0x22, "MSR A: DSR active and delta-DSR");
    br_w(&r, 0, 14, 0x00);
    mb_close(b);
    case_end("I3 SC16IS752: two channels by sub-address, GPIO change interrupt in channel A's IIR, "
             "IOLATCH, IODir clear, GPIO outputs, IOControl modem pins");
}

/* ---- I4 SC16IS740 ------------------------------------------------------------------------- */

static void case_i4(void) {
    Br r = br_open("SFX-SC16IS740", 0, 0, NULL);
    MB *b = r.b;

    case_begin();
    br_expect_text(&r, 0, 10, "XXXXXXXX", "IODir reads X");
    br_expect_text(&r, 0, 11, "XXXXXXXX", "IOState reads X");
    br_expect_text(&r, 0, 12, "XXXXXXXX", "IOIntEna reads X");
    br_expect_text(&r, 0, 13, "XXXXXXXX", "reserved 0x0D reads X");
    br_expect_text(&r, 0, 14, "XXXXXXXX", "IOControl reads X");
    br_expect(&r, 0, 8, 0x40, "TXLVL is there");
    br_expect_text(&r, 0, 6, "XXX00000", "MSR: CD, RI, DSR have no pins (X), CTS high reads 0");
    br_w(&r, 0, 3, 0x03);
    check(br_w(&r, 0, 14, 0x08) == LOGIC_1, "IOControl[3]: the software reset still works (NACK)");
    br_expect(&r, 0, 3, 0x1D, "LCR after the software reset");
    mb_close(b);
    case_end("I4 SC16IS740: no GPIO (IODir, IOState, IOIntEna, IOControl read X), software reset");
}

/* ---- I5 PI7C9X760 / PI7C9X762 ------------------------------------------------------------- */

static void pi_sfr_on(Br *r, unsigned ch, unsigned sfr) {
    br_w(r, ch, 3, 0xBF);
    br_w(r, ch, 13, 0x5A);
    br_w(r, ch, 7, sfr);
}

static void case_i5(void) {
    Br r = br_open("SFX-PI7C9X760", 0, 0, NULL);
    MB *b = r.b;
    LogicValue acks[66];
    long long t;
    unsigned i;
    int ok;

    case_begin();
    br_expect(&r, 0, 7, 0xFF, "SPR power-up 0xFF");
    br_expect_text(&r, 0, 11, "XXXXXXXX", "IOState: inputs with no pull-up read X");
    br_w(&r, 0, 10, 0xFF);
    br_expect(&r, 0, 11, 0xFF, "IOState as outputs: the latch's reset value 0xFF");
    br_w(&r, 0, 10, 0x00);
    pi_sfr_on(&r, 0, 0x04);
    br_expect(&r, 0, 13, 0x5A, "SFREN");
    br_expect(&r, 0, 4, 0x10, "CPR reset 0x10");
    br_expect(&r, 0, 9, 0x06, "TRCTL reset 0x06");
    br_expect(&r, 0, 2, 0x00, "ASR");
    br_w(&r, 0, 8, 0x37);
    br_expect(&r, 0, 8, 0x37, "TIDLE reads back what was written");
    br_w(&r, 0, 13, 0x00);
    br_expect(&r, 0, 7, 0x00, "SFREN != 0x5A: address 7 is XOFF2 again (reset 0x00)");
    /* CPR M = 2: XIN / 2 into the generator. */
    br_w(&r, 0, 13, 0x5A);
    br_w(&r, 0, 4, 0x20);
    br_program(&r, 0, 1);
    br_w(&r, 0, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, 2 * BIT1, NS(60), "CPR M = 2: a bit is 2 x 16 XIN periods");
    mb_run(b, 12 * BIT1 * 2);
    /* CPR N = 4: sample rate 16 - SCR + N = 20. */
    pi_sfr_on(&r, 0, 0x04);
    br_w(&r, 0, 4, 0x14);
    br_w(&r, 0, 9, 0x07);   /* TRCTL[0] TX idle insertion on, with TIDLE: no effect */
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, BIT1 * 20 / 16, NS(60), "CPR N = 4: 20 XIN periods per bit");
    mb_run(b, 30 * BIT1);
    /* TIDLE with TRCTL[0] on: stored only; back-to-back bytes stay back to back. */
    br_w(&r, 0, 2, 0x01);
    br_w(&r, 0, 15, 0x04);
    br_w(&r, 0, 0, 0x55);
    br_w(&r, 0, 0, 0x55);
    br_w(&r, 0, 15, 0x00);
    mb_wait(b, "TX", LOGIC_0, NS(100000), NS(20));
    mb_run(b, BIT1 * 20 / 16 * 19 / 2);
    mb_expect_pin(b, "TX", LOGIC_1, "mid stop bit of the first frame");
    mb_run(b, BIT1 * 20 / 16);
    mb_expect_pin(b, "TX", LOGIC_0, "TIDLE stored only: the next start bit follows at once");
    mb_run(b, 24 * BIT1);
    /* NACK on a THR write with the TX FIFO full. */
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x04);
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(0, 0));
    for (i = 0; i < 65; i++) {
        acks[i] = i2c_write(b, i);
    }
    i2c_stop(b);
    ok = 1;
    for (i = 0; i < 64; i++) {
        ok &= acks[i] == LOGIC_0;
    }
    check(ok, "64 THR bytes: ACK");
    check(acks[64] == LOGIC_1, "the 65th with the FIFO full: NACK");
    br_expect(&r, 0, 8, 0, "TXLVL 0");
    pi_sfr_on(&r, 0, 0x04);
    br_expect(&r, 0, 6, 64, "TFD 64");
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x00);
    /* RFD / RLS. */
    br_program(&r, 0, 1);
    mb_drive_byte(b, "RX", 0x41, BIT1 * 20 / 16, 1);
    mb_drive_byte(b, "RX", 0x42, BIT1 * 20 / 16, 0);   /* framing error */
    mb_run(b, 4 * BIT1);
    pi_sfr_on(&r, 0, 0x04);
    br_expect(&r, 0, 5, 3, "RFD: 3 bytes (the framing error's resync adds 0xFF)");
    br_w(&r, 0, 7, 0x44);
    br_expect(&r, 0, 5, 1, "RLS (SFR[6] = 1): 1 byte with an error");
    /* SFR[4]: bursts auto-increment the sub-address. */
    br_w(&r, 0, 7, 0x10);
    br_w(&r, 0, 3, 0x03);
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(7, 0));
    i2c_write(b, 0x99);   /* SPR */
    i2c_write(b, 0x00);   /* TXLVL (read-only) */
    i2c_stop(b);
    br_expect(&r, 0, 7, 0x99, "burst write: SPR");
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(3, 0));
    i2c_restart(b);
    i2c_write(b, (r.addr << 1) | 1u);
    mb_expect_u(i2c_read(b, 1, NULL), 0x03, "burst read: LCR");
    mb_expect_u(i2c_read(b, 0, NULL), 0x00, "then MCR (auto-increment)");
    i2c_stop(b);
    mb_close(b);
    /* PI7C9X762: the SFR bank per channel; GPIO registers shared. */
    r = br_open("SFX-PI7C9X762", 1, 0, NULL);
    b = r.b;
    pi_sfr_on(&r, 1, 0x04);
    br_w(&r, 1, 8, 0x21);
    br_expect(&r, 1, 8, 0x21, "channel B TIDLE");
    br_w(&r, 0, 3, 0xBF);
    br_expect(&r, 0, 7, 0x00, "channel A: address 7 is still XOFF2 (its SFREN is 0)");
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 1, 3, 0x03);
    br_w(&r, 1, 10, 0x0F);
    br_expect(&r, 0, 10, 0x0F, "IODir is shared by both channels");
    mb_close(b);
    case_end("I5 PI7C9X760, PI7C9X762: SFR bank (SFREN 0x5A, SFR, ASR, CPR M/N, TRCTL, RFD/RLS, TFD), "
             "TIDLE stored only, NACK on a full TX FIFO, SFR[4] bursts, per-channel bank");
}

/* ---- I6 XR20M1170 ------------------------------------------------------------------------ */

static void case_i6(void) {
    Br r;
    MB *b;
    long long t;
    LogicValue acks[66];
    unsigned i;

    case_begin();
    address_table("SFX-XR20M1170", 1);
    r = br_open("SFX-XR20M1170", 0, 1, NULL);
    b = r.b;
    br_expect(&r, 0, 3, 0x1D, "LCR reset 0x1D");
    br_expect(&r, 0, 7, 0xFF, "SPR power-up 0xFF");
    br_w(&r, 0, 3, 0xBF);
    br_w(&r, 0, 2, 0x10);
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 4, 0x04);
    br_expect(&r, 0, 6, 0x0F, "TCR reset 0x0F (EFR[4] = 1, MCR[2] = 1)");
    br_w(&r, 0, 4, 0x00);
    /* DLD: divisor 1 + 8/16. */
    br_w(&r, 0, 3, 0x80);
    br_expect(&r, 0, 0, 0x01, "DLL power-up 0x01");
    br_w(&r, 0, 2, 0x08);
    br_expect(&r, 0, 2, 0x08, "DLD at 2 with LCR[7] = 1 and EFR[4] = 1");
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, BIT1 * 3 / 2, NS(120), "DLD 8/16: 1.5 x 16 XTAL1 periods per bit");
    mb_run(b, 20 * BIT1);
    br_w(&r, 0, 3, 0x80);
    br_w(&r, 0, 2, 0x18);
    br_w(&r, 0, 3, 0x03);
    br_w(&r, 0, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, BIT1 * 3 / 4, NS(120), "DLD[5:4] = 01: 8x sampling, 1.5 x 8 periods per bit");
    mb_run(b, 20 * BIT1);
    /* NACK with the TX FIFO full. */
    br_w(&r, 0, 2, 0x07);
    br_w(&r, 0, 15, 0x04);
    i2c_start(b);
    i2c_write(b, r.addr << 1);
    i2c_write(b, br_sub(0, 0));
    for (i = 0; i < 65; i++) {
        acks[i] = i2c_write(b, i);
    }
    i2c_stop(b);
    check(acks[63] == LOGIC_0 && acks[64] == LOGIC_1, "XR20M1170: NACK for the 65th THR byte");
    mb_close(b);
    /* The 16-pin package: no GPIO or modem pins. */
    r = br_open("SFX-XR20M1170-tssop16", 1, 1, NULL);
    br_expect(&r, 0, 3, 0x1D, "tssop16: the register map answers over SPI");
    mb_close(r.b);
    case_end("I6 XR20M1170: 8-address table (A1 = SCL as VCC, SDA as GND), DLD fraction and 8x, "
             "TCR 0x0F, NACK on a full TX FIFO, tssop16");
}

/* ---- I7 XR20M1280 ------------------------------------------------------------------------ */

static unsigned xr_sub(unsigned reg) {
    return (reg & 7u) << 3;
}

static void xr_w(Br *r, unsigned reg, unsigned v) {
    br_w(r, 0, reg, v);   /* reg < 8: bits [5:3] as [6:3] with bit 6 = 0 */
}

static void case_i7(void) {
    Br r = br_open("SFX-XR20M1280-qfn32", 1, 1, NULL);
    MB *b = r.b;
    char text[20];
    long long t;
    unsigned v;

    case_begin();
    mb_spi(b, (0x80u | xr_sub(7)) << 8, 16, text, 0);
    check(strcmp(text + 8, "11111111") == 0, "SPR reset 0xFF at sub-address bits [5:3] = 111");
    br_expect(&r, 0, 3, 0x00, "LCR reset 0x00");
    br_expect(&r, 0, 5, 0x60, "LSR at 5");
    xr_w(&r, 7, 0x5A);
    br_expect(&r, 0, 7, 0x5A, "SPR written");
    mb_spi(b, (0x80u | 0x40u | xr_sub(7)) << 8, 16, text, 0);
    check(strcmp(text + 8, "01011010") == 0, "SPI bit 6 is not an address bit");
    /* Independent TX and RX generators (DLD[7:6]). */
    xr_w(&r, 3, 0xBF);
    xr_w(&r, 2, 0x10);
    xr_w(&r, 3, 0x80);
    xr_w(&r, 2, 0x40);   /* independent, TX selected */
    xr_w(&r, 0, 0x01);
    xr_w(&r, 1, 0x00);
    xr_w(&r, 2, 0xC0);   /* RX selected */
    xr_w(&r, 0, 0x02);
    xr_w(&r, 1, 0x00);
    br_expect(&r, 0, 0, 0x02, "DLL (RX generator selected)");
    xr_w(&r, 2, 0x40);
    br_expect(&r, 0, 0, 0x01, "DLL (TX generator selected)");
    xr_w(&r, 3, 0x03);
    xr_w(&r, 0, 0x55);
    t = br_bit_time(b, "TX");
    expect_near(t, BIT1, NS(60), "TX at divisor 1");
    mb_run(b, 12 * BIT1);
    mb_drive_byte(b, "RX", 0xA5, 2 * BIT1, 1);
    mb_run(b, 4 * BIT1);
    br_expect(&r, 0, 5, 0x61, "RX at divisor 2: a good frame");
    br_expect(&r, 0, 0, 0xA5, "RHR 0xA5");
    mb_drive_byte(b, "RX", 0xA5, BIT1, 1);
    mb_run(b, 20 * BIT1);
    v = br_r(&r, 0, 0, NULL);
    check(v != 0xA5, "a frame at the TX rate does not decode on the RX generator");
    br_r(&r, 0, 5, NULL);
    /* FC (LCR = 0xBF, address 0): the RX FIFO count; TRG writes. */
    xr_w(&r, 2, 0x01);
    mb_drive_byte(b, "RX", 0x31, 2 * BIT1, 1);
    mb_drive_byte(b, "RX", 0x32, 2 * BIT1, 1);
    mb_run(b, 4 * BIT1);
    xr_w(&r, 3, 0xBF);
    br_expect(&r, 0, 0, 2, "FC: 2 bytes in the RX FIFO");
    xr_w(&r, 1, 0x30);   /* FCTR: table D */
    xr_w(&r, 0, 0x02);   /* TRG (RX): 2 */
    xr_w(&r, 3, 0x03);
    xr_w(&r, 1, 0x01);
    br_expect(&r, 0, 2, 0xC4, "table D, TRG 2: RX data at 2 bytes");
    br_r(&r, 0, 0, NULL);
    br_expect(&r, 0, 2, 0xC1, "below the trigger after one read (no timeout yet)");
    br_r(&r, 0, 0, NULL);
    xr_w(&r, 1, 0x00);
    /* GPIO: MCR[2] = 1 turns DTR#/GPIO0 into a GPIO; SFR[0] maps the GPIO set. */
    xr_w(&r, 4, 0x01);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "DTR/GPIO0", LOGIC_0, "MCR[2] = 0: DTR# low (MCR[0] = 1)");
    xr_w(&r, 4, 0x04);
    xr_w(&r, 6, 0x05);   /* SFR: GPIO access, GPIO interrupt enable */
    xr_w(&r, 3, 0xBF);
    xr_w(&r, 7, 0xFE);   /* GPIOSEL: GPIO0 an output */
    xr_w(&r, 4, 0x02);   /* GPIOINT: GPIO1 */
    xr_w(&r, 3, 0x03);
    xr_w(&r, 7, 0x01);   /* GPIOLVL */
    mb_run(b, NS(1000));
    mb_expect_pin(b, "DTR/GPIO0", LOGIC_1, "GPIO0 output high (GPIOLVL)");
    xr_w(&r, 7, 0x00);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "DTR/GPIO0", LOGIC_0, "GPIO0 output low");
    mb_set(b, "DSR/GPIO1", LOGIC_0);
    mb_run(b, NS(1000));
    br_r(&r, 0, 7, NULL);
    mb_set(b, "DSR/GPIO1", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "GPIO1 low -> high: IRQ low");
    br_expect(&r, 0, 2, 0x81, "ISR[6] differs from ISR[7] (FIFOs on): the GPIO interrupt");
    mb_expect_u(br_r(&r, 0, 7, NULL) & 0x02u, 0x02, "GPIOLVL reads GPIO1 high");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_1, "the GPIOLVL read clears it");
    mb_close(b);
    case_end("I7 XR20M1280: 3-bit sub-address map, LCR 0x00, SPR 0xFF, independent TX/RX baud "
             "generators, FC, FCTR table D with TRG, GPIO set with ISR[6]");
}

/* ---- I8 / I9: the SC18IS604 -------------------------------------------------------------- */

/* The SC18IS604 with its own driver, PULLUP-3V3 parts on SCL, SDA and INT,
   the JF test target on the I2C bus (address 0x2A: A0 tied to SCL, A1 to
   SDA), and for I9 an SC16IS750 in I2C mode on the same bus. */
typedef struct {
    MB b;
    SimInstance *tgt;
    SimInstance *u2;
    SimInstance *drv2;
    SimInstance *clk2;
    const Part *p2;
    size_t pins2;
    SimTime half2;
} SpBench;

static void sp_pull(SpBench *t, SimNet *net, const char *name) {
    const Part *pu = part_library_find(library, "PULLUP-3V3");
    SimInstance *inst;

    if (pu == NULL) {
        fprintf(stderr, "no PULLUP-3V3\n");
        exit(2);
    }
    inst = sim_add_instance(t->b.sim, kind_model(pu), name, pu);
    sim_connect(t->b.sim, net, inst, 0);
}

static size_t sp_pin2(const SpBench *t, const char *name) {
    size_t i;

    for (i = 0; i < t->p2->pin_count; i++) {
        if (strcmp(t->p2->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s\n", name);
    exit(2);
}

static void sp_set2(SpBench *t, const char *name, LogicValue v) {
    sim_schedule_drive(t->b.sim, t->drv2, sp_pin2(t, name), v, DRIVE_STRONG, 0);
}

static LogicValue sp_get2(SpBench *t, const char *name) {
    return sim_read_pin(t->b.sim, t->u2, sp_pin2(t, name));
}

static SpBench *sp_open(int with_uart) {
    SpBench *t = calloc(1, sizeof *t);
    MB *b = &t->b;
    SimNet **nets;
    SimNet *scl;
    SimNet *sda;
    size_t i;

    b->part = part_library_find(library, "SFX-SC18IS604");
    if (b->part == NULL) {
        fprintf(stderr, "no SFX-SC18IS604\n");
        exit(2);
    }
    b->pins = b->part->pin_count;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    nets = calloc(b->pins, sizeof *nets);
    for (i = 0; i < b->pins; i++) {
        nets[i] = sim_add_net(b->sim, b->part->pins[i].name);
        sim_connect(b->sim, nets[i], b->chip, i);
        sim_connect(b->sim, nets[i], b->drv, i);
    }
    scl = nets[mb_pin(b, "SCL")];
    sda = nets[mb_pin(b, "SDA")];
    sp_pull(t, scl, "PU1");
    sp_pull(t, sda, "PU2");
    sp_pull(t, nets[mb_pin(b, "INT")], "PU3");
    t->tgt = sim_add_instance(b->sim, &TARGET, "T1", NULL);
    for (i = 0; i < TG_SCL; i++) {
        sim_connect(b->sim, sim_add_net(b->sim, "tg"), t->tgt, i);
    }
    sim_connect(b->sim, scl, t->tgt, TG_SCL);
    sim_connect(b->sim, sda, t->tgt, TG_SDA);
    sim_connect(b->sim, scl, t->tgt, TG_A0);
    sim_connect(b->sim, sda, t->tgt, TG_A1);
    if (with_uart) {
        t->p2 = part_library_find(library, "SFX-SC16IS750");
        t->pins2 = t->p2->pin_count;
        t->half2 = XT_HALF;
        t->u2 = sim_add_instance(b->sim, kind_model(t->p2), "U2", t->p2);
        t->drv2 = sim_add_instance(b->sim, &DRIVER, "TB2", &t->pins2);
        t->clk2 = sim_add_instance(b->sim, &CLOCK, "CLK2", &t->half2);
        for (i = 0; i < t->pins2; i++) {
            const char *name = t->p2->pins[i].name;
            SimNet *net;

            if (strcmp(name, "SCL/SCLK") == 0) {
                sim_connect(b->sim, scl, t->u2, i);
                continue;
            }
            if (strcmp(name, "SDA") == 0) {
                sim_connect(b->sim, sda, t->u2, i);
                continue;
            }
            net = sim_add_net(b->sim, name);
            sim_connect(b->sim, net, t->u2, i);
            if (strcmp(name, "XTAL1") == 0) {
                sim_connect(b->sim, net, t->clk2, 0);
            } else {
                sim_connect(b->sim, net, t->drv2, i);
            }
        }
    }
    free(nets);
    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "MOSI";
    b->miso = "MISO";
    b->scl = "SCL";
    b->sda = "SDA";
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    mb_set(b, "CS", LOGIC_1);
    mb_set(b, "SCLK", LOGIC_1);   /* mode 3: SCLK idles high */
    mb_set(b, "MOSI", LOGIC_0);
    mb_set(b, "RESET", LOGIC_1);
    if (with_uart) {
        sp_set2(t, "I2C/SPI", LOGIC_1);
        sp_set2(t, "CS/A0", LOGIC_1);
        sp_set2(t, "SI/A1", LOGIC_1);
        sp_set2(t, "RX", LOGIC_1);
        sp_set2(t, "CTS", LOGIC_1);
        sp_set2(t, "RESET", LOGIC_0);
        mb_run(b, NS(4000));
        sp_set2(t, "RESET", LOGIC_1);
    }
    mb_run(b, NS(4000));
    return t;
}

static void sp_close(SpBench *t) {
    sim_destroy(t->b.sim);
    free(t);
}

static Tgt *sp_tgt(SpBench *t) {
    return sim_instance_state(t->tgt);
}

#define SP_HALF NS(500)

/* One SPI message (mode 3, MSB first on the wire): n bytes out, MISO in. */
static void sp_msg(SpBench *t, const uint8_t *out, unsigned n, uint8_t *in, char *text) {
    MB *b = &t->b;
    unsigned i;
    unsigned bit;

    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SP_HALF);
    for (i = 0; i < n; i++) {
        uint8_t v = 0;

        for (bit = 0; bit < 8; bit++) {
            LogicValue l;

            mb_set(b, "SCLK", LOGIC_0);
            mb_set(b, "MOSI", (out[i] >> (7 - bit)) & 1u ? LOGIC_1 : LOGIC_0);
            mb_run(b, SP_HALF);
            l = mb_get(b, "MISO");
            if (text != NULL) {
                text[i * 8 + bit] = "01XZ"[l];
            }
            v = (uint8_t)((v << 1) | (l == LOGIC_1 ? 1u : 0u));
            mb_set(b, "SCLK", LOGIC_1);
            mb_run(b, SP_HALF);
        }
        if (in != NULL) {
            in[i] = v;
        }
    }
    if (text != NULL) {
        text[n * 8] = '\0';
    }
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SP_HALF);
}

static void sp_reg_w(SpBench *t, unsigned reg, unsigned v) {
    uint8_t m[3] = {0x20, (uint8_t)reg, (uint8_t)v};

    sp_msg(t, m, 3, NULL, NULL);
}

static unsigned sp_reg_r(SpBench *t, unsigned reg) {
    uint8_t m[3] = {0x21, (uint8_t)reg, 0};
    uint8_t in[3];

    sp_msg(t, m, 3, in, NULL);
    return in[2];
}

/* Waits for INT low (the transaction's end); the time waited or -1. */
static long long sp_wait_int(SpBench *t, SimTime limit) {
    return mb_wait(&t->b, "INT", LOGIC_0, limit, NS(500));
}

/* Runs a message and waits for its INT; returns I2CStat (INT released). */
static unsigned sp_do(SpBench *t, const uint8_t *m, unsigned n, SimTime limit) {
    sp_msg(t, m, n, NULL, NULL);
    if (sp_wait_int(t, limit) < 0) {
        return 0x100;
    }
    return sp_reg_r(t, 0x04);
}

/* SCL fall-to-fall spacing in the middle of a transaction. */
static long long sp_scl_period(SpBench *t, SimTime limit) {
    MB *b = &t->b;
    SimTime step = NS(50);
    SimTime spent = 0;
    SimTime falls[6];
    unsigned n = 0;
    LogicValue last = mb_get(b, "SCL");

    while (spent < limit && n < 6) {
        LogicValue now;

        mb_run(b, step);
        spent += step;
        now = mb_get(b, "SCL");
        if (last == LOGIC_1 && now == LOGIC_0) {
            falls[n++] = spent;
        }
        last = now;
    }
    return n == 6 ? (long long)(falls[5] - falls[3]) / 2 : -1;
}

static void case_i8(void) {
    SpBench *t = sp_open(0);
    MB *b = &t->b;
    uint8_t m[24];
    uint8_t in[24];
    char text[200];
    unsigned st;
    long long p;
    long long w;

    case_begin();
    mb_expect_pin(b, "MISO", LOGIC_Z, "MISO Z while CS is high");
    mb_expect_u(sp_reg_r(t, 0x00), 0x00, "IOConfig reset");
    mb_expect_u(sp_reg_r(t, 0x02), 0x19, "I2CClock reset 0x19");
    mb_expect_u(sp_reg_r(t, 0x03), 0x00, "I2CTO reset");
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "I2CStat reset 0xF0");
    mb_expect_u(sp_reg_r(t, 0x05), 0x00, "I2CAdr reset");
    mb_expect_pin(b, "INT", LOGIC_1, "INT released after reset");
    /* 0x00 write N bytes: START, address, data, STOP; 0xF3 while busy. */
    m[0] = 0x00; m[1] = 2; m[2] = 0x54; m[3] = 0x41; m[4] = 0x42;
    sp_msg(t, m, 5, NULL, NULL);
    mb_run(b, NS(60000));
    mb_expect_u(sp_reg_r(t, 0x04), 0xF3, "I2CStat 0xF3 while the transfer runs");
    mb_expect_pin(b, "INT", LOGIC_1, "no INT while busy");
    p = sp_scl_period(t, NS(200000));
    expect_near(p, (long long)NS(13333), NS(120), "I2CClock 0x19: SCL 15 MHz / 200 = 75 kHz");
    w = sp_wait_int(t, NS(1000000));
    check(w >= 0, "INT at the end of the write");
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "I2CStat 0xF0: done");
    mb_run(b, NS(1000));
    mb_expect_pin(b, "INT", LOGIC_1, "reading I2CStat releases INT");
    check(strcmp(sp_tgt(t)->log, "S A54 W41 W42 P ") == 0, "the target saw START, 0x54, 0x41, 0x42, STOP");
    /* 0x01 read N bytes into the buffer; 0x06 reads it out. */
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x01; m[1] = 2; m[2] = 0x55;
    st = sp_do(t, m, 3, NS(1000000));
    mb_expect_u(st, 0xF0, "read 2: I2CStat 0xF0");
    check(strcmp(sp_tgt(t)->log, "S A55 N0 KA N1 KN P ") == 0, "read: ACK the first byte, NACK the last");
    memset(m, 0, sizeof m);
    m[0] = 0x06;
    sp_msg(t, m, 3, in, text);
    check(in[1] == 0x42 && in[2] == 0x43, "0x06 returns the buffer: 0x42, 0x43");
    check(strncmp(text, "ZZZZZZZZ", 8) == 0, "MISO Z during the command byte");
    /* 0x02 read after write and 0x03 write after write: a repeated START between. */
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x02; m[1] = 1; m[2] = 1; m[3] = 0x54; m[4] = 0x10; m[5] = 0x55;
    mb_expect_u(sp_do(t, m, 6, NS(1000000)), 0xF0, "read after write: 0xF0");
    check(strcmp(sp_tgt(t)->log, "S A54 W10 Sr A55 N0 KN P ") == 0, "read after write: S W, Sr R, P");
    m[0] = 0x06; m[1] = 0;
    sp_msg(t, m, 2, in, NULL);
    mb_expect_u(in[1], 0x10, "the byte read back");
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x03; m[1] = 1; m[2] = 1; m[3] = 0x54; m[4] = 0x20; m[5] = 0x54; m[6] = 0x21;
    mb_expect_u(sp_do(t, m, 7, NS(1000000)), 0xF0, "write after write: 0xF0");
    check(strcmp(sp_tgt(t)->log, "S A54 W20 Sr A54 W21 P ") == 0, "write after write: S W, Sr W, P");
    /* Address NACK: 0xF1. */
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x00; m[1] = 1; m[2] = 0x56; m[3] = 0x00;
    mb_expect_u(sp_do(t, m, 4, NS(1000000)), 0xF1, "no target at 0x2B: I2CStat 0xF1");
    /* A byte count of 0 or more than the bytes sent: 0xF9. */
    m[0] = 0x00; m[1] = 0; m[2] = 0x54;
    mb_expect_u(sp_do(t, m, 3, NS(100000)), 0xF9, "N = 0: I2CStat 0xF9");
    m[0] = 0x00; m[1] = 3; m[2] = 0x54; m[3] = 0x01;
    mb_expect_u(sp_do(t, m, 4, NS(100000)), 0xF9, "N = 3 with one byte sent: 0xF9");
    /* I2CClock 7: 15 MHz / 56 = 268 kHz. */
    sp_reg_w(t, 0x02, 7);
    mb_expect_u(sp_reg_r(t, 0x02), 7, "I2CClock written");
    m[0] = 0x00; m[1] = 2; m[2] = 0x54; m[3] = 0x41; m[4] = 0x42;
    sp_msg(t, m, 5, NULL, NULL);
    mb_run(b, NS(45000));
    p = sp_scl_period(t, NS(100000));
    expect_near(p, (long long)NS(3733), NS(120), "I2CClock 7: SCL period 8 x 7 / 15 MHz");
    sp_wait_int(t, NS(1000000));
    sp_reg_r(t, 0x04);
    /* Arbitration: another controller holds SDA low; the SC18IS604 loses at
       its first 1 bit, waits for the bus (STOP) and starts again. */
    sp_tgt(t)->log[0] = '\0';
    m[0] = 0x00; m[1] = 1; m[2] = 0x54; m[3] = 0x41;
    sp_msg(t, m, 4, NULL, NULL);
    mb_wait(b, "SDA", LOGIC_0, NS(100000), NS(20));
    mb_set(b, "SDA", LOGIC_0);
    mb_run(b, NS(20000));
    mb_expect_pin(b, "SCL", LOGIC_1, "after losing arbitration SCL is released");
    mb_set(b, "SDA", LOGIC_Z);   /* SDA rises with SCL high: STOP */
    mb_expect_u(sp_wait_int(t, NS(1000000)) >= 0, 1, "the transfer is retried and ends");
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "after the retry: 0xF0");
    check(strstr(sp_tgt(t)->log, "S A54 W41 P ") != NULL, "the retried transfer reached the target");
    /* Timeout (I2CTO): SCL held low; ((7 + 1) / 15 MHz) x 512 = 273 us. */
    sp_reg_w(t, 0x03, 0x01);
    mb_set(b, "SCL", LOGIC_0);
    m[0] = 0x00; m[1] = 1; m[2] = 0x54; m[3] = 0x41;
    sp_msg(t, m, 4, NULL, NULL);
    mb_run(b, NS(250000));
    mb_expect_pin(b, "INT", LOGIC_1, "no timeout at 250 us");
    w = sp_wait_int(t, NS(100000));
    check(w >= 0, "INT by 350 us");
    mb_expect_u(sp_reg_r(t, 0x04), 0xF8, "I2CStat 0xF8: timeout");
    mb_set(b, "SCL", LOGIC_Z);
    sp_reg_w(t, 0x03, 0x00);
    /* 0x18: LSB first (0x81), MSB first again (0x42). */
    m[0] = 0x18; m[1] = 0x81;
    sp_msg(t, m, 2, NULL, NULL);
    m[0] = 0x84; m[1] = 0x40; m[2] = 0x00;   /* 0x21, 0x02 sent LSB first */
    sp_msg(t, m, 3, in, NULL);
    mb_expect_u(in[2], 0xE0, "LSB first: I2CClock 0x07 comes out as 0xE0");
    m[0] = 0x18; m[1] = 0x42;
    sp_msg(t, m, 2, NULL, NULL);
    mb_expect_u(sp_reg_r(t, 0x02), 7, "MSB first again");
    /* 0xFE: the version string into the buffer, INT. */
    m[0] = 0xFE;
    sp_msg(t, m, 1, NULL, NULL);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "INT", LOGIC_0, "INT after 0xFE");
    memset(m, 0, sizeof m);
    m[0] = 0x06;
    sp_msg(t, m, 17, in, NULL);
    check(memcmp(in + 1, "SC18IS604 1.0.0", 16) == 0, "version string \"SC18IS604 1.0.0\\0\"");
    sp_reg_r(t, 0x04);
    /* GPIO: push-pull, open drain, GPIO4 input. */
    sp_reg_w(t, 0x00, 0x02);
    sp_reg_w(t, 0x01, 0x01);
    mb_run(b, NS(100));
    mb_expect_pin(b, "GPIO0", LOGIC_1, "GPIO0 push-pull high");
    sp_reg_w(t, 0x00, 0x03);
    mb_run(b, NS(100));
    mb_expect_pin(b, "GPIO0", LOGIC_Z, "GPIO0 open drain, latch 1: released");
    sp_reg_w(t, 0x01, 0x00);
    mb_run(b, NS(100));
    mb_expect_pin(b, "GPIO0", LOGIC_0, "GPIO0 open drain, latch 0: low");
    mb_set(b, "GPIO4", LOGIC_1);
    mb_set(b, "GPIO1", LOGIC_0);
    mb_set(b, "GPIO2", LOGIC_1);
    mb_set(b, "GPIO3", LOGIC_0);
    mb_expect_u(sp_reg_r(t, 0x01), 0x14, "IOState reads the pins (GPIO4 and GPIO2 high)");
    /* Deep power-down: the waking frame is ignored. */
    m[0] = 0x30; m[1] = 0x5A; m[2] = 0xA5;
    sp_msg(t, m, 3, NULL, NULL);
    sp_reg_w(t, 0x05, 0x44);
    mb_expect_u(sp_reg_r(t, 0x05), 0x00, "the frame that wakes it is ignored");
    sp_reg_w(t, 0x05, 0x44);
    mb_expect_u(sp_reg_r(t, 0x05), 0x44, "awake: I2CAdr written");
    /* RESET: a 50 ns pulse is filtered, a 200 ns one resets. */
    mb_set(b, "RESET", LOGIC_0);
    mb_run(b, NS(50));
    mb_set(b, "RESET", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_u(sp_reg_r(t, 0x05), 0x44, "a 50 ns RESET pulse is rejected");
    mb_set(b, "RESET", LOGIC_0);
    mb_run(b, NS(200));
    mb_set(b, "RESET", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_u(sp_reg_r(t, 0x05), 0x00, "a 200 ns RESET pulse resets");
    mb_expect_u(sp_reg_r(t, 0x02), 0x19, "I2CClock back to 0x19");
    sp_close(t);
    case_end("I8 SC18IS604: commands 0x00-0x03, 0x06, 0x18, 0x20/0x21, 0x30, 0xFE; START, STOP and "
             "repeated START at the target; ACK/NACK and 0xF0-0xF9 status codes; I2CClock rates; "
             "arbitration; timeout; GPIO; RESET filter");
}

/* I9: SPI -> SC18IS604 -> I2C (PULLUP-3V3 on SDA and SCL) -> SC16IS750 -> TX. */
static void case_i9(void) {
    SpBench *t = sp_open(1);
    MB *b = &t->b;
    uint8_t m[8];
    uint8_t in[4];
    char frame[16];
    uint16_t frames[4];
    size_t n;
    unsigned i;
    static const uint8_t SETUP[4][2] = {{0x18, 0x80}, {0x00, 0x10}, {0x08, 0x00}, {0x18, 0x03}};
    int ok = 1;

    case_begin();
    sp_reg_w(t, 0x02, 7);
    for (i = 0; i < 4; i++) {
        m[0] = 0x00; m[1] = 2; m[2] = 0x90; m[3] = SETUP[i][0]; m[4] = SETUP[i][1];
        ok &= sp_do(t, m, 5, NS(1000000)) == 0xF0;
    }
    check(ok, "LCR, DLL, DLH, LCR written over I2C: 0xF0 each");
    m[0] = 0x02; m[1] = 1; m[2] = 1; m[3] = 0x90; m[4] = 0x18; m[5] = 0x91;
    mb_expect_u(sp_do(t, m, 6, NS(1000000)), 0xF0, "read after write of LCR: 0xF0");
    m[0] = 0x06; m[1] = 0;
    sp_msg(t, m, 2, in, NULL);
    mb_expect_u(in[1], 0x03, "the SC16IS750's LCR read through the bridge");
    m[0] = 0x00; m[1] = 2; m[2] = 0x90; m[3] = 0x00; m[4] = 0x55;
    sp_msg(t, m, 5, NULL, NULL);
    /* The THR write lands at the I2C data byte; its frame follows. */
    {
        SimTime spent = 0;

        while (spent < NS(400000) && sp_get2(t, "TX") != LOGIC_0) {
            mb_run(b, NS(50));
            spent += NS(50);
        }
        mb_run(b, 16 * BIT1 / 2);
        frame[0] = "01XZ"[sp_get2(t, "TX")];
        for (i = 1; i <= 9; i++) {
            mb_run(b, 16 * BIT1);
            frame[i] = "01XZ"[sp_get2(t, "TX")];
        }
        frame[10] = '\0';
    }
    check(strcmp(frame, "0101010101") == 0, "the SC16IS750 sends 0x55 on TX");
    sp_wait_int(t, NS(1000000));
    mb_expect_u(sp_reg_r(t, 0x04), 0xF0, "the THR write: 0xF0");
    n = sim_instance_type(t->u2)->serial_sent(sim_instance_state(t->u2), 0, 0, frames, 4);
    check(n == 1 && frames[0] == 0x55, "the SC16IS750's terminal log: 0x55");
    /* The SC16IS750 NACKs the byte that sets IOControl[3]: 0xF2. */
    m[0] = 0x00; m[1] = 2; m[2] = 0x90; m[3] = 0x70; m[4] = 0x08;
    mb_expect_u(sp_do(t, m, 5, NS(1000000)), 0xF2, "SRESET byte NACKed by the target: 0xF2");
    m[0] = 0x02; m[1] = 1; m[2] = 1; m[3] = 0x90; m[4] = 0x18; m[5] = 0x91;
    sp_do(t, m, 6, NS(1000000));
    m[0] = 0x06; m[1] = 0;
    sp_msg(t, m, 2, in, NULL);
    mb_expect_u(in[1], 0x1D, "the SC16IS750 was reset (LCR 0x1D)");
    sp_close(t);
    case_end("I9 SC18IS604 -> SC16IS750: SPI commands, I2C with PULLUP-3V3 on SDA/SCL, the "
             "SC16IS750 sends 0x55; read after write; data NACK 0xF2");
}

static void run_i_cases(void) {
    case_i1();
    case_i2();
    case_i3();
    case_i4();
    case_i5();
    case_i6();
    case_i7();
    case_i8();
    case_i9();
}
