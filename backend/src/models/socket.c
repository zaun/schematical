/*
 * socket: an IC socket (plan-51-A), the part a DIP or PLCC chip plugs into.
 * Passive terminals, the header.c shape: each pin is just its net, so the
 * model drives nothing and has no hooks, and a socket in a circuit joins
 * nothing. The device names the family ("DIP" or "PLCC") and a DIP's row
 * spacing; a part names its mounting and contact finish. Which chip
 * footprints a socket holds is the common part key "fits" (library.c).
 */

#include <math.h>
#include <string.h>

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"family", "rowSpacing", NULL};
static const char *const PART_KEYS[] = {"mounting", "finish", NULL};

typedef struct {
    const char *family;     /* "DIP" or "PLCC" */
    uint32_t row_um;        /* DIP: 5080, 7620, 10160 or 15240; PLCC: 0 */
} SocketDevice;

static void *socket_load_device(KindContext *ctx, const cJSON *json) {
    SocketDevice *device = kind_alloc(ctx, sizeof(*device));
    const char *family;
    const cJSON *spacing;

    if (device == NULL || kind_string(ctx, json, "family", &family) != 0) {
        return NULL;
    }
    spacing = kind_member(ctx, json, "rowSpacing", 0);
    if (strcmp(family, "DIP") == 0) {
        double mm;
        long um;

        if (kind_number(ctx, json, "rowSpacing", &mm) != 0) {
            return NULL;
        }
        um = lround(mm * 1000.0);
        if (um != 5080 && um != 7620 && um != 10160 && um != 15240) {
            kind_error(ctx, "\"rowSpacing\" must be 5.08, 7.62, 10.16 or 15.24");
            return NULL;
        }
        device->row_um = (uint32_t)um;
    } else if (strcmp(family, "PLCC") == 0) {
        if (spacing != NULL) {
            kind_error(ctx, "a PLCC socket has no \"rowSpacing\"");
            return NULL;
        }
        device->row_um = 0;
    } else {
        kind_error(ctx, "unknown socket family \"%s\"", family);
        return NULL;
    }
    device->family = family;
    return device;
}

static void *socket_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                size_t pin_count) {
    const SocketDevice *device = raw;
    size_t i;

    for (i = 0; i < pin_count; i++) {
        if (pins[i].role != PIN_INOUT) {
            kind_error(ctx, "socket pin \"%s\" must be inout", pins[i].designator);
            return NULL;
        }
    }
    if (strcmp(device->family, "DIP") == 0) {
        if (pin_count < 4 || pin_count > 64 || pin_count % 2 != 0) {
            kind_error(ctx, "a DIP socket needs an even pin count of 4 to 64");
            return NULL;
        }
    } else if (pin_count != 20 && pin_count != 28 && pin_count != 32 && pin_count != 44 &&
               pin_count != 52 && pin_count != 68 && pin_count != 84) {
        kind_error(ctx, "a PLCC socket needs 20, 28, 32, 44, 52, 68 or 84 pins");
        return NULL;
    }
    return (void *)device;
}

static int socket_load_part(KindContext *ctx, const void *raw, const void *bound,
                            const cJSON *json, Part *out) {
    const SocketDevice *device = raw;
    PartSocket *info = kind_alloc(ctx, sizeof(*info));
    const char *mounting;
    const cJSON *finish;

    (void)bound;
    if (info == NULL || kind_string(ctx, json, "mounting", &mounting) != 0) {
        return -1;
    }
    if (strcmp(mounting, "through-hole") != 0 && strcmp(mounting, "surface-mount") != 0) {
        return kind_error(ctx, "unknown socket mounting \"%s\"", mounting);
    }
    finish = kind_member(ctx, json, "finish", 0);
    if (finish != NULL) {
        if (!cJSON_IsString(finish) || finish->valuestring[0] == '\0') {
            return kind_error(ctx, "\"finish\" must be a non-empty string");
        }
        info->finish = kind_strdup(ctx, finish->valuestring);
        if (info->finish == NULL) {
            return -1;
        }
    }
    info->family = device->family;
    info->row_um = device->row_um;
    info->mounting = mounting;
    out->socket = info;
    return 0;
}

static size_t socket_pin_count(const void *config) { return ((const Part *)config)->pin_count; }

/* No state: any non-NULL pointer tells the engine creation worked. */
static void *socket_create(const void *config) { return (void *)((const Part *)config)->socket; }

static const SimComponentType SOCKET_MODEL = {
    "socket", socket_pin_count, socket_create, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL
};

const PartKindOps SOCKET_KIND = {
    PART_SOCKET, "socket", &SOCKET_MODEL, DEVICE_KEYS, PART_KEYS,
    socket_load_device, socket_bind_pinout, socket_load_part
};
