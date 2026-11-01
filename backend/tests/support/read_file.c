#include "read_file.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    char *text;
    long length;

    if (file == NULL) {
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    text = malloc((size_t)length + 1);
    if (text != NULL && fread(text, 1, (size_t)length, file) != (size_t)length) {
        free(text);
        text = NULL;
    }
    if (text != NULL) {
        text[length] = '\0';
    }
    fclose(file);
    return text;
}

#define FOOTPRINTS_FILE "footprints.json"

static int by_name(const void *a, const void *b) {
    return strcmp(*(char *const *)a, *(char *const *)b);
}

int load_parts_dir(PartLibrary *library, const char *dir) {
    DIR *handle = opendir(dir);
    struct dirent *entry;
    char *names[64];
    size_t count = 0;
    size_t i;
    int result = 0;

    if (handle == NULL) {
        fprintf(stderr, "cannot open %s\n", dir);
        return -1;
    }
    while ((entry = readdir(handle)) != NULL && count < 64) {
        size_t length = strlen(entry->d_name);

        if (length > 5 && strcmp(entry->d_name + length - 5, ".json") == 0) {
            names[count] = malloc(length + 1);
            strcpy(names[count++], entry->d_name);
        }
    }
    closedir(handle);
    qsort(names, count, sizeof(names[0]), by_name);

    /* Footprints first: every part file may name them. */
    {
        char path[4096];
        char *text;
        char *error = NULL;

        snprintf(path, sizeof path, "%s/%s", dir, FOOTPRINTS_FILE);
        text = read_file(path);
        if (text == NULL) {
            fprintf(stderr, "cannot read %s\n", path);
            result = -1;
        } else if (load_footprints(library, text, FOOTPRINTS_FILE, &error) != 0) {
            fprintf(stderr, "%s\n", error);
            free(error);
            result = -1;
        }
        free(text);
    }

    for (i = 0; i < count; i++) {
        char path[4096];
        char *text;
        char *error = NULL;

        if (strcmp(names[i], FOOTPRINTS_FILE) == 0) {
            free(names[i]);
            continue;
        }
        snprintf(path, sizeof path, "%s/%s", dir, names[i]);
        text = read_file(path);
        if (text == NULL) {
            fprintf(stderr, "cannot read %s\n", path);
            result = -1;
        } else if (load_parts(library, text, names[i], &error) != 0) {
            fprintf(stderr, "%s\n", error);
            free(error);
            result = -1;
        }
        free(text);
        free(names[i]);
    }
    return result;
}
