/*
 * Footprints: parses Parts/footprints.json, checks every rule of
 * docs/parts-format.md § Footprints, and expands each parametric footprint
 * (dual, quad, grid) or literal one (explicit) into a flat pad list in the
 * file's arena. library.c registers the result.
 */

#include "footprint.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "library_internal.h"

#define MAX_PINS 1000u

typedef struct {
    double width, height;
    PadShape shape;
    double radius, chamfer;
    unsigned corners;
} PadSpec;

/* What a dual or quad footprint puts on one slot. */
typedef struct {
    double x, y;
    int turned;   /* top/bottom row of a quad: the pad is rotated 90° */
} Slot;

static const char *const FILE_KEYS[] = {"format", "version", "footprints", NULL};
static const char *const COMMON_KEYS[] = {"id", "description", "note", "style", "pins", "body", NULL};
static const char *const DUAL_KEYS[] = {
    "pad", "drill", "pin1Shape", "centre", "slots", "pitch", "span", "perSide", NULL
};
static const char *const QUAD_KEYS[] = {
    "pad", "drill", "pin1Shape", "centre", "slots", "sides", "pitch", "spanX", "spanY", NULL
};
static const char *const GRID_KEYS[] = {"pad", "drill", "rows", "columns", "pitch", "balls", NULL};
static const char *const EXPLICIT_KEYS[] = {"pads", NULL};
static const char *const SIZE_KEYS[] = {"width", "height", "shape", "radius", "chamfer", "corners", NULL};
static const char *const BODY_KEYS[] = {"width", "height", "round", NULL};
static const char *const PAD_KEYS[] = {
    "pin", "x", "y", "width", "height", "shape", "drill", "radius", "chamfer", "corners", NULL
};
static const char *const CORNER_NAMES[] = {"tl", "tr", "br", "bl"};   /* bit 1 << index */

/* Keys a style does not take, each with its own message. */
static const char *const GRID_FORBIDDEN[] = {"slots", "centre", "pin1Shape", NULL};
static const char *const EXPLICIT_FORBIDDEN[] = {"pad", "drill", "slots", "centre", "pin1Shape", NULL};

/* Rounds away float noise (and -0) so goldens print stable numbers. */
static double clean(double value) {
    return round(value * 1e6) / 1e6 + 0.0;
}

static int positive(KindContext *ctx, const cJSON *object, const char *key, double *out) {
    if (kind_number(ctx, object, key, out) != 0) {
        return -1;
    }
    if (!(*out > 0)) {
        return kind_error(ctx, "\"%s\" must be greater than 0", key);
    }
    return 0;
}

static int shape(KindContext *ctx, const cJSON *object, const char *key, PadShape *out) {
    const char *text;

    if (kind_string(ctx, object, key, &text) != 0) {
        return -1;
    }
    if (strcmp(text, "rect") == 0) {
        *out = PAD_RECT;
    } else if (strcmp(text, "round") == 0) {
        *out = PAD_ROUND;
    } else {
        return kind_error(ctx, "\"%s\" must be \"rect\" or \"round\", not \"%s\"", key, text);
    }
    return 0;
}

/*
 * Cuts must fit their edges: on each side, chamfer x (cut corners at its two
 * ends) is at most the side's length.
 */
static int check_cuts(KindContext *ctx, double width, double height, double chamfer,
                      unsigned corners) {
    static const struct { unsigned a, b; int across; const char *name; } edges[] = {
        {PAD_CORNER_TL, PAD_CORNER_TR, 1, "top"},
        {PAD_CORNER_TR, PAD_CORNER_BR, 0, "right"},
        {PAD_CORNER_BR, PAD_CORNER_BL, 1, "bottom"},
        {PAD_CORNER_BL, PAD_CORNER_TL, 0, "left"},
    };
    unsigned i;

    for (i = 0; i < 4; i++) {
        double length = edges[i].across ? width : height;
        int cuts = ((corners & edges[i].a) != 0) + ((corners & edges[i].b) != 0);

        if (cuts * chamfer > length + 1e-9) {
            return kind_error(ctx, "\"chamfer\" %g cuts %s the %s edge (%g long)", chamfer,
                              cuts == 2 ? "both ends of" : "past", edges[i].name, length);
        }
    }
    return 0;
}

/*
 * The optional corner keys of a pad: "radius" (every corner rounded, at most
 * half the shorter side), or "chamfer" with "corners" (the named corners cut
 * at 45 degrees, fitting every edge). Rect pads only.
 */
static int corner_keys(KindContext *ctx, const cJSON *json, PadShape pad_shape, double width,
                       double height, double *radius, double *chamfer, unsigned *corners) {
    const cJSON *radius_json = cJSON_GetObjectItemCaseSensitive(json, "radius");
    const cJSON *chamfer_json = cJSON_GetObjectItemCaseSensitive(json, "chamfer");
    const cJSON *corners_json = cJSON_GetObjectItemCaseSensitive(json, "corners");
    const cJSON *item;
    double limit = (width < height ? width : height) / 2;

    *radius = 0;
    *chamfer = 0;
    *corners = 0;
    if (radius_json == NULL && chamfer_json == NULL) {
        if (corners_json != NULL) {
            return kind_error(ctx, "\"corners\" needs \"chamfer\"");
        }
        return 0;
    }
    if (pad_shape != PAD_RECT) {
        return kind_error(ctx, "\"%s\" needs a \"rect\" pad",
                          radius_json != NULL ? "radius" : "chamfer");
    }
    if (radius_json != NULL && chamfer_json != NULL) {
        return kind_error(ctx, "a pad takes \"radius\" or \"chamfer\", not both");
    }
    if (radius_json != NULL) {
        if (corners_json != NULL) {
            return kind_error(ctx, "\"corners\" needs \"chamfer\"; \"radius\" rounds every corner");
        }
        if (positive(ctx, json, "radius", radius) != 0) {
            return -1;
        }
        if (*radius > limit + 1e-9) {
            return kind_error(ctx, "\"radius\" %g is more than half the pad's shorter side (%g)",
                              *radius, limit);
        }
        return 0;
    }
    if (positive(ctx, json, "chamfer", chamfer) != 0) {
        return -1;
    }
    if (corners_json == NULL) {
        return kind_error(ctx, "\"chamfer\" needs \"corners\"");
    }
    if (!cJSON_IsArray(corners_json) || cJSON_GetArraySize(corners_json) == 0) {
        return kind_error(ctx, "\"corners\" must be a non-empty array of \"tl\", \"tr\", \"br\", \"bl\"");
    }
    cJSON_ArrayForEach(item, corners_json) {
        unsigned bit = 0;
        unsigned i;

        for (i = 0; i < 4 && cJSON_IsString(item); i++) {
            if (strcmp(item->valuestring, CORNER_NAMES[i]) == 0) {
                bit = 1u << i;
            }
        }
        if (bit == 0) {
            return kind_error(ctx, "\"corners\" must be a non-empty array of \"tl\", \"tr\", \"br\", \"bl\"");
        }
        if (*corners & bit) {
            return kind_error(ctx, "corner \"%s\" is named twice", item->valuestring);
        }
        *corners |= bit;
    }
    return check_cuts(ctx, width, height, *chamfer, *corners);
}

/* Swaps left and right corners (tl <-> tr, bl <-> br). */
static unsigned flip_x(unsigned corners) {
    return ((corners & PAD_CORNER_TL) ? PAD_CORNER_TR : 0) | ((corners & PAD_CORNER_TR) ? PAD_CORNER_TL : 0) |
           ((corners & PAD_CORNER_BR) ? PAD_CORNER_BL : 0) | ((corners & PAD_CORNER_BL) ? PAD_CORNER_BR : 0);
}

/* Swaps top and bottom corners (tl <-> bl, tr <-> br). */
static unsigned flip_y(unsigned corners) {
    return ((corners & PAD_CORNER_TL) ? PAD_CORNER_BL : 0) | ((corners & PAD_CORNER_BL) ? PAD_CORNER_TL : 0) |
           ((corners & PAD_CORNER_TR) ? PAD_CORNER_BR : 0) | ((corners & PAD_CORNER_BR) ? PAD_CORNER_TR : 0);
}

/*
 * A parametric pad's corners are named for a pad up and left of the body
 * centre; the footprint is mirror-symmetric about both centre lines. Right of
 * centre mirrors left/right, below mirrors top/bottom, and a pad on a centre
 * line takes both sides.
 */
static unsigned mirrored_corners(unsigned corners, double x, double y) {
    const double near = 1e-9;

    corners = x > near ? flip_x(corners) : x < -near ? corners : corners | flip_x(corners);
    return y > near ? flip_y(corners) : y < -near ? corners : corners | flip_y(corners);
}

/* {"width", "height", "shape"} and the corner keys under `key`. */
static int pad_spec(KindContext *ctx, const cJSON *object, const char *key, PadSpec *out) {
    const cJSON *json = kind_member(ctx, object, key, 1);

    if (json == NULL) {
        return -1;
    }
    kind_push(ctx, "%s", key);
    if (kind_check_keys(ctx, json, SIZE_KEYS, NULL) != 0 ||
        positive(ctx, json, "width", &out->width) != 0 ||
        positive(ctx, json, "height", &out->height) != 0 ||
        shape(ctx, json, "shape", &out->shape) != 0 ||
        corner_keys(ctx, json, out->shape, out->width, out->height, &out->radius, &out->chamfer,
                    &out->corners) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

/*
 * Copies a spec's size, shape and corners onto a pad at (out->x, out->y). A
 * mirrored pad on a centre line can gain corners, so its cuts are checked
 * again.
 */
static int put_spec(KindContext *ctx, FootprintPad *out, const PadSpec *spec, int turned,
                    int mirror) {
    out->width = turned ? spec->height : spec->width;
    out->height = turned ? spec->width : spec->height;
    out->shape = spec->shape;
    out->radius = spec->radius;
    out->chamfer = spec->chamfer;
    out->corners = mirror ? mirrored_corners(spec->corners, out->x, out->y) : spec->corners;
    if (out->chamfer > 0 && out->corners != spec->corners) {
        kind_push(ctx, "pad %s (mirrored corners)", out->designator != NULL ? out->designator : "-");
        if (check_cuts(ctx, out->width, out->height, out->chamfer, out->corners) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    return 0;
}

/* A drill (0 = none) must fit inside the pad. */
static int check_drill(KindContext *ctx, double drill, double width, double height) {
    if (!(drill >= 0)) {
        return kind_error(ctx, "\"drill\" must be 0 or more");
    }
    if (drill > 0 && (drill >= width || drill >= height)) {
        return kind_error(ctx, "\"drill\" %g must be smaller than the pad (%g x %g)", drill, width,
                          height);
    }
    return 0;
}

/* A whole number in 0..max, or -1 when the item is not one. */
static long whole(const cJSON *item, unsigned max) {
    if (!cJSON_IsNumber(item) || item->valuedouble != floor(item->valuedouble) ||
        item->valuedouble < 0 || item->valuedouble > max) {
        return -1;
    }
    return (long)item->valuedouble;
}

static char *number_text(KindContext *ctx, unsigned number) {
    char text[16];

    snprintf(text, sizeof text, "%u", number);
    return kind_strdup(ctx, text);
}

/* ---- dual and quad: pins on slots ---------------------------------------- */

/*
 * Reads "slots" into pin_slot[pins] (1-based slots, 0 = the centre pad), or
 * fills pin n -> slot n when there is none. Returns -1 on error.
 */
static int read_slots(KindContext *ctx, const cJSON *json, unsigned pins, unsigned slot_count,
                      int has_centre, unsigned *pin_slot) {
    const cJSON *slots = cJSON_GetObjectItemCaseSensitive(json, "slots");
    const cJSON *item;
    unsigned char *used;
    unsigned i = 0;
    int centre_used = 0;

    if (slots == NULL) {
        if (pins > slot_count) {
            return kind_error(ctx, "%u pins do not fit on %u slots; add \"slots\" or more slots",
                              pins, slot_count);
        }
        for (i = 0; i < pins; i++) {
            pin_slot[i] = i + 1;
        }
        return 0;
    }
    kind_push(ctx, "slots");
    if (!cJSON_IsArray(slots) || (unsigned)cJSON_GetArraySize(slots) != pins) {
        return kind_error(ctx, "\"slots\" must be an array of %u slots, one per pin", pins);
    }
    used = kind_alloc(ctx, slot_count + 1);
    if (used == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(item, slots) {
        long slot = whole(item, MAX_PINS * 4);

        if (slot < 0) {
            return kind_error(ctx, "slots[%u] must be a whole number", i);
        }
        if (slot == 0) {
            if (!has_centre) {
                return kind_error(ctx, "slot 0 (the centre pad) needs \"centre\"");
            }
            if (centre_used) {
                return kind_error(ctx, "two pins are on slot 0 (the centre pad)");
            }
            centre_used = 1;
        } else if ((unsigned long)slot > slot_count) {
            return kind_error(ctx, "slot %ld is beyond %u slots", slot, slot_count);
        } else if (used[slot]) {
            return kind_error(ctx, "slot %ld is used twice", slot);
        } else {
            used[slot] = 1;
        }
        pin_slot[i++] = (unsigned)slot;
    }
    kind_pop(ctx);
    return 0;
}

/* Builds the pads of a dual or quad footprint from its slot geometry. */
static int emit_slotted(KindContext *ctx, const cJSON *json, PartFootprint *fp, const Slot *slots,
                        unsigned slot_count) {
    const cJSON *centre_json = cJSON_GetObjectItemCaseSensitive(json, "centre");
    PadSpec pad;
    PadSpec centre = {0};
    PadShape pin1 = PAD_RECT;
    int has_pin1 = cJSON_GetObjectItemCaseSensitive(json, "pin1Shape") != NULL;
    double drill = 0;
    unsigned *pin_slot;
    FootprintPad *pads;
    size_t n = 0;
    int centre_used = 0;
    unsigned i;

    if (pad_spec(ctx, json, "pad", &pad) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "drill") != NULL &&
        kind_number(ctx, json, "drill", &drill) != 0) {
        return -1;
    }
    if (check_drill(ctx, drill, pad.width, pad.height) != 0) {
        return -1;
    }
    if (has_pin1 && shape(ctx, json, "pin1Shape", &pin1) != 0) {
        return -1;
    }
    if (centre_json != NULL && pad_spec(ctx, json, "centre", &centre) != 0) {
        return -1;
    }
    pin_slot = kind_alloc(ctx, fp->pins * sizeof(*pin_slot));
    pads = kind_alloc(ctx, (fp->pins + 1) * sizeof(*pads));
    if (pin_slot == NULL || pads == NULL ||
        read_slots(ctx, json, fp->pins, slot_count, centre_json != NULL, pin_slot) != 0) {
        return -1;
    }
    for (i = 0; i < fp->pins; i++) {
        FootprintPad *out = &pads[n++];

        out->designator = number_text(ctx, i + 1);
        if (out->designator == NULL) {
            return -1;
        }
        if (pin_slot[i] == 0) {
            if (put_spec(ctx, out, &centre, 0, 0) != 0) {
                return -1;
            }
            centre_used = 1;
        } else {
            const Slot *slot = &slots[pin_slot[i] - 1];

            out->x = clean(slot->x);
            out->y = clean(slot->y);
            out->drill = drill;
            if (i == 0 && has_pin1) {
                /* pin1Shape is a plain shape: no rounded or cut corners. */
                PadSpec plain = {pad.width, pad.height, pin1, 0, 0, 0};

                if (put_spec(ctx, out, &plain, slot->turned, 0) != 0) {
                    return -1;
                }
            } else if (put_spec(ctx, out, &pad, slot->turned, 1) != 0) {
                return -1;
            }
        }
    }
    if (centre_json != NULL && !centre_used) {
        if (put_spec(ctx, &pads[n++], &centre, 0, 0) != 0) {
            return -1;
        }
    }
    fp->pads = pads;
    fp->pad_count = n;
    return 0;
}

/* Two columns: slots 1..P down the left, P+1..2P up the right. */
static int load_dual(KindContext *ctx, const cJSON *json, PartFootprint *fp) {
    double pitch, span;
    uint64_t per_side = fp->pins / 2;
    Slot *slots;
    unsigned s;

    if (positive(ctx, json, "pitch", &pitch) != 0 || positive(ctx, json, "span", &span) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "perSide") != NULL) {
        if (kind_uint(ctx, json, "perSide", MAX_PINS, &per_side) != 0) {
            return -1;
        }
        if (per_side == 0) {
            return kind_error(ctx, "\"perSide\" must be at least 1");
        }
    } else if (fp->pins % 2 != 0) {
        return kind_error(ctx, "an odd \"pins\" (%u) needs \"perSide\"", fp->pins);
    }
    slots = kind_alloc(ctx, 2 * per_side * sizeof(*slots));
    if (slots == NULL) {
        return -1;
    }
    for (s = 1; s <= 2 * per_side; s++) {
        double centre_row = ((double)per_side - 1) / 2;

        if (s <= per_side) {
            slots[s - 1].x = -span / 2;
            slots[s - 1].y = ((double)s - 1 - centre_row) * pitch;
        } else {
            slots[s - 1].x = span / 2;
            slots[s - 1].y = ((double)(2 * per_side - s) - centre_row) * pitch;
        }
    }
    return emit_slotted(ctx, json, fp, slots, (unsigned)(2 * per_side));
}

/* Four sides, counter-clockwise from the top of the left side. */
static int load_quad(KindContext *ctx, const cJSON *json, PartFootprint *fp) {
    const cJSON *sides_json = kind_member(ctx, json, "sides", 1);
    const cJSON *item;
    double pitch, span_x, span_y;
    unsigned sides[4];
    unsigned total = 0;
    unsigned side, i;
    Slot *slots;
    size_t n = 0;

    if (sides_json == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(sides_json) || cJSON_GetArraySize(sides_json) != 4) {
        return kind_error(ctx, "\"sides\" must be 4 slot counts [left, bottom, right, top]");
    }
    side = 0;
    cJSON_ArrayForEach(item, sides_json) {
        long count = whole(item, MAX_PINS);

        if (count < 0) {
            return kind_error(ctx, "\"sides\" must be 4 slot counts [left, bottom, right, top]");
        }
        sides[side++] = (unsigned)count;
        total += (unsigned)count;
    }
    if (total == 0) {
        return kind_error(ctx, "\"sides\" has no slots");
    }
    if (positive(ctx, json, "pitch", &pitch) != 0 || positive(ctx, json, "spanX", &span_x) != 0 ||
        positive(ctx, json, "spanY", &span_y) != 0) {
        return -1;
    }
    slots = kind_alloc(ctx, total * sizeof(*slots));
    if (slots == NULL) {
        return -1;
    }
    for (side = 0; side < 4; side++) {
        double middle = ((double)sides[side] - 1) / 2;

        for (i = 0; i < sides[side]; i++) {
            Slot *slot = &slots[n++];
            double along = ((double)i - middle) * pitch;

            switch (side) {
                case 0: slot->x = -span_x / 2; slot->y = along; break;              /* left, down */
                case 1: slot->x = along; slot->y = span_y / 2; slot->turned = 1; break;  /* bottom, right */
                case 2: slot->x = span_x / 2; slot->y = -along; break;              /* right, up */
                default: slot->x = -along; slot->y = -span_y / 2; slot->turned = 1; break; /* top, left */
            }
        }
    }
    return emit_slotted(ctx, json, fp, slots, total);
}

/* ---- grid ------------------------------------------------------------------ */

typedef struct {
    const char *name;
    int row;
    unsigned column;
} Ball;

static int by_position(const void *a, const void *b) {
    const Ball *x = a;
    const Ball *y = b;

    if (x->row != y->row) {
        return x->row < y->row ? -1 : 1;
    }
    return x->column < y->column ? -1 : x->column > y->column;
}

static int load_grid(KindContext *ctx, const cJSON *json, PartFootprint *fp) {
    const cJSON *balls_json;
    const cJSON *item;
    uint64_t rows, columns;
    double pitch, drill = 0;
    PadSpec pad;
    Ball *balls;
    FootprintPad *pads;
    unsigned n = 0;
    unsigned i;

    if (kind_uint(ctx, json, "rows", 20, &rows) != 0 ||
        kind_uint(ctx, json, "columns", 30, &columns) != 0) {
        return -1;
    }
    if (rows == 0 || columns == 0) {
        return kind_error(ctx, "\"rows\" and \"columns\" must be at least 1");
    }
    if (positive(ctx, json, "pitch", &pitch) != 0 || pad_spec(ctx, json, "pad", &pad) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "drill") != NULL &&
        kind_number(ctx, json, "drill", &drill) != 0) {
        return -1;
    }
    if (check_drill(ctx, drill, pad.width, pad.height) != 0) {
        return -1;
    }
    balls_json = kind_member(ctx, json, "balls", 1);
    if (balls_json == NULL) {
        return -1;
    }
    kind_push(ctx, "balls");
    if (!cJSON_IsArray(balls_json) || (unsigned)cJSON_GetArraySize(balls_json) != fp->pins) {
        return kind_error(ctx, "\"balls\" must be an array of %u ball names, one per pin", fp->pins);
    }
    balls = kind_alloc(ctx, fp->pins * sizeof(*balls));
    pads = kind_alloc(ctx, fp->pins * sizeof(*pads));
    if (balls == NULL || pads == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(item, balls_json) {
        Ball *ball = &balls[n];
        const char *name;
        unsigned other;

        if (!cJSON_IsString(item)) {
            return kind_error(ctx, "\"balls\" must be an array of ball names");
        }
        name = item->valuestring;
        if (!kind_is_ball_name(name)) {
            return kind_error(ctx, "\"%s\" is not a ball name", name);
        }
        ball->row = kind_ball_row(name[0]);
        ball->column = (unsigned)strtoul(name + 1, NULL, 10);
        if (kind_ball_row(name[1]) >= 0 || ball->row >= (int)rows || ball->column > columns) {
            return kind_error(ctx, "ball \"%s\" is outside the %u-row, %u-column grid", name,
                              (unsigned)rows, (unsigned)columns);
        }
        for (other = 0; other < n; other++) {
            if (strcmp(balls[other].name, name) == 0) {
                return kind_error(ctx, "ball \"%s\" is used twice", name);
            }
        }
        ball->name = kind_strdup(ctx, name);
        if (ball->name == NULL) {
            return -1;
        }
        n++;
    }
    kind_pop(ctx);
    qsort(balls, n, sizeof(*balls), by_position);
    for (i = 0; i < n; i++) {
        pads[i].designator = balls[i].name;
        pads[i].x = clean(((double)balls[i].column - 1 - ((double)columns - 1) / 2) * pitch);
        pads[i].y = clean(((double)balls[i].row - ((double)rows - 1) / 2) * pitch);
        pads[i].drill = drill;
        if (put_spec(ctx, &pads[i], &pad, 0, 1) != 0) {
            return -1;
        }
    }
    fp->pads = pads;
    fp->pad_count = n;
    return 0;
}

/* ---- explicit ------------------------------------------------------------- */

static int load_explicit(KindContext *ctx, const cJSON *json, PartFootprint *fp) {
    const cJSON *pads_json = kind_member(ctx, json, "pads", 1);
    const cJSON *item;
    FootprintPad *read;
    FootprintPad *pads;
    unsigned *pin_of;
    size_t *where;                      /* pads per pin */
    size_t count, i, n = 0;
    unsigned pin;

    if (pads_json == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(pads_json) || cJSON_GetArraySize(pads_json) == 0) {
        return kind_error(ctx, "\"pads\" must be a non-empty array of pads");
    }
    count = (size_t)cJSON_GetArraySize(pads_json);
    read = kind_alloc(ctx, count * sizeof(*read));
    pads = kind_alloc(ctx, count * sizeof(*pads));
    pin_of = kind_alloc(ctx, count * sizeof(*pin_of));
    where = kind_alloc(ctx, (fp->pins + 1) * sizeof(*where));
    if (read == NULL || pads == NULL || pin_of == NULL || where == NULL) {
        return -1;
    }
    i = 0;
    cJSON_ArrayForEach(item, pads_json) {
        FootprintPad *pad = &read[i];
        uint64_t number;

        kind_push(ctx, "pads[%zu]", i);
        if (kind_check_keys(ctx, item, PAD_KEYS, NULL) != 0 ||
            kind_uint(ctx, item, "pin", fp->pins, &number) != 0 ||
            kind_number(ctx, item, "x", &pad->x) != 0 ||
            kind_number(ctx, item, "y", &pad->y) != 0 ||
            positive(ctx, item, "width", &pad->width) != 0 ||
            positive(ctx, item, "height", &pad->height) != 0 ||
            shape(ctx, item, "shape", &pad->shape) != 0 ||
            corner_keys(ctx, item, pad->shape, pad->width, pad->height, &pad->radius,
                        &pad->chamfer, &pad->corners) != 0) {
            return -1;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "drill") != NULL &&
            kind_number(ctx, item, "drill", &pad->drill) != 0) {
            return -1;
        }
        if (check_drill(ctx, pad->drill, pad->width, pad->height) != 0) {
            return -1;
        }
        pad->x = clean(pad->x);
        pad->y = clean(pad->y);
        pin_of[i] = (unsigned)number;
        if (number > 0) {
            where[number]++;
            pad->designator = number_text(ctx, (unsigned)number);
            if (pad->designator == NULL) {
                return -1;
            }
        }
        kind_pop(ctx);
        i++;
    }
    /* Grouped by pin, file order within a pin (a pin may have several pads),
       then the unnumbered pads in file order. */
    for (pin = 1; pin <= fp->pins; pin++) {
        if (where[pin] == 0) {
            return kind_error(ctx, "pin %u has no pad", pin);
        }
        for (i = 0; i < count; i++) {
            if (pin_of[i] == pin) {
                pads[n++] = read[i];
            }
        }
    }
    for (i = 0; i < count; i++) {
        if (pin_of[i] == 0) {
            pads[n++] = read[i];
        }
    }
    fp->pads = pads;
    fp->pad_count = n;
    return 0;
}

/* ---- One footprint, one file ---------------------------------------------- */

static int valid_id(const char *id) {
    if (*id == '\0') {
        return 0;
    }
    for (; *id != '\0'; id++) {
        if (!((*id >= 'A' && *id <= 'Z') || (*id >= '0' && *id <= '9') || *id == '.' ||
              *id == '-')) {
            return 0;
        }
    }
    return 1;
}

static int load_footprint(KindContext *ctx, const cJSON *json, const PartLibrary *library,
                          const FootprintList *file, PartFootprint *fp) {
    const cJSON *body;
    const char *style;
    const char *const *forbidden = NULL;
    const char *const *style_keys;
    uint64_t pins;
    size_t i;

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "expected an object");
    }
    if (kind_string(ctx, json, "id", &fp->id) != 0) {
        return -1;
    }
    {
        /* "footprints[3]" becomes "footprints[3] (SOIC-14)" for later messages. */
        char segment[sizeof ctx->path[0]];

        snprintf(segment, sizeof segment, "%s",
                 ctx->depth - 1 < KIND_PATH_DEPTH ? ctx->path[ctx->depth - 1] : "");
        kind_pop(ctx);
        kind_push(ctx, "%s (%s)", segment, fp->id);
    }
    if (!valid_id(fp->id)) {
        return kind_error(ctx, "\"id\" must use only A-Z, 0-9, '.' and '-'");
    }
    if (part_library_find_footprint(library, fp->id) != NULL) {
        return kind_error(ctx, "duplicate footprint id \"%s\"", fp->id);
    }
    for (i = 0; i < file->count; i++) {
        if (strcmp(file->items[i]->id, fp->id) == 0) {
            return kind_error(ctx, "duplicate footprint id \"%s\"", fp->id);
        }
    }
    if (kind_string(ctx, json, "style", &style) != 0) {
        return -1;
    }
    if (strcmp(style, "dual") == 0) {
        style_keys = DUAL_KEYS;
    } else if (strcmp(style, "quad") == 0) {
        style_keys = QUAD_KEYS;
    } else if (strcmp(style, "grid") == 0) {
        style_keys = GRID_KEYS;
        forbidden = GRID_FORBIDDEN;
    } else if (strcmp(style, "explicit") == 0) {
        style_keys = EXPLICIT_KEYS;
        forbidden = EXPLICIT_FORBIDDEN;
    } else {
        return kind_error(ctx, "unknown style \"%s\"", style);
    }
    for (; forbidden != NULL && *forbidden != NULL; forbidden++) {
        if (cJSON_GetObjectItemCaseSensitive(json, *forbidden) != NULL) {
            return kind_error(ctx, "\"%s\" is not allowed on %s %s footprint", *forbidden,
                              style[0] == 'e' ? "an" : "a", style);
        }
    }
    if (kind_check_keys(ctx, json, COMMON_KEYS, style_keys) != 0 ||
        kind_string(ctx, json, "description", &fp->description) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "note") != NULL &&
        !cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json, "note"))) {
        return kind_error(ctx, "\"note\" must be a string");
    }
    if (kind_uint(ctx, json, "pins", MAX_PINS, &pins) != 0) {
        return -1;
    }
    if (pins == 0) {
        return kind_error(ctx, "\"pins\" must be at least 1");
    }
    fp->pins = (unsigned)pins;

    body = kind_member(ctx, json, "body", 1);
    if (body == NULL) {
        return -1;
    }
    kind_push(ctx, "body");
    if (kind_check_keys(ctx, body, BODY_KEYS, NULL) != 0 ||
        positive(ctx, body, "width", &fp->body_width) != 0 ||
        positive(ctx, body, "height", &fp->body_height) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(body, "round") != NULL &&
        kind_bool(ctx, body, "round", &fp->body_round) != 0) {
        return -1;
    }
    kind_pop(ctx);

    switch (style_keys == DUAL_KEYS ? 0 : style_keys == QUAD_KEYS ? 1 : style_keys == GRID_KEYS ? 2 : 3) {
        case 0: return load_dual(ctx, json, fp);
        case 1: return load_quad(ctx, json, fp);
        case 2: return load_grid(ctx, json, fp);
        default: return load_explicit(ctx, json, fp);
    }
}

static int list_add(FootprintList *list, const PartFootprint *fp) {
    if (list->count == list->capacity) {
        size_t capacity = list->capacity == 0 ? 64 : list->capacity * 2;
        const PartFootprint **items = realloc(list->items, capacity * sizeof(*items));

        if (items == NULL) {
            return -1;
        }
        list->items = items;
        list->capacity = capacity;
    }
    list->items[list->count++] = fp;
    return 0;
}

int footprint_read_file(KindContext *ctx, const cJSON *root, const PartLibrary *library,
                        FootprintList *out) {
    const cJSON *item;
    const char *format;
    uint64_t version;
    size_t i = 0;

    if (kind_check_keys(ctx, root, FILE_KEYS, NULL) != 0 ||
        kind_string(ctx, root, "format", &format) != 0) {
        return -1;
    }
    if (strcmp(format, "schematical-footprints") != 0) {
        return kind_error(ctx, "\"format\" must be \"schematical-footprints\", not \"%s\"", format);
    }
    if (kind_uint(ctx, root, "version", UINT32_MAX, &version) != 0) {
        return -1;
    }
    if (version != 1) {
        return kind_error(ctx, "unsupported version %llu (expected 1)", (unsigned long long)version);
    }
    item = kind_member(ctx, root, "footprints", 1);
    if (item == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(item)) {
        return kind_error(ctx, "\"footprints\" must be an array");
    }
    for (item = item->child; item != NULL; item = item->next) {
        PartFootprint *fp = kind_alloc(ctx, sizeof(*fp));

        if (fp == NULL) {
            return -1;
        }
        kind_push(ctx, "footprints[%zu]", i);
        if (load_footprint(ctx, item, library, out, fp) != 0 || kind_failed(ctx)) {
            return -1;
        }
        kind_pop(ctx);
        if (list_add(out, fp) != 0) {
            return kind_error(ctx, "out of memory");
        }
        i++;
    }
    return 0;
}
