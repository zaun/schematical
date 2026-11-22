/*
 * load_part_file (plan-32-A): a single part file (format "schematical-part")
 * with one device, one part, a uuid and optional inline footprints loads
 * into a library that already holds the bundled files, all or nothing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parts.h"
#include "read_file.h"

static int failures;

#define CHECK(cond, ...)                                     \
    do {                                                     \
        if (!(cond)) {                                       \
            fprintf(stderr, "FAIL %s:%d: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__);                    \
            fputc('\n', stderr);                             \
            failures++;                                      \
        }                                                    \
    } while (0)

#define UUID_A "6F1C2E8A-3B7D-4C55-9A61-0E2B4D7F9C10"
#define UUID_B "0A1B2C3D-4E5F-4061-8273-94A5B6C7D8E9"

/* One explicit 5-pad footprint, in the footprints.json entry format. */
#define FOOTPRINT(id, pins_key)                                                                  \
    "{\"id\":\"" id "\",\"description\":\"Test pads\",\"style\":\"explicit\",\"pins\":5,"        \
    "\"body\":{\"width\":3.0,\"height\":3.0},\"pads\":["                                         \
    "{\"pin\":1,\"x\":-1,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"             \
    "{\"pin\":2,\"x\":-1,\"y\":0,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"              \
    "{\"pin\":3,\"x\":-1,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"              \
    "{\"pin\":4,\"x\":1,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"}" pins_key "]}"

#define PAD5 ",{\"pin\":5,\"x\":1,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"}"

/* A copy of the bundled 1G00 device (same device id), with one part. */
#define DEVICE(part_name, footprint)                                                             \
    "{\"id\":\"1G00\",\"function\":\"NAND\",\"description\":\"Single 2-input NAND gate\","       \
    "\"drawer\":[\"Custom\",\"Gates\"],"                                                         \
    "\"pinouts\":{\"p5\":[{\"number\":1,\"name\":\"A\",\"role\":\"input\"},"                     \
    "{\"number\":2,\"name\":\"B\",\"role\":\"input\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"}," \
    "{\"number\":4,\"name\":\"Y\",\"role\":\"output\"},{\"number\":5,\"name\":\"VCC\",\"role\":\"power\"}]}," \
    "\"gates\":[{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"}],"                   \
    "\"timing\":{\"HC\":{\"rise\":9,\"fall\":9,\"vcc\":[2,6]}},"                                 \
    "\"parts\":[{\"name\":\"" part_name "\",\"manufacturer\":\"Generic\",\"package\":\"SOT-23\"," \
    "\"pinout\":\"p5\",\"footprint\":\"" footprint "\",\"family\":\"HC\"}]}"

/* The same device with a manufacturer and a datasheet id (plan-46-A). */
#define SHEET_DEVICE(part_name, manufacturer, datasheet)                                         \
    "{\"id\":\"1G00\",\"function\":\"NAND\",\"description\":\"Single 2-input NAND gate\","       \
    "\"drawer\":[\"Custom\",\"Gates\"],"                                                         \
    "\"pinouts\":{\"p5\":[{\"number\":1,\"name\":\"A\",\"role\":\"input\"},"                     \
    "{\"number\":2,\"name\":\"B\",\"role\":\"input\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"}," \
    "{\"number\":4,\"name\":\"Y\",\"role\":\"output\"},{\"number\":5,\"name\":\"VCC\",\"role\":\"power\"}]}," \
    "\"gates\":[{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"}],"                   \
    "\"timing\":{\"HC\":{\"rise\":9,\"fall\":9,\"vcc\":[2,6]}},"                                 \
    "\"parts\":[{\"name\":\"" part_name "\",\"manufacturer\":\"" manufacturer "\","              \
    "\"package\":\"SOT-23\",\"datasheet\":\"" datasheet "\","                                    \
    "\"pinout\":\"p5\",\"footprint\":\"SOT23-5\",\"family\":\"HC\"}]}"

#define HEAD(uuid) "{\"format\":\"schematical-part\",\"version\":1,\"uuid\":\"" uuid "\",\"kind\":\"gate\","

/* Loads `text`; expects a failure whose message contains `want`, and the
   library's part and footprint counts unchanged (case g). */
static void expect_error(PartLibrary *library, const char *label, const char *text,
                         const char *want) {
    size_t parts = part_library_count(library);
    size_t footprints = part_library_footprint_count(library);
    char *error = NULL;

    if (load_part_file(library, text, "test.schpart", &error) == 0) {
        CHECK(0, "%s: loaded, expected an error containing \"%s\"", label, want);
        return;
    }
    CHECK(error != NULL && strstr(error, want) != NULL, "%s: error \"%s\" lacks \"%s\"", label,
          error != NULL ? error : "(null)", want);
    CHECK(part_library_count(library) == parts, "%s: part count changed %zu -> %zu", label, parts,
          part_library_count(library));
    CHECK(part_library_footprint_count(library) == footprints,
          "%s: footprint count changed %zu -> %zu", label, footprints,
          part_library_footprint_count(library));
    if (error != NULL) {
        printf("ok %s: %s\n", label, error);
    }
    free(error);
}

int main(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    size_t parts, footprints;
    const Part *part;

    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        fprintf(stderr, "could not load the bundled parts\n");
        return 1;
    }
    parts = part_library_count(library);
    footprints = part_library_footprint_count(library);

    /* (a) A valid gate file with one inline footprint. */
    {
        static const char TEXT[] = HEAD(UUID_A) "\"footprints\":[" FOOTPRINT("CUSTOM-6F1C2E8A", PAD5)
                                   "],\"devices\":[" DEVICE("MY-NAND", "CUSTOM-6F1C2E8A") "]}";

        CHECK(load_part_file(library, TEXT, "a.schpart", &error) == 0, "(a) %s", error);
        free(error);
        error = NULL;
        part = part_library_find(library, "MY-NAND");
        CHECK(part != NULL, "(a) MY-NAND not found");
        CHECK(part != NULL && part->uuid != NULL && strcmp(part->uuid, UUID_A) == 0,
              "(a) uuid differs");
        CHECK(part_library_find_uuid(library, UUID_A) == part, "(a) find by uuid");
        CHECK(part != NULL && part->footprint != NULL &&
                  strcmp(part->footprint->id, "CUSTOM-6F1C2E8A") == 0 &&
                  part->footprint->pad_count == 5,
              "(a) inline footprint not attached");
        CHECK(part_library_find_footprint(library, "CUSTOM-6F1C2E8A") != NULL,
              "(a) inline footprint not in the library");
        CHECK(part_library_count(library) == parts + 1, "(a) part count");
        CHECK(part_library_footprint_count(library) == footprints + 1, "(a) footprint count");
        CHECK(part_library_find(library, "SN74AHC1G00DBVR") != NULL &&
                  part_library_find(library, "SN74AHC1G00DBVR")->uuid == NULL,
              "(a) a bundled part has a uuid");
        puts("ok (a) inline footprint part loads, found by name and uuid");
    }

    /* (b) A bundled footprint and no inline footprints. */
    {
        static const char TEXT[] = HEAD(UUID_B) "\"devices\":[" DEVICE("MY-NAND-2", "SOT23-5") "]}";

        CHECK(load_part_file(library, TEXT, "b.schpart", &error) == 0, "(b) %s", error);
        free(error);
        error = NULL;
        part = part_library_find(library, "MY-NAND-2");
        CHECK(part != NULL && part->footprint != NULL && strcmp(part->footprint->id, "SOT23-5") == 0,
              "(b) bundled footprint not attached");
        CHECK(part_library_footprint_count(library) == footprints + 1, "(b) footprint count");
        puts("ok (b) bundled footprint part loads");
    }

    /* (c) Missing uuid. */
    expect_error(library, "(c)",
                 "{\"format\":\"schematical-part\",\"version\":1,\"kind\":\"gate\",\"devices\":["
                 DEVICE("C-PART", "SOT23-5") "]}",
                 "\"uuid\"");
    expect_error(library, "(c) lowercase", HEAD("6f1c2e8a-3b7d-4c55-9a61-0e2b4d7f9c11")
                 "\"devices\":[" DEVICE("C-PART", "SOT23-5") "]}", "\"uuid\" must be");
    expect_error(library, "(c) reused", HEAD(UUID_B) "\"devices\":[" DEVICE("C-PART", "SOT23-5") "]}",
                 "is already loaded");

    /* (d) Two devices; one device with two parts. */
    expect_error(library, "(d)",
                 HEAD("11111111-2222-4333-8444-555555555555") "\"devices\":["
                 DEVICE("D-PART", "SOT23-5") "," DEVICE("D-PART-2", "SOT23-5") "]}",
                 "exactly one device");
    expect_error(library, "(d) two parts",
                 HEAD("11111111-2222-4333-8444-555555555555") "\"devices\":[{\"id\":\"x\","
                 "\"function\":\"f\",\"description\":\"d\",\"pinouts\":{},\"parts\":[{},{}]}]}",
                 "exactly one part");

    /* (e) An inline footprint id equal to a bundled one. */
    expect_error(library, "(e)",
                 HEAD("22222222-2222-4333-8444-555555555555") "\"footprints\":["
                 FOOTPRINT("SOT23-5", PAD5) "],\"devices\":[" DEVICE("E-PART", "SOT23-5") "]}",
                 "duplicate footprint id \"SOT23-5\"");

    /* (f) An inline footprint with a pad missing: the footprint error text. */
    expect_error(library, "(f)",
                 HEAD("33333333-2222-4333-8444-555555555555") "\"footprints\":["
                 FOOTPRINT("CUSTOM-33333333", "") "],\"devices\":["
                 DEVICE("F-PART", "CUSTOM-33333333") "]}",
                 "footprints[0] (CUSTOM-33333333): pin 5 has no pad");

    /* (g) A file whose footprint loads but whose part is rejected (a gate on
       an unknown pin) leaves both counts unchanged; expect_error checks them. */
    expect_error(library, "(g)",
                 HEAD("44444444-2222-4333-8444-555555555555") "\"footprints\":["
                 FOOTPRINT("CUSTOM-44444444", PAD5) "],\"devices\":["
                 DEVICE("MY-NAND", "CUSTOM-44444444") "]}",
                 "devices[0] (1G00): parts[0] (MY-NAND): duplicate part name \"MY-NAND\"");
    CHECK(part_library_find_footprint(library, "CUSTOM-44444444") == NULL,
          "(g) a rejected file's footprint stayed");
    CHECK(part_library_find_uuid(library, "44444444-2222-4333-8444-555555555555") == NULL,
          "(g) a rejected file's uuid stayed");

    /* (h) An unknown top-level key. */
    expect_error(library, "(h)",
                 HEAD("55555555-2222-4333-8444-555555555555") "\"extra\":1,\"devices\":["
                 DEVICE("H-PART", "SOT23-5") "]}",
                 "extra");

    /* (i) part_library_unload_last_part_file takes back the latest file: its
       part, uuid and inline footprint go, and the same file loads again. */
    {
        static const char TEXT[] = HEAD("77777777-2222-4333-8444-555555555555") "\"footprints\":["
                                   FOOTPRINT("CUSTOM-77777777", PAD5) "],\"devices\":["
                                   DEVICE("I-PART", "CUSTOM-77777777") "]}";
        size_t before = part_library_count(library);
        size_t before_fp = part_library_footprint_count(library);
        int round;

        for (round = 0; round < 3; round++) {
            CHECK(load_part_file(library, TEXT, "i.schpart", &error) == 0, "(i) %s", error);
            free(error);
            error = NULL;
            CHECK(part_library_find(library, "I-PART") != NULL, "(i) round %d: not loaded", round);
            CHECK(part_library_unload_last_part_file(library) == 0, "(i) round %d: unload", round);
            CHECK(part_library_find(library, "I-PART") == NULL, "(i) round %d: name stayed", round);
            CHECK(part_library_find_uuid(library, "77777777-2222-4333-8444-555555555555") == NULL,
                  "(i) round %d: uuid stayed", round);
            CHECK(part_library_find_footprint(library, "CUSTOM-77777777") == NULL,
                  "(i) round %d: footprint stayed", round);
            CHECK(part_library_count(library) == before &&
                      part_library_footprint_count(library) == before_fp,
                  "(i) round %d: counts changed", round);
            CHECK(part_library_unload_last_part_file(library) == -1, "(i) round %d: second unload",
                  round);
        }
        CHECK(part_library_find(library, "MY-NAND") != NULL &&
                  part_library_find(library, "SN74AHC1G00DBVR") != NULL &&
                  part_library_find_uuid(library, UUID_A) != NULL,
              "(i) earlier parts lost");
        /* Any other load ends the chance to take a part file back. */
        CHECK(load_part_file(library, TEXT, "i.schpart", &error) == 0, "(i) reload %s", error);
        free(error);
        error = NULL;
        CHECK(load_footprints(library,
                              "{\"format\":\"schematical-footprints\",\"version\":1,\"footprints\":[]}",
                              "empty.json", &error) == 0, "(i) empty footprints %s", error);
        free(error);
        error = NULL;
        CHECK(part_library_unload_last_part_file(library) == -1, "(i) unload after another load");
        CHECK(part_library_find(library, "I-PART") != NULL, "(i) part lost by a refused unload");
        puts("ok (i) unload takes back the latest part file");
    }

    /* (j) A pin's "activeLow" (plan-39-A) survives a single part file. */
    {
        static const char TEXT[] =
            HEAD("7A2B3C4D-5E6F-4071-8293-A4B5C6D7E8F9") "\"devices\":["
            "{\"id\":\"1G00\",\"function\":\"NAND\",\"description\":\"Single 2-input NAND gate\","
            "\"drawer\":[\"Custom\",\"Gates\"],"
            "\"pinouts\":{\"p5\":[{\"number\":1,\"name\":\"A/CS\",\"role\":\"input\",\"activeLow\":[\"CS\"]},"
            "{\"number\":2,\"name\":\"B\",\"role\":\"input\",\"activeLow\":true},"
            "{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"},"
            "{\"number\":4,\"name\":\"Y\",\"role\":\"output\"},{\"number\":5,\"name\":\"VCC\",\"role\":\"power\"}]},"
            "\"gates\":[{\"op\":\"nand\",\"inputs\":[\"A/CS\",\"B\"],\"output\":\"Y\"}],"
            "\"timing\":{\"HC\":{\"rise\":9,\"fall\":9,\"vcc\":[2,6]}},"
            "\"parts\":[{\"name\":\"J-PART\",\"manufacturer\":\"Generic\",\"package\":\"SOT-23\","
            "\"pinout\":\"p5\",\"footprint\":\"SOT23-5\",\"family\":\"HC\"}]}]}";

        CHECK(load_part_file(library, TEXT, "j.schpart", &error) == 0, "(j) %s", error);
        free(error);
        error = NULL;
        part = part_library_find(library, "J-PART");
        CHECK(part != NULL && part->pins[0].active_low == 2 && part->pins[1].active_low == 1 &&
                  part->pins[3].active_low == 0,
              "(j) active_low masks %u %u %u", part != NULL ? (unsigned)part->pins[0].active_low : 0u,
              part != NULL ? (unsigned)part->pins[1].active_low : 0u,
              part != NULL ? (unsigned)part->pins[3].active_low : 0u);
        puts("ok (j) activeLow loads from a single part file");
    }

    /* Inline datasheets (plan-46-A): an optional top-level "datasheets" array
       of datasheets.json entries without "md", named by the part. */
    {
        size_t sheets = part_library_datasheet_count(library);
        static const char TEXT[] =
            HEAD("A0000000-2222-4333-8444-555555555555")
            "\"datasheets\":[{\"id\":\"CUSTOM-A0000000\",\"url\":\"https://example.com/x.pdf\"}],"
            "\"devices\":[" SHEET_DEVICE("DS-PART", "Texas Instruments", "CUSTOM-A0000000") "]}";
        int round;

        /* (ds-a) It loads, and the part's datasheet is the inline entry. */
        CHECK(load_part_file(library, TEXT, "ds.schpart", &error) == 0, "(ds-a) %s", error);
        free(error);
        error = NULL;
        part = part_library_find(library, "DS-PART");
        CHECK(part != NULL && part->datasheet != NULL &&
                  strcmp(part->datasheet->url, "https://example.com/x.pdf") == 0 &&
                  part->datasheet->md == NULL,
              "(ds-a) the part's datasheet is not the inline entry");
        CHECK(part_library_find_datasheet(library, "CUSTOM-A0000000") != NULL,
              "(ds-a) inline datasheet not in the library");
        CHECK(part_library_datasheet_count(library) == sheets + 1, "(ds-a) datasheet count");
        puts("ok (ds-a) inline datasheet part loads");

        /* (ds-b) Unload takes the entry back; the same text loads again. */
        CHECK(part_library_unload_last_part_file(library) == 0, "(ds-b) unload");
        CHECK(part_library_datasheet_count(library) == sheets, "(ds-b) count after unload %zu != %zu",
              part_library_datasheet_count(library), sheets);
        CHECK(part_library_find_datasheet(library, "CUSTOM-A0000000") == NULL,
              "(ds-b) inline datasheet stayed");
        CHECK(part_library_find_datasheet(library, "sn74hc00") != NULL,
              "(ds-b) a bundled datasheet was lost");
        for (round = 0; round < 2; round++) {
            CHECK(load_part_file(library, TEXT, "ds.schpart", &error) == 0, "(ds-b) round %d: %s",
                  round, error);
            free(error);
            error = NULL;
            CHECK(part_library_unload_last_part_file(library) == 0, "(ds-b) round %d: unload",
                  round);
            CHECK(part_library_datasheet_count(library) == sheets, "(ds-b) round %d: count",
                  round);
        }
        puts("ok (ds-b) load, unload, load again");

        /* (ds-c) An inline id equal to a bundled one. */
        expect_error(library, "(ds-c)",
                     HEAD("A0000001-2222-4333-8444-555555555555")
                     "\"datasheets\":[{\"id\":\"sn74hc00\",\"url\":\"https://example.com/x.pdf\"}],"
                     "\"devices\":[" SHEET_DEVICE("DS-C", "Texas Instruments", "sn74hc00") "]}",
                     "duplicate datasheet id \"sn74hc00\"");
        /* (ds-d) Not https. */
        expect_error(library, "(ds-d)",
                     HEAD("A0000002-2222-4333-8444-555555555555")
                     "\"datasheets\":[{\"id\":\"CUSTOM-X\",\"url\":\"http://example.com/x.pdf\"}],"
                     "\"devices\":[" SHEET_DEVICE("DS-D", "Texas Instruments", "CUSTOM-X") "]}",
                     "\"url\" must start with https://");
        /* (ds-e) An "md" key. */
        expect_error(library, "(ds-e)",
                     HEAD("A0000003-2222-4333-8444-555555555555")
                     "\"datasheets\":[{\"id\":\"CUSTOM-X\",\"url\":\"https://example.com/x.pdf\","
                     "\"md\":\"gate-and/x.md\"}],"
                     "\"devices\":[" SHEET_DEVICE("DS-E", "Texas Instruments", "CUSTOM-X") "]}",
                     "datasheets[0]: an inline datasheet has no \"md\"");
        /* (ds-f) An entry the part does not name. */
        expect_error(library, "(ds-f)",
                     HEAD("A0000004-2222-4333-8444-555555555555")
                     "\"datasheets\":[{\"id\":\"CUSTOM-X\",\"url\":\"https://example.com/x.pdf\"}],"
                     "\"devices\":[" SHEET_DEVICE("DS-F", "Texas Instruments", "sn74hc00") "]}",
                     "datasheet \"CUSTOM-X\" is not used");
        /* (ds-g) Not an array. */
        expect_error(library, "(ds-g)",
                     HEAD("A0000005-2222-4333-8444-555555555555") "\"datasheets\":{},"
                     "\"devices\":[" SHEET_DEVICE("DS-G", "Texas Instruments", "sn74hc00") "]}",
                     "\"datasheets\" must be an array");
        /* (ds-h) A Generic part naming the inline entry. */
        expect_error(library, "(ds-h)",
                     HEAD("A0000006-2222-4333-8444-555555555555")
                     "\"datasheets\":[{\"id\":\"CUSTOM-X\",\"url\":\"https://example.com/x.pdf\"}],"
                     "\"devices\":[" SHEET_DEVICE("DS-H", "Generic", "CUSTOM-X") "]}",
                     "a Generic part has no datasheet");
        CHECK(part_library_datasheet_count(library) == sheets, "(ds) datasheet count changed");
        CHECK(part_library_find_datasheet(library, "CUSTOM-X") == NULL,
              "(ds) a rejected file's datasheet stayed");
    }

    /* (k) "originId" (plan-53-A): a valid one loads and changes nothing; a
       malformed or lower-case one is refused. */
    {
        static const char OK[] =
            "{\"format\":\"schematical-part\",\"version\":1,"
            "\"uuid\":\"7A1B2C3D-4E5F-4061-8273-94A5B6C7D8E9\","
            "\"originId\":\"" UUID_A "\",\"kind\":\"gate\","
            "\"devices\":[" DEVICE("ORIGIN-NAND", "SOT23-5") "]}";

        CHECK(load_part_file(library, OK, "k.schpart", &error) == 0, "(k) %s",
              error != NULL ? error : "");
        free(error);
        error = NULL;
        part = part_library_find_uuid(library, "7A1B2C3D-4E5F-4061-8273-94A5B6C7D8E9");
        CHECK(part != NULL && strcmp(part->name, "ORIGIN-NAND") == 0, "(k) part with originId");
        puts("ok (k) a valid originId loads");
        expect_error(library, "(k) bad originId",
                     "{\"format\":\"schematical-part\",\"version\":1,"
                     "\"uuid\":\"8A1B2C3D-4E5F-4061-8273-94A5B6C7D8E9\","
                     "\"originId\":\"not-a-uuid\",\"kind\":\"gate\","
                     "\"devices\":[" DEVICE("ORIGIN-BAD", "SOT23-5") "]}",
                     "originId");
        expect_error(library, "(k) lower-case originId",
                     "{\"format\":\"schematical-part\",\"version\":1,"
                     "\"uuid\":\"9A1B2C3D-4E5F-4061-8273-94A5B6C7D8E9\","
                     "\"originId\":\"6f1c2e8a-3b7d-4c55-9a61-0e2b4d7f9c10\",\"kind\":\"gate\","
                     "\"devices\":[" DEVICE("ORIGIN-LOW", "SOT23-5") "]}",
                     "originId");
    }

    /* load_parts still rejects the single part format, and keeps its rules. */
    {
        if (load_parts(library, HEAD("66666666-2222-4333-8444-555555555555") "\"devices\":[]}",
                       "x.json", &error) == 0) {
            CHECK(0, "load_parts accepted a single part file");
        }
        free(error);
        error = NULL;
    }

    part_library_destroy(library);
    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("PASS: load_part_file (plan-32-A cases a-h, plan-32-C unload case i, plan-39-A case j, plan-53-A case k)");
    return 0;
}
