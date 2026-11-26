/* serialmem_bench: shared by the programs split from serialmem_test.c (plan-68-H). */
#include "serialmem_bench.h"


int failures;
PartLibrary *library;

void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

int load_fixtures(void) {
    static const char *const FILES[] = {
        FILE_TEXT("e40", E40_KEYS, "FX-E40", ",\"maxClock\":5"),
        FILE_TEXT("e320", E320_KEYS, "FX-E320", ""),
        FILE_TEXT("fr", FR_KEYS, "FX-FR", ""),
        FILE_TEXT("fl", FL_KEYS, "FX-FL", ""),
        FILE_TEXT("fb", FB_KEYS, "FX-FB", ""),
        FILE_TEXT("df", DF_KEYS, "FX-DF", ""),
        FILE_TEXT("pe", PE_KEYS, "FX-PE", "")
    };
    size_t i;

    for (i = 0; i < sizeof FILES / sizeof FILES[0]; i++) {
        char *error = NULL;

        if (load_parts(library, FILES[i], "serialmem-fixture.json", &error) != 0) {
            fprintf(stderr, "fixture %zu: %s\n", i, error != NULL ? error : "(none)");
            free(error);
            return -1;
        }
    }
    return 0;
}

Rig rig_open(const char *name, unsigned mode) {
    Rig r;

    r.part = part_library_find(library, name);
    r.b = bench_create(r.part);
    r.mode = mode;
    bench_set(r.b, "CS#", LOGIC_1);
    bench_set(r.b, "WP#", LOGIC_1);
    bench_set(r.b, "HOLD#", LOGIC_1);
    bench_set(r.b, "SI", LOGIC_0);
    bench_set(r.b, "SCK", mode == 3 ? LOGIC_1 : LOGIC_0);
    bench_run(r.b, NS(1000));
    return r;
}

void rig_close(Rig *r) {
    bench_destroy(r->b);
}

void cs_low(Rig *r) {
    bench_set(r->b, "CS#", LOGIC_0);
    bench_run(r->b, HALF);
}

void cs_high(Rig *r) {
    if (r->mode == 0) {
        bench_run(r->b, HALF);   /* the last SCK fall settles first */
    }
    bench_set(r->b, "CS#", LOGIC_1);
    bench_run(r->b, HALF);
}

/* One SCK cycle with SI = si; the level SO has before the rising edge. */
LogicValue bit_clock(Rig *r, LogicValue si) {
    LogicValue so;

    if (r->mode == 3) {
        bench_set(r->b, "SCK", LOGIC_0);
    }
    bench_set(r->b, "SI", si);
    bench_run(r->b, HALF);
    so = bench_get(r->b, "SO");
    bench_set(r->b, "SCK", LOGIC_1);
    bench_run(r->b, HALF);
    if (r->mode == 0) {
        bench_set(r->b, "SCK", LOGIC_0);
    }
    return so;
}

void byte_clock(Rig *r, uint8_t out, Rx *rx, size_t index) {
    int bit;

    for (bit = 7; bit >= 0; bit--) {
        LogicValue so = bit_clock(r, (out >> bit) & 1 ? LOGIC_1 : LOGIC_0);

        if (rx != NULL) {
            if (so == LOGIC_1) {
                rx->v[index] |= (uint8_t)(1u << bit);
            } else if (so == LOGIC_Z) {
                rx->z[index] |= (uint8_t)(1u << bit);
            } else if (so == LOGIC_X) {
                rx->x[index] |= (uint8_t)(1u << bit);
            }
        }
    }
}

/* One transaction: ntx bytes out, then nrx bytes read (zeros out). */
void spi(Rig *r, const uint8_t *tx, size_t ntx, size_t nrx, Rx *rx) {
    size_t i;

    if (rx != NULL) {
        memset(rx, 0, sizeof *rx);
    }
    cs_low(r);
    for (i = 0; i < ntx; i++) {
        byte_clock(r, tx[i], NULL, 0);
    }
    for (i = 0; i < nrx; i++) {
        byte_clock(r, 0, rx, i);
    }
    cs_high(r);
}

/* A transaction cut after `bits` bits (the bytes of tx, MSB first). */
void spi_bits(Rig *r, const uint8_t *tx, unsigned bits) {
    unsigned i;

    cs_low(r);
    for (i = 0; i < bits; i++) {
        bit_clock(r, (tx[i / 8] >> (7 - i % 8)) & 1 ? LOGIC_1 : LOGIC_0);
    }
    cs_high(r);
}

int rx_is(const Rx *rx, size_t i, uint8_t value) {
    return rx->v[i] == value && rx->z[i] == 0 && rx->x[i] == 0;
}
int rx_z(const Rx *rx, size_t i) {
    return rx->z[i] == 0xFF;
}
int rx_x(const Rx *rx, size_t i) {
    return rx->x[i] == 0xFF;
}

void wait_us(Rig *r, double us) {
    bench_run(r->b, (SimTime)(us * 1e6));
}

/* The first RDSR byte (opcode 05). */
uint8_t rdsr(Rig *r) {
    Rx rx;

    SPI(r, &rx, 1, 0x05);
    return rx.v[0];
}

Rx rdsr_rx(Rig *r) {
    Rx rx;

    SPI(r, &rx, 1, 0x05);
    return rx;
}

/* The memory model's contents, without the SPI. */
int mem(Rig *r, uint32_t address, uint8_t *value, uint8_t *known) {
    const SimComponentType *type = kind_model(r->part);
    uint32_t v = 0, k = 0;

    if (type->memory_read(bench_state(r->b), address, 1, &v, &k) != 1) {
        return 0;
    }
    *value = (uint8_t)v;
    *known = (uint8_t)k;
    return 1;
}

int mem_is(Rig *r, uint32_t address, uint8_t want) {
    uint8_t v, k;

    return mem(r, address, &v, &k) && k == 0xFF && v == want;
}

int mem_unknown(Rig *r, uint32_t address) {
    uint8_t v, k;

    return mem(r, address, &v, &k) && k == 0;
}
