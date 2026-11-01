#include "lights.h"

void lights_set(Light *lights, size_t index, int on, SimTime now) {
    Light *light = &lights[index];

    if (light->on) {
        light->on_total += now - light->changed_at;
    }
    light->changed_at = now;
    if (light->on && !on) {
        light->last_on = now;
    }
    if (on) {
        light->ever_on = 1;
    }
    light->on = (unsigned char)(on ? 1 : 0);
}

size_t lights_read(const Light *lights, size_t count, SimTime since,
                   unsigned char *out, size_t cap) {
    size_t i;

    for (i = 0; i < count && i < cap; i++) {
        const Light *light = &lights[i];

        out[i] = (unsigned char)(light->on || (light->ever_on && light->last_on >= since));
    }
    return count;
}

size_t lights_time(const Light *lights, size_t count, SimTime now,
                   uint64_t *out, size_t cap) {
    size_t i;

    for (i = 0; i < count && i < cap; i++) {
        const Light *light = &lights[i];

        out[i] = light->on_total + (light->on ? now - light->changed_at : 0);
    }
    return count;
}
