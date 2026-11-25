/*
 * socket_test (plan-51-A): the socket kind (IC sockets) and the "fits" part
 * key. A DIP and a PLCC socket load and read back their facts and the chip
 * footprints they hold; every rule of the loader refuses what it should
 * (the family, the row spacing, the pin counts and roles, the mounting, the
 * finish, "fits"); "fits" is refused on another kind; and a socket in a
 * circuit is a passive part whose pins join nothing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "listing.h"
#include "parts.h"
#include "read_file.h"

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* The footprints the cases need that the bundled library does not have: a
   12-pin DIP, and a footprint whose 14 pads are named like balls (A1 … B7),
   so it has no pad "1" … "14". PDIP-14, CDIP-14 and PLCC-28 are bundled. */
static const char *const TEST_FOOTPRINTS =
    "{\"format\":\"schematical-footprints\",\"version\":1,\"footprints\":["
    "{\"id\":\"T-DIP-12\",\"description\":\"12-pin test DIP\",\"style\":\"dual\",\"pins\":12,"
    "\"body\":{\"width\":6.35,\"height\":15.0},\"pitch\":2.54,\"span\":7.62,"
    "\"pad\":{\"width\":1.5,\"height\":1.5,\"shape\":\"round\"},\"drill\":0.75},"
    "{\"id\":\"T-BALL-14\",\"description\":\"14 ball-named test pads\",\"style\":\"explicit\","
    "\"pins\":14,\"body\":{\"width\":8.0,\"height\":4.0},\"pads\":["
    "{\"ball\":\"A1\",\"x\":-3.0,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"A2\",\"x\":-2.0,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"A3\",\"x\":-1.0,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"A4\",\"x\":0.0,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"A5\",\"x\":1.0,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"A6\",\"x\":2.0,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"A7\",\"x\":3.0,\"y\":-1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"B1\",\"x\":-3.0,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"B2\",\"x\":-2.0,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"B3\",\"x\":-1.0,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"B4\",\"x\":0.0,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"B5\",\"x\":1.0,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"B6\",\"x\":2.0,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"},"
    "{\"ball\":\"B7\",\"x\":3.0,\"y\":1,\"width\":0.6,\"height\":0.6,\"shape\":\"rect\"}]}]}";

/* `count` inout pins numbered 1 … count; pin `bad` (1-based, 0 = none) has
   role "input" instead. */
static void pins_json(char *out, size_t size, unsigned count, unsigned bad) {
    size_t used = 0;
    unsigned i;

    for (i = 1; i <= count && used < size; i++) {
        used += (size_t)snprintf(out + used, size - used,
                                 "%s{\"number\":%u,\"name\":\"%u\",\"role\":\"%s\"}",
                                 i > 1 ? "," : "", i, i, i == bad ? "input" : "inout");
    }
}

/* One socket device with one part "T-<tag>". `device_keys` (each ending in a
   comma) go on the device, `part_keys` (each starting with a comma) on the
   part; `footprint` is the part's own footprint. */
static char *socket_file(const char *tag, const char *device_keys, unsigned pins, unsigned bad,
                         const char *footprint, const char *part_keys) {
    char list[4096];
    char *text = malloc(8192);

    pins_json(list, sizeof list, pins, bad);
    snprintf(text, 8192,
             "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"socket\",\"devices\":["
             "{\"id\":\"socket-%s\",\"function\":\"IC socket\",\"description\":\"A test socket\","
             "\"drawer\":[\"Connector\",\"IC Socket\"],%s\"pinouts\":{\"p\":[%s]},"
             "\"parts\":[{\"name\":\"T-%s\",\"manufacturer\":\"Generic\",\"package\":\"Socket\","
             "\"pinout\":\"p\",\"footprint\":\"%s\"%s}]}]}",
             tag, device_keys, list, tag, footprint, part_keys);
    return text;
}

/* Loads `text`; expects a failure whose message contains `want` and the
   library's part count unchanged. */
static void expect_error(const char *label, char *text, const char *want) {
    size_t parts = part_library_count(library);
    char *error = NULL;
    int status = load_parts(library, text, "socket.json", &error);
    char what[640];

    snprintf(what, sizeof what, "%s: refused with %s (got: %s)", label, want,
             status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
    expect(status != 0 && error != NULL && strstr(error, want) != NULL &&
               part_library_count(library) == parts,
           what);
    free(error);
    free(text);
}

#define DIP14 "\"family\":\"DIP\",\"rowSpacing\":7.62,"
#define FITS14 ",\"mounting\":\"through-hole\",\"fits\":[\"PDIP-14\",\"CDIP-14\"]"

static void good_cases(void) {
    char *error = NULL;
    char *text = socket_file("dip14", DIP14, 14, 0, "PDIP-14", FITS14);
    const Part *part;
    char *line;

    expect(load_parts(library, text, "socket.json", &error) == 0,
           "(a) a 14-pin through-hole DIP socket with two fits loads");
    if (error != NULL) {
        printf("     %s\n", error);
    }
    free(error);
    free(text);
    part = part_library_find(library, "T-dip14");
    expect(part != NULL && part->kind == PART_SOCKET && part->socket != NULL &&
               part->pin_count == 14 && part->simulated == 1,
           "(a) a socket part with 14 pins, simulated");
    expect(part != NULL && part->socket != NULL && strcmp(part->socket->family, "DIP") == 0 &&
               part->socket->row_um == 7620 &&
               strcmp(part->socket->mounting, "through-hole") == 0 && part->socket->finish == NULL,
           "(a) family DIP, row 7620 um, through-hole, no finish");
    expect(part != NULL && part->fit_count == 2 && part->fits != NULL &&
               strcmp(part->fits[0]->id, "PDIP-14") == 0 &&
               strcmp(part->fits[1]->id, "CDIP-14") == 0,
           "(a) fits PDIP-14 then CDIP-14, in file order");
    line = part != NULL ? listing_line(part) : NULL;
    expect(line != NULL && strstr(line, "|socket|") != NULL &&
               strstr(line, "family=DIP row=7620 mounting=through-hole finish=- fits=2") != NULL,
           "(a) the listing line names the facts");
    free(line);

    text = socket_file("plcc28", "\"family\":\"PLCC\",", 28, 0, "PLCC-28",
                       ",\"mounting\":\"surface-mount\",\"finish\":\"tin\",\"fits\":[\"PLCC-28\"]");
    expect(load_parts(library, text, "socket.json", &error) == 0,
           "(b) a 28-pin surface-mount PLCC socket with a finish loads");
    if (error != NULL) {
        printf("     %s\n", error);
    }
    free(error);
    free(text);
    part = part_library_find(library, "T-plcc28");
    expect(part != NULL && part->socket != NULL && strcmp(part->socket->family, "PLCC") == 0 &&
               part->socket->row_um == 0 && strcmp(part->socket->mounting, "surface-mount") == 0 &&
               part->socket->finish != NULL && strcmp(part->socket->finish, "tin") == 0 &&
               part->fit_count == 1,
           "(b) family PLCC, row 0, surface-mount, finish tin, one fit");
}

static void device_cases(void) {
    expect_error("(c) family SIP", socket_file("c1", "\"family\":\"SIP\",", 14, 0, "PDIP-14", FITS14),
                 "unknown socket family \"SIP\"");
    expect_error("(c) a DIP without rowSpacing",
                 socket_file("c2", "\"family\":\"DIP\",", 14, 0, "PDIP-14", FITS14),
                 "missing \"rowSpacing\"");
    expect_error("(c) rowSpacing 6.0",
                 socket_file("c3", "\"family\":\"DIP\",\"rowSpacing\":6.0,", 14, 0, "PDIP-14", FITS14),
                 "\"rowSpacing\" must be 5.08, 7.62, 10.16 or 15.24");
    expect_error("(c) a PLCC with rowSpacing",
                 socket_file("c4", "\"family\":\"PLCC\",\"rowSpacing\":7.62,", 28, 0, "PLCC-28",
                             ",\"mounting\":\"surface-mount\",\"fits\":[\"PLCC-28\"]"),
                 "a PLCC socket has no \"rowSpacing\"");
    expect_error("(d) a DIP with 13 pins", socket_file("d1", DIP14, 13, 0, "PDIP-14", FITS14),
                 "a DIP socket needs an even pin count of 4 to 64");
    expect_error("(d) a DIP with 2 pins", socket_file("d2", DIP14, 2, 0, "PDIP-14", FITS14),
                 "a DIP socket needs an even pin count of 4 to 64");
    expect_error("(d) a DIP with 66 pins", socket_file("d3", DIP14, 66, 0, "PDIP-14", FITS14),
                 "a DIP socket needs an even pin count of 4 to 64");
    expect_error("(d) a PLCC with 30 pins",
                 socket_file("d4", "\"family\":\"PLCC\",", 30, 0, "PLCC-28",
                             ",\"mounting\":\"surface-mount\",\"fits\":[\"PLCC-28\"]"),
                 "a PLCC socket needs 20, 28, 32, 44, 52, 68 or 84 pins");
    expect_error("(d) a pin with role input", socket_file("d5", DIP14, 14, 3, "PDIP-14", FITS14),
                 "socket pin \"3\" must be inout");
}

static void part_cases(void) {
    expect_error("(e) mounting clip",
                 socket_file("e1", DIP14, 14, 0, "PDIP-14",
                             ",\"mounting\":\"clip\",\"fits\":[\"PDIP-14\"]"),
                 "unknown socket mounting \"clip\"");
    expect_error("(e) no mounting",
                 socket_file("e2", DIP14, 14, 0, "PDIP-14", ",\"fits\":[\"PDIP-14\"]"),
                 "missing \"mounting\"");
    expect_error("(e) an empty finish",
                 socket_file("e3", DIP14, 14, 0, "PDIP-14",
                             ",\"mounting\":\"through-hole\",\"finish\":\"\",\"fits\":[\"PDIP-14\"]"),
                 "\"finish\" must be a non-empty string");
}

static void fits_cases(void) {
    expect_error("(f) no fits",
                 socket_file("f1", DIP14, 14, 0, "PDIP-14", ",\"mounting\":\"through-hole\""),
                 "missing \"fits\"");
    expect_error("(f) fits []",
                 socket_file("f2", DIP14, 14, 0, "PDIP-14",
                             ",\"mounting\":\"through-hole\",\"fits\":[]"),
                 "\"fits\" must be a non-empty array of footprint ids");
    expect_error("(f) fits is not an array",
                 socket_file("f3", DIP14, 14, 0, "PDIP-14",
                             ",\"mounting\":\"through-hole\",\"fits\":\"PDIP-14\""),
                 "\"fits\" must be a non-empty array of footprint ids");
    expect_error("(f) fits holds a number",
                 socket_file("f4", DIP14, 14, 0, "PDIP-14",
                             ",\"mounting\":\"through-hole\",\"fits\":[14]"),
                 "\"fits\" must be a non-empty array of footprint ids");
    expect_error("(f) an unknown id",
                 socket_file("f5", DIP14, 14, 0, "PDIP-14",
                             ",\"mounting\":\"through-hole\",\"fits\":[\"NO-SUCH-14\"]"),
                 "unknown fits footprint \"NO-SUCH-14\"");
    expect_error("(f) an id twice",
                 socket_file("f6", DIP14, 14, 0, "PDIP-14",
                             ",\"mounting\":\"through-hole\",\"fits\":[\"PDIP-14\",\"PDIP-14\"]"),
                 "fits footprint \"PDIP-14\" listed twice");
    expect_error("(f) a 12-pin footprint on a 14-pin socket",
                 socket_file("f7", DIP14, 14, 0, "PDIP-14",
                             ",\"mounting\":\"through-hole\",\"fits\":[\"T-DIP-12\"]"),
                 "fits footprint \"T-DIP-12\" has 12 pins; the part has 14");
    expect_error("(f) a 14-pin footprint with no pad of the pin's designator",
                 socket_file("f8", DIP14, 14, 0, "PDIP-14",
                             ",\"mounting\":\"through-hole\",\"fits\":[\"T-BALL-14\"]"),
                 "pin \"1\" has no pad in fits footprint \"T-BALL-14\"");
}

/* A gate part ("1G00" shape, a device id of its own) with a "fits" key. */
static void other_kind_case(void) {
    expect_error("(g) fits on a gate part",
                 strdup("{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"gate\","
                        "\"devices\":[{\"id\":\"T-1G00\",\"function\":\"NAND\","
                        "\"description\":\"Single 2-input NAND gate\",\"drawer\":[\"Custom\"],"
                        "\"pinouts\":{\"p5\":[{\"number\":1,\"name\":\"A\",\"role\":\"input\"},"
                        "{\"number\":2,\"name\":\"B\",\"role\":\"input\"},"
                        "{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"},"
                        "{\"number\":4,\"name\":\"Y\",\"role\":\"output\"},"
                        "{\"number\":5,\"name\":\"VCC\",\"role\":\"power\"}]},"
                        "\"gates\":[{\"op\":\"nand\",\"inputs\":[\"A\",\"B\"],\"output\":\"Y\"}],"
                        "\"timing\":{\"HC\":{\"rise\":9,\"fall\":9,\"vcc\":[2,6]}},"
                        "\"parts\":[{\"name\":\"T-FITS-GATE\",\"manufacturer\":\"Generic\","
                        "\"package\":\"SOT-23\",\"pinout\":\"p5\",\"footprint\":\"SOT23-5\","
                        "\"family\":\"HC\",\"fits\":[\"SOT23-5\"]}]}]}"),
                 "\"fits\" is only for a socket");
}

/* A placed socket joins nothing: two pins on one net driven high, two on
   another that nothing drives. */
static void circuit_case(void) {
    Circuit *c = circuit_create(library);
    size_t sock = 0, supply = 0, hot = 0, cold = 0;
    int built;

    built = circuit_add_part(c, "T-dip14", &sock) == 0 &&
            circuit_add_part(c, "5V", &supply) == 0 && circuit_add_net(c, &hot) == 0 &&
            circuit_add_net(c, &cold) == 0 && circuit_connect(c, supply, "1", hot) == 0 &&
            circuit_connect(c, sock, "1", hot) == 0 && circuit_connect(c, sock, "2", hot) == 0 &&
            circuit_connect(c, sock, "3", cold) == 0 && circuit_connect(c, sock, "4", cold) == 0;
    expect(built, "(h) a socket and a supply are added and wired: two socket pins on each net");
    expect(circuit_start(c) == 0, "(h) circuit_start succeeds");
    circuit_advance(c, 1000000);
    expect(circuit_net_value(c, hot) == CIRCUIT_1 && circuit_net_value(c, cold) == CIRCUIT_Z,
           "(h) the driven net is 1 and the other stays undriven: the pins join nothing");
    circuit_destroy(c);
}

int main(void) {
    char *error = NULL;

    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    if (load_footprints(library, TEST_FOOTPRINTS, "socket_test footprints", &error) != 0) {
        printf("FAIL: cannot load the test footprints: %s\n", error != NULL ? error : "(null)");
        return 1;
    }
    good_cases();
    device_cases();
    part_cases();
    fits_cases();
    other_kind_case();
    circuit_case();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("socket_test: all passed\n");
    return 0;
}
