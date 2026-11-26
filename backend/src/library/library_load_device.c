/* library/library_load_device.c: part of the model in library/library.c (split by plan-68-I/J). */
#include "library_load_int.h"
#include "logic_alloc.h"
int lib_load_device(KindContext *ctx, PartLibrary *library, FileParts *file,
                       const PartKindOps *ops, const cJSON *device_json) {
    Part common = {0};
    const char *id;
    const char *const *drawer;
    size_t drawer_count;
    const cJSON *pinouts_json;
    const cJSON *parts_json;
    const cJSON *item;
    Pinout *pinouts;
    size_t pinout_count = 0;
    void *device;
    char *device_key;
    size_t i;

    if (kind_check_keys(ctx, device_json, lib_DEVICE_KEYS, ops->device_keys) != 0 ||
        kind_string(ctx, device_json, "id", &id) != 0) {
        return -1;
    }
    {
        /* "devices[3]" becomes "devices[3] (74_00)" for later messages. */
        char segment[sizeof ctx->path[0]];

        snprintf(segment, sizeof segment, "%s",
                 ctx->depth - 1 < KIND_PATH_DEPTH ? ctx->path[ctx->depth - 1] : "");
        kind_pop(ctx);
        kind_push(ctx, "%s (%s)", segment, id);
    }
    if (kind_string(ctx, device_json, "function", &common.function) != 0 ||
        kind_string(ctx, device_json, "description", &common.description) != 0 ||
        kind_names(ctx, device_json, "drawer", &drawer, &drawer_count) != 0) {
        return -1;
    }
    if (drawer_count > 3) {
        return kind_error(ctx, "\"drawer\" has at most 3 levels");
    }
    item = cJSON_GetObjectItemCaseSensitive(device_json, "note");
    if (item != NULL && !cJSON_IsString(item)) {
        return kind_error(ctx, "\"note\" must be a string");
    }
    /* Optional; absent = the part has a model (plan-36-F). */
    common.simulated = 1;
    item = cJSON_GetObjectItemCaseSensitive(device_json, "simulated");
    if (item != NULL) {
        if (!cJSON_IsBool(item)) {
            return kind_error(ctx, "\"simulated\" must be true or false");
        }
        common.simulated = cJSON_IsTrue(item);
    }

    device_key = kind_alloc(ctx, strlen(ops->name) + strlen(id) + 2);
    if (device_key == NULL) {
        return -1;
    }
    sprintf(device_key, "%s/%s", ops->name, id);
    if ((!file->single && lib_name_set_contains(&library->devices, device_key)) ||
        lib_name_set_contains(&file->devices, device_key)) {
        return kind_error(ctx, "duplicate device id \"%s\"", id);
    }
    if (lib_name_set_add(&file->devices, device_key, device_key) != 0) {
        return kind_error(ctx, "out of memory");
    }

    common.kind = ops->kind;
    common.drawer = drawer;
    common.drawer_count = drawer_count;

    pinouts_json = kind_member(ctx, device_json, "pinouts", 1);
    if (pinouts_json == NULL) {
        return -1;
    }
    if (!cJSON_IsObject(pinouts_json) || pinouts_json->child == NULL) {
        return kind_error(ctx, "\"pinouts\" must be a non-empty object");
    }
    pinouts = kind_alloc(ctx, (size_t)cJSON_GetArraySize(pinouts_json) * sizeof(*pinouts));
    if (pinouts == NULL) {
        return -1;
    }
    for (item = pinouts_json->child; item != NULL; item = item->next) {
        const cJSON *other;

        for (other = pinouts_json->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                return kind_error(ctx, "duplicate pinout \"%s\"", item->string);
            }
        }
        kind_push(ctx, "pinouts.%s", item->string);
        pinouts[pinout_count].name = kind_strdup(ctx, item->string);
        if (pinouts[pinout_count].name == NULL ||
            lib_load_pinout(ctx, item, &pinouts[pinout_count]) != 0) {
            return -1;
        }
        kind_pop(ctx);
        pinout_count++;
    }

    device = ops->load_device(ctx, device_json);
    if (device == NULL || kind_failed(ctx)) {
        return kind_error(ctx, "invalid device");
    }
    for (i = 0; i < pinout_count; i++) {
        kind_push(ctx, "pinouts.%s", pinouts[i].name);
        pinouts[i].bound = ops->bind_pinout(ctx, device, pinouts[i].pins, pinouts[i].pin_count);
        if (pinouts[i].bound == NULL || kind_failed(ctx)) {
            return kind_error(ctx, "invalid pinout");
        }
        kind_pop(ctx);
    }

    parts_json = kind_member(ctx, device_json, "parts", 1);
    if (parts_json == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(parts_json)) {
        return kind_error(ctx, "\"parts\" must be an array");
    }
    i = 0;
    cJSON_ArrayForEach(item, parts_json) {
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "name");

        if (cJSON_IsString(name)) {
            kind_push(ctx, "parts[%zu] (%s)", i, name->valuestring);
        } else {
            kind_push(ctx, "parts[%zu]", i);
        }
        if (lib_load_part(ctx, library, file, ops, item, device, &common, pinouts, pinout_count) != 0) {
            return -1;
        }
        kind_pop(ctx);
        i++;
    }
    return 0;
}

static int load_file(KindContext *ctx, PartLibrary *library, FileParts *file, const cJSON *root) {
    const cJSON *item;
    const char *format;
    const char *kind_name;
    uint64_t version;
    const PartKindOps *ops;
    size_t i = 0;

    if (kind_check_keys(ctx, root, lib_FILE_KEYS, NULL) != 0 ||
        kind_string(ctx, root, "format", &format) != 0) {
        return -1;
    }
    if (strcmp(format, "schematical-parts") != 0) {
        return kind_error(ctx, "\"format\" must be \"schematical-parts\", not \"%s\"", format);
    }
    if (kind_uint(ctx, root, "version", UINT32_MAX, &version) != 0) {
        return -1;
    }
    if (version != 1) {
        return kind_error(ctx, "unsupported version %llu (expected 1)",
                          (unsigned long long)version);
    }
    if (kind_string(ctx, root, "kind", &kind_name) != 0) {
        return -1;
    }
    ops = kind_find(kind_name);
    if (ops == NULL) {
        return kind_error(ctx, "unknown kind \"%s\"", kind_name);
    }
    item = kind_member(ctx, root, "devices", 1);
    if (item == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(item)) {
        return kind_error(ctx, "\"devices\" must be an array");
    }
    for (item = item->child; item != NULL; item = item->next) {
        kind_push(ctx, "devices[%zu]", i);
        if (lib_load_device(ctx, library, file, ops, item) != 0) {
            return -1;
        }
        kind_pop(ctx);
        i++;
    }
    return 0;
}

char *lib_copy_message(const char *text) {
    char *copy = logic_malloc(strlen(text) + 1);

    if (copy != NULL) {
        strcpy(copy, text);
    }
    return copy;
}

/* Hands a loaded file's arena to the library. Every load ends here, so it
   also forgets the latest single part file (only load_part_file sets it
   again, after this). */
static void adopt_arena(PartLibrary *library, KindContext *ctx) {
    ArenaChunk *last = ctx->arena;

    library->last_part_file = 0;
    if (last != NULL) {
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = library->arenas;
        library->arenas = ctx->arena;
        ctx->arena = NULL;
    }
}

/* Sets *error (or frees the message) the way load_parts and load_footprints report. */
void lib_report(KindContext *ctx, int result, char **error) {
    if (result != 0 && error != NULL) {
        *error = logic_alloc_disown(ctx->error != NULL ? ctx->error : lib_copy_message("out of memory"));
    } else {
        logic_free(ctx->error);
    }
}

static int footprint_table_build(const PartLibrary *library, const FootprintList *extra,
                                 FootprintTable *out) {
    size_t needed = library->footprint_count + extra->count;
    size_t i;
    int ok;

    memset(out, 0, sizeof *out);
    out->all = logic_malloc((needed > 0 ? needed : 1) * sizeof(*out->all));
    ok = out->all != NULL && lib_name_set_merge(&out->ids, &library->footprint_ids) == 0;
    for (i = 0; ok && i < extra->count; i++) {
        ok = lib_name_set_add(&out->ids, extra->items[i]->id, extra->items[i]) == 0;
    }
    if (!ok) {
        logic_free(out->all);
        lib_name_set_free(&out->ids);
        memset(out, 0, sizeof *out);
        return -1;
    }
    if (library->footprint_count > 0) {
        memcpy(out->all, library->footprints, library->footprint_count * sizeof(*out->all));
    }
    if (extra->count > 0) {
        memcpy(out->all + library->footprint_count, extra->items, extra->count * sizeof(*out->all));
    }
    out->count = needed;
    return 0;
}

static void footprint_table_adopt(PartLibrary *library, FootprintTable *table) {
    logic_free(library->footprints);
    lib_name_set_free(&library->footprint_ids);
    library->footprints = table->all;
    library->footprint_count = table->count;
    library->footprint_ids = table->ids;
}

/*
 * Adds a loaded file's parts (and, for a single part file, its footprints and
 * its uuid) to the library, all or nothing: every grown table is built on the
 * side first, so an out-of-memory failure leaves the library as it was. Takes
 * the context's arena on success.
 */
int lib_commit_parts(KindContext *ctx, PartLibrary *library, const FileParts *file,
                        const FootprintList *footprints, const DatasheetList *datasheets,
                        const char *uuid) {
    size_t needed = library->count + file->count;
    size_t sheet_count = datasheets != NULL ? datasheets->count : 0;
    NameSet names = {0};
    NameSet devices = {0};
    NameSet uuids = {0};
    NameSet sheet_ids = {0};
    const PartDatasheet **sheets = NULL;
    FootprintTable table = {0};
    size_t i;
    int ok;

    ok = lib_name_set_merge(&names, &library->names) == 0 &&
         lib_name_set_merge(&names, &file->names) == 0 &&
         lib_name_set_merge(&devices, &library->devices) == 0 &&
         (file->single || lib_name_set_merge(&devices, &file->devices) == 0) &&
         lib_name_set_merge(&uuids, &library->uuids) == 0 &&
         (uuid == NULL || file->count != 1 || lib_name_set_add(&uuids, uuid, file->parts[0]) == 0) &&
         (footprints == NULL || footprints->count == 0 ||
          footprint_table_build(library, footprints, &table) == 0);
    if (ok && sheet_count > 0) {
        /* The grown datasheet table, built on the side like the others. */
        sheets = logic_malloc((library->datasheet_count + sheet_count) * sizeof(*sheets));
        ok = sheets != NULL && lib_name_set_merge(&sheet_ids, &library->datasheet_ids) == 0;
        for (i = 0; ok && i < sheet_count; i++) {
            ok = lib_name_set_add(&sheet_ids, datasheets->items[i]->id, datasheets->items[i]) == 0;
        }
    }
    /* Last, so a failure before it leaves even the parts array as it was. */
    if (ok && needed > library->capacity) {
        size_t capacity = library->capacity == 0 ? 1024 : library->capacity;
        const Part **parts;

        while (capacity < needed) {
            capacity *= 2;
        }
        parts = logic_realloc(library->parts, capacity * sizeof(*parts));
        ok = parts != NULL;
        if (ok) {
            library->parts = parts;
            library->capacity = capacity;
        }
    }
    if (!ok) {
        lib_name_set_free(&names);
        lib_name_set_free(&devices);
        lib_name_set_free(&uuids);
        lib_name_set_free(&sheet_ids);
        logic_free(sheets);
        logic_free(table.all);
        lib_name_set_free(&table.ids);
        return kind_error(ctx, "out of memory");
    }
    lib_name_set_free(&library->names);
    lib_name_set_free(&library->devices);
    lib_name_set_free(&library->uuids);
    library->names = names;
    library->devices = devices;
    library->uuids = uuids;
    if (table.all != NULL) {
        footprint_table_adopt(library, &table);
    }
    if (sheets != NULL) {
        if (library->datasheet_count > 0) {
            memcpy(sheets, library->datasheets, library->datasheet_count * sizeof(*sheets));
        }
        memcpy(sheets + library->datasheet_count, datasheets->items,
               sheet_count * sizeof(*sheets));
        logic_free(library->datasheets);
        lib_name_set_free(&library->datasheet_ids);
        library->datasheets = sheets;
        library->datasheet_count += sheet_count;
        library->datasheet_ids = sheet_ids;
    }
    memcpy(library->parts + library->count, file->parts, file->count * sizeof(*file->parts));
    library->count += file->count;
    adopt_arena(library, ctx);
    return 0;
}

int load_footprints(PartLibrary *library, const char *json, const char *source, char **error) {
    KindContext ctx;
    FootprintList file = {0};
    cJSON *root;
    int result = -1;

    if (error != NULL) {
        *error = NULL;
    }
    if (library == NULL || json == NULL) {
        if (error != NULL) {
            *error = logic_alloc_disown(lib_copy_message("load_footprints: no library or text"));
        }
        return -1;
    }
    memset(&ctx, 0, sizeof ctx);
    ctx.source = source != NULL ? source : "footprints";

    root = cJSON_ParseWithLength(json, strlen(json));
    if (root == NULL) {
        const char *where = cJSON_GetErrorPtr();

        kind_error(&ctx, "invalid JSON at byte %ld", where != NULL ? (long)(where - json) : -1L);
    } else if (footprint_read_file(&ctx, root, library, &file) == 0 && !kind_failed(&ctx)) {
        FootprintTable table;

        if (footprint_table_build(library, &file, &table) != 0) {
            kind_error(&ctx, "out of memory");
        } else {
            footprint_table_adopt(library, &table);
            adopt_arena(library, &ctx);
            result = 0;
        }
    }

    cJSON_Delete(root);
    logic_free(file.items);
    lib_free_chunks(ctx.arena);
    lib_report(&ctx, result, error);
    return result;
}

static const char *const DATASHEET_FILE_KEYS[] = {"format", "version", "datasheets", NULL};
static const char *const DATASHEET_KEYS[] = {"id", "url", "md", NULL};

/* Reads and checks one datasheet entry (the caller has pushed its place):
   its keys, a non-empty id new to the library and to `ids`, an https URL, and
   the optional "md" path, which an inline entry of a single part file
   (`allow_md` 0) may not have. Adds the entry to `ids`. */
int lib_read_datasheet_entry(KindContext *ctx, const cJSON *item, const PartLibrary *library,
                                NameSet *ids, int allow_md, PartDatasheet **out) {
    PartDatasheet *sheet = kind_alloc(ctx, sizeof(*sheet));

    if (sheet == NULL || kind_check_keys(ctx, item, DATASHEET_KEYS, NULL) != 0 ||
        kind_string(ctx, item, "id", &sheet->id) != 0 ||
        kind_string(ctx, item, "url", &sheet->url) != 0) {
        return -1;
    }
    if (sheet->id[0] == '\0') {
        return kind_error(ctx, "empty datasheet id");
    }
    if (strncmp(sheet->url, "https://", 8) != 0 || sheet->url[8] == '\0') {
        return kind_error(ctx, "datasheet \"%s\": \"url\" must start with https://",
                          sheet->id);
    }
    sheet->md = NULL;
    if (cJSON_GetObjectItemCaseSensitive(item, "md") != NULL) {
        size_t len;

        if (!allow_md) {
            return kind_error(ctx, "an inline datasheet has no \"md\"");
        }
        if (kind_string(ctx, item, "md", &sheet->md) != 0) {
            return -1;
        }
        /* A relative path to a .md file: "<group>/<id>.md". */
        len = strlen(sheet->md);
        if (len < 4 || strcmp(sheet->md + len - 3, ".md") != 0 || sheet->md[0] == '/' ||
            strstr(sheet->md, "..") != NULL) {
            return kind_error(ctx, "datasheet \"%s\": \"md\" must be a relative path "
                              "to a .md file", sheet->id);
        }
    }
    if (lib_name_set_contains(&library->datasheet_ids, sheet->id) ||
        lib_name_set_contains(ids, sheet->id)) {
        return kind_error(ctx, "duplicate datasheet id \"%s\"", sheet->id);
    }
    if (lib_name_set_add(ids, sheet->id, sheet) != 0) {
        return kind_error(ctx, "out of memory");
    }
    *out = sheet;
    return 0;
}

/* Reads every entry of a datasheets file into `items` (arena) and `ids`. */
static int read_datasheets(KindContext *ctx, const cJSON *root, const PartLibrary *library,
                           const PartDatasheet ***items, size_t *count, NameSet *ids) {
    const char *format;
    uint64_t version;
    const cJSON *list;
    const cJSON *item;
    size_t n = 0;

    if (kind_check_keys(ctx, root, DATASHEET_FILE_KEYS, NULL) != 0 ||
        kind_string(ctx, root, "format", &format) != 0) {
        return -1;
    }
    if (strcmp(format, "schematical-datasheets") != 0) {
        return kind_error(ctx, "\"format\" must be \"schematical-datasheets\", not \"%s\"",
                          format);
    }
    if (kind_uint(ctx, root, "version", UINT32_MAX, &version) != 0) {
        return -1;
    }
    if (version != 1) {
        return kind_error(ctx, "unsupported version %llu (expected 1)",
                          (unsigned long long)version);
    }
    list = kind_member(ctx, root, "datasheets", 1);
    if (list == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(list)) {
        return kind_error(ctx, "\"datasheets\" must be an array");
    }
    *items = kind_alloc(ctx, ((size_t)cJSON_GetArraySize(list) + 1) * sizeof(**items));
    if (*items == NULL) {
        return -1;
    }
    for (item = list->child; item != NULL; item = item->next) {
        PartDatasheet *sheet;

        kind_push(ctx, "datasheets[%zu]", n);
        if (lib_read_datasheet_entry(ctx, item, library, ids, 1, &sheet) != 0) {
            return -1;
        }
        (*items)[n++] = sheet;
        kind_pop(ctx);
    }
    *count = n;
    return 0;
}

int load_datasheets(PartLibrary *library, const char *json, const char *source, char **error) {
    KindContext ctx;
    const PartDatasheet **items = NULL;
    size_t count = 0;
    NameSet ids = {0};
    cJSON *root;
    int result = -1;

    if (error != NULL) {
        *error = NULL;
    }
    if (library == NULL || json == NULL) {
        if (error != NULL) {
            *error = logic_alloc_disown(lib_copy_message("load_datasheets: no library or text"));
        }
        return -1;
    }
    memset(&ctx, 0, sizeof ctx);
    ctx.source = source != NULL ? source : "datasheets";

    root = cJSON_ParseWithLength(json, strlen(json));
    if (root == NULL) {
        const char *where = cJSON_GetErrorPtr();

        kind_error(&ctx, "invalid JSON at byte %ld", where != NULL ? (long)(where - json) : -1L);
    } else if (read_datasheets(&ctx, root, library, &items, &count, &ids) == 0 &&
               !kind_failed(&ctx)) {
        size_t needed = library->datasheet_count + count;
        const PartDatasheet **all = logic_malloc((needed > 0 ? needed : 1) * sizeof(*all));
        NameSet merged = {0};

        if (all == NULL || lib_name_set_merge(&merged, &library->datasheet_ids) != 0 ||
            lib_name_set_merge(&merged, &ids) != 0) {
            logic_free(all);
            lib_name_set_free(&merged);
            kind_error(&ctx, "out of memory");
        } else {
            if (library->datasheet_count > 0) {
                memcpy(all, library->datasheets, library->datasheet_count * sizeof(*all));
            }
            if (count > 0) {
                memcpy(all + library->datasheet_count, items, count * sizeof(*all));
            }
            logic_free(library->datasheets);
            lib_name_set_free(&library->datasheet_ids);
            library->datasheets = all;
            library->datasheet_count = needed;
            library->datasheet_ids = merged;
            adopt_arena(library, &ctx);
            result = 0;
        }
    }

    cJSON_Delete(root);
    lib_name_set_free(&ids);
    lib_free_chunks(ctx.arena);
    lib_report(&ctx, result, error);
    return result;
}

int load_parts(PartLibrary *library, const char *json, const char *source, char **error) {
    KindContext ctx;
    FileParts file = {0};
    cJSON *root;
    int result = -1;

    if (error != NULL) {
        *error = NULL;
    }
    if (library == NULL || json == NULL) {
        if (error != NULL) {
            *error = logic_alloc_disown(lib_copy_message("load_parts: no library or text"));
        }
        return -1;
    }
    memset(&ctx, 0, sizeof ctx);
    ctx.source = source != NULL ? source : "parts";

    root = cJSON_ParseWithLength(json, strlen(json));
    if (root == NULL) {
        const char *where = cJSON_GetErrorPtr();

        kind_error(&ctx, "invalid JSON at byte %ld",
                   where != NULL ? (long)(where - json) : -1L);
    } else if (load_file(&ctx, library, &file, root) == 0 && !kind_failed(&ctx) &&
               lib_commit_parts(&ctx, library, &file, NULL, NULL, NULL) == 0) {
        result = 0;
    }

    cJSON_Delete(root);
    logic_free(file.parts);
    lib_name_set_free(&file.names);
    lib_name_set_free(&file.devices);
    lib_free_chunks(ctx.arena);
    lib_report(&ctx, result, error);
    return result;
}

