/*
 * cpu_test: split from cpu_test.c by plan-68-H; the shared helpers are
 * support/cpu_test_bench.c.
 */

#include "cpu_test_bench.h"

/* ---- 5.1 Both cores ------------------------------------------------------- */

/* 1. Power-up lost. */
static void case_power_up(const char *name) {
    Bench *b = bench_open(name);
    CpuState state;
    char what[128];
    int x = 1;
    size_t i;

    run_cycles(b, 5);
    for (i = 0; i < 16; i++) {
        x = x && pin_value(b, b->a[i]) == LOGIC_X;
    }
    snprintf(what, sizeof what, "%s 1: power-up address, R/W, SYNC read X", name);
    expect(x && pin_value(b, b->rw) == LOGIC_X && pin_value(b, b->sync) == LOGIC_X, what);
    snprintf(what, sizeof what, "%s 1: power-up status LOST, no registers", name);
    expect(cpu_state_of(b, &state) && state.status == CPU_LOST && state.register_count == 0, what);
    bench_close(b);
}

/* 1. The same through the circuit API, and its refusal for other kinds. */
static void case_circuit_api(void) {
    Circuit *circuit = circuit_create(library);
    CpuState state;
    size_t cpu, gate;
    const char *error;

    expect(circuit_add_part(circuit, W65C02S, &cpu) == 0 &&
           circuit_add_part(circuit, "SN74HC00N", &gate) == 0 && circuit_start(circuit) == 0 &&
           circuit_advance(circuit, NS(1000)),
           "1: circuit with a W65C02S starts");
    expect(circuit_cpu_state(circuit, cpu, &state) == 0 && state.status == CPU_LOST &&
           state.register_count == 0,
           "1: circuit_cpu_state reports LOST");
    expect(circuit_cpu_state(circuit, gate, &state) == -1 &&
           (error = circuit_error(circuit)) != NULL && strcmp(error, "part 1 is not a processor") == 0,
           "1: circuit_cpu_state on a gate: \"part 1 is not a processor\"");
    circuit_destroy(circuit);
}

/* 2. Reset sequence: dummy reads at 0000 and the stack (the bench serves X
   there), the vector, then the first fetch; the CPU stays running. */
static void case_reset_sequence(const char *name) {
    static const int WANT[] = {0x0000, 0x0100, 0x01FF, 0x01FE, 0xFFFC, 0xFFFD, 0xC000};
    Bench *b = boot(name, NULL, 0);
    int ok = 1;
    size_t i;

    run_to_cycle(b, 6);
    for (i = 0; i < 7; i++) {
        const Cycle *c = cycle(b, i);

        ok = ok && c->addr == WANT[i] && c->rw == '1' && c->sync == (i == 6 ? '1' : '0');
    }
    expect(ok, label(name, "2: reset reads 0000 0100 01FF 01FE FFFC FFFD, then SYNC C000"));
    expect(status_is(b, CPU_RUNNING), label(name, "2: running after X at 0000 and 01xx"));
    bench_close(b);
}

/* 3. A 1-cycle reset is too short. */
static void case_short_reset(const char *name) {
    Bench *b = bench_open(name);

    set_pin(b, b->res, LOGIC_0);
    run_cycles(b, 1);
    set_pin(b, b->res, LOGIC_1);
    run_cycles(b, 3);
    expect(status_is(b, CPU_LOST), label(name, "3: RES low for 1 cycle stays LOST"));
    bench_close(b);
}

static const SimTime *timing(const Bench *b) {
    return ((const CpuGrade *)part_internal(b->part)->data)->t;
}

/* 3b. RES released inside tPCS before a fall (an asynchronous reset switch):
   the release takes effect one cycle later; the CPU starts normally. */
static void case_reset_setup(const char *name) {
    Bench *b = bench_open(name);
    const SimTime *t = ((const CpuGrade *)part_internal(b->part)->data)->t;
    size_t i;
    int ok = 1;

    load_byte(b, 0xFFFC, 0x00);
    load_byte(b, 0xFFFD, 0xC0);
    for (i = 0; i < 0x100; i++) {
        load_byte(b, (uint16_t)(0xC000 + i), 0xEA);
    }
    set_pin(b, b->res, LOGIC_0);
    run_cycles(b, 3);
    run_to(b, b->next_fall - t[9] + NS(1));   /* t[9] = tpcs */
    set_pin(b, b->res, LOGIC_1);
    b->release = b->next_fall + NS(1000);   /* one cycle later than in time */
    run_cycles(b, 2);
    b->reset_log = b->log_count;
    run_to_cycle(b, 6);
    for (i = 0; i < 7; i++) {
        ok = ok && cycle(b, i)->rw == '1';
    }
    expect(ok && cycle(b, 4)->addr == 0xFFFC && cycle(b, 6)->addr == 0xC000 &&
               status_is(b, CPU_RUNNING),
           label(name, "3: RES released inside tPCS: reset ends one cycle later, CPU runs"));
    bench_close(b);
}

/* 4. The address window after the PHI2 fall that ends the SYNC cycle of
   A9 41 (C000 -> C001: A0 0 -> 1). */
static void case_address_window(const char *name) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
    const SimTime *t = timing(b);
    SimTime f = fall_at(b, 7);   /* ends cycle 6, the fetch at C000 */
    LogicValue before, during, after;

    run_to(b, f + t[4] - NS(1));
    before = pin_value(b, b->a[0]);
    run_to(b, f + t[4] + NS(1));
    during = pin_value(b, b->a[0]);
    run_to(b, f + t[3] + NS(1));
    after = pin_value(b, b->a[0]);
    expect(cycle(b, 6)->addr == 0xC000 && cycle(b, 6)->sync == '1',
           label(name, "4: cycle 6 is the SYNC fetch at C000"));
    expect(before == LOGIC_0, label(name, "4: A0 still 0 just before tAH"));
    expect(during == LOGIC_X, label(name, "4: A0 X just after tAH"));
    expect(after == LOGIC_1 && address_is(b, 0xC001), label(name, "4: address C001 at tADS"));
    bench_close(b);
}

/* 5. STA $0200 with A = 41: the write cycle's R/W and data windows. */
static void case_write_timing(const char *name) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0x8D, 0x00, 0x02};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
    const SimTime *t = timing(b);
    SimTime f = fall_at(b, 11);  /* starts cycle 11, the write */
    SimTime rise = f + NS(500);
    SimTime end = f + NS(1000);
    int rw_before, rw_after, d_x, d_early, d_valid, d_hold, d_z;

    run_to(b, f + t[3] - NS(1));
    rw_before = pin_value(b, b->rw);
    run_to(b, f + t[3] + NS(1));
    rw_after = pin_value(b, b->rw);
    run_to(b, rise + NS(1));
    d_x = bus_data(b);
    run_to(b, rise + t[6] - NS(1));
    d_early = bus_data(b);
    run_to(b, rise + t[6] + NS(1));
    d_valid = bus_data(b);
    run_to(b, end + t[7] - NS(1));
    d_hold = bus_data(b);
    run_to(b, end + t[7] + NS(1));
    d_z = bus_data(b);
    expect(rw_before != LOGIC_0 && rw_after == LOGIC_0, label(name, "5: R/W 0 at tADS"));
    expect(d_x == -1 && d_early == -1, label(name, "5: data X from PHI2 rise"));
    expect(d_valid == 0x41, label(name, "5: data 41 at rise + tMDS"));
    expect(d_hold == 0x41 && d_z == -2, label(name, "5: data held to fall + tDHW, then Z"));
    expect(cycle(b, 11)->addr == 0x0200 && cycle(b, 11)->rw == '0' && b->known[0x200] &&
               b->mem[0x200] == 0x41,
           label(name, "5: the bench stores 41 at 0200"));
    bench_close(b);
}

/* 6. LDA $0300 reading X: lost. */
static void case_used_x(const char *name) {
    static const uint8_t PROGRAM[] = {0xAD, 0x00, 0x03};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);

    run_to_cycle(b, 10);
    expect(cycle(b, 9)->addr == 0x0300 && status_is(b, CPU_LOST),
           label(name, "6: LDA $0300 with X at 0300 goes LOST"));
    bench_close(b);
}

/* 7. The opcode byte changes just inside / just outside tDSR. */
static void case_setup(const char *name) {
    static const uint8_t PROGRAM[] = {0xA9, 0xEA, 0xEA, 0xEA};
    int pass;

    for (pass = 0; pass < 2; pass++) {
        Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
        const SimTime *t = timing(b);
        SimTime f = fall_at(b, 7);   /* ends the fetch at C000 */
        SimTime poke = pass == 0 ? f - t[8] + NS(1) : f - t[8] - NS(1);

        run_to(b, f - NS(300));
        b->poke_value = 0xEA;
        sim_schedule_timer(b->sim, b->tb, T_POKE, poke - sim_now(b->sim));
        run_to_cycle(b, 9);
        if (pass == 0) {
            expect(status_is(b, CPU_LOST),
                   label(name, "7: opcode change tDSR - 1 ns before the fall: LOST"));
        } else {
            expect(status_is(b, CPU_RUNNING) && cycle(b, 8)->addr == 0xC001 &&
                       cycle(b, 8)->sync == '1',
                   label(name, "7: opcode change tDSR + 1 ns before the fall: runs EA (NOP)"));
        }
        bench_close(b);
    }
}

static size_t find_cycle(const Bench *b, size_t from, int addr, char rw) {
    size_t i;

    for (i = from; b->reset_log + i < b->log_count; i++) {
        if (cycle(b, i)->addr == addr && cycle(b, i)->rw == rw) {
            return i;
        }
    }
    return (size_t)-1;
}

static size_t count_cycles(const Bench *b, size_t from, int addr) {
    size_t i;
    size_t n = 0;

    for (i = from; b->reset_log + i < b->log_count; i++) {
        n += cycle(b, i)->addr == addr;
    }
    return n;
}

/* 8. IRQ (after CLI) and NMI. */
static void case_interrupts(const char *name) {
    static const uint8_t CLI[] = {0x58};
    Bench *b = boot(name, CLI, sizeof CLI);
    size_t v;

    run_to_cycle(b, 10);
    set_pin(b, b->irq, LOGIC_0);
    run_to_cycle(b, 30);
    v = find_cycle(b, 10, 0xFFFE, '1');
    expect(v != (size_t)-1 && cycle(b, v + 1)->addr == 0xFFFF,
           label(name, "8: IRQ reads the vector at FFFE/FFFF"));
    expect(v != (size_t)-1 && v >= 3 && cycle(b, v - 1)->rw == '0' &&
               (cycle(b, v - 1)->addr & 0xFF00) == 0x0100 && cycle(b, v - 1)->data >= 0 &&
               (cycle(b, v - 1)->data & 0x10) == 0,
           label(name, "8: the P pushed by IRQ has B = 0"));
    bench_close(b);

    b = boot(name, NULL, 0);
    run_to_cycle(b, 10);
    set_pin(b, b->nmi, LOGIC_0);
    run_to_cycle(b, 60);
    v = find_cycle(b, 10, 0xFFFA, '1');
    expect(v != (size_t)-1 && cycle(b, v + 1)->addr == 0xFFFB,
           label(name, "8: NMI 1->0 reads the vector at FFFA/FFFB"));
    expect(count_cycles(b, 10, 0xFFFA) == 1, label(name, "8: NMI held at 0 does not re-trigger"));
    bench_close(b);
}

static void set_rdy(Bench *b, int level) {
    if (level) {
        sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_1, DRIVE_WEAK, 0);
    } else {
        set_pin(b, b->rdy, LOGIC_0);
    }
}

/* 9. RDY low on the read of 0300 holds it. */
static void case_rdy_read(const char *name) {
    static const uint8_t PROGRAM[] = {0xAD, 0x00, 0x03};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
    CpuState before, after;
    int ok = 1;
    size_t i;

    load_byte(b, 0x0300, 0x55);
    run_to(b, fall_at(b, 9) + NS(200));   /* cycle 9 reads 0300 */
    set_rdy(b, 0);
    cpu_state_of(b, &before);
    run_cycles(b, 3);
    cpu_state_of(b, &after);
    set_rdy(b, 1);
    run_to_cycle(b, 13);
    for (i = 9; i <= 12; i++) {
        ok = ok && cycle(b, i)->addr == 0x0300 && cycle(b, i)->rw == '1';
    }
    expect(ok, label(name, "9: the read of 0300 repeats while RDY is low"));
    expect(after.cycles == before.cycles + 3, label(name, "9: the cycle count grows while held"));
    expect(cycle(b, 13)->addr == 0xC003 && cycle(b, 13)->sync == '1',
           label(name, "9: released, the next fetch is C003"));
    bench_close(b);
}

/* 10. SO falling sets V. */
static void case_so(const char *name) {
    Bench *b = boot(name, NULL, 0);
    CpuState before, after;

    run_to_cycle(b, 10);
    cpu_state_of(b, &before);
    set_pin(b, b->so, LOGIC_0);
    run_cycles(b, 2);
    cpu_state_of(b, &after);
    expect((reg(&before, "P") & 0x40) == 0 && (reg(&after, "P") & 0x40) != 0,
           label(name, "10: SO 1->0 sets V"));
    bench_close(b);
}

/* 11. Registers, the last instruction and the cycle count. */
static void case_read_back(const char *name) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0xA2, 0x05, 0xA0, 0x07};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
    CpuState state;

    run_cycles(b, 12);   /* to the fetch of C006 */
    cpu_state_of(b, &state);
    expect(state.register_count == 6 && reg(&state, "A") == 0x41 && reg(&state, "X") == 0x05 &&
               reg(&state, "Y") == 0x07,
           label(name, "11: registers A 41, X 05, Y 07"));
    expect(state.has_instruction && strcmp(state.instruction_text, "LDY #$07") == 0 &&
               state.instruction_address == 0xC004 && state.instruction_length == 2 &&
               state.instruction_bytes[0] == 0xA0 && state.instruction_bytes[1] == 0x07,
           label(name, "11: last instruction LDY #$07 at C004, bytes A0 07"));
    expect(state.cycles == b->log_count - b->reset_log,
           label(name, "11: the cycle count equals the logged cycles since reset"));
    bench_close(b);

    {
        /* JSR reads its high byte after two stack writes; it still counts. */
        static const uint8_t JSR[] = {0x20, 0x00, 0xC1};

        b = boot(name, JSR, sizeof JSR);
        load_byte(b, 0xC100, 0xEA);
        run_to_cycle(b, 12);   /* JSR is cycles 6-11; the fetch at C100 is 12 */
        cpu_state_of(b, &state);
        expect(cycle(b, 12)->addr == 0xC100 && state.has_instruction &&
                   strcmp(state.instruction_text, "JSR $C100") == 0 &&
                   state.instruction_length == 3 && state.instruction_bytes[2] == 0xC1,
               label(name, "11: last instruction JSR $C100, bytes 20 00 C1"));
        bench_close(b);
    }
}

/* 12. A high phase shorter than tPWH; a period shorter than tCYC. */
static void case_clock(const char *name) {
    Bench *b = boot(name, NULL, 0);
    const SimTime *t = timing(b);

    run_cycles(b, 3);
    b->low = NS(600);               /* the next low phase: keeps the period above tCYC */
    run_cycles(b, 1);
    b->high = t[1] - NS(5);         /* the next high phase */
    b->low = NS(500);
    run_to(b, sim_now(b->sim) + NS(500));
    b->high = NS(500);
    run_cycles(b, 2);
    expect(status_is(b, CPU_LOST), label(name, "12: a high phase below tPWH: LOST"));
    bench_close(b);

    b = boot(name, NULL, 0);
    run_cycles(b, 3);
    /* A period below tCYC with each phase at its minimum width. The
       W65C02S's tCYC is tPWH + tPWL, so there the phases are short too. */
    if (t[0] >= t[1] + t[2] + NS(20)) {
        b->low = t[2];
        run_cycles(b, 1);
        b->high = t[1];
    } else {
        b->low = t[0] / 2 - NS(5);
        run_cycles(b, 1);
        b->high = t[0] / 2 - NS(5);
    }
    run_to(b, sim_now(b->sim) + b->low);
    b->high = NS(500);
    b->low = NS(500);
    run_cycles(b, 2);
    expect(status_is(b, CPU_LOST), label(name, "12: a period below tCYC: LOST"));
    bench_close(b);
}

/* ---- 5.2 W65C02S only ------------------------------------------------------- */

/* 13. RDY low during STA's write holds the write. */
static void case_rdy_write_wdc(void) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0x8D, 0x00, 0x02};
    Bench *b = boot(W65C02S, PROGRAM, sizeof PROGRAM);
    int ok = 1;
    size_t i;

    run_to(b, fall_at(b, 11) + NS(200));   /* cycle 11 writes 0200 */
    set_rdy(b, 0);
    run_cycles(b, 2);
    set_rdy(b, 1);
    run_to_cycle(b, 14);
    for (i = 11; i <= 13; i++) {
        ok = ok && cycle(b, i)->addr == 0x0200 && cycle(b, i)->rw == '0' &&
             cycle(b, i)->data == 0x41;
    }
    expect(ok, "W65C02S 13: RDY low holds the write: 0200, R/W 0, data 41 on every held cycle");
    expect(cycle(b, 14)->addr == 0xC005 && cycle(b, 14)->sync == '1' && b->mem[0x200] == 0x41,
           "W65C02S 13: released, the write completes and the next fetch is C005");
    bench_close(b);
}

/* 14. VPB on the vector reads of reset and IRQ only. */
static void case_vpb(void) {
    static const uint8_t CLI[] = {0x58};
    Bench *b = boot(W65C02S, CLI, sizeof CLI);
    int ok = 1;
    size_t i;

    run_to_cycle(b, 10);
    set_pin(b, b->irq, LOGIC_0);
    run_to_cycle(b, 24);
    for (i = 0; i <= 24; i++) {
        const Cycle *c = cycle(b, i);
        int vector = c->addr >= 0xFFFC;

        ok = ok && (c->vpb == '0') == vector;
    }
    expect(ok && cycle(b, 4)->vpb == '0' && cycle(b, 5)->vpb == '0' &&
               find_cycle(b, 10, 0xFFFE, '1') != (size_t)-1,
           "W65C02S 14: VPB low exactly on the FFFC/FFFD and FFFE/FFFF reads");
    bench_close(b);
}

/* 15. MLB on the three RMW cycles of INC $0300. */
static void case_mlb(void) {
    static const uint8_t PROGRAM[] = {0xEE, 0x00, 0x03};
    Bench *b = boot(W65C02S, PROGRAM, sizeof PROGRAM);
    int ok = 1;
    size_t i;

    load_byte(b, 0x0300, 0x10);
    run_to_cycle(b, 14);
    for (i = 0; i <= 14; i++) {
        ok = ok && (cycle(b, i)->mlb == '0') == (i >= 9 && i <= 11);
    }
    expect(ok && cycle(b, 9)->addr == 0x0300 && cycle(b, 10)->addr == 0x0300 &&
               cycle(b, 11)->addr == 0x0300 && cycle(b, 11)->rw == '0' && b->mem[0x300] == 0x11,
           "W65C02S 15: MLB low on exactly the read, repeat read and write of 0300");
    bench_close(b);
}

static int bus_floats(const Bench *b) {
    int ok = pin_value(b, b->rw) == LOGIC_Z;
    size_t i;

    for (i = 0; i < 16; i++) {
        ok = ok && pin_value(b, b->a[i]) == LOGIC_Z;
    }
    for (i = 0; i < 8; i++) {
        ok = ok && pin_value(b, b->d[i]) == LOGIC_Z;
    }
    return ok;
}

/* 16. BE low floats the bus; the CPU keeps running. While BE is low the
   bench takes the bus (address C000, R/W 1), so the CPU reads NOPs. */
static void case_be(void) {
    Bench *b = boot(W65C02S, NULL, 0);
    CpuState before, after;
    SimTime t;
    size_t i;
    int restored = 1;

    run_cycles(b, 10);
    cpu_state_of(b, &before);
    t = sim_now(b->sim);
    set_pin(b, b->be, LOGIC_0);
    run_to(b, t + NS(25) + NS(1));
    expect(bus_floats(b), "W65C02S 16: BE 0: address, data and RWB Z after tBVD");
    for (i = 0; i < 16; i++) {
        set_pin(b, b->a[i], (0xC000 >> i) & 1 ? LOGIC_1 : LOGIC_0);
    }
    set_pin(b, b->rw, LOGIC_1);
    run_cycles(b, 3);
    cpu_state_of(b, &after);
    expect(after.status == CPU_RUNNING && after.cycles == before.cycles + 3,
           "W65C02S 16: the CPU keeps running with BE low (cycles +3)");
    for (i = 0; i < 16; i++) {
        set_pin(b, b->a[i], LOGIC_Z);
    }
    set_pin(b, b->rw, LOGIC_Z);
    t = sim_now(b->sim);
    set_pin(b, b->be, LOGIC_1);
    run_to(b, t + NS(25) + NS(1));
    for (i = 0; i < 16; i++) {
        LogicValue v = pin_value(b, b->a[i]);

        restored = restored && (v == LOGIC_0 || v == LOGIC_1);
    }
    expect(restored && pin_value(b, b->rw) == LOGIC_1, "W65C02S 16: BE 1 restores address and RWB");
    bench_close(b);
}

/* 17. WAI: waits with RDY low; IRQ with I set resumes at the next
   instruction. STP: only a reset leaves it. */
static void case_wai_stp(void) {
    static const uint8_t WAI[] = {0xCB};
    static const uint8_t STP[] = {0xDB};
    Bench *b = boot(W65C02S, WAI, sizeof WAI);
    size_t i;
    int no_vector = 1;

    run_to_cycle(b, 12);
    expect(status_is(b, CPU_WAITING) && pin_value(b, b->rdy) == LOGIC_0,
           "W65C02S 17: WAI: status WAITING, RDY driven 0");
    set_pin(b, b->irq, LOGIC_0);
    run_cycles(b, 4);
    for (i = 0; b->reset_log + i < b->log_count; i++) {
        no_vector = no_vector && cycle(b, i)->addr != 0xFFFE;
    }
    expect(status_is(b, CPU_RUNNING) && no_vector && pin_value(b, b->rdy) == LOGIC_1 &&
               find_cycle(b, 13, 0xC002, '1') != (size_t)-1,
           "W65C02S 17: IRQ with I set resumes at the next instruction, no vector, RDY released");
    bench_close(b);

    b = boot(W65C02S, STP, sizeof STP);
    run_to_cycle(b, 12);
    set_pin(b, b->nmi, LOGIC_0);
    set_pin(b, b->irq, LOGIC_0);
    run_cycles(b, 5);
    expect(status_is(b, CPU_STOPPED), "W65C02S 17: STP: STOPPED, and IRQ/NMI do not leave it");
    set_pin(b, b->nmi, LOGIC_1);
    set_pin(b, b->irq, LOGIC_1);
    do_reset(b);
    run_to_cycle(b, 5);   /* the reset sequence's vector read; STP at C000 comes next */
    expect(status_is(b, CPU_RUNNING) && cycle(b, 4)->addr == 0xFFFC,
           "W65C02S 17: a reset leaves STOPPED");
    bench_close(b);
}

/* 18. The program of the sub-plan's outcome. */
static void case_program(void) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0x8D, 0x00, 0x02, 0xDB};
    Bench *b = boot(W65C02S, PROGRAM, sizeof PROGRAM);
    CpuState state;
    size_t w;

    run_to_cycle(b, 20);
    w = find_cycle(b, 0, 0x0200, '0');
    cpu_state_of(b, &state);
    expect(w != (size_t)-1 && cycle(b, w)->data == 0x41 && b->mem[0x200] == 0x41,
           "W65C02S 18: the bench sees the write of 41 to 0200");
    expect(state.status == CPU_STOPPED && reg(&state, "A") == 0x41,
           "W65C02S 18: A = 41, status Stopped");
    expect(state.has_instruction && strcmp(state.instruction_text, "STP") == 0 &&
               state.instruction_address == 0xC005,
           "W65C02S 18: last instruction STP");
    bench_close(b);
}

/* ---- 5.3 NMOS only ------------------------------------------------------------ */

/* 19. The NMOS chip ignores RDY on a write. */
static void case_rdy_write_nmos(void) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0x8D, 0x00, 0x02};
    Bench *b = boot(MPS6502, PROGRAM, sizeof PROGRAM);

    run_to(b, fall_at(b, 11) + NS(200));   /* cycle 11 writes 0200 */
    set_rdy(b, 0);
    run_to_cycle(b, 13);
    expect(cycle(b, 11)->addr == 0x0200 && cycle(b, 11)->rw == '0' && cycle(b, 12)->rw == '1' &&
               cycle(b, 12)->addr == 0xC005 && b->mem[0x200] == 0x41,
           "MPS6502 19: RDY low during the write: it completes in one cycle");
    bench_close(b);
}

/* 20. A JAM opcode. */
static void case_jam(void) {
    static const uint8_t PROGRAM[] = {0x02};
    Bench *b = boot(MPS6502, PROGRAM, sizeof PROGRAM);

    run_to_cycle(b, 12);
    expect(status_is(b, CPU_JAMMED), "MPS6502 20: opcode 02: JAMMED");
    bench_close(b);
}

/* The 6502 boards: count 1..16 in zero page and at 0200, then STP (W65C02S)
   or JMP $8012 (MPS6502). */
static void board_6502(const char *name, int wdc) {
    static const uint8_t COMMON[] = {0xA2, 0xFF, 0x9A, 0xA9, 0x00, 0x85, 0x10, 0xE6, 0x10, 0xA5,
                                     0x10, 0x8D, 0x00, 0x02, 0xC9, 0x10, 0xD0, 0xF5};
    uint8_t image[sizeof COMMON + 3];
    size_t count = sizeof COMMON;
    CpuBoard *board;
    CpuState state;
    char what[160];
    uint16_t pc;

    memcpy(image, COMMON, sizeof COMMON);
    if (wdc) {
        image[count++] = 0xDB;
    } else {
        image[count++] = 0x4C;
        image[count++] = 0x12;
        image[count++] = 0x80;
    }
    board = cpu_board_create(library, name, image, count, 0x8000);
    if (board == NULL || !cpu_board_run(board, NS(2000000))) {
        expect(0, "board did not run");
        cpu_board_destroy(board);
        return;
    }
    cpu_board_state(board, &state);
    pc = (uint16_t)reg_value(&state, "PC");
    snprintf(what, sizeof what, "%s board: SRAM 0010 = 10 and 0200 = 10", name);
    expect(sram_is(board, 0x0010, 0x10) && sram_is(board, 0x0200, 0x10), what);
    snprintf(what, sizeof what, "%s board: A = 10, X = FF, S = FF", name);
    expect(state.register_count > 0 && reg_value(&state, "A") == 0x10 &&
               reg_value(&state, "X") == 0xFF && reg_value(&state, "S") == 0xFF,
           what);
    /* The core's PC is the next byte to read: 8012-8015 in the JMP loop. */
    snprintf(what, sizeof what, "%s board: %s", name,
             wdc ? "status STOPPED, last instruction STP"
                 : "RUNNING with PC in 8012-8015, last instruction JMP $8012");
    expect(wdc ? state.status == CPU_STOPPED && strcmp(state.instruction_text, "STP") == 0
               : state.status == CPU_RUNNING && pc >= 0x8012 && pc <= 0x8015 &&
                     strcmp(state.instruction_text, "JMP $8012") == 0 &&
                     state.instruction_address == 0x8012,
           what);
    cpu_board_destroy(board);
}

int main(void) {
    size_t i;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_fixture() != 0) {
        return 2;
    }
    for (i = 0; i < sizeof CORE_NAMES / sizeof CORE_NAMES[0]; i++) {
        case_power_up(CORE_NAMES[i]);
        case_reset_sequence(CORE_NAMES[i]);
        case_short_reset(CORE_NAMES[i]);
        case_reset_setup(CORE_NAMES[i]);
        case_address_window(CORE_NAMES[i]);
        case_write_timing(CORE_NAMES[i]);
        case_used_x(CORE_NAMES[i]);
        case_setup(CORE_NAMES[i]);
        case_interrupts(CORE_NAMES[i]);
        case_rdy_read(CORE_NAMES[i]);
        case_so(CORE_NAMES[i]);
        case_read_back(CORE_NAMES[i]);
        case_clock(CORE_NAMES[i]);
    }
    case_rdy_write_wdc();
    case_vpb();
    case_mlb();
    case_be();
    case_wai_stp();
    case_program();
    case_rdy_write_nmos();
    case_jam();
    board_6502("W65C02S6TPG-14", 1);
    board_6502("MPS6502", 0);
    case_circuit_api();

    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_test: %d failed\n", failures);
        return 1;
    }
    printf("cpu_test: all passed\n");
    return 0;
}
