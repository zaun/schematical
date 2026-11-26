/*
 * library_footprint_more_check (plan-68-L): footprints.json's rejections that
 * library_footprint_check (at its size limit) does not reach — each style's
 * keys of the wrong type or missing, slot and ball rules, the file's own
 * keys — each with the message the loader gives, and nothing added.
 */

#include "library_check_bench.h"

#define FP_FILE(FPS) "{'format':'schematical-footprints','version':1,'footprints':[" FPS "]}"
#define BODY "'body':{'width':4,'height':4},"
#define PAD "'pad':{'width':1,'height':1,'shape':'rect'},"
#define DUAL(EXTRA) FP_FILE("{'id':'D','description':'d','style':'dual','pins':4," BODY "'pitch':1,'span':3," EXTRA "}")
#define DUAL_PAD(EXTRA) DUAL(PAD EXTRA)
#define QUAD(EXTRA) FP_FILE("{'id':'Q','description':'d','style':'quad','pins':4," BODY PAD EXTRA "}")
#define HEADER(EXTRA) FP_FILE("{'id':'H','description':'d','style':'header','pins':4," BODY PAD "'pitch':2.54," EXTRA "}")
#define GRID(EXTRA) FP_FILE("{'id':'G','description':'d','style':'grid','pins':2," BODY EXTRA "}")
#define EXPLICIT(EXTRA) FP_FILE("{'id':'E','description':'d','style':'explicit','pins':2," BODY EXTRA "}")
#define EPAD(PIN, EXTRA) "{" PIN ",'x':0,'y':0,'width':1,'height':1,'shape':'rect'" EXTRA "}"

static void expect_fp_fail(const char *name, const char *text, const char *expect) {
    PartLibrary *library = part_library_create();
    char *source = json(text);
    char *error = NULL;
    int result = load_footprints(library, source, "fp.json", &error);

    if (result != -1 || error == NULL || strstr(error, expect) == NULL || part_library_footprint_count(library) != 0) {
        printf("FAIL %s: result %d, count %zu, error \"%s\" (wanted \"%s\")\n", name, result,
               part_library_footprint_count(library), error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    free(source);
    part_library_destroy(library);
}

static void check_file_and_entry(void) {
    expect_fp_fail("version not a number", "{'format':'schematical-footprints','version':'1','footprints':[]}",
                   "\"version\" must be a whole number");
    expect_fp_fail("no footprints", "{'format':'schematical-footprints','version':1}", "missing \"footprints\"");
    expect_fp_fail("footprints not an array", "{'format':'schematical-footprints','version':1,'footprints':{}}",
                   "\"footprints\" must be an array");
    expect_fp_fail("an entry not an object", FP_FILE("5"), "expected an object");
    expect_fp_fail("no id", FP_FILE("{'description':'d','style':'dual','pins':4}"), "missing \"id\"");
    expect_fp_fail("an empty id", FP_FILE("{'id':'','description':'d','style':'dual','pins':4}"), "\"id\"");
    expect_fp_fail("no style", FP_FILE("{'id':'X','description':'d','pins':4}"), "missing \"style\"");
    expect_fp_fail("note not a string", FP_FILE("{'id':'X','description':'d','note':5,'style':'dual','pins':4}"),
                   "\"note\" must be a string");
    expect_fp_fail("pins not a number", FP_FILE("{'id':'X','description':'d','style':'dual','pins':'4'}"),
                   "\"pins\" must be a whole number");
    expect_fp_fail("round not true or false",
                   FP_FILE("{'id':'X','description':'d','style':'dual','pins':4,'body':{'width':1,'height':1,'round':1}}"),
                   "\"round\" must be true or false");
}

static void check_dual_and_pads(void) {
    expect_fp_fail("pad shape not a string", DUAL("'pad':{'width':1,'height':1,'shape':5}"), "\"shape\" must be a string");
    expect_fp_fail("chamfer 0", DUAL("'pad':{'width':1,'height':1,'shape':'rect','chamfer':0,'corners':['tl']}"),
                   "\"chamfer\" must be greater than 0");
    expect_fp_fail("no pad",
                   FP_FILE("{'id':'D','description':'d','style':'dual','pins':4," BODY "'pitch':1,'span':3}"),
                   "missing \"pad\"");
    expect_fp_fail("drill below 0", DUAL_PAD("'drill':-1"), "\"drill\" must be 0 or more");
    expect_fp_fail("drill not a number", DUAL_PAD("'drill':'x'"), "\"drill\" must be a number");
    expect_fp_fail("perSide not a number", DUAL_PAD("'perSide':'2'"), "\"perSide\" must be a whole number");
}

static void check_quad(void) {
    expect_fp_fail("quad with no sides", QUAD("'pitch':1,'spanX':3,'spanY':3"), "missing \"sides\"");
    expect_fp_fail("quad side not a count", QUAD("'sides':[1,'a',1,1],'pitch':1,'spanX':3,'spanY':3"),
                   "\"sides\" must be 4 slot counts [left, bottom, right, top]");
    expect_fp_fail("quad with no slots", QUAD("'sides':[0,0,0,0],'pitch':1,'spanX':3,'spanY':3"), "\"sides\" has no slots");
    expect_fp_fail("quad with no pitch", QUAD("'sides':[1,1,1,1],'spanX':3,'spanY':3"), "missing \"pitch\"");
}

static void check_header(void) {
    expect_fp_fail("header rows not a number", HEADER("'rows':'2','firstRowY':0"), "\"rows\" must be a whole number");
    expect_fp_fail("header with no firstRowY", HEADER("'rows':1"), "missing \"firstRowY\"");
    expect_fp_fail("header rowPitch not a number", HEADER("'rows':2,'firstRowY':0,'rowPitch':'x'"),
                   "\"rowPitch\" must be a number");
    expect_fp_fail("header rowPitch 0", HEADER("'rows':2,'firstRowY':0,'rowPitch':0"), "\"rowPitch\" must not be 0");
    expect_fp_fail("header drill 0", HEADER("'rows':1,'firstRowY':0,'drill':0"), "\"drill\" must be greater than 0");
}

static void check_grid(void) {
    expect_fp_fail("grid with 0 rows", GRID(PAD "'rows':0,'columns':2,'pitch':1,'balls':['A1','A2']"),
                   "\"rows\" and \"columns\" must be at least 1");
    expect_fp_fail("grid with no pitch", GRID(PAD "'rows':1,'columns':2,'balls':['A1','A2']"), "missing \"pitch\"");
    expect_fp_fail("grid drill not a number", GRID(PAD "'rows':1,'columns':2,'pitch':1,'drill':'x','balls':['A1','A2']"),
                   "\"drill\" must be a number");
    expect_fp_fail("grid drill too big", GRID(PAD "'rows':1,'columns':2,'pitch':1,'drill':2,'balls':['A1','A2']"),
                   "must be smaller than the pad");
    expect_fp_fail("grid with no balls", GRID(PAD "'rows':1,'columns':2,'pitch':1"), "missing \"balls\"");
    expect_fp_fail("grid ball not a name", GRID(PAD "'rows':1,'columns':2,'pitch':1,'balls':['A1',2]"),
                   "\"balls\" must be an array of ball names");
    expect_fp_fail("grid mirrored chamfers do not fit",
                   GRID("'pad':{'width':1,'height':0.4,'shape':'rect','chamfer':0.25,'corners':['tl','tr']},"
                        "'rows':1,'columns':2,'pitch':2,'balls':['A1','A2']"),
                   "(mirrored corners)");
}

static void check_explicit(void) {
    expect_fp_fail("explicit with no pads",
                   FP_FILE("{'id':'E','description':'d','style':'explicit','pins':2,'body':{'width':4,'height':4}}"),
                   "missing \"pads\"");
    expect_fp_fail("explicit with no pad", EXPLICIT("'pads':[]"), "\"pads\" must be a non-empty array of pads");
    expect_fp_fail("explicit pad key", EXPLICIT("'pads':[" EPAD("'pin':1", ",'x2':0") "]"), "unknown key \"x2\"");
    expect_fp_fail("explicit balls after numbers", EXPLICIT("'pads':[" EPAD("'pin':1", "") "," EPAD("'ball':'A1'", "") "]"),
                   "\"ball\" pads and numbered pads in one footprint");
    expect_fp_fail("explicit ball not a string", EXPLICIT("'pads':[" EPAD("'ball':5", "") "]"), "\"ball\" must be a string");
    expect_fp_fail("explicit drill not a number", EXPLICIT("'pads':[" EPAD("'pin':1", ",'drill':'x'") "]"),
                   "\"drill\" must be a number");
    expect_fp_fail("explicit slotLength not a number",
                   EXPLICIT("'pads':[" EPAD("'pin':1", ",'drill':0.4,'slotLength':'x'") "]"),
                   "\"slotLength\" must be a number");
}

int main(void) {
    check_file_and_entry();
    check_dual_and_pads();
    check_quad();
    check_header();
    check_grid();
    check_explicit();
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
