#include "library_load_int.h"
#include "logic_alloc.h"

static uint64_t hash_name(const char *text) {
    uint64_t hash = 0xcbf29ce484222325ull;

    for (; *text != '\0'; text++) {
        hash ^= (unsigned char)*text;
        hash *= 0x100000001b3ull;
    }
    return hash;
}

static size_t name_slot(const NameSet *set, const char *key) {
    size_t mask = set->capacity - 1;
    size_t slot = (size_t)hash_name(key) & mask;

    while (set->keys[slot] != NULL && strcmp(set->keys[slot], key) != 0) {
        slot = (slot + 1) & mask;
    }
    return slot;
}

int lib_name_set_contains(const NameSet *set, const char *key) {
    return set->capacity > 0 && set->keys[name_slot(set, key)] != NULL;
}

static const void *name_set_get(const NameSet *set, const char *key) {
    size_t slot;

    if (set->capacity == 0) {
        return NULL;
    }
    slot = name_slot(set, key);
    return set->keys[slot] != NULL ? set->values[slot] : NULL;
}

/* Returns -1 when out of memory. */
int lib_name_set_add(NameSet *set, const char *key, const void *value) {
    if ((set->count + 1) * 2 > set->capacity) {
        NameSet bigger = {0};
        size_t i;

        bigger.capacity = set->capacity == 0 ? 64 : set->capacity * 2;
        bigger.keys = logic_calloc(bigger.capacity, sizeof(*bigger.keys));
        bigger.values = logic_calloc(bigger.capacity, sizeof(*bigger.values));
        if (bigger.keys == NULL || bigger.values == NULL) {
            logic_free(bigger.keys);
            logic_free(bigger.values);
            return -1;
        }
        for (i = 0; i < set->capacity; i++) {
            if (set->keys[i] != NULL) {
                size_t slot = name_slot(&bigger, set->keys[i]);

                bigger.keys[slot] = set->keys[i];
                bigger.values[slot] = set->values[i];
                bigger.count++;
            }
        }
        logic_free(set->keys);
        logic_free(set->values);
        *set = bigger;
    }
    {
        size_t slot = name_slot(set, key);

        if (set->keys[slot] == NULL) {
            set->keys[slot] = key;
            set->values[slot] = value;
            set->count++;
        }
    }
    return 0;
}

/* Adds every entry of `from` to `to`. */
int lib_name_set_merge(NameSet *to, const NameSet *from) {
    size_t i;

    for (i = 0; i < from->capacity; i++) {
        if (from->keys[i] != NULL && lib_name_set_add(to, from->keys[i], from->values[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

void lib_name_set_free(NameSet *set) {
    logic_free(set->keys);
    logic_free(set->values);
    memset(set, 0, sizeof(*set));
}

PartLibrary *part_library_create(void) {
    logic_alloc_init();
    return logic_calloc(1, sizeof(PartLibrary));
}

void lib_free_chunks(ArenaChunk *chunk) {
    while (chunk != NULL) {
        ArenaChunk *next = chunk->next;

        logic_free(chunk);
        chunk = next;
    }
}

void part_library_destroy(PartLibrary *library) {
    if (library == NULL) {
        return;
    }
    logic_free(library->parts);
    logic_free(library->footprints);
    lib_name_set_free(&library->footprint_ids);
    logic_free(library->datasheets);
    lib_name_set_free(&library->datasheet_ids);
    lib_name_set_free(&library->names);
    lib_name_set_free(&library->devices);
    lib_name_set_free(&library->uuids);
    lib_free_chunks(library->arenas);
    logic_free(library);
}

size_t part_library_count(const PartLibrary *library) {
    return library != NULL ? library->count : 0;
}

const Part *part_library_at(const PartLibrary *library, size_t index) {
    return library != NULL && index < library->count ? library->parts[index] : NULL;
}

const Part *part_library_find(const PartLibrary *library, const char *name) {
    if (library == NULL || name == NULL) {
        return NULL;
    }
    return name_set_get(&library->names, name);
}

const Part *part_library_find_uuid(const PartLibrary *library, const char *uuid) {
    if (library == NULL || uuid == NULL) {
        return NULL;
    }
    return name_set_get(&library->uuids, uuid);
}

size_t part_library_footprint_count(const PartLibrary *library) {
    return library != NULL ? library->footprint_count : 0;
}

const PartFootprint *part_library_footprint_at(const PartLibrary *library, size_t index) {
    return library != NULL && index < library->footprint_count ? library->footprints[index] : NULL;
}

const PartFootprint *part_library_find_footprint(const PartLibrary *library, const char *id) {
    const PartFootprint *found;

    if (library == NULL || id == NULL) {
        return NULL;
    }
    found = name_set_get(&library->footprint_ids, id);
    if (found == NULL && library->staged_footprints != NULL) {
        size_t i;

        for (i = 0; i < library->staged_footprints->count; i++) {
            if (strcmp(library->staged_footprints->items[i]->id, id) == 0) {
                return library->staged_footprints->items[i];
            }
        }
    }
    return found;
}

size_t part_library_datasheet_count(const PartLibrary *library) {
    return library != NULL ? library->datasheet_count : 0;
}

const PartDatasheet *part_library_datasheet_at(const PartLibrary *library, size_t index) {
    if (library == NULL || index >= library->datasheet_count) {
        return NULL;
    }
    return library->datasheets[index];
}

const PartDatasheet *part_library_find_datasheet(const PartLibrary *library, const char *id) {
    const PartDatasheet *found;

    if (library == NULL || id == NULL) {
        return NULL;
    }
    found = name_set_get(&library->datasheet_ids, id);
    if (found == NULL && library->staged_datasheets != NULL) {
        size_t i;

        for (i = 0; i < library->staged_datasheets->count; i++) {
            if (strcmp(library->staged_datasheets->items[i]->id, id) == 0) {
                return library->staged_datasheets->items[i];
            }
        }
    }
    return found;
}

/* ---- Loading -------------------------------------------------------------- */

const char *const lib_FILE_KEYS[] = {"format", "version", "kind", "devices", NULL};
const char *const lib_DEVICE_KEYS[] = {
    "id", "function", "description", "drawer", "note", "simulated", "pinouts", "parts", NULL
};
const char *const lib_PART_KEYS[] = {
    "name", "package", "pinout", "flags", "footprint", "connectorFootprint",
    "footprintChoices", "fits", "manufacturer", "datasheet", NULL
};
const char *const lib_PIN_KEYS[] = {"number", "ball", "name", "role", "group", "side",
                                         "activeLow", NULL};

const char *const lib_ROLE_NAMES[] = {
    "input", "output", "inout", "power", "ground", "nc", "absent"
};
const char *const lib_FLAG_NAMES[] = {
    "obsolete", "military", "automotive", "enhanced", "ceramic"
};

/* ---- Single part files (format "schematical-part") ------------------------ */

static const char *const PART_FILE_KEYS[] = {
    "format", "version", "uuid", "originId", "kind", "footprints", "datasheets", "devices", NULL
};

/* 8-4-4-4-12 uppercase hexadecimal digits: Foundation's UUID.uuidString. */
static int valid_uuid(const char *text) {
    size_t i;

    if (strlen(text) != 36) {
        return 0;
    }
    for (i = 0; i < 36; i++) {
        char c = text[i];

        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') {
                return 0;
            }
        } else if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
            return 0;
        }
    }
    return 1;
}

/* Reads a part file's optional inline "datasheets" (arena): datasheets.json
   entries without "md", their ids new to the library and to the file. */
static int read_inline_datasheets(KindContext *ctx, const PartLibrary *library,
                                  const cJSON *list, DatasheetList *out) {
    NameSet ids = {0};
    const cJSON *item;
    size_t n = 0;
    int status = 0;

    if (!cJSON_IsArray(list)) {
        return kind_error(ctx, "\"datasheets\" must be an array");
    }
    out->items = kind_alloc(ctx, ((size_t)cJSON_GetArraySize(list) + 1) * sizeof(*out->items));
    if (out->items == NULL) {
        return -1;
    }
    for (item = list->child; item != NULL && status == 0; item = item->next) {
        PartDatasheet *sheet;

        kind_push(ctx, "datasheets[%zu]", n);
        status = lib_read_datasheet_entry(ctx, item, library, &ids, 0, &sheet);
        if (status == 0) {
            out->items[n++] = sheet;
            kind_pop(ctx);
        }
    }
    lib_name_set_free(&ids);
    out->count = status == 0 ? n : 0;
    return status;
}

/* Checks one part file and loads its footprints, its datasheets and its
   device into the context's arena (`file`, `footprints`, `datasheets`); the
   library itself is not changed. */
static int read_part_file(KindContext *ctx, PartLibrary *library, const cJSON *root,
                          FileParts *file, FootprintList *footprints,
                          DatasheetList *datasheets, const char **uuid) {
    const cJSON *devices;
    const cJSON *device;
    const cJSON *parts;
    const cJSON *inline_footprints;
    const cJSON *inline_datasheets;
    const char *format;
    const char *kind_name;
    const PartKindOps *ops;
    uint64_t version;
    size_t i;
    int status;

    if (!cJSON_IsObject(root)) {
        return kind_error(ctx, "expected an object");
    }
    if (kind_check_keys(ctx, root, PART_FILE_KEYS, NULL) != 0 ||
        kind_string(ctx, root, "format", &format) != 0) {
        return -1;
    }
    if (strcmp(format, "schematical-part") != 0) {
        return kind_error(ctx, "\"format\" must be \"schematical-part\", not \"%s\"", format);
    }
    if (kind_uint(ctx, root, "version", UINT32_MAX, &version) != 0) {
        return -1;
    }
    if (version != 1) {
        return kind_error(ctx, "unsupported version %llu (expected 1)",
                          (unsigned long long)version);
    }
    if (kind_string(ctx, root, "uuid", uuid) != 0) {
        return -1;
    }
    if (!valid_uuid(*uuid)) {
        return kind_error(ctx, "\"uuid\" must be an uppercase UUID "
                          "(XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX), not \"%s\"", *uuid);
    }
    if (lib_name_set_contains(&library->uuids, *uuid)) {
        return kind_error(ctx, "uuid \"%s\" is already loaded", *uuid);
    }
    /* "originId": the uuid the part's first version had (plan-53-A). Only its
       shape is checked; nothing reads it. */
    if (cJSON_GetObjectItemCaseSensitive(root, "originId") != NULL) {
        const char *origin = NULL;

        if (kind_string(ctx, root, "originId", &origin) != 0) {
            return -1;
        }
        if (!valid_uuid(origin)) {
            return kind_error(ctx, "\"originId\" must be an uppercase UUID "
                              "(XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX), not \"%s\"", origin);
        }
    }
    if (kind_string(ctx, root, "kind", &kind_name) != 0) {
        return -1;
    }
    ops = kind_find(kind_name);
    if (ops == NULL) {
        return kind_error(ctx, "unknown kind \"%s\"", kind_name);
    }
    inline_footprints = cJSON_GetObjectItemCaseSensitive(root, "footprints");
    if (inline_footprints != NULL &&
        (footprint_read_entries(ctx, inline_footprints, library, footprints) != 0 ||
         kind_failed(ctx))) {
        return -1;
    }
    inline_datasheets = cJSON_GetObjectItemCaseSensitive(root, "datasheets");
    if (inline_datasheets != NULL &&
        (read_inline_datasheets(ctx, library, inline_datasheets, datasheets) != 0 ||
         kind_failed(ctx))) {
        return -1;
    }
    devices = kind_member(ctx, root, "devices", 1);
    if (devices == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(devices) || cJSON_GetArraySize(devices) != 1) {
        return kind_error(ctx, "\"devices\" must be an array of exactly one device");
    }
    device = devices->child;
    kind_push(ctx, "devices[0]");
    parts = cJSON_IsObject(device) ? cJSON_GetObjectItemCaseSensitive(device, "parts") : NULL;
    if (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) != 1) {
        return kind_error(ctx, "\"parts\" must be an array of exactly one part");
    }
    /* The inline footprints and datasheets resolve like bundled ones while the
       part loads. */
    library->staged_footprints = footprints;
    library->staged_datasheets = datasheets;
    status = lib_load_device(ctx, library, file, ops, device);
    library->staged_footprints = NULL;
    library->staged_datasheets = NULL;
    if (status != 0 || kind_failed(ctx)) {
        return -1;
    }
    kind_pop(ctx);
    if (file->count != 1) {
        return kind_error(ctx, "\"devices\" must hold exactly one part");
    }
    /* A file carries no dead entries: the part names every inline datasheet. */
    for (i = 0; i < datasheets->count; i++) {
        if (file->parts[0]->datasheet != datasheets->items[i]) {
            return kind_error(ctx, "datasheet \"%s\" is not used", datasheets->items[i]->id);
        }
    }
    ((Part *)file->parts[0])->uuid = *uuid;
    return 0;
}

int load_part_file(PartLibrary *library, const char *json, const char *source, char **error) {
    KindContext ctx;
    FileParts file = {0};
    FootprintList footprints = {0};
    DatasheetList datasheets = {0};
    const char *uuid = NULL;
    cJSON *root;
    int result = -1;

    if (error != NULL) {
        *error = NULL;
    }
    if (library == NULL || json == NULL) {
        if (error != NULL) {
            *error = logic_alloc_disown(lib_copy_message("load_part_file: no library or text"));
        }
        return -1;
    }
    memset(&ctx, 0, sizeof ctx);
    ctx.source = source != NULL ? source : "part";
    file.single = 1;

    root = cJSON_ParseWithLength(json, strlen(json));
    if (root == NULL) {
        const char *where = cJSON_GetErrorPtr();

        kind_error(&ctx, "invalid JSON at byte %ld", where != NULL ? (long)(where - json) : -1L);
    } else if (read_part_file(&ctx, library, root, &file, &footprints, &datasheets, &uuid) == 0 &&
               !kind_failed(&ctx)) {
        size_t count = library->count;
        size_t footprint_count = library->footprint_count;
        size_t datasheet_count = library->datasheet_count;
        ArenaChunk *arenas = library->arenas;

        if (lib_commit_parts(&ctx, library, &file, &footprints, &datasheets, uuid) == 0) {
            library->last_part_file = 1;
            library->last_count = count;
            library->last_footprint_count = footprint_count;
            library->last_datasheet_count = datasheet_count;
            library->last_arenas = arenas;
            result = 0;
        }
    }

    library->staged_footprints = NULL;
    library->staged_datasheets = NULL;
    cJSON_Delete(root);
    logic_free(file.parts);
    logic_free(footprints.items);
    lib_name_set_free(&file.names);
    lib_name_set_free(&file.devices);
    lib_free_chunks(ctx.arena);
    lib_report(&ctx, result, error);
    return result;
}

static int id_listed(const char *id, const char *const *list, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        if (list[i] != NULL && strcmp(list[i], id) == 0) {
            return 1;
        }
    }
    return 0;
}

int part_library_retire_part(PartLibrary *library, const char *uuid,
                             const char *const *footprint_ids, size_t footprint_count,
                             const char *const *datasheet_ids, size_t datasheet_count) {
    const Part *target;
    NameSet names = {0};
    NameSet uuids = {0};
    NameSet ids = {0};
    NameSet sheet_ids = {0};
    size_t i, kept;
    int ok = 1;

    if (library == NULL || uuid == NULL) {
        return -1;
    }
    target = name_set_get(&library->uuids, uuid);
    if (target == NULL) {
        return -1;
    }
    /* The new tables are built on the side, so an out-of-memory failure
       leaves the library as it was. */
    for (i = 0; ok && i < library->count; i++) {
        const Part *part = library->parts[i];

        if (part == target) {
            continue;
        }
        ok = lib_name_set_add(&names, part->name, part) == 0 &&
             (part->uuid == NULL || lib_name_set_add(&uuids, part->uuid, part) == 0);
    }
    for (i = 0; ok && i < library->footprint_count; i++) {
        if (!id_listed(library->footprints[i]->id, footprint_ids, footprint_count)) {
            ok = lib_name_set_add(&ids, library->footprints[i]->id, library->footprints[i]) == 0;
        }
    }
    for (i = 0; ok && i < library->datasheet_count; i++) {
        if (!id_listed(library->datasheets[i]->id, datasheet_ids, datasheet_count)) {
            ok = lib_name_set_add(&sheet_ids, library->datasheets[i]->id, library->datasheets[i]) == 0;
        }
    }
    if (!ok) {
        lib_name_set_free(&names);
        lib_name_set_free(&uuids);
        lib_name_set_free(&ids);
        lib_name_set_free(&sheet_ids);
        return -1;
    }
    lib_name_set_free(&library->names);
    lib_name_set_free(&library->uuids);
    lib_name_set_free(&library->footprint_ids);
    lib_name_set_free(&library->datasheet_ids);
    library->names = names;
    library->uuids = uuids;
    library->footprint_ids = ids;
    library->datasheet_ids = sheet_ids;
    /* Compact the arrays in place; the removed entries' memory stays in the arenas. */
    for (i = 0, kept = 0; i < library->count; i++) {
        if (library->parts[i] != target) {
            library->parts[kept++] = library->parts[i];
        }
    }
    library->count = kept;
    for (i = 0, kept = 0; i < library->footprint_count; i++) {
        if (!id_listed(library->footprints[i]->id, footprint_ids, footprint_count)) {
            library->footprints[kept++] = library->footprints[i];
        }
    }
    library->footprint_count = kept;
    for (i = 0, kept = 0; i < library->datasheet_count; i++) {
        if (!id_listed(library->datasheets[i]->id, datasheet_ids, datasheet_count)) {
            library->datasheets[kept++] = library->datasheets[i];
        }
    }
    library->datasheet_count = kept;
    library->last_part_file = 0;
    return 0;
}

int part_library_unload_last_part_file(PartLibrary *library) {
    NameSet names = {0};
    NameSet uuids = {0};
    NameSet ids = {0};
    NameSet sheet_ids = {0};
    size_t i;
    int ok = 1;

    if (library == NULL || !library->last_part_file) {
        return -1;
    }
    /* Rebuild the name tables from what stays. */
    for (i = 0; ok && i < library->last_count; i++) {
        const Part *part = library->parts[i];

        ok = lib_name_set_add(&names, part->name, part) == 0 &&
             (part->uuid == NULL || lib_name_set_add(&uuids, part->uuid, part) == 0);
    }
    for (i = 0; ok && i < library->last_footprint_count; i++) {
        ok = lib_name_set_add(&ids, library->footprints[i]->id, library->footprints[i]) == 0;
    }
    for (i = 0; ok && i < library->last_datasheet_count; i++) {
        ok = lib_name_set_add(&sheet_ids, library->datasheets[i]->id, library->datasheets[i]) == 0;
    }
    if (!ok) {
        lib_name_set_free(&names);
        lib_name_set_free(&uuids);
        lib_name_set_free(&ids);
        lib_name_set_free(&sheet_ids);
        return -1;
    }
    lib_name_set_free(&library->names);
    lib_name_set_free(&library->uuids);
    lib_name_set_free(&library->footprint_ids);
    lib_name_set_free(&library->datasheet_ids);
    library->names = names;
    library->uuids = uuids;
    library->footprint_ids = ids;
    library->datasheet_ids = sheet_ids;
    library->count = library->last_count;
    library->footprint_count = library->last_footprint_count;
    library->datasheet_count = library->last_datasheet_count;
    /* The file's arena chunks were put in front of the old head. */
    while (library->arenas != library->last_arenas) {
        ArenaChunk *next = library->arenas->next;

        logic_free(library->arenas);
        library->arenas = next;
    }
    library->last_part_file = 0;
    return 0;
}
