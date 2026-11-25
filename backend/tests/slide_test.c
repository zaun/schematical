/*
 * slide_test (plan-58-B): the switch kind's "states" and "rest" keys (slide
 * switches): the loader and its errors, and a slide switch in a circuit.
 * Fixture parts (inline, on top of the library):
 *   SLIDE-SPDT    SPDT On-On,      pins C A B,             states [[1],[2]]          24 V
 *   SLIDE-SPDT3   SPDT On-Off-On,  pins C A B,             states [[1],[],[2]] rest 2
 *   SLIDE-DPDT3   DPDT On-Off-On,  pins C1 A1 B1 C2 A2 B2, states [[1,3],[],[2,4]] rest 2
 *   SLIDE-SPST    SPST On-Off,     pins A B,               states [[1],[]]     rest 2
 *   SLIDE-LV      SPDT On-On,      pins C A B,             3 V (the voltage probe)
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"

static int failures;

#define CHECK(cond, what)                                   \
    do {                                                    \
        if (cond) {                                         \
            printf("ok   %s\n", what);                      \
        } else {                                            \
            printf("FAIL %s\n", what);                      \
            failures++;                                     \
        }                                                   \
    } while (0)

#define PIN(N, NAME) "{'number':" #N ",'name':'" NAME "','role':'inout'}"
#define P2 "'p2':[" PIN(1, "A") "," PIN(2, "B") "]"
#define P3 "'p3':[" PIN(1, "C") "," PIN(2, "A") "," PIN(3, "B") "]"
#define P6 "'p6':[" PIN(1, "C1") "," PIN(2, "A1") "," PIN(3, "B1") "," PIN(4, "C2") "," PIN(5, "A2") "," PIN(6, "B2") "]"
#define SPDT_CONTACTS "[['C','A'],['C','B']]"
#define DPDT_CONTACTS "[['C1','A1'],['C1','B1'],['C2','A2'],['C2','B2']]"
#define HEAD "{'format':'schematical-parts','version':1,'kind':'switch','devices':["
#define DEVICE(ID, PINOUTS, CONTACTS, EXTRA, VOLTS, NAME, PINOUT, FOOTPRINT)                              \
    "{'id':'" ID "','function':'f','description':'d','drawer':['Switches & Relays','Slide'],"           \
    "'pinouts':{" PINOUTS "},'actuator':'Standard','contacts':" CONTACTS "," EXTRA "'ratedVolts':" VOLTS "," \
    "'parts':[{'name':'" NAME "','package':'THT','pinout':'" PINOUT "','footprint':'" FOOTPRINT "'}]}"

static const char FIXTURE[] =
    HEAD
    DEVICE("spdt", P3, SPDT_CONTACTS, "'states':[[1],[2]],", "24", "SLIDE-SPDT", "p3", "PINHDR-1X03-V") ","
    DEVICE("spdt3", P3, SPDT_CONTACTS, "'states':[[1],[],[2]],'rest':2,", "24", "SLIDE-SPDT3", "p3", "PINHDR-1X03-V") ","
    DEVICE("dpdt3", P6, DPDT_CONTACTS, "'states':[[1,3],[],[2,4]],'rest':2,", "24", "SLIDE-DPDT3", "p6", "SOT23-6") ","
    DEVICE("spst", P2, "[['A','B']]", "'states':[[1],[]],'rest':2,", "24", "SLIDE-SPST", "p2", "RES-0402") ","
    DEVICE("lv", P3, SPDT_CONTACTS, "'states':[[1],[2]],", "3", "SLIDE-LV", "p3", "PINHDR-1X03-V")
    "]}";

/* One device of the bad fixtures: SPDT contacts plus EXTRA keys. */
#define BAD(EXTRA)                                                                       \
    HEAD DEVICE("bad", P3, SPDT_CONTACTS, EXTRA, "24", "SLIDE-BAD", "p3", "PINHDR-1X03-V") "]}"

static PartLibrary *library;

static char *unquote(const char *text) {
    size_t n = strlen(text) + 1, i;
    char *copy = malloc(n);

    if (copy == NULL) {
        exit(1);
    }
    for (i = 0; i < n; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

/* Loads one bad fixture; 1 when it is refused with an error containing `message`. */
static int refused(const char *fixture, const char *message) {
    char *text = unquote(fixture);
    char *error = NULL;
    int ok;

    ok = load_parts(library, text, "slide-bad.json", &error) != 0 && error != NULL &&
         strstr(error, message) != NULL;
    if (!ok) {
        printf("     got: %s\n", error != NULL ? error : "(loaded)");
    }
    free(error);
    free(text);
    return ok;
}

static uint32_t mask_of(const char *name, unsigned state) {
    uint32_t mask = 0xFFFFFFFFu;

    return part_switch_state_contacts(part_library_find(library, name), state, &mask) ? mask : 0xFFFFFFFFu;
}

/* 1. The loader: states, rest, the masks. */
static void case_loader(void) {
    const Part *spdt = part_library_find(library, "SLIDE-SPDT");
    const Part *spdt3 = part_library_find(library, "SLIDE-SPDT3");
    const Part *dpdt3 = part_library_find(library, "SLIDE-DPDT3");
    const Part *spst = part_library_find(library, "SLIDE-SPST");
    uint32_t mask = 0;

    CHECK(spdt != NULL && spdt3 != NULL && dpdt3 != NULL && spst != NULL, "fixture: the five slide parts load");
    CHECK(spdt->dip_switch->positions == 2 && spdt->dip_switch->states == 2 && spdt->dip_switch->rest == 1,
          "SLIDE-SPDT: 2 contacts, 2 states, rest defaults to 1");
    CHECK(mask_of("SLIDE-SPDT", 1) == 1 && mask_of("SLIDE-SPDT", 2) == 2, "SLIDE-SPDT: state 1 closes contact 1, state 2 contact 2");
    CHECK(spdt3->dip_switch->states == 3 && spdt3->dip_switch->rest == 2, "SLIDE-SPDT3: 3 states, rest 2");
    CHECK(mask_of("SLIDE-SPDT3", 1) == 1 && mask_of("SLIDE-SPDT3", 2) == 0 && mask_of("SLIDE-SPDT3", 3) == 2,
          "SLIDE-SPDT3: states [[1],[],[2]]");
    CHECK(dpdt3->dip_switch->positions == 4 && dpdt3->dip_switch->states == 3 && dpdt3->dip_switch->rest == 2,
          "SLIDE-DPDT3: 4 contacts, 3 states, rest 2");
    CHECK(mask_of("SLIDE-DPDT3", 1) == 5 && mask_of("SLIDE-DPDT3", 2) == 0 && mask_of("SLIDE-DPDT3", 3) == 10,
          "SLIDE-DPDT3: states [[1,3],[],[2,4]]");
    CHECK(spst->dip_switch->states == 2 && spst->dip_switch->rest == 2 && mask_of("SLIDE-SPST", 1) == 1 &&
              mask_of("SLIDE-SPST", 2) == 0,
          "SLIDE-SPST: states [[1],[]], rest 2");
    CHECK(part_switch_state_contacts(spdt, 0, &mask) == 0 && part_switch_state_contacts(spdt, 3, &mask) == 0 &&
              part_switch_state_contacts(spdt3, 4, &mask) == 0,
          "part_switch_state_contacts: state 0 and states past the last are refused");
    CHECK(part_switch_state_contacts(part_library_find(library, "5V"), 1, &mask) == 0 &&
              part_switch_state_contacts(NULL, 1, &mask) == 0,
          "part_switch_state_contacts: a part that is not a slide switch (or NULL) is refused");
}

/* 2. The loader's errors (plan-58-B § 4.1). */
static void case_errors(void) {
    CHECK(refused(BAD("'states':[[1]],"), "\"states\" must be 2 or 3 arrays of contact numbers"),
          "error: one state");
    CHECK(refused(BAD("'states':[[1],[2],[1],[2]],"), "\"states\" must be 2 or 3 arrays of contact numbers"),
          "error: four states");
    CHECK(refused(BAD("'states':[1,2],"), "\"states\" must be 2 or 3 arrays of contact numbers"),
          "error: states that are not arrays");
    CHECK(refused(BAD("'states':[[3],[2]],"), "states[0]: contact 3 does not exist (the device has 2 contacts)"),
          "error: a contact the device does not have");
    CHECK(refused(BAD("'states':[[1],[0]],"), "states[1]: contact 0 does not exist (the device has 2 contacts)"),
          "error: contact 0");
    CHECK(refused(BAD("'states':[[1,1],[2]],"), "states[0]: contact 1 twice"), "error: a contact twice in one state");
    CHECK(refused(BAD("'states':[[1],[2]],'rest':3,"), "\"rest\" must be 1 \xE2\x80\xA6 2"), "error: rest past the last state");
    CHECK(refused(BAD("'states':[[1],[2]],'rest':0,"), "\"rest\" must be 1 \xE2\x80\xA6 2"), "error: rest 0");
    CHECK(refused(BAD("'rest':1,"), "\"rest\" needs \"states\""), "error: rest without states");
    CHECK(refused(BAD("'states':[[1],[2]],'momentary':true,"), "\"states\" cannot be combined with \"momentary\""),
          "error: states with momentary");
    CHECK(refused(BAD("'states':[[1],[2]],'normallyClosed':false,"), "\"states\" cannot be combined with \"normallyClosed\""),
          "error: states with normallyClosed");
    CHECK(refused(BAD("'states':[[1],[2]],'joined':[['A','B']],"), "\"states\" cannot be combined with \"joined\""),
          "error: states with joined");
}

int main(void) {
    char *text = unquote(FIXTURE);
    char *error = NULL;

    library = part_library_create();
    if (text == NULL || library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        fprintf(stderr, "cannot load %s\n", PARTS_DIR);
        return 1;
    }
    if (load_parts(library, text, "slide-fixture.json", &error) != 0) {
        fprintf(stderr, "fixture: %s\n", error != NULL ? error : "(none)");
        return 1;
    }
    free(text);
    case_loader();
    case_errors();
    part_library_destroy(library);
    if (failures) {
        printf("FAIL: %d checks failed\n", failures);
        return 1;
    }
    puts("PASS: slide switches (states, rest) load, and a bad fixture is refused with its message");
    return 0;
}
