/* serial_bench_bridge (split from the serial tests, plan-68-G). */
#include "serial_bench.h"
#include "serial_bench_bridge.h"


const char *const BR_PULLS_SC[] = {"SCL/SCLK", "SDA", "IRQ", NULL};
const char *const BR_PULLS_XR[] = {"SCL", "SDA", "IRQ", NULL};

void br_reset(Br *r) {
    MB *b = r->b;

    mb_set(b, "RESET", LOGIC_0);
    mb_run(b, NS(4000));
    mb_set(b, "RESET", LOGIC_1);
    mb_run(b, NS(4000));
}

/* A bridge on a bench. xr: the MaxLinear pin names. ties as mb_open. */
Br br_open(const char *part, int spi, int xr, const char *const *ties) {
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

unsigned br_sub(unsigned reg, unsigned ch) {
    return ((reg & 15u) << 3) | ((ch & 3u) << 1);
}

/* A register write: the I2C ACK of the data byte (0 = ACK) or 0 for SPI. */
LogicValue br_w(Br *r, unsigned ch, unsigned reg, unsigned v) {
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
unsigned br_r(Br *r, unsigned ch, unsigned reg, char *text) {
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

void br_expect(Br *r, unsigned ch, unsigned reg, unsigned want, const char *what) {
    mb_expect_u(br_r(r, ch, reg, NULL), want, what);
}

void br_expect_text(Br *r, unsigned ch, unsigned reg, const char *want, const char *what) {
    char text[16];
    char message[160];

    br_r(r, ch, reg, text);
    snprintf(message, sizeof message, "%s: read %s, want %s", what, text, want);
    check(strcmp(text, want) == 0, message);
}

/* 8N1 with divisor d. */
void br_program(Br *r, unsigned ch, unsigned d) {
    br_w(r, ch, 3, 0x80);
    br_w(r, ch, 0, d & 0xFFu);
    br_w(r, ch, 1, d >> 8);
    br_w(r, ch, 3, 0x03);
}

/* SPI mode 0, one frame of n bytes; MISO bytes into in (may be NULL). */
void br_spi_frame(MB *b, const uint8_t *out, unsigned n, uint8_t *in, char *text) {
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
long long br_width(MB *b, const char *pin, LogicValue level, SimTime limit) {
    SimTime spent = 0;
    SimTime step = NS(20);

    while (spent <= limit && mb_get(b, pin) == level) {
        mb_run(b, step);
        spent += step;
    }
    return (long long)spent;
}

int near(long long got, long long want, long long tol) {
    return got >= want - tol && got <= want + tol;
}

void expect_near(long long got, long long want, long long tol, const char *what) {
    char message[200];

    snprintf(message, sizeof message, "%s: %lld ns, want %lld ns", what, got / 1000, want / 1000);
    check(near(got, want, tol), message);
}

/* One bit time of a 0x55 frame on `pin`: the width of data bit 0 (a 1
   after the start bit), measured from its edges. */
long long br_bit_time(MB *b, const char *pin) {
    if (mb_wait(b, pin, LOGIC_0, NS(400000), NS(20)) < 0 ||
        mb_wait(b, pin, LOGIC_1, NS(400000), NS(20)) < 0) {
        return -1;
    }
    return br_width(b, pin, LOGIC_1, NS(400000));
}

/* Waits for a start bit on `pin` and returns the frame "start d0..d7 stop". */
void br_frame(MB *b, const char *pin, SimTime bt, char *out, SimTime limit) {
    if (mb_wait(b, pin, LOGIC_0, limit, NS(20)) < 0) {
        strcpy(out, "(none)");
        return;
    }
    mb_frame(b, pin, bt, 9, out);
}

/* The address of strap combination (a1 tie, a0 tie) per the datasheet table. */
unsigned nxp_address(int a1, int a0) {
    static const unsigned BASE[4] = {0x4C, 0x48, 0x50, 0x54};   /* VSS, VDD, SCL, SDA */
    static const unsigned OFF[4] = {1, 0, 2, 3};

    return BASE[a1] + OFF[a0];
}

unsigned xr_address(int a1, int a0) {
    static const unsigned BASE[4] = {0x34, 0x30, 0x30, 0x34};   /* GND/SDA 0x34, VCC/SCL 0x30 */
    static const unsigned OFF[4] = {1, 0, 2, 3};

    return BASE[a1] + OFF[a0];
}

/* Every A1/A0 tie (low, high, SCL, SDA): the table's address ACKs, the one
   above it NACKs. */
void address_table(const char *part, int xr) {
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
