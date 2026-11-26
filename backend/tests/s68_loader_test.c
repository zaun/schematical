/*
 * s68_loader_test (plan-68-S): the loader rules of the pio, cpu, mcu and pld
 * kinds that no shipped device breaks. Each case is one change to a shipped
 * device (support/s68_mutate.c) that the loader must refuse with its message,
 * or, for the few accepted shapes, load with the facts read back.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "s68_mutate.h"

static int failures;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "ok" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

#define REJECTS(kind, device, patch, want) \
    expect(s68_rejects(kind, device, patch, want), kind " " device ": " want)

/* A patch adding `extra` pins (numbers 41...) to the p40 pinout, every part
   of the device on p40 with no footprint (the pin count is what is refused). */
static const char *wide_pinout(char *out, size_t cap, int extra, const char *part) {
    size_t n = (size_t)snprintf(out, cap, "{'parts':[%s]", part);
    int i;

    for (i = 0; i < extra; i++) {
        n += (size_t)snprintf(out + n, cap - n,
                              ",'pinouts.p40.%d':{'number':%d,'name':'X%d','role':'nc'}", 40 + i,
                              41 + i, i);
    }
    snprintf(out + n, cap - n, "}");
    return out;
}

/* ---- pio (pio.c, pio_pia6521.c, pio_via6522.c) --------------------------------- */

static void pio_cases(void) {
    char wide[8192];
    const Part *part;

    REJECTS("pio", "W65C22N", "{'data':5}", "data");
    REJECTS("pio", "W65C22N", "{'data':['D0','D1','D2']}", "\"data\" needs exactly 8 pin names");
    REJECTS("pio", "W65C22N", "{'phi2':'NOPE'}", "NOPE");
    REJECTS("pio", "W65C22N", "{'pb.7':'NOPE'}", "pb[7]");
    REJECTS("pio", "W65C22N", "{'cb2':'NOPE'}", "cb2");
    REJECTS("pio", "W65C22N", "{'irq':'NOPE'}", "irq");
    REJECTS("pio", "W65C21N", "{'irqb':'NOPE'}", "irqb");
    REJECTS("pio", "W65C22N", "{'model':5}", "model");
    REJECTS("pio", "W65C21N", "{'irq':'IRQAB'}", "\"irq\" is not a key of model \"pia6521\"");
    REJECTS("pio", "W65C22N", "{'irqa':'IRQB'}", "\"irqa\" is not a key of model \"via6522\"");
    REJECTS("pio", "W65C22N", "{'grades':null}", "grades");
    REJECTS("pio", "W65C22N", "{'grades':{}}", "\"grades\" must be a non-empty object");
    REJECTS("pio", "W65C22N", "{'grades':[1]}", "\"grades\" must be a non-empty object");
    REJECTS("pio", "W65C22N", "{'grades.14':5}", "a grade is an object of timing values");
    REJECTS("pio", "W65C22N", "{'grades.14.tfoo':5}", "tfoo");
    REJECTS("pio", "W65C22N", "{'grades.14.tacr':'slow'}", "tacr");
    REJECTS("pio", "W65C22N", "{'grades.14.tcyc':0}", "\"tcyc\" must be more than 0");
    REJECTS("pio", "W65C21N", "{'grades.14.tc':0}", "\"tc\" must be more than 0");
    REJECTS("pio", "W65C22N", "{'grades.14.note':5}", "\"note\" must be a string");
    REJECTS("pio", "W65C22N", "{'grades.14+':{'tcyc':70,'tpwh':35,'tpwl':35}}",
            "duplicate key \"14\"");
    REJECTS("pio", "W65C22N", "{'parts.0.grade':5}", "grade");
    REJECTS("pio", "W65C22N", "{'parts.0.grade':'99'}", "grade \"99\" is not in \"grades\"");
    REJECTS("pio", "W65C22N",
            wide_pinout(wide, sizeof wide, 25,
                        "{'name':'VIA-WIDE','manufacturer':'Generic','package':'PDIP',"
                        "'pinout':'p40','grade':'14'}"),
            "a via6522 has at most 64 pins");

    /* Two grades: the second one is found by name, with its own facts. */
    part = s68_loads("pio", "W65C22N",
                     "{'grades.7':{'tcyc':140,'tpwh':70,'tpwl':70,'vcc':[4.5,5.5]},'parts.0.grade':'7'}");
    expect(part != NULL && part->pio != NULL && strcmp(part->pio->grade, "7") == 0 &&
               part->pio->max_hz == 1000000000000ull / 140000,
           "pio W65C22N: a second grade \"7\" loads; tcyc 140 ns gives 7.14 MHz");
}

/* ---- cpu (cpu.c) --------------------------------------------------------------- */

static void cpu_cases(void) {
    char wide[8192];
    const Part *part;

    REJECTS("cpu", "W65C02S", "{'core':5}", "core");
    REJECTS("cpu", "W65C02S", "{'address':5}", "address");
    REJECTS("cpu", "W65C02S", "{'data':5}", "data");
    REJECTS("cpu", "W65C02S", "{'grades':null}", "grades");
    REJECTS("cpu", "W65C02S", "{'grades':{}}", "\"grades\" must be a non-empty object");
    REJECTS("cpu", "W65C02S", "{'grades.14':'fast'}", "a grade is an object of timing values");
    REJECTS("cpu", "W65C02S", "{'grades.14.tfoo':1}", "tfoo");
    REJECTS("cpu", "W65C02S", "{'grades.14.tads':'late'}", "tads");
    REJECTS("cpu", "W65C02S", "{'grades.14.note':[]}", "\"note\" must be a string");
    REJECTS("cpu", "W65C02S", "{'grades.14+':{'tcyc':70,'tpwh':35,'tpwl':35}}",
            "duplicate key \"14\"");
    REJECTS("cpu", "W65C02S", "{'address.15':'NOPE'}", "address[15]");
    REJECTS("cpu", "W65C02S", "{'data.7':'NOPE'}", "data[7]");
    REJECTS("cpu", "W65C02S", "{'parts.0.grade':14}", "grade");
    REJECTS("cpu", "W65C02S",
            wide_pinout(wide, sizeof wide, 25,
                        "{'name':'CPU-WIDE','manufacturer':'Generic','package':'PDIP',"
                        "'pinout':'p40','grade':'14'}"),
            "a processor has at most 64 pins");

    part = s68_loads("cpu", "MCS6502", NULL);
    expect(part != NULL && part->kind == PART_CPU,
           "cpu MCS6502: the unchanged copy loads (the cases above change one thing each)");
}

/* ---- mcu (mcu.c) --------------------------------------------------------------- */

static void mcu_cases(void) {
    const Part *part;

    REJECTS("mcu", "atmega328p", "{'model':5}", "model");
    REJECTS("mcu", "atmega328p", "{'core':5}", "core");
    REJECTS("mcu", "atmega328p", "{'eeprom':-1}", "eeprom");
    REJECTS("mcu", "atmega328p", "{'clock':'fast'}", "clock");
    REJECTS("mcu", "atmega328p", "{'ram':'lots'}", "ram");
    REJECTS("mcu", "atmega328p", "{'vcc':'5V'}", "vcc");
    REJECTS("mcu", "atmega328p", "{'parts.0.clock':0}", "\"clock\" must be above 0");
    REJECTS("mcu", "atmega328p", "{'parts.0.psram':'x'}", "psram");
    REJECTS("mcu", "at90can128",
            "{'pinouts.quad64':[{'number':1,'name':'GND','role':'ground'},"
            "{'number':2,'name':'PA0','role':'inout'}],'parts':[{'name':'CAN-NOPOWER',"
            "'manufacturer':'Generic','package':'TQFP','pinout':'quad64'}]}",
            "an mcu pinout needs a power pin");

    part = s68_loads("mcu", "at90can128", "{'parts.0.psram':65536,'parts.0.clock':8000000}");
    expect(part != NULL && part->mcu != NULL && part->mcu->psram == 65536 &&
               part->mcu->max_hz == 8000000 && part->mcu->flash == 131072,
           "mcu at90can128: a part's own psram and clock override the device's");
}

/* ---- pld (pld.c) --------------------------------------------------------------- */

static void pld_cases(void) {
    REJECTS("pld", "atf16v8b", "{'vcc':'5V'}", "vcc");
    REJECTS("pld", "atf16v8b", "{'vcc':[5.5,4.5]}", "vcc");
}

int main(void) {
    pio_cases();
    cpu_cases();
    mcu_cases();
    pld_cases();
    s68_free();
    if (failures > 0) {
        printf("s68_loader_test: %d failures\n", failures);
        return 1;
    }
    printf("s68_loader_test: all passed\n");
    return 0;
}
