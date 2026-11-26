/*
 * library_reject_check (plan-68-L): the part library's API on missing
 * arguments, the loaders' rejections of malformed parts, part, footprint and
 * datasheet files (each message the loader gives), and the kind helpers'
 * own checks, called directly.
 */

#include "library_check_bench.h"
#include "library_load_int.h"
#include "logic_alloc.h"

#define UUID "6F1C2E8A-3B7D-4C55-9A61-0E2B4D7F9C10"
#define PART_HEAD(BODY) "{'format':'schematical-part','version':1,'uuid':'" UUID "'," BODY
#define SUPPLY_FILE(EXTRA) PART_HEAD("'kind':'supply'," EXTRA "'devices':[" VCC_DEVICE "]}")

/* A device of kind `KIND` whose fields are `FIELDS` (single quotes). */
#define ONE(KIND, FIELDS) HEAD(KIND) "{" FIELDS "}" TAIL
#define SUPPLY_PINOUT "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
#define SUPPLY_PART "'parts':[{'name':'X','package':'','pinout':'pin'}]"
#define SUPPLY(FIELDS) ONE("supply", "'id':'X','function':'X','description':'d','drawer':[]," FIELDS)

/* Loads a part file into a library holding the fixture footprints and
   expects failure with `expect` in the message. */
static void expect_part_fail(const char *name, const char *text, const char *expect) {
    PartLibrary *library = part_library_create();
    char *source = json(text);
    char *error = NULL;
    int result;

    load_fixture_footprints(library);
    result = load_part_file(library, source, "part.json", &error);
    if (result != -1 || error == NULL || strstr(error, expect) == NULL || part_library_count(library) != 0) {
        printf("FAIL %s: result %d, error \"%s\" (wanted \"%s\")\n", name, result, error ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    free(source);
    part_library_destroy(library);
}

/* Loads a datasheets file and expects failure with `expect` in the message. */
static void expect_sheets_fail(const char *name, const char *text, const char *expect) {
    PartLibrary *library = part_library_create();
    char *source = json(text);
    char *error = NULL;
    int result = load_datasheets(library, source, "datasheets.json", &error);

    if (result != -1 || error == NULL || strstr(error, expect) == NULL || part_library_datasheet_count(library) != 0) {
        printf("FAIL %s: result %d, error \"%s\" (wanted \"%s\")\n", name, result, error ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    free(source);
    part_library_destroy(library);
}

/* Each loader and lookup on a missing library or text. */
static void check_missing_arguments(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;

    CHECK(part_library_find(NULL, "x") == NULL && part_library_find(library, NULL) == NULL);
    CHECK(part_library_find_uuid(NULL, UUID) == NULL && part_library_find_uuid(library, NULL) == NULL);
    CHECK(part_library_find_footprint(NULL, "T1") == NULL && part_library_find_footprint(library, NULL) == NULL);
    CHECK(part_library_datasheet_at(NULL, 0) == NULL && part_library_datasheet_at(library, 0) == NULL);
    CHECK(part_library_find_datasheet(NULL, "x") == NULL && part_library_find_datasheet(library, NULL) == NULL);
    CHECK(part_library_retire_part(NULL, UUID, NULL, 0, NULL, 0) == -1);
    CHECK(part_library_retire_part(library, NULL, NULL, 0, NULL, 0) == -1);
    CHECK(part_pin_analog(NULL, 0) == 0 && part_pin_tristate(NULL, 0) == 0);

    CHECK(load_part_file(NULL, "{}", "x", &error) == -1 && error != NULL &&
          strstr(error, "load_part_file: no library or text") != NULL);
    free(error);
    CHECK(load_part_file(library, NULL, "x", NULL) == -1);
    CHECK(load_footprints(library, NULL, "x", &error) == -1 && error != NULL &&
          strstr(error, "load_footprints: no library or text") != NULL);
    free(error);
    CHECK(load_footprints(NULL, "{}", "x", NULL) == -1);
    CHECK(load_datasheets(library, NULL, "x", &error) == -1 && error != NULL &&
          strstr(error, "load_datasheets: no library or text") != NULL);
    free(error);
    CHECK(load_datasheets(NULL, "{}", "x", NULL) == -1);
    CHECK(load_parts(library, NULL, "x", &error) == -1 && error != NULL &&
          strstr(error, "load_parts: no library or text") != NULL);
    free(error);
    CHECK(load_parts(NULL, "{}", "x", NULL) == -1);
    part_library_destroy(library);
}

/* The parts file's and the device's own rules. */
static void check_parts_file_rejections(void) {
    expect_fail("version not a number", "{'format':'schematical-parts','version':'1','kind':'supply','devices':[]}",
                "\"version\" must be a whole number");
    expect_fail("no kind", "{'format':'schematical-parts','version':1,'devices':[]}", "missing \"kind\"");
    expect_fail("no devices", "{'format':'schematical-parts','version':1,'kind':'supply'}", "missing \"devices\"");
    expect_fail("devices not an array", "{'format':'schematical-parts','version':1,'kind':'supply','devices':{}}",
                "\"devices\" must be an array");
    expect_fail("no function", ONE("supply", "'id':'X','description':'d','drawer':[]," SUPPLY_PINOUT SUPPLY_PART),
                "missing \"function\"");
    expect_fail("note not a string", SUPPLY("'note':5," SUPPLY_PINOUT SUPPLY_PART), "\"note\" must be a string");
    expect_fail("no pinouts", SUPPLY("'volts':5," SUPPLY_PART), "missing \"pinouts\"");
    expect_fail("empty pinouts", SUPPLY("'pinouts':{},'volts':5," SUPPLY_PART), "\"pinouts\" must be a non-empty object");
    expect_fail("duplicate pinout",
                SUPPLY("'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}],"
                       "'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5," SUPPLY_PART),
                "duplicate pinout \"pin\"");
    expect_fail("no parts", SUPPLY(SUPPLY_PINOUT "'x':0"), "unknown key \"x\"");
    expect_fail("parts missing", SUPPLY("'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5"),
                "missing \"parts\"");
    expect_fail("parts not an array", SUPPLY(SUPPLY_PINOUT "'parts':{}"), "\"parts\" must be an array");
    expect_fail("a part with no name", SUPPLY(SUPPLY_PINOUT "'parts':[{'package':'','pinout':'pin'}]"),
                "parts[0]: missing \"name\"");
    expect_fail("an empty part name", SUPPLY(SUPPLY_PINOUT "'parts':[{'name':'','package':'','pinout':'pin'}]"),
                "empty part name");
    expect_fail("flags not an array", SUPPLY(SUPPLY_PINOUT "'parts':[{'name':'X','package':'','pinout':'pin','flags':'obsolete'}]"),
                "\"flags\" must be an array");
    expect_fail("a flag not a string", SUPPLY(SUPPLY_PINOUT "'parts':[{'name':'X','package':'','pinout':'pin','flags':[1]}]"),
                "\"flags\" must be an array of strings");
    expect_fail("footprintChoices with no footprint",
                ONE("led", "'id':'led','function':'LED','description':'d','drawer':[],"
                    "'pinouts':{'two-pin':[{'number':1,'name':'K','role':'input'},{'number':2,'name':'A','role':'input'}]},"
                    "'colors':['Red'],'anode':'A','cathode':'K',"
                    "'parts':[{'name':'L','package':'0603','pinout':'two-pin','footprintChoices':[]}]"),
                "missing \"footprint\"");
    expect_fail("manufacturer not a string",
                SUPPLY(SUPPLY_PINOUT "'parts':[{'name':'X','package':'','pinout':'pin','manufacturer':5}]"),
                "\"manufacturer\" must be a string");
    expect_fail("datasheet not a string",
                SUPPLY(SUPPLY_PINOUT "'parts':[{'name':'X','package':'','pinout':'pin','manufacturer':'M','datasheet':5}]"),
                "\"datasheet\" must be a string");
}

/* A pinout's pins. */
static void check_pin_rejections(void) {
#define PIN_CASE(NAME, PIN, EXPECT) \
    expect_fail(NAME, SUPPLY("'pinouts':{'pin':[" PIN "]},'volts':5," SUPPLY_PART), EXPECT)
    PIN_CASE("pin with an unknown key", "{'number':1,'name':'VCC','role':'power','x':1}", "pin 1: unknown key \"x\"");
    PIN_CASE("pin number not a number", "{'number':'one','name':'VCC','role':'power'}", "\"number\" must be a whole number");
    PIN_CASE("ball not a string", "{'ball':5,'name':'VCC','role':'power'}", "\"ball\" must be a string");
    PIN_CASE("pin with no name", "{'number':1,'role':'power'}", "missing \"name\"");
    PIN_CASE("group not a string", "{'number':1,'name':'VCC','role':'power','group':5}", "\"group\" must be a string");
    PIN_CASE("side not a string", "{'number':1,'name':'VCC','role':'power','side':5}", "\"side\" must be a string");
#undef PIN_CASE
}

/* A single part file's own rules. */
static void check_part_file_rejections(void) {
    expect_part_fail("part file not an object", "[]", "expected an object");
    expect_part_fail("part file format", "{'format':'schematical-parts','version':1}",
                     "\"format\" must be \"schematical-part\", not \"schematical-parts\"");
    expect_part_fail("part file version not a number", "{'format':'schematical-part','version':'1'}",
                     "\"version\" must be a whole number");
    expect_part_fail("part file version 2", "{'format':'schematical-part','version':2}", "unsupported version 2");
    expect_part_fail("uuid dash out of place",
                     "{'format':'schematical-part','version':1,'uuid':'6F1C2E8AX3B7D-4C55-9A61-0E2B4D7F9C10'}",
                     "\"uuid\" must be an uppercase UUID");
    expect_part_fail("originId not a string", SUPPLY_FILE("'originId':5,"), "\"originId\" must be a string");
    expect_part_fail("no kind", PART_HEAD("'devices':[" VCC_DEVICE "]}"), "missing \"kind\"");
    expect_part_fail("unknown kind", PART_HEAD("'kind':'nope','devices':[]}"), "unknown kind \"nope\"");
    expect_part_fail("no devices", PART_HEAD("'kind':'supply'}"), "missing \"devices\"");
}

/* The datasheets file. */
static void check_datasheet_rejections(void) {
    PartLibrary *library = part_library_create();
    char *one = json("{'format':'schematical-datasheets','version':1,'datasheets':[{'id':'a','url':'https://a'}]}");
    char *two = json("{'format':'schematical-datasheets','version':1,'datasheets':[{'id':'b','url':'https://b'}]}");
    char *error = NULL;

    expect_sheets_fail("datasheets version not a number", "{'format':'schematical-datasheets','version':'1'}",
                       "\"version\" must be a whole number");
    expect_sheets_fail("datasheets version 2", "{'format':'schematical-datasheets','version':2}", "unsupported version 2");
    expect_sheets_fail("no datasheets", "{'format':'schematical-datasheets','version':1}", "missing \"datasheets\"");
    expect_sheets_fail("datasheets not an array", "{'format':'schematical-datasheets','version':1,'datasheets':{}}",
                       "\"datasheets\" must be an array");
    expect_sheets_fail("md not a string",
                       "{'format':'schematical-datasheets','version':1,'datasheets':[{'id':'a','url':'https://a','md':5}]}",
                       "\"md\" must be a string");
    /* A second file adds to the first. */
    CHECK(load_datasheets(library, one, "one.json", &error) == 0 && error == NULL);
    CHECK(load_datasheets(library, two, "two.json", &error) == 0 && error == NULL);
    CHECK(part_library_datasheet_count(library) == 2);
    CHECK(part_library_find_datasheet(library, "a") != NULL && part_library_find_datasheet(library, "b") != NULL);
    free(one);
    free(two);
    part_library_destroy(library);
}

/* The kind helpers, on a context of their own. */
static void check_kind_helpers(void) {
    KindContext ctx;
    cJSON *root = cJSON_Parse("{\"s\":\"x\",\"n\":1,\"names\":5,\"mixed\":[\"a\",1],\"t\":-1,\"v\":{\"inputMax\":\"x\"}}");
    const char *const *names = NULL;
    size_t count = 0;
    double number = 0;
    uint64_t ps = 0;
    PartVoltage base = {0};
    const PartVoltage *out = NULL;
    PartPin pins[3] = {{.name = "A"}, {.name = "A"}, {.name = "B"}};
    size_t found[1];

    memset(&ctx, 0, sizeof ctx);
    ctx.source = "helpers";
    CHECK(kind_alloc(&ctx, 0) != NULL);
    CHECK(kind_check_keys(&ctx, cJSON_GetObjectItem(root, "s"), NULL, NULL) == -1 &&
          strstr(ctx.error, "expected an object") != NULL);
    logic_free(ctx.error);
    ctx.error = NULL;
    CHECK(kind_uint(&ctx, root, "s", 10, &ps) == -1 && strstr(ctx.error, "\"s\" must be a whole number") != NULL);
    logic_free(ctx.error);
    ctx.error = NULL;
    CHECK(kind_number(&ctx, root, "s", &number) == -1 && strstr(ctx.error, "\"s\" must be a number") != NULL);
    logic_free(ctx.error);
    ctx.error = NULL;
    CHECK(kind_names(&ctx, root, "names", &names, &count) == -1 &&
          strstr(ctx.error, "\"names\" must be an array of strings") != NULL);
    logic_free(ctx.error);
    ctx.error = NULL;
    CHECK(kind_names(&ctx, root, "mixed", &names, &count) == -1 &&
          strstr(ctx.error, "\"mixed\" must be an array of strings") != NULL);
    logic_free(ctx.error);
    ctx.error = NULL;
    CHECK(kind_ns(&ctx, cJSON_GetObjectItem(root, "t"), "t", &ps) == -1 &&
          strstr(ctx.error, "\"t\" must be a time in ns from 0 to 1e9") != NULL);
    logic_free(ctx.error);
    ctx.error = NULL;
    CHECK(kind_voltage(&ctx, cJSON_GetObjectItem(root, "v"), &base, &out) == -1 &&
          strstr(ctx.error, "\"inputMax\"") != NULL);
    logic_free(ctx.error);
    ctx.error = NULL;
    CHECK(kind_pins(&ctx, pins, 3, "A", found, 1) == 0 && strstr(ctx.error, "too many pins named \"A\"") != NULL);
    logic_free(ctx.error);
    lib_free_chunks(ctx.arena);
    cJSON_Delete(root);
}

int main(void) {
    check_missing_arguments();
    check_parts_file_rejections();
    check_pin_rejections();
    check_part_file_rejections();
    check_datasheet_rejections();
    check_kind_helpers();
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
