/*
 * charlcd_cov_test (plan-68-R): HD44780-type bus cases the charlcd programs
 * do not drive, on the charlcd bench (tests/support/charlcd_bench.h) and its
 * FX-LCD fixture (16x2, the Newhaven timing), by the model's rules (charlcd.c,
 * plan-11-B section 4): reads of DDRAM and CGRAM data; an unknown N (the
 * address map of either line count), an unknown instruction bit, I/D, S or
 * S/C, R/W or RS, each making only what it may have changed unknown; the
 * empty instruction 00h (no busy time); E going from X to Z (no edge); and a
 * device with no hold time (a write takes effect at the fall of E).
 *
 * A data bit left Z on the bus reads X (charlcd.c: "Z reads as X").
 */

#include "charlcd_bench.h"

/* An 8-bit write with the bits outside `known` released (X to the module). */
static void wr_x(Lcd *l, int rs, uint8_t byte, uint8_t known) {
    access_write(l, rs, byte, known);
}

static void status(Lcd *l, uint8_t *v, uint8_t *k) {
    rd(l, 0, v, k);
}

static Lcd ready(void) {
    Lcd l = lcd_open("FX-LCD");

    init8(&l);                     /* 8-bit, N = 1, display on, I/D = 1, S = 0 */
    clear(&l);
    return l;
}

static void data_reads(void) {
    Lcd l = ready();
    uint8_t v, k;

    cmd(&l, 0x85, US(40));         /* DDRAM 05h */
    wr(&l, 1, 'Q');
    bench_run(l.b, US(50));
    cmd(&l, 0x85, US(40));
    rd(&l, 1, &v, &k);
    bench_run(l.b, US(50));
    expect(v == 'Q' && k == 0xFF, "a data read at DDRAM 05h returns the 'Q' written there");
    expect(read_ac(&l) == 0x06, "... and moves AC to 06h");
    cmd(&l, 0x48, US(40));         /* CGRAM 08h: character 1, row 0 */
    wr(&l, 1, 0x15);
    bench_run(l.b, US(50));
    cmd(&l, 0x48, US(40));
    rd(&l, 1, &v, &k);
    bench_run(l.b, US(50));
    expect((v & 0x1F) == 0x15 && (k & 0x1F) == 0x1F, "a data read at CGRAM 08h returns the 15h written there");
    bench_destroy(l.b);
}

static void unknown_n(void) {
    Lcd l = ready();
    uint8_t v, k;

    wr_x(&l, 0, 0x38, 0xF7);       /* Function set with N (bit 3) unknown */
    bench_run(l.b, US(40));
    cmd(&l, 0x90, US(40));         /* DDRAM 10h: the same cell for N = 0 and N = 1 */
    wr(&l, 1, 'A');
    bench_run(l.b, US(50));
    expect(read_ac(&l) == 0x11, "N unknown: a write at 10h moves AC to 11h (the same for either N)");
    cmd(&l, 0x90, US(40));
    rd(&l, 1, &v, &k);
    bench_run(l.b, US(50));
    expect(v == 'A' && k == 0xFF, "N unknown: the 'A' at 10h reads back");
    cmd(&l, 0xA7, US(40));         /* 27h: next is 40h for N = 1, 28h for N = 0 */
    wr(&l, 1, 'B');
    bench_run(l.b, US(50));
    expect(read_ac(&l) == -1, "N unknown: after a write at 27h AC is unknown (40h or 28h)");
    cmd(&l, 0xC5, US(40));         /* 45h: cell 45 for N = 1, cell 69 for N = 0 */
    wr(&l, 1, 'C');
    bench_run(l.b, US(50));
    cmd(&l, 0x90, US(40));
    rd(&l, 1, &v, &k);
    bench_run(l.b, US(50));
    expect(k != 0xFF, "N unknown: a write at 45h may hit either cell, so all of DDRAM is unknown (10h too)");
    bench_destroy(l.b);
}

static void lost_and_empty(void) {
    Lcd l = ready();
    uint8_t v, k;

    cmd(&l, 0x85, US(40));
    wr(&l, 0, 0x00);               /* 0000 0000: no instruction */
    status(&l, &v, &k);
    expect(k == 0xFF && v == 0x05, "instruction 00h: no busy time and AC still 05h (read at once)");
    bench_run(l.b, US(40));
    wr_x(&l, 0, 0x01, 0x7F);       /* bit 7 unknown: which instruction is not known */
    bench_run(l.b, MS(2));
    status(&l, &v, &k);
    expect(k == 0x00, "an instruction with DB7 unknown: LOST, BF and AC read unknown");
    wr(&l, 1, 'Z');                /* a data write while LOST: both RAMs may be hit */
    bench_run(l.b, US(50));
    expect(read_ac(&l) == -1, "a data write while LOST leaves AC unknown");
    wr(&l, 0, 0x30);               /* resync: Function Set, DL = 1 */
    status(&l, &v, &k);
    expect((k & 0x80) == 0, "the resync's busy time after LOST: BF reads unknown (DL known again)");
    bench_run(l.b, US(50));
    status(&l, &v, &k);
    expect((k & 0x80) == 0x80 && (v & 0x80) == 0, "... and known 0 once that instruction's time is over");
    bench_destroy(l.b);
}

static void unknown_shifts(void) {
    Lcd l = ready();
    LcdState st;
    uint8_t v, k;

    /* S/C unknown: the cursor or the display moved */
    cmd(&l, 0x85, US(40));
    wr_x(&l, 0, 0x14, 0xF7);
    bench_run(l.b, US(40));
    expect(read_ac(&l) == -1, "cursor/display shift with S/C unknown: AC unknown");
    lcd_state(&l, &st);
    expect(image_all(&st, LCD_DOT_X), "... and the display shift unknown: every dot X");
    /* a data write with AC unknown (DDRAM, S = 0): all of DDRAM unknown */
    wr(&l, 1, 'Y');
    bench_run(l.b, US(50));
    lcd_state(&l, &st);
    expect(st.ddram_known[0][0] == 0 && st.ddram_known[0][79] == 0,
           "a data write with AC unknown: every DDRAM cell may be the one, all unknown");
    bench_destroy(l.b);

    /* S = 1 and AC unknown: a data write may hit any cell and shift the display */
    l = ready();
    cmd(&l, 0x07, US(40));         /* I/D = 1, S = 1 */
    cmd(&l, 0x85, US(40));
    wr_x(&l, 0, 0x14, 0xF7);       /* S/C unknown: AC (and the shift) unknown */
    bench_run(l.b, US(40));
    wr(&l, 1, 'W');
    bench_run(l.b, US(50));
    cmd(&l, 0x80, US(40));         /* AC known again; the shift is still unknown */
    lcd_state(&l, &st);
    expect(st.ddram_known[0][5] == 0 && image_all(&st, LCD_DOT_X),
           "S = 1, AC unknown: a data write makes DDRAM unknown and the shift stays unknown after AC is set");
    bench_destroy(l.b);

    /* I/D unknown with S = 1: the shift on a write goes an unknown way */
    l = ready();
    wr_x(&l, 0, 0x05, 0xFD);
    bench_run(l.b, US(40));
    wr(&l, 1, 'S');
    bench_run(l.b, US(50));
    lcd_state(&l, &st);
    expect(image_all(&st, LCD_DOT_X) && read_ac(&l) == -1,
           "entry mode I/D unknown, S = 1: a write leaves the shift and AC unknown");
    bench_destroy(l.b);

    /* S unknown: a write may have shifted the display */
    l = ready();
    wr_x(&l, 0, 0x06, 0xFE);
    bench_run(l.b, US(40));
    wr(&l, 1, 'T');
    bench_run(l.b, US(50));
    lcd_state(&l, &st);
    expect(image_all(&st, LCD_DOT_X) && read_ac(&l) == 0x01,
           "entry mode S unknown: after a write the shift is unknown, AC (I/D = 1) is 01h");
    bench_destroy(l.b);

    /* CGRAM with AC unknown: only CGRAM is hit */
    l = ready();
    cmd(&l, 0x85, US(40));
    wr(&l, 1, 'Q');
    bench_run(l.b, US(50));
    cmd(&l, 0x40, US(40));
    wr_x(&l, 0, 0x10, 0xF7);       /* S/C unknown: AC unknown, still CGRAM */
    bench_run(l.b, US(40));
    wr(&l, 1, 0x1F);
    bench_run(l.b, US(50));
    cmd(&l, 0x85, US(40));
    rd(&l, 1, &v, &k);
    bench_run(l.b, US(50));
    lcd_state(&l, &st);
    expect(v == 'Q' && k == 0xFF && st.cgram_known[0][0] == 0 && st.cgram_known[0][63] == 0,
           "a CGRAM write with AC unknown: all of CGRAM unknown, DDRAM 05h still 'Q'");
    bench_destroy(l.b);
}

static void unknown_bus(void) {
    Lcd l = ready();
    uint8_t v, k;

    /* R/W unknown at E rise: it may be a write, LOST; the bus goes X */
    lv(&l, l.rs, 0);
    bench_set(l.b, l.rw, LOGIC_X);
    put_db(&l, 0, 0);
    bench_run(l.b, NS(50));
    lv(&l, l.e, 1);
    bench_run(l.b, NS(500));
    expect(db_all(&l, LOGIC_X), "R/W unknown at E rise: the module may drive, DB reads X");
    lv(&l, l.e, 0);
    bench_run(l.b, US(2));
    status(&l, &v, &k);
    expect(k == 0x00, "... and it was LOST: BF and AC unknown");
    bench_destroy(l.b);

    /* RS unknown in a read: a data read moves AC, a status read does not */
    l = ready();
    cmd(&l, 0x85, US(40));
    bench_set(l.b, l.rs, LOGIC_X);
    lv(&l, l.rw, 1);
    put_db(&l, 0, 0);
    bench_run(l.b, NS(50));
    lv(&l, l.e, 1);
    bench_run(l.b, NS(500));
    lv(&l, l.e, 0);
    bench_run(l.b, US(2));
    expect(read_ac(&l) == -1, "a read with RS unknown: AC unknown (05h or 06h)");
    bench_destroy(l.b);

    /* E from X to Z: both read X, no edge; with R/W 1 and RS 0 E X is at most a status read */
    l = ready();
    cmd(&l, 0x85, US(40));
    lv(&l, l.rs, 0);
    lv(&l, l.rw, 1);
    bench_set(l.b, l.e, LOGIC_X);
    bench_run(l.b, US(1));
    bench_set(l.b, l.e, LOGIC_Z);
    bench_run(l.b, US(1));
    lv(&l, l.e, 0);
    bench_run(l.b, US(1));
    expect(read_ac(&l) == 0x05, "E X then Z with R/W 1 and RS 0: AC still 05h");
    bench_destroy(l.b);

    /* E high then X in a read: the read may end, the module's drive goes X */
    l = ready();
    lv(&l, l.rs, 0);
    lv(&l, l.rw, 1);
    put_db(&l, 0, 0);
    bench_run(l.b, NS(50));
    lv(&l, l.e, 1);
    bench_run(l.b, NS(500));
    bench_set(l.b, l.e, LOGIC_X);
    bench_run(l.b, NS(5));
    expect(db_all(&l, LOGIC_X), "E going X in a read: the data the module drives goes X");
    lv(&l, l.e, 0);
    bench_run(l.b, US(2));
    bench_destroy(l.b);
}

/* 4-bit mode: E X with R/W 1 may have been one nibble of a read, so the phase is unknown and
   only a resync (0011 on DB7-4) is taken. */
static void four_bit_phase(void) {
    Lcd l = lcd_open("FX-LCD");
    uint8_t v, k;

    at(&l, MS(100));
    cmd(&l, 0x30, MS(5));
    cmd(&l, 0x30, US(160));
    cmd(&l, 0x30, US(160));
    cmd(&l, 0x20, US(40));         /* 4-bit */
    l.four = 1;
    cmd(&l, 0x28, US(40));
    cmd(&l, 0x0C, US(40));
    expect(read_ac(&l) == 0x00, "4-bit mode set up: AC 00h");
    lv(&l, l.rs, 0);
    lv(&l, l.rw, 1);
    bench_set(l.b, l.e, LOGIC_X);
    bench_run(l.b, US(1));
    lv(&l, l.e, 0);
    bench_run(l.b, US(1));
    wr(&l, 0, 0x80);               /* nibbles 8, 0: not a resync */
    bench_run(l.b, US(40));
    status(&l, &v, &k);
    expect(k == 0x00, "4-bit, E X in a read: the nibble phase is unknown, a non-resync write is LOST");
    bench_destroy(l.b);
}

static void no_hold(void) {
    Lcd l = lcd_open("FX-LCD-H0");
    uint8_t v, k;

    init8(&l);
    clear(&l);
    cmd(&l, 0x83, US(40));
    wr(&l, 1, 'H');
    bench_run(l.b, US(50));
    cmd(&l, 0x83, US(40));
    rd(&l, 1, &v, &k);
    expect(v == 'H' && k == 0xFF, "tH = tAH = 0: a write takes effect at the fall of E (03h reads 'H')");
    bench_destroy(l.b);
}

#define TIMING_H0 "{'tc':1200,'tpw':140,'tas':0,'tah':0,'tdsw':40,'th':0,'tddr':100,'tdhr':10}"

int main(void) {
    char *error;

    setvbuf(stdout, NULL, _IOLBF, 0);   /* the leak check ends with _exit: keep the lines */
    library = fixture_library();
    error = load_file(library, device_file("FX-LCD", "16", "2", "'A00'", "'blue-negative'", DATA,
                                           TIMING, EXEC, BACKLIGHT VCC),
                      "fx-lcd.json");
    if (error == NULL) {
        error = load_file(library, device_file("FX-LCD-H0", "16", "2", "'A00'", "'yellow-green'", DATA,
                                               TIMING_H0, EXEC, VCC),
                          "fx-lcd-h0.json");
    }
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    data_reads();
    unknown_n();
    lost_and_empty();
    unknown_shifts();
    unknown_bus();
    four_bit_phase();
    no_hold();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("charlcd_cov_test: all passed\n");
    return 0;
}
