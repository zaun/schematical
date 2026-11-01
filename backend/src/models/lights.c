#include "lights.h"

void lights_set(Light *lights, size_t index, int on, SimTime now) {
    Light *light = &lights[index];

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
