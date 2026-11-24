/* Barrel power connectors, USB connectors and card slots. They are passive terminals; a
 * three-terminal barrel jack additionally has a normally-closed sleeve
 * contact that opens when a plug is inserted. A USB connector ("usb": "A",
 * "B" or "C") has no centre pin and no contacts; its ratings are optional.
 * A card slot ("card": what it takes, "cardPitch": the contact pitch in mm;
 * memory / SIM sockets, memory-module sockets, card-edge sockets) has no
 * centre pin, contacts, usb or ratings (plan-54-L). */

#include <math.h>
#include <string.h>

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"centerPin", "ratedVolts", "ratedAmps", "contacts", "usb", "card", "cardPitch",
                                          "testPoint", NULL};

typedef struct {
    PartConnector info;
    const char *(*contacts)[2];
} ConnectorDevice;

typedef struct { size_t count; size_t (*pins)[2]; } ConnectorPins;

static int usb_load(KindContext *ctx, const cJSON *json, ConnectorDevice *device) {
    const char *type;
    double amps;

    if (kind_string(ctx, json, "usb", &type) != 0) return -1;
    if (strcmp(type, "A") != 0 && strcmp(type, "B") != 0 && strcmp(type, "C") != 0) {
        return kind_error(ctx, "\"usb\" must be \"A\", \"B\" or \"C\"");
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "centerPin") != NULL) {
        return kind_error(ctx, "a USB connector has no \"centerPin\"");
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "contacts") != NULL) {
        return kind_error(ctx, "a USB connector has no \"contacts\"");
    }
    device->info.usb = type[0];
    if (cJSON_GetObjectItemCaseSensitive(json, "ratedVolts") != NULL) {
        if (kind_volts(ctx, json, "ratedVolts", 1000, &device->info.rated_mv) != 0) return -1;
        if (device->info.rated_mv == 0) return kind_error(ctx, "ratings must be above 0");
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "ratedAmps") != NULL) {
        if (kind_number(ctx, json, "ratedAmps", &amps) != 0) return -1;
        if (amps <= 0 || amps > 100) return kind_error(ctx, "\"ratedAmps\" must be above 0 and at most 100");
        device->info.rated_ma = (uint32_t)llround(amps * 1000);
    }
    return 0;
}

static int card_load(KindContext *ctx, const cJSON *json, ConnectorDevice *device) {
    static const char *const FORBIDDEN[] = {"usb", "centerPin", "contacts", "ratedVolts", "ratedAmps", NULL};
    const cJSON *card = cJSON_GetObjectItemCaseSensitive(json, "card");
    size_t i;
    double pitch;

    for (i = 0; FORBIDDEN[i] != NULL; i++) {
        if (cJSON_GetObjectItemCaseSensitive(json, FORBIDDEN[i]) != NULL) {
            return kind_error(ctx, "a card slot has no \"%s\"", FORBIDDEN[i]);
        }
    }
    if (!cJSON_IsString(card) || card->valuestring[0] == '\0' || strlen(card->valuestring) > 40) {
        return kind_error(ctx, "\"card\" must be a string of 1 to 40 characters");
    }
    device->info.card = kind_strdup(ctx, card->valuestring);
    if (device->info.card == NULL) return -1;
    if (cJSON_GetObjectItemCaseSensitive(json, "cardPitch") != NULL) {
        if (kind_number(ctx, json, "cardPitch", &pitch) != 0) return -1;
        if (!(pitch > 0) || pitch > 20) return kind_error(ctx, "\"cardPitch\" must be above 0 and at most 20 mm");
        device->info.card_pitch_um = (uint32_t)llround(pitch * 1000);
    }
    return 0;
}

/* A test point (plan-57): one passive pin, no ratings, contacts or other mode's keys. */
static int test_point_load(KindContext *ctx, const cJSON *json, ConnectorDevice *device) {
    static const char *const FORBIDDEN[] = {"centerPin", "ratedVolts", "ratedAmps", "contacts", "usb", "card",
                                            "cardPitch", NULL};
    size_t i;

    if (!cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(json, "testPoint"))) {
        return kind_error(ctx, "\"testPoint\" must be true");
    }
    for (i = 0; FORBIDDEN[i] != NULL; i++) {
        if (cJSON_GetObjectItemCaseSensitive(json, FORBIDDEN[i]) != NULL) {
            return kind_error(ctx, "a test point has no \"%s\"", FORBIDDEN[i]);
        }
    }
    device->info.test_point = 1;
    return 0;
}

static void *connector_load_device(KindContext *ctx, const cJSON *json) {
    ConnectorDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *contacts;
    const cJSON *item;
    double center_pin, amps;
    size_t count = 0;

    if (device == NULL) return NULL;
    if (cJSON_GetObjectItemCaseSensitive(json, "testPoint") != NULL) {
        return test_point_load(ctx, json, device) == 0 ? device : NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "card") != NULL) {
        return card_load(ctx, json, device) == 0 ? device : NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "cardPitch") != NULL) {
        kind_error(ctx, "\"cardPitch\" is only for card slots");
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "usb") != NULL) {
        return usb_load(ctx, json, device) == 0 ? device : NULL;
    }
    if (kind_number(ctx, json, "centerPin", &center_pin) != 0 ||
        kind_volts(ctx, json, "ratedVolts", 1000, &device->info.rated_mv) != 0 ||
        kind_number(ctx, json, "ratedAmps", &amps) != 0) return NULL;
    if (center_pin <= 0 || center_pin > 10) {
        kind_error(ctx, "\"centerPin\" must be above 0 and at most 10 mm");
        return NULL;
    }
    if (amps <= 0 || amps > 100) {
        kind_error(ctx, "\"ratedAmps\" must be above 0 and at most 100");
        return NULL;
    }
    device->info.center_diameter_um = (uint32_t)llround(center_pin * 1000);
    device->info.rated_ma = (uint32_t)llround(amps * 1000);
    if (device->info.rated_mv == 0 || device->info.rated_ma == 0) {
        kind_error(ctx, "ratings must be above 0");
        return NULL;
    }
    contacts = kind_member(ctx, json, "contacts", 0);
    if (contacts == NULL) return device;
    if (!cJSON_IsArray(contacts) || cJSON_GetArraySize(contacts) == 0) {
        kind_error(ctx, "\"contacts\" must be a non-empty array of [pin, pin]");
        return NULL;
    }
    device->contacts = kind_alloc(ctx, (size_t)cJSON_GetArraySize(contacts) * sizeof(*device->contacts));
    if (device->contacts == NULL) return NULL;
    cJSON_ArrayForEach(item, contacts) {
        const cJSON *a = cJSON_GetArrayItem(item, 0);
        const cJSON *b = cJSON_GetArrayItem(item, 1);
        kind_push(ctx, "contacts[%zu]", count);
        if (!cJSON_IsArray(item) || cJSON_GetArraySize(item) != 2 || !cJSON_IsString(a) ||
            !cJSON_IsString(b) || strcmp(a->valuestring, b->valuestring) == 0) {
            kind_error(ctx, "a contact is [different pin name, different pin name]");
            return NULL;
        }
        device->contacts[count][0] = kind_strdup(ctx, a->valuestring);
        device->contacts[count][1] = kind_strdup(ctx, b->valuestring);
        kind_pop(ctx);
        count++;
    }
    device->info.contacts = (unsigned)count;
    return device;
}

static void *connector_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                   size_t pin_count) {
    const ConnectorDevice *device = raw;
    ConnectorPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t i;

    if (bound != NULL && device->info.test_point) {
        if (pin_count != 1) {
            kind_error(ctx, "a test point has exactly one pin");
            return NULL;
        }
    } else if (bound == NULL || pin_count < 2) {
        if (bound != NULL) kind_error(ctx, "a connector has at least two pins");
        return NULL;
    }
    bound->count = device->info.contacts;
    if (bound->count == 0) return bound;
    bound->pins = kind_alloc(ctx, bound->count * sizeof(*bound->pins));
    if (bound->pins == NULL) return NULL;
    for (i = 0; i < bound->count; i++) {
        kind_push(ctx, "contacts[%zu]", i);
        if (kind_pin(ctx, pins, pin_count, device->contacts[i][0], &bound->pins[i][0]) != 0 ||
            kind_pin(ctx, pins, pin_count, device->contacts[i][1], &bound->pins[i][1]) != 0) return NULL;
        kind_pop(ctx);
    }
    return bound;
}

int connector_contact_pins(const Part *part, unsigned position, size_t *a, size_t *b) {
    const ConnectorPins *pins;
    if (part == NULL || part->kind != PART_CONNECTOR || position < 1) return 0;
    pins = part_internal(part)->bound;
    if (position > pins->count) return 0;
    *a = pins->pins[position - 1][0]; *b = pins->pins[position - 1][1];
    return 1;
}

static int connector_load_part(KindContext *ctx, const void *device, const void *bound,
                               const cJSON *json, Part *out) {
    (void)ctx; (void)bound; (void)json;
    out->connector = &((const ConnectorDevice *)device)->info;
    return 0;
}

static size_t connector_pin_count(const void *config) { return ((const Part *)config)->pin_count; }
static void *connector_create(const void *config) { return (void *)part_internal(config)->bound; }

static int connector_set(Sim *sim, SimInstance *instance, void *state, size_t position, int on) {
    const ConnectorPins *pins = state;
    if (position < 1 || position > pins->count) return 0;
    sim_set_contact(sim, instance, pins->pins[position - 1][0], pins->pins[position - 1][1], on);
    return 1;
}

static const SimComponentType CONNECTOR_MODEL = {
    "connector", connector_pin_count, connector_create, NULL, NULL, NULL, NULL,
    NULL, connector_set, NULL, NULL
};

const PartKindOps CONNECTOR_KIND = {
    PART_CONNECTOR, "connector", &CONNECTOR_MODEL, DEVICE_KEYS, NULL,
    connector_load_device, connector_bind_pinout, connector_load_part
};
