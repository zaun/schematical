/*
 * library_footprint_check: split from library_check.c by plan-68-H; the shared helpers are
 * support/library_check_bench.c.
 */

#include "library_check_bench.h"

/* ---- Footprints (plan-03-A) ------------------------------------------------ */

#define FP_FILE(FPS) "{'format':'schematical-footprints','version':1,'footprints':[" FPS "]}"

static int load_fp(PartLibrary *library, const char *text, char **error) {
    char *source = json(text);
    int result = load_footprints(library, source, "fp.json", error);

    free(source);
    return result;
}

/* Loads footprints into a fresh library; expects failure with `expect`, nothing added. */
static void expect_fp_fail(const char *name, const char *text, const char *expect) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    int result = load_fp(library, text, &error);

    if (result != -1 || error == NULL || strstr(error, expect) == NULL ||
        part_library_footprint_count(library) != 0) {
        printf("FAIL %s: result %d, count %zu, error \"%s\" (wanted \"%s\")\n", name, result,
               part_library_footprint_count(library), error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    part_library_destroy(library);
}

/* Loads good footprints, then a part file expected to fail with `expect`. */
static void expect_part_fail(const char *name, const char *footprints, const char *parts,
                             const char *expect) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    int result;

    CHECK(load_fp(library, footprints, &error) == 0);
    free(error);
    error = NULL;
    result = load(library, parts, &error);
    if (result != -1 || error == NULL || strstr(error, expect) == NULL ||
        part_library_count(library) != 0) {
        printf("FAIL %s: result %d, count %zu, error \"%s\" (wanted \"%s\")\n", name, result,
               part_library_count(library), error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    part_library_destroy(library);
}

static int near(double a, double b) {
    return fabs(a - b) < 1e-9;
}

/* Checks pad `index` of a footprint; designator NULL = unnumbered. */
static void check_pad(const PartFootprint *fp, size_t index, const char *designator, double x,
                      double y, double width, double height, double drill, PadShape shape) {
    const FootprintPad *pad;
    int same_name;

    if (fp == NULL || index >= fp->pad_count) {
        printf("FAIL pad %zu of %s: missing\n", index, fp != NULL ? fp->id : "(null)");
        failures++;
        return;
    }
    pad = &fp->pads[index];
    same_name = designator == NULL ? pad->designator == NULL
                                   : pad->designator != NULL && strcmp(pad->designator, designator) == 0;
    if (!same_name || !near(pad->x, x) || !near(pad->y, y) || !near(pad->width, width) ||
        !near(pad->height, height) || !near(pad->drill, drill) || pad->shape != shape) {
        printf("FAIL pad %zu of %s: %s %g,%g %gx%g d%g %d (wanted %s %g,%g %gx%g d%g %d)\n", index,
               fp->id, pad->designator != NULL ? pad->designator : "-", pad->x, pad->y, pad->width,
               pad->height, pad->drill, (int)pad->shape, designator != NULL ? designator : "-", x, y,
               width, height, drill, (int)shape);
        failures++;
    }
}

#define FP_DIP14 \
    "{'id':'DIP-14','description':'d','note':'n','style':'dual','pins':14," \
    "'body':{'width':6.35,'height':19.3},'pitch':2.54,'span':7.62," \
    "'pad':{'width':1.6,'height':1.6,'shape':'round'},'drill':0.8,'pin1Shape':'rect'}"
#define FP_SOT5 \
    "{'id':'SOT-5','description':'d','style':'dual','pins':5,'perSide':3,'slots':[1,2,3,4,6]," \
    "'body':{'width':1.6,'height':2.9},'pitch':0.95,'span':2.6," \
    "'pad':{'width':1.1,'height':0.6,'shape':'rect'}}"
#define FP_LCCC20 \
    "{'id':'LCCC-20','description':'d','style':'quad','pins':20,'sides':[5,5,5,5]," \
    "'slots':[18,19,20,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]," \
    "'body':{'width':8.9,'height':8.9},'pitch':1.27,'spanX':8,'spanY':8," \
    "'pad':{'width':2,'height':0.65,'shape':'rect'},'centre':{'width':3,'height':3,'shape':'rect'}}"
#define FP_X2SON5 \
    "{'id':'X2SON-5','description':'d','style':'dual','pins':5,'perSide':2,'slots':[1,2,0,3,4]," \
    "'body':{'width':0.8,'height':0.8},'pitch':1,'span':2," \
    "'pad':{'width':0.5,'height':0.3,'shape':'rect'},'centre':{'width':0.4,'height':0.4,'shape':'round'}}"
#define FP_EXPLICIT4 \
    "{'id':'OSC-4','description':'d','style':'explicit','pins':4,'body':{'width':2,'height':1.2}," \
    "'pads':[{'pin':2,'x':-0.3,'y':0.4,'width':0.3,'height':0.3,'shape':'rect'}," \
    "{'pin':1,'x':-0.9,'y':0,'width':0.6,'height':1.2,'shape':'rect','drill':0.2}," \
    "{'pin':0,'x':0,'y':0,'width':0.2,'height':0.2,'shape':'round'}," \
    "{'pin':4,'x':0.9,'y':0,'width':0.6,'height':1.2,'shape':'rect'}," \
    "{'pin':3,'x':0.3,'y':0.4,'width':0.3,'height':0.3,'shape':'round'}]}"
#define FP_GRID \
    "{'id':'BGA-4','description':'d','style':'grid','pins':4,'rows':2,'columns':3,'pitch':0.5," \
    "'balls':['B3','A1','A2','B1'],'body':{'width':1.5,'height':1,'round':true}," \
    "'pad':{'width':0.25,'height':0.25,'shape':'round'}}"

static void check_footprint_styles(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;

    CHECK(load_fp(library, FP_FILE(FP_DIP14 "," FP_SOT5 "," FP_LCCC20), &error) == 0);
    CHECK(error == NULL);
    free(error);
    error = NULL;
    /* A second call adds to the first. */
    CHECK(load_fp(library, FP_FILE(FP_X2SON5 "," FP_EXPLICIT4 "," FP_GRID), &error) == 0);
    CHECK(error == NULL);
    CHECK(part_library_footprint_count(library) == 6);
    CHECK(part_library_footprint_at(library, 0) == part_library_find_footprint(library, "DIP-14"));
    CHECK(part_library_footprint_at(library, 5) == part_library_find_footprint(library, "BGA-4"));
    CHECK(part_library_find_footprint(library, "NOPE") == NULL);

    fp = part_library_find_footprint(library, "DIP-14");
    CHECK(fp != NULL && fp->pins == 14 && fp->pad_count == 14 && near(fp->body_height, 19.3) &&
          !fp->body_round && strcmp(fp->description, "d") == 0);
    check_pad(fp, 0, "1", -3.81, -7.62, 1.6, 1.6, 0.8, PAD_RECT);
    check_pad(fp, 1, "2", -3.81, -5.08, 1.6, 1.6, 0.8, PAD_ROUND);
    check_pad(fp, 6, "7", -3.81, 7.62, 1.6, 1.6, 0.8, PAD_ROUND);
    check_pad(fp, 7, "8", 3.81, 7.62, 1.6, 1.6, 0.8, PAD_ROUND);
    check_pad(fp, 13, "14", 3.81, -7.62, 1.6, 1.6, 0.8, PAD_ROUND);

    fp = part_library_find_footprint(library, "SOT-5");
    CHECK(fp != NULL && fp->pad_count == 5);
    check_pad(fp, 0, "1", -1.3, -0.95, 1.1, 0.6, 0, PAD_RECT);
    check_pad(fp, 1, "2", -1.3, 0, 1.1, 0.6, 0, PAD_RECT);
    check_pad(fp, 2, "3", -1.3, 0.95, 1.1, 0.6, 0, PAD_RECT);
    check_pad(fp, 3, "4", 1.3, 0.95, 1.1, 0.6, 0, PAD_RECT);
    check_pad(fp, 4, "5", 1.3, -0.95, 1.1, 0.6, 0, PAD_RECT);   /* slot 6; slot 5 is empty */

    fp = part_library_find_footprint(library, "LCCC-20");
    CHECK(fp != NULL && fp->pad_count == 21);
    check_pad(fp, 0, "1", 0, -4, 0.65, 2, 0, PAD_RECT);         /* top centre, turned */
    check_pad(fp, 2, "3", -2.54, -4, 0.65, 2, 0, PAD_RECT);     /* top, leftmost */
    check_pad(fp, 3, "4", -4, -2.54, 2, 0.65, 0, PAD_RECT);     /* left, top */
    check_pad(fp, 12, "13", 2.54, 4, 0.65, 2, 0, PAD_RECT);     /* bottom, rightmost */
    check_pad(fp, 13, "14", 4, 2.54, 2, 0.65, 0, PAD_RECT);     /* right, bottom */
    check_pad(fp, 20, NULL, 0, 0, 3, 3, 0, PAD_RECT);           /* unnumbered centre, last */

    fp = part_library_find_footprint(library, "X2SON-5");
    CHECK(fp != NULL && fp->pad_count == 5);
    check_pad(fp, 0, "1", -1, -0.5, 0.5, 0.3, 0, PAD_RECT);
    check_pad(fp, 1, "2", -1, 0.5, 0.5, 0.3, 0, PAD_RECT);
    check_pad(fp, 2, "3", 0, 0, 0.4, 0.4, 0, PAD_ROUND);        /* pin 3 is the centre */
    check_pad(fp, 3, "4", 1, 0.5, 0.5, 0.3, 0, PAD_RECT);
    check_pad(fp, 4, "5", 1, -0.5, 0.5, 0.3, 0, PAD_RECT);

    fp = part_library_find_footprint(library, "OSC-4");
    CHECK(fp != NULL && fp->pad_count == 5);
    check_pad(fp, 0, "1", -0.9, 0, 0.6, 1.2, 0.2, PAD_RECT);
    check_pad(fp, 1, "2", -0.3, 0.4, 0.3, 0.3, 0, PAD_RECT);
    check_pad(fp, 2, "3", 0.3, 0.4, 0.3, 0.3, 0, PAD_ROUND);
    check_pad(fp, 3, "4", 0.9, 0, 0.6, 1.2, 0, PAD_RECT);
    check_pad(fp, 4, NULL, 0, 0, 0.2, 0.2, 0, PAD_ROUND);

    fp = part_library_find_footprint(library, "BGA-4");
    CHECK(fp != NULL && fp->pad_count == 4 && fp->body_round);
    check_pad(fp, 0, "A1", -0.5, -0.25, 0.25, 0.25, 0, PAD_ROUND);
    check_pad(fp, 1, "A2", 0, -0.25, 0.25, 0.25, 0, PAD_ROUND);
    check_pad(fp, 2, "B1", -0.5, 0.25, 0.25, 0.25, 0, PAD_ROUND);
    check_pad(fp, 3, "B3", 0.5, 0.25, 0.25, 0.25, 0, PAD_ROUND);

    /* Ids are unique across calls; a failed file adds nothing. */
    free(error);
    error = NULL;
    CHECK(load_fp(library, FP_FILE(FP_GRID), &error) == -1);
    CHECK(error != NULL && strstr(error, "footprints[0] (BGA-4): duplicate footprint id \"BGA-4\"") != NULL);
    printf("ok   footprint id repeated across calls: %s\n", error != NULL ? error : "(none)");
    CHECK(part_library_footprint_count(library) == 6);
    free(error);
    part_library_destroy(library);
    printf("ok   footprints of every style expand to the expected pads\n");
}

/* One footprint F of a style, KEYS supplying the rest. */
#define FP_DUAL(KEYS) \
    FP_FILE("{'id':'F','description':'d','style':'dual','body':{'width':1,'height':1}," \
            "'pad':{'width':0.5,'height':0.5,'shape':'rect'}," KEYS "}")
#define FP_QUAD(KEYS) \
    FP_FILE("{'id':'F','description':'d','style':'quad','body':{'width':1,'height':1}," \
            "'pad':{'width':0.5,'height':0.5,'shape':'rect'},'pitch':0.5,'spanX':3,'spanY':3," KEYS "}")
#define FP_GRID_WITH(KEYS) \
    FP_FILE("{'id':'F','description':'d','style':'grid','body':{'width':1,'height':1}," \
            "'pad':{'width':0.2,'height':0.2,'shape':'round'},'rows':2,'columns':2,'pitch':0.5," KEYS "}")
#define FP_EXPLICIT_WITH(KEYS) \
    FP_FILE("{'id':'F','description':'d','style':'explicit','pins':2,'body':{'width':1,'height':1}," KEYS "}")
#define EPAD(PIN, X) "{'pin':" PIN ",'x':" X ",'y':0,'width':0.4,'height':0.4,'shape':'rect'}"

/* Checks a pad's corner keys: radius, chamfer and the PAD_CORNER_* bits. */
static void check_corners(const PartFootprint *fp, size_t index, double radius, double chamfer,
                          unsigned corners) {
    const FootprintPad *pad = fp != NULL && index < fp->pad_count ? &fp->pads[index] : NULL;

    if (pad == NULL || !near(pad->radius, radius) || !near(pad->chamfer, chamfer) ||
        pad->corners != corners) {
        printf("FAIL corners of pad %zu of %s: r%g c%g %u (wanted r%g c%g %u)\n", index,
               fp != NULL ? fp->id : "(null)", pad != NULL ? pad->radius : -1,
               pad != NULL ? pad->chamfer : -1, pad != NULL ? pad->corners : 0u, radius, chamfer,
               corners);
        failures++;
    }
}

enum { TL = PAD_CORNER_TL, TR = PAD_CORNER_TR, BR = PAD_CORNER_BR, BL = PAD_CORNER_BL };

static void check_footprint_corners(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;

    CHECK(load_fp(library, FP_FILE(
        /* Inner corners cut, named for the top-left pad; centre a diamond. */
        "{'id':'CUT-5','description':'d','style':'dual','pins':5,'perSide':3,'slots':[1,2,3,0,4]," \
        "'body':{'width':1,'height':1},'pitch':0.5,'span':1," \
        "'pad':{'width':0.4,'height':0.2,'shape':'rect','chamfer':0.05,'corners':['br']}," \
        "'centre':{'width':0.3,'height':0.3,'shape':'rect','chamfer':0.15,'corners':['tl','tr','br','bl']}}," \
        /* Rounded quad pads; pin 1 keeps its plain pin1Shape. */
        "{'id':'ROUND-Q','description':'d','style':'quad','pins':4,'sides':[1,1,1,1]," \
        "'body':{'width':2,'height':2},'pitch':0.5,'spanX':2,'spanY':2," \
        "'pad':{'width':0.6,'height':0.3,'shape':'rect','radius':0.15},'pin1Shape':'rect'}," \
        "{'id':'CUT-Q','description':'d','style':'quad','pins':8,'sides':[2,2,2,2]," \
        "'body':{'width':2,'height':2},'pitch':0.5,'spanX':2,'spanY':2," \
        "'pad':{'width':0.6,'height':0.3,'shape':'rect','chamfer':0.1,'corners':['tl']}}," \
        /* Explicit pads take corners as named. */
        "{'id':'CUT-E','description':'d','style':'explicit','pins':2,'body':{'width':1,'height':1}," \
        "'pads':[{'pin':1,'x':-0.5,'y':0,'width':0.4,'height':0.4,'shape':'rect','chamfer':0.1,'corners':['tl','bl']}," \
        "{'pin':2,'x':0.5,'y':0,'width':0.4,'height':0.2,'shape':'rect','radius':0.1}]}"), &error) == 0);
    CHECK(error == NULL);

    fp = part_library_find_footprint(library, "CUT-5");
    check_pad(fp, 0, "1", -0.5, -0.5, 0.4, 0.2, 0, PAD_RECT);
    check_corners(fp, 0, 0, 0.05, BR);           /* up-left: as named */
    check_corners(fp, 1, 0, 0.05, BR | TR);      /* on the centre line: both inner corners */
    check_corners(fp, 2, 0, 0.05, TR);           /* down-left: top/bottom mirrored */
    check_corners(fp, 3, 0, 0.15, TL | TR | BR | BL);   /* pin 4 = the centre, as named */
    check_corners(fp, 4, 0, 0.05, TL);           /* pin 5 on slot 4, down-right: both mirrored */

    fp = part_library_find_footprint(library, "ROUND-Q");
    check_pad(fp, 0, "1", -1, 0, 0.6, 0.3, 0, PAD_RECT);
    check_corners(fp, 0, 0, 0, 0);               /* pin1Shape: plain */
    check_pad(fp, 1, "2", 0, 1, 0.3, 0.6, 0, PAD_RECT);
    check_corners(fp, 1, 0.15, 0, 0);            /* turned, still rounded */

    fp = part_library_find_footprint(library, "CUT-Q");
    check_corners(fp, 0, 0, 0.1, TL);            /* left side, upper */
    check_corners(fp, 1, 0, 0.1, BL);            /* left side, lower */
    check_corners(fp, 2, 0, 0.1, BL);            /* bottom row, left (turned) */
    check_corners(fp, 3, 0, 0.1, BR);            /* bottom row, right */
    check_corners(fp, 6, 0, 0.1, TR);            /* top row, right */
    check_corners(fp, 7, 0, 0.1, TL);            /* top row, left */

    fp = part_library_find_footprint(library, "CUT-E");
    check_corners(fp, 0, 0, 0.1, TL | BL);
    check_corners(fp, 1, 0.1, 0, 0);
    free(error);
    part_library_destroy(library);
    printf("ok   rounded and cut pad corners expand as named and mirrored\n");

#define CPAD(KEYS) FP_DUAL("'pins':2,'pitch':1,'span':1,'centre':{'width':0.4,'height':0.2," KEYS "}")
    expect_fp_fail("pad corners: radius on a round pad", CPAD("'shape':'round','radius':0.05"),
                   "centre: \"radius\" needs a \"rect\" pad");
    expect_fp_fail("pad corners: chamfer on a round pad",
                   CPAD("'shape':'round','chamfer':0.05,'corners':['tl']"),
                   "centre: \"chamfer\" needs a \"rect\" pad");
    expect_fp_fail("pad corners: both", CPAD("'shape':'rect','radius':0.05,'chamfer':0.05,'corners':['tl']"),
                   "a pad takes \"radius\" or \"chamfer\", not both");
    expect_fp_fail("pad corners: radius too big", CPAD("'shape':'rect','radius':0.11"),
                   "\"radius\" 0.11 is more than half the pad's shorter side (0.1)");
    expect_fp_fail("pad corners: radius 0", CPAD("'shape':'rect','radius':0"),
                   "\"radius\" must be greater than 0");
    expect_fp_fail("pad corners: chamfer past an edge", CPAD("'shape':'rect','chamfer':0.25,'corners':['tl']"),
                   "\"chamfer\" 0.25 cuts past the left edge (0.2 long)");
    expect_fp_fail("pad corners: cuts meet on an edge", CPAD("'shape':'rect','chamfer':0.15,'corners':['tl','bl']"),
                   "\"chamfer\" 0.15 cuts both ends of the left edge (0.2 long)");
    expect_fp_fail("pad corners: chamfer without corners", CPAD("'shape':'rect','chamfer':0.05"),
                   "\"chamfer\" needs \"corners\"");
    expect_fp_fail("pad corners: corners without chamfer", CPAD("'shape':'rect','corners':['tl']"),
                   "\"corners\" needs \"chamfer\"");
    expect_fp_fail("pad corners: corners with radius", CPAD("'shape':'rect','radius':0.05,'corners':['tl']"),
                   "\"corners\" needs \"chamfer\"; \"radius\" rounds every corner");
    expect_fp_fail("pad corners: empty corners", CPAD("'shape':'rect','chamfer':0.05,'corners':[]"),
                   "\"corners\" must be a non-empty array of \"tl\", \"tr\", \"br\", \"bl\"");
    expect_fp_fail("pad corners: unknown corner", CPAD("'shape':'rect','chamfer':0.05,'corners':['top']"),
                   "\"corners\" must be a non-empty array of \"tl\", \"tr\", \"br\", \"bl\"");
    expect_fp_fail("pad corners: corner twice", CPAD("'shape':'rect','chamfer':0.05,'corners':['tl','tl']"),
                   "corner \"tl\" is named twice");
#undef CPAD
    expect_fp_fail("pad corners: mirrored onto the centre line",
                   FP_FILE("{'id':'F','description':'d','style':'dual','body':{'width':1,'height':1},"
                           "'pins':6,'pitch':1,'span':2,'pad':{'width':0.5,'height':0.2,'shape':'rect',"
                           "'chamfer':0.15,'corners':['br']}}"),
                   "pad 2 (mirrored corners): \"chamfer\" 0.15 cuts both ends of the right edge (0.2 long)");
    expect_fp_fail("pad corners: explicit pad",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") ",{'pin':2,'x':1,'y':0,'width':0.4,"
                                    "'height':0.4,'shape':'round','radius':0.1}]"),
                   "pads[1]: \"radius\" needs a \"rect\" pad");
}

static void check_footprint_cases(void) {
    expect_fp_fail("footprints: bad JSON",
                   "{'format':'schematical-footprints','version':1,'footprints':[" FP_SOT5 ",",
                   "invalid JSON at byte");
    expect_fp_fail("footprints: wrong format",
                   "{'format':'schematical-parts','version':1,'footprints':[]}",
                   "fp.json: \"format\" must be \"schematical-footprints\"");
    expect_fp_fail("footprints: version 2",
                   "{'format':'schematical-footprints','version':2,'footprints':[]}",
                   "unsupported version 2");
    expect_fp_fail("footprints: unknown file key",
                   "{'format':'schematical-footprints','version':1,'footprints':[],'kind':'led'}",
                   "unknown key \"kind\"");
    expect_fp_fail("footprint: unknown key", FP_DUAL("'pins':2,'pitch':1,'span':1,'colour':'x'"),
                   "footprints[0] (F): unknown key \"colour\"");
    expect_fp_fail("footprint: another style's key", FP_DUAL("'pins':2,'pitch':1,'span':1,'rows':2"),
                   "footprints[0] (F): unknown key \"rows\"");
    expect_fp_fail("footprint: duplicate key", FP_DUAL("'pins':2,'pitch':1,'span':1,'span':2"),
                   "duplicate key \"span\"");
    expect_fp_fail("footprint: missing span", FP_DUAL("'pins':2,'pitch':1"),
                   "footprints[0] (F): missing \"span\"");
    expect_fp_fail("footprint: missing body",
                   FP_FILE("{'id':'F','description':'d','style':'dual','pins':2,'pitch':1,'span':1,"
                           "'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"),
                   "missing \"body\"");
    expect_fp_fail("footprint: unknown style",
                   FP_FILE("{'id':'F','description':'d','style':'radial','pins':2,'body':{'width':1,'height':1}}"),
                   "unknown style \"radial\"");
    expect_fp_fail("footprint: bad id",
                   FP_FILE("{'id':'soic 14','description':'d','style':'dual','pins':2}"),
                   "\"id\" must use only A-Z, 0-9, '.' and '-'");
    expect_fp_fail("footprint: duplicate id in one file", FP_FILE(FP_SOT5 "," FP_SOT5),
                   "footprints[1] (SOT-5): duplicate footprint id \"SOT-5\"");
    expect_fp_fail("footprint: pins 0", FP_DUAL("'pins':0,'pitch':1,'span':1"),
                   "\"pins\" must be at least 1");
    expect_fp_fail("footprint: body width 0",
                   FP_FILE("{'id':'F','description':'d','style':'dual','pins':2,'pitch':1,'span':1,"
                           "'body':{'width':0,'height':1},'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"),
                   "footprints[0] (F): body: \"width\" must be greater than 0");
    expect_fp_fail("footprint: pad height -1",
                   FP_FILE("{'id':'F','description':'d','style':'dual','pins':2,'pitch':1,'span':1,"
                           "'body':{'width':1,'height':1},'pad':{'width':0.5,'height':-1,'shape':'rect'}}"),
                   "pad: \"height\" must be greater than 0");
    expect_fp_fail("footprint: pad shape oval",
                   FP_FILE("{'id':'F','description':'d','style':'dual','pins':2,'pitch':1,'span':1,"
                           "'body':{'width':1,'height':1},'pad':{'width':0.5,'height':0.5,'shape':'oval'}}"),
                   "\"shape\" must be \"rect\" or \"round\", not \"oval\"");
    expect_fp_fail("footprint: pitch 0", FP_DUAL("'pins':2,'pitch':0,'span':1"),
                   "\"pitch\" must be greater than 0");
    expect_fp_fail("footprint: span -1", FP_DUAL("'pins':2,'pitch':1,'span':-1"),
                   "\"span\" must be greater than 0");
    expect_fp_fail("footprint: drill as wide as the pad", FP_DUAL("'pins':2,'pitch':1,'span':1,'drill':0.5"),
                   "\"drill\" 0.5 must be smaller than the pad (0.5 x 0.5)");
    expect_fp_fail("footprint: pin1Shape square", FP_DUAL("'pins':2,'pitch':1,'span':1,'pin1Shape':'square'"),
                   "\"pin1Shape\" must be \"rect\" or \"round\"");
    expect_fp_fail("dual: odd pins without perSide", FP_DUAL("'pins':5,'pitch':1,'span':1"),
                   "an odd \"pins\" (5) needs \"perSide\"");
    expect_fp_fail("dual: perSide 0", FP_DUAL("'pins':2,'perSide':0,'pitch':1,'span':1"),
                   "\"perSide\" must be at least 1");
    expect_fp_fail("dual: perSide x 2 < pins without slots", FP_DUAL("'pins':5,'perSide':2,'pitch':1,'span':1"),
                   "5 pins do not fit on 4 slots");
    expect_fp_fail("quad: slot total < pins without slots", FP_QUAD("'pins':5,'sides':[1,1,1,1]"),
                   "5 pins do not fit on 4 slots");
    expect_fp_fail("quad: three sides", FP_QUAD("'pins':3,'sides':[1,1,1]"),
                   "\"sides\" must be 4 slot counts [left, bottom, right, top]");
    expect_fp_fail("slots: wrong length", FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,2]"),
                   "slots: \"slots\" must be an array of 3 slots, one per pin");
    expect_fp_fail("slots: duplicate", FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,2,2]"),
                   "slots: slot 2 is used twice");
    expect_fp_fail("slots: beyond the slot count",
                   FP_DUAL("'pins':5,'perSide':3,'pitch':1,'span':1,'slots':[1,2,3,4,7]"),
                   "footprints[0] (F): slots: slot 7 is beyond 6 slots");
    expect_fp_fail("slots: not a whole number", FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,2,2.5]"),
                   "slots[2] must be a whole number");
    expect_fp_fail("slots: 0 without centre", FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,0,2]"),
                   "slot 0 (the centre pad) needs \"centre\"");
    expect_fp_fail("slots: two 0s",
                   FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,0,0],"
                           "'centre':{'width':0.3,'height':0.3,'shape':'rect'}"),
                   "two pins are on slot 0 (the centre pad)");
    expect_fp_fail("grid: centre", FP_GRID_WITH("'pins':1,'balls':['A1'],'centre':{'width':0.3,'height':0.3,'shape':'rect'}"),
                   "\"centre\" is not allowed on a grid footprint");
    expect_fp_fail("grid: slots", FP_GRID_WITH("'pins':1,'balls':['A1'],'slots':[1]"),
                   "\"slots\" is not allowed on a grid footprint");
    expect_fp_fail("grid: pin1Shape", FP_GRID_WITH("'pins':1,'balls':['A1'],'pin1Shape':'rect'"),
                   "\"pin1Shape\" is not allowed on a grid footprint");
    expect_fp_fail("grid: rows 21",
                   FP_FILE("{'id':'F','description':'d','style':'grid','body':{'width':1,'height':1},"
                           "'pad':{'width':0.2,'height':0.2,'shape':'round'},'rows':21,'columns':2,"
                           "'pitch':0.5,'pins':1,'balls':['A1']}"),
                   "\"rows\" must be a whole number from 0 to 20");
    expect_fp_fail("grid: ball outside the grid (row)", FP_GRID_WITH("'pins':2,'balls':['A1','C1']"),
                   "balls: ball \"C1\" is outside the 2-row, 2-column grid");
    expect_fp_fail("grid: ball outside the grid (column)", FP_GRID_WITH("'pins':2,'balls':['A1','B3']"),
                   "ball \"B3\" is outside the 2-row, 2-column grid");
    expect_fp_fail("grid: two-letter row", FP_GRID_WITH("'pins':2,'balls':['A1','AA1']"),
                   "ball \"AA1\" is outside the 2-row, 2-column grid");
    expect_fp_fail("grid: duplicate ball", FP_GRID_WITH("'pins':2,'balls':['A1','A1']"),
                   "ball \"A1\" is used twice");
    expect_fp_fail("grid: wrong balls length", FP_GRID_WITH("'pins':3,'balls':['A1','A2']"),
                   "\"balls\" must be an array of 3 ball names, one per pin");
    expect_fp_fail("grid: not a ball name", FP_GRID_WITH("'pins':2,'balls':['A1','I1']"),
                   "\"I1\" is not a ball name");
    expect_fp_fail("explicit: pin missing", FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("0", "1") "]"),
                   "pin 2 has no pad");
    expect_fp_fail("explicit: pin beyond pins",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("3", "1") "]"),
                   "pads[1]: \"pin\" must be a whole number from 0 to 2");
    expect_fp_fail("explicit: parametric pad",
                   FP_EXPLICIT_WITH("'pad':{'width':0.5,'height':0.5,'shape':'rect'},"
                                    "'pads':[" EPAD("1", "0") "," EPAD("2", "1") "]"),
                   "\"pad\" is not allowed on an explicit footprint");
    expect_fp_fail("explicit: parametric pitch",
                   FP_EXPLICIT_WITH("'pitch':1,'pads':[" EPAD("1", "0") "," EPAD("2", "1") "]"),
                   "unknown key \"pitch\"");
    expect_fp_fail("explicit: pad drill too big",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") ",{'pin':2,'x':1,'y':0,'width':0.4,"
                                    "'height':0.4,'shape':'rect','drill':0.4}]"),
                   "pads[1]: \"drill\" 0.4 must be smaller than the pad (0.4 x 0.4)");
}

/* plan-11-A: an explicit pin may have several pads; they come out grouped by
   pin, file order within a pin, unnumbered last. */
static void check_explicit_repeated_pins(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;

    CHECK(load_fp(library,
                  FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("2", "1") "," EPAD("1", "2") "]"),
                  &error) == 0);
    fp = part_library_find_footprint(library, "F");
    CHECK(fp != NULL && fp->pins == 2 && fp->pad_count == 3);
    check_pad(fp, 0, "1", 0, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 1, "1", 2, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 2, "2", 1, 0, 0.4, 0.4, 0, PAD_RECT);
    part_library_destroy(library);
    printf("ok   explicit: pin repeated loads as 1, 1, 2\n");

    library = part_library_create();
    CHECK(load_fp(library,
                  FP_EXPLICIT_WITH("'pads':[" EPAD("2", "0") "," EPAD("1", "-1") "," EPAD("0", "3") ","
                                   EPAD("1", "1") "]"),
                  &error) == 0);
    fp = part_library_find_footprint(library, "F");
    CHECK(fp != NULL && fp->pins == 2 && fp->pad_count == fp->pins + 2);
    check_pad(fp, 0, "1", -1, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 1, "1", 1, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 2, "2", 0, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 3, NULL, 3, 0, 0.4, 0.4, 0, PAD_RECT);
    free(error);
    part_library_destroy(library);
    printf("ok   explicit: pin 1 at x -1 and 1 comes out pads[0], pads[1], file order\n");

    expect_fp_fail("explicit: pin 1 twice, pin 2 missing",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "-1") "," EPAD("1", "1") "]"), "pin 2 has no pad");
}

/* plan-36-J: explicit pads named by ball (a staggered WLCSP, a dual-row
   QFN's A1/B1 terminals). Grouped by ball in order of first use, then the
   unnumbered pads; a ball-pinout part matches them by designator. */
#define BPAD(BALL, X) "{'ball':'" BALL "','x':" X ",'y':0,'width':0.4,'height':0.4,'shape':'rect'}"
#define FP_EBALL(PINS, PADS) \
    FP_FILE("{'id':'EB','description':'d','style':'explicit','pins':" PINS ",'body':{'width':1,'height':1}," \
            "'pads':[" PADS "]}")
static void check_explicit_balls(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;
    const Part *part;
    char *parts;

    CHECK(load_fp(library, FP_EBALL("2", BPAD("B2", "0") "," EPAD("0", "3") "," BPAD("A1", "-1") "," BPAD("B2", "1")),
                  &error) == 0);
    fp = part_library_find_footprint(library, "EB");
    CHECK(fp != NULL && fp->pins == 2 && fp->pad_count == 4);
    check_pad(fp, 0, "B2", 0, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 1, "B2", 1, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 2, "A1", -1, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 3, NULL, 3, 0, 0.4, 0.4, 0, PAD_RECT);
    printf("ok   explicit ball pads: B2, B2, A1, then the unnumbered pad\n");
    parts = json(HEAD("led") "{'id':'led-eb','function':'LED','description':'d','drawer':[],"
                 "'pinouts':{'bga':[" BALL_LED_PINS "]},'colors':['Red'],'anode':'A','cathode':'K',"
                 "'parts':[{'name':'LED-EB','package':'WLCSP','pinout':'bga','footprint':'EB'}]}" TAIL);
    CHECK(load_parts(library, parts, "eb.json", &error) == 0);
    if (error != NULL) {
        printf("FAIL ball part on explicit ball pads: %s\n", error);
        failures++;
    }
    part = part_library_find(library, "LED-EB");
    CHECK(part != NULL && part->footprint == fp);
    free(parts);
    free(error);
    part_library_destroy(library);
    printf("ok   a ball-pinout part loads on an explicit footprint with ball pads A1 B2\n");

    expect_fp_fail("explicit balls: a ball missing", FP_EBALL("2", BPAD("A1", "0") "," BPAD("A1", "1")),
                   "1 balls for 2 pins");
    expect_fp_fail("explicit balls: one ball too many",
                   FP_EBALL("2", BPAD("A1", "0") "," BPAD("B2", "1") "," BPAD("C3", "2")), "more than 2 balls");
    expect_fp_fail("explicit balls: mixed with numbered pads", FP_EBALL("2", BPAD("A1", "0") "," EPAD("2", "1")),
                   "\"ball\" pads and numbered pads in one footprint");
    expect_fp_fail("explicit balls: pin and ball",
                   FP_EBALL("1", "{'pin':1,'ball':'A1','x':0,'y':0,'width':0.4,'height':0.4,'shape':'rect'}"),
                   "a pad has \"pin\" or \"ball\", not both");
    expect_fp_fail("explicit balls: not a ball name", FP_EBALL("1", BPAD("I1", "0")), "\"I1\" is not a ball name");
}

/* Slotted and non-plated holes on explicit pads (USB connector shells). */
#define SLOT_PAD(KEYS) "{'pin':2,'x':3,'y':0,'width':1.0,'height':2.0,'shape':'round','drill':0.6" KEYS "}"
#define NPTH_PAD(KEYS) "{'pin':0,'x':-3,'y':0,'shape':'round'," KEYS "}"
static void check_explicit_holes(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;

    CHECK(load_fp(library,
                  FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," SLOT_PAD(",'slotLength':1.4") ","
                                   NPTH_PAD("'width':0.9,'height':0.9,'drill':0.9,'plated':false") ","
                                   NPTH_PAD("'width':0.6,'height':1.2,'drill':0.6,'slotLength':1.2,'plated':false") "]"),
                  &error) == 0);
    fp = part_library_find_footprint(library, "F");
    CHECK(fp != NULL && fp->pad_count == 4);
    CHECK(fp->pads[0].slot_length == 0 && !fp->pads[0].npth);
    CHECK(fp->pads[1].drill == 0.6 && fp->pads[1].slot_length == 1.4 && !fp->pads[1].npth);
    CHECK(fp->pads[2].npth && fp->pads[2].drill == 0.9 && fp->pads[2].slot_length == 0);
    CHECK(fp->pads[3].npth && fp->pads[3].slot_length == 1.2);
    free(error);
    part_library_destroy(library);
    printf("ok   explicit: slotted and non-plated holes load\n");

    expect_fp_fail("explicit: slot without drill",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") ",{'pin':2,'x':3,'y':0,'width':1.0,'height':2.0,"
                                    "'shape':'round','slotLength':1.4}]"),
                   "\"slotLength\" needs a \"drill\"");
    expect_fp_fail("explicit: slot not longer than drill",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," SLOT_PAD(",'slotLength':0.6") "]"),
                   "\"slotLength\" 0.6 must be more than the drill 0.6");
    expect_fp_fail("explicit: slot past the pad",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," SLOT_PAD(",'slotLength':2.0") "]"),
                   "\"slotLength\" 2 must be smaller than the pad's long side 2");
    expect_fp_fail("explicit: slot in a square pad",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") ",{'pin':2,'x':3,'y':0,'width':1.5,'height':1.5,"
                                    "'shape':'round','drill':0.6,'slotLength':1.0}]"),
                   "a slotted pad must be longer along its slot (not square)");
    expect_fp_fail("explicit: plated true",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("2", "1") ","
                                    NPTH_PAD("'width':0.9,'height':0.9,'drill':0.9,'plated':true") "]"),
                   "\"plated\" may only be false");
    expect_fp_fail("explicit: non-plated pin",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") ",{'pin':2,'x':3,'y':0,'width':0.9,'height':0.9,"
                                    "'shape':'round','drill':0.9,'plated':false}]"),
                   "a non-plated hole is a \"pin\": 0 pad");
    expect_fp_fail("explicit: non-plated without drill",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("2", "1") ","
                                    NPTH_PAD("'width':0.9,'height':0.9,'plated':false") "]"),
                   "a non-plated hole needs a \"drill\"");
    expect_fp_fail("explicit: non-plated pad bigger than its hole",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("2", "1") ","
                                    NPTH_PAD("'width':1.5,'height':1.5,'drill':0.9,'plated':false") "]"),
                   "a non-plated pad is the hole itself");
}

/* Opposite-side pads and edge-mounted footprints (plan-54-E). */
#define OPP_PAD(PIN, X, KEYS) \
    "{'pin':" PIN ",'x':" X ",'y':0,'width':0.4,'height':1.2,'shape':'rect'" KEYS "}"
static void check_opposite_pads(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;

    CHECK(load_fp(library,
                  FP_EXPLICIT_WITH("'edgeMount':true,'pads':[" OPP_PAD("1", "0", "") "," OPP_PAD("2", "1", ",'opposite':true") "]"),
                  &error) == 0);
    fp = part_library_find_footprint(library, "F");
    CHECK(fp != NULL && fp->pad_count == 2 && fp->edge_mount == 1);
    CHECK(fp->pads[0].opposite == 0 && fp->pads[1].opposite == 1);
    free(error);
    part_library_destroy(library);
    printf("ok   explicit: opposite-side pads and edge mount load\n");

    library = part_library_create();
    error = NULL;
    CHECK(load_fp(library, FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("2", "1") "]"), &error) == 0);
    fp = part_library_find_footprint(library, "F");
    CHECK(fp != NULL && fp->edge_mount == 0 && fp->pads[0].opposite == 0 && fp->pads[1].opposite == 0);
    free(error);
    part_library_destroy(library);
    printf("ok   explicit: a footprint without the keys has edge_mount 0 and opposite 0\n");

    expect_fp_fail("explicit: opposite false",
                   FP_EXPLICIT_WITH("'pads':[" OPP_PAD("1", "0", "") "," OPP_PAD("2", "1", ",'opposite':false") "]"),
                   "\"opposite\" may only be true");
    expect_fp_fail("explicit: opposite 1",
                   FP_EXPLICIT_WITH("'pads':[" OPP_PAD("1", "0", "") "," OPP_PAD("2", "1", ",'opposite':1") "]"),
                   "\"opposite\" may only be true");
    expect_fp_fail("explicit: opposite with a drill",
                   FP_EXPLICIT_WITH("'pads':[" OPP_PAD("1", "0", "") "," OPP_PAD("2", "1", ",'drill':0.3,'opposite':true") "]"),
                   "an opposite pad is surface mount");
    expect_fp_fail("explicit: opposite with a slot",
                   FP_EXPLICIT_WITH("'pads':[" OPP_PAD("1", "0", "") ","
                                    OPP_PAD("2", "1", ",'drill':0.3,'slotLength':0.8,'opposite':true") "]"),
                   "an opposite pad is surface mount");
    expect_fp_fail("explicit: opposite non-plated",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("2", "1") ","
                                    NPTH_PAD("'width':0.9,'height':0.9,'drill':0.9,'plated':false,'opposite':true") "]"),
                   "an opposite pad is surface mount");
    expect_fp_fail("footprint: edgeMount false",
                   FP_EXPLICIT_WITH("'edgeMount':false,'pads':[" EPAD("1", "0") "," EPAD("2", "1") "]"),
                   "\"edgeMount\" may only be true");
    expect_fp_fail("footprint: edgeMount yes",
                   FP_EXPLICIT_WITH("'edgeMount':'yes','pads':[" EPAD("1", "0") "," EPAD("2", "1") "]"),
                   "\"edgeMount\" may only be true");
    expect_fp_fail("footprint: opposite on a dual footprint's pad",
                   FP_FILE("{'id':'F','description':'d','style':'dual','pins':2,'pitch':1,'span':1,'body':{'width':1,'height':1},"
                           "'pad':{'width':0.5,'height':0.5,'shape':'rect','opposite':true}}"),
                   "opposite");
}

/* An LED with one pinout of PINS whose part names FOOTPRINT (a JSON value). */
#define FP_LED(PINS, FOOTPRINT) \
    HEAD("led") "{'id':'led-fp','function':'LED','description':'d','drawer':[]," \
    "'pinouts':{'p':[" PINS "]},'colors':['Red'],'anode':'A','cathode':'K'," \
    "'parts':[{'name':'LED-FP','package':'X','pinout':'p','footprint':" FOOTPRINT "}]}" TAIL
#define FP_LED_NUMBERED "{'number':1,'name':'K','role':'input'},{'number':2,'name':'A','role':'input'}"
#define FP_LED_BALLS "{'ball':'A1','name':'K','role':'input'},{'ball':'B2','name':'A','role':'input'}"
#define FP_CHIP2 \
    "{'id':'CHIP-2','description':'d','style':'dual','pins':2,'perSide':1,'body':{'width':1,'height':0.5}," \
    "'pitch':1,'span':0.9,'pad':{'width':0.5,'height':0.7,'shape':'rect'}}"
#define FP_BGA2 \
    "{'id':'BGA-2','description':'d','style':'grid','pins':2,'rows':2,'columns':2,'pitch':0.5," \
    "'balls':['A1','A2'],'body':{'width':1,'height':1},'pad':{'width':0.2,'height':0.2,'shape':'round'}}"

#define FP_CHIP2B \
    "{'id':'CHIP-2B','description':'d','style':'dual','pins':2,'perSide':1,'body':{'width':1,'height':0.5}," \
    "'pitch':2,'span':0.9,'pad':{'width':0.5,'height':0.7,'shape':'rect'}}"
/* The FP_LED part on CHIP-2 with CONN (a JSON value) as its connectorFootprint. */
#define FP_LED_CONN(CONN) \
    HEAD("led") "{'id':'led-fp','function':'LED','description':'d','drawer':[]," \
    "'pinouts':{'p':[" FP_LED_NUMBERED "]},'colors':['Red'],'anode':'A','cathode':'K'," \
    "'parts':[{'name':'LED-FP','package':'X','pinout':'p','footprint':'CHIP-2'," \
    "'connectorFootprint':" CONN "}]}" TAIL

/* ---- plan-23-A: the header footprint style ----------------------------- */

#define FP_HEADER(KEYS) \
    FP_FILE("{'id':'H','description':'d','style':'header','body':{'width':7.62,'height':5.08}," \
            "'pad':{'width':1.8,'height':1.8,'shape':'round'},'drill':1.1,'pitch':2.54," KEYS "}")

static void check_header_footprints(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;

    CHECK(load_fp(library,
                  FP_FILE("{'id':'H2X3','description':'d','style':'header','pins':6,'rows':2,"
                          "'body':{'width':7.62,'height':5.08},'pitch':2.54,'rowPitch':2.54,'firstRowY':-1.27,"
                          "'pad':{'width':1.8,'height':1.8,'shape':'round'},'drill':1.1,'pin1Shape':'rect'},"
                          "{'id':'H1X3','description':'d','style':'header','pins':3,'rows':1,"
                          "'body':{'width':7.62,'height':2.54},'pitch':2.54,'firstRowY':-2.77,"
                          "'pad':{'width':1.8,'height':1.8,'shape':'round'},'drill':1.1}"),
                  &error) == 0);
    if (error != NULL) {
        printf("FAIL header footprints: %s\n", error);
        failures++;
    }
    fp = part_library_find_footprint(library, "H2X3");
    CHECK(fp != NULL && fp->pins == 6 && fp->pad_count == 6);
    check_pad(fp, 0, "1", -2.54, -1.27, 1.8, 1.8, 1.1, PAD_RECT);
    check_pad(fp, 1, "2", -2.54, 1.27, 1.8, 1.8, 1.1, PAD_ROUND);
    check_pad(fp, 2, "3", 0, -1.27, 1.8, 1.8, 1.1, PAD_ROUND);
    check_pad(fp, 3, "4", 0, 1.27, 1.8, 1.8, 1.1, PAD_ROUND);
    check_pad(fp, 4, "5", 2.54, -1.27, 1.8, 1.8, 1.1, PAD_ROUND);
    check_pad(fp, 5, "6", 2.54, 1.27, 1.8, 1.8, 1.1, PAD_ROUND);
    printf("ok   header 2x3: zigzag pads, pin 1 rect\n");
    fp = part_library_find_footprint(library, "H1X3");
    CHECK(fp != NULL && fp->pins == 3 && fp->pad_count == 3);
    check_pad(fp, 0, "1", -2.54, -2.77, 1.8, 1.8, 1.1, PAD_ROUND);
    check_pad(fp, 1, "2", 0, -2.77, 1.8, 1.8, 1.1, PAD_ROUND);
    check_pad(fp, 2, "3", 2.54, -2.77, 1.8, 1.8, 1.1, PAD_ROUND);
    printf("ok   header 1x3: one row at firstRowY -2.77\n");
    free(error);
    part_library_destroy(library);

    expect_fp_fail("header: rows 3", FP_HEADER("'pins':6,'rows':3,'rowPitch':2.54,'firstRowY':0"),
                   "\"rows\" must be 1 or 2");
    expect_fp_fail("header: pins not a multiple of rows",
                   FP_HEADER("'pins':7,'rows':2,'rowPitch':2.54,'firstRowY':0"),
                   "\"pins\" 7 is not a multiple of \"rows\" 2");
    expect_fp_fail("header: 2 rows without rowPitch", FP_HEADER("'pins':6,'rows':2,'firstRowY':0"),
                   "\"rowPitch\" is required with 2 rows");
    expect_fp_fail("header: 1 row with rowPitch",
                   FP_HEADER("'pins':3,'rows':1,'rowPitch':2.54,'firstRowY':0"),
                   "\"rowPitch\" is not allowed with 1 row");
}

static void check_part_footprints(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load_fp(library, FP_FILE(FP_CHIP2), &error) == 0);
    CHECK(load(library, FP_LED(FP_LED_NUMBERED, "'CHIP-2'"), &error) == 0);
    CHECK(load(library, HEAD("supply") VCC_DEVICE TAIL, &error) == 0);
    part = part_library_find(library, "LED-FP");
    CHECK(part != NULL && part->footprint == part_library_find_footprint(library, "CHIP-2"));
    part = part_library_find(library, "5V");
    CHECK(part != NULL && part->footprint == NULL);
    free(error);
    part_library_destroy(library);
    printf("ok   a part's \"footprint\" resolves; a part without one has none\n");

    expect_part_fail("part: unknown footprint", FP_FILE(FP_CHIP2), FP_LED(FP_LED_NUMBERED, "'CHIP-3'"),
                     "parts[0] (LED-FP): unknown footprint \"CHIP-3\"");
    expect_part_fail("part: pad count mismatch", FP_FILE(FP_SOT5), FP_LED(FP_LED_NUMBERED, "'SOT-5'"),
                     "footprint \"SOT-5\" has 5 pads, pinout \"p\" has 2 pins");
    expect_part_fail("part: pin without a pad", FP_FILE(FP_BGA2), FP_LED(FP_LED_BALLS, "'BGA-2'"),
                     "pin \"B2\" has no pad in footprint \"BGA-2\"");
    expect_part_fail("part: numbered pins on a grid footprint", FP_FILE(FP_BGA2),
                     FP_LED(FP_LED_NUMBERED, "'BGA-2'"), "pin \"1\" has no pad in footprint \"BGA-2\"");
    expect_part_fail("part: footprint not a string", FP_FILE(FP_CHIP2), FP_LED(FP_LED_NUMBERED, "2"),
                     "\"footprint\" must be a string");

    /* plan-11-A: "connectorFootprint", optional, checked like "footprint". */
    {
        PartLibrary *lib = part_library_create();
        char *err = NULL;

        CHECK(load_fp(lib, FP_FILE(FP_CHIP2 "," FP_CHIP2B), &err) == 0);
        CHECK(load(lib, FP_LED_CONN("'CHIP-2B'"), &err) == 0);
        part = part_library_find(lib, "LED-FP");
        CHECK(part != NULL && part->footprint == part_library_find_footprint(lib, "CHIP-2") &&
              part->connector_footprint != NULL &&
              strcmp(part->connector_footprint->id, "CHIP-2B") == 0);
        free(err);
        part_library_destroy(lib);
        printf("ok   a part's \"connectorFootprint\" resolves\n");
    }
    expect_part_fail("part: unknown connectorFootprint", FP_FILE(FP_CHIP2), FP_LED_CONN("'HDR-1X17'"),
                     "parts[0] (LED-FP): unknown connectorFootprint \"HDR-1X17\"");
    expect_part_fail("part: connectorFootprint pad count mismatch", FP_FILE(FP_CHIP2 "," FP_SOT5),
                     FP_LED_CONN("'SOT-5'"),
                     "connectorFootprint \"SOT-5\" has 5 pads, pinout \"p\" has 2 pins");
    expect_part_fail("part: pin without a pad in connectorFootprint", FP_FILE(FP_CHIP2 "," FP_BGA2),
                     FP_LED_CONN("'BGA-2'"), "pin \"1\" has no pad in connectorFootprint \"BGA-2\"");
    expect_fail("supply part with connectorFootprint",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin','connectorFootprint':'T1'}]}" TAIL,
                "parts[0] (VCC): a supply has no footprint");

    /* plan-03-C: the key is required on every part but a supply, which rejects it. */
    expect_fail("non-supply part without footprint",
                HEAD("led") "{'id':'led-nf','function':'LED','description':'d','drawer':[],"
                "'pinouts':{'p':[" FP_LED_NUMBERED "]},'colors':['Red'],'anode':'A','cathode':'K',"
                "'parts':[{'name':'LED-NF','package':'X','pinout':'p'}]}" TAIL,
                "parts[0] (LED-NF): missing \"footprint\"");
    expect_fail("supply part with footprint",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin','footprint':'T1'}]}" TAIL,
                "parts[0] (VCC): a supply has no footprint");
}

int main(void) {
    check_footprint_styles();
    check_explicit_repeated_pins();
    check_explicit_balls();
    check_explicit_holes();
    check_opposite_pads();
    check_footprint_corners();
    check_footprint_cases();
    check_header_footprints();
    check_part_footprints();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("library_check: all passed\n");
    return 0;
}
