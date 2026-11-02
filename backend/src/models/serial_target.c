/*
 * SPI target and I2C target front ends (serial_target.h; plan-12-I section
 * 3.1, landed with plan-12-J).
 */

#include "serial_target.h"

#include <string.h>

#include "serial.h"

static LogicValue input_level(const Sim *sim, const SimInstance *instance, size_t pin) {
    return serial_level(sim, instance, pin, LOGIC_X);
}

/* ---- SPI target ---------------------------------------------------------------- */

void serial_spi_init(SerialSpi *spi, const SerialSpiPins *pins) {
    memset(spi, 0, sizeof *spi);
    spi->pins = pins;
    spi->sclk = LOGIC_X;
    spi->cs = LOGIC_1;
    spi->out_bit = -1;
    spi->miso = LOGIC_Z;
    spi->enabled = 1;
}

static void spi_drive(SerialSpi *spi, Sim *sim, SimInstance *instance, LogicValue value,
                      SimTime delay) {
    if (spi->miso == value || spi->pins->miso == SERIAL_NO_PIN) {
        spi->miso = value;
        return;
    }
    spi->miso = value;
    sim_schedule_inertial_drive(sim, instance, spi->pins->miso, value, DRIVE_STRONG, delay);
}

static LogicValue spi_out_level(const SerialSpi *spi) {
    int b = spi->out_bit;

    if (!spi->active || b < 0 || !((spi->out.drive >> b) & 1u)) {
        return LOGIC_Z;
    }
    if (spi->x || !((spi->out.known >> b) & 1u)) {
        return LOGIC_X;
    }
    return (spi->out.data >> b) & 1u ? LOGIC_1 : LOGIC_0;
}

static void spi_fetch(SerialSpi *spi, const SerialSpiEvents *events, unsigned index) {
    SerialSpiByte none = {0, 0, 0};

    spi->out = events->next != NULL ? events->next(events->context, index) : none;
    spi->out_bit = 7;
}

static void spi_begin(SerialSpi *spi, Sim *sim, SimInstance *instance,
                      const SerialSpiEvents *events, int x) {
    spi->active = 1;
    spi->x = x;
    spi->bits = 0;
    spi->in = 0;
    spi->in_known = 0;
    spi->out_bit = -1;
    if (events->begin != NULL) {
        events->begin(events->context);
    }
    if ((spi->pins->mode & 1u) == 0) {
        spi_fetch(spi, events, 0);
        spi_drive(spi, sim, instance, spi_out_level(spi), spi->t_valid);
    }
}

static void spi_sample(SerialSpi *spi, Sim *sim, SimInstance *instance,
                       const SerialSpiEvents *events) {
    LogicValue v = spi->x ? LOGIC_X : input_level(sim, instance, spi->pins->mosi);
    unsigned index;
    unsigned n;

    spi->in = (uint8_t)((spi->in << 1) | (v == LOGIC_1 ? 1u : 0u));
    spi->in_known = (uint8_t)((spi->in_known << 1) | (v == LOGIC_X ? 0u : 1u));
    spi->bits++;
    index = (spi->bits - 1) / 8;
    n = (spi->bits - 1) % 8 + 1;
    if (events->bit != NULL) {
        events->bit(events->context, index, n, spi->in, spi->in_known, &spi->out);
    }
    if (n == 8) {
        uint8_t data = spi->in;
        uint8_t known = spi->in_known;

        spi->in = 0;
        spi->in_known = 0;
        if (events->received != NULL) {
            events->received(events->context, index, data, known);
        }
    }
}

static void spi_shift(SerialSpi *spi, Sim *sim, SimInstance *instance,
                      const SerialSpiEvents *events) {
    if (events->shift != NULL) {
        events->shift(events->context, spi->bits);
    }
    if (!spi->active) {
        return;
    }
    if ((spi->pins->mode & 1u) == 0 && spi->bits == 0) {
        return;   /* a trailing edge before any bit: nothing to shift */
    }
    if (spi->bits % 8 == 0) {
        spi_fetch(spi, events, spi->bits / 8);
    } else {
        spi->out_bit = 7 - (int)(spi->bits % 8);
    }
    spi_drive(spi, sim, instance, spi_out_level(spi), spi->t_out);
}

void serial_spi_start(SerialSpi *spi, Sim *sim, SimInstance *instance) {
    LogicValue s = input_level(sim, instance, spi->pins->sclk);

    spi->sclk = s == LOGIC_X ? LOGIC_X : s;
    spi->cs = serial_level(sim, instance, spi->pins->cs, LOGIC_1);
    spi->active = 0;
    spi->miso = LOGIC_X;   /* forces the first drive */
    spi_drive(spi, sim, instance, LOGIC_Z, 0);
}

void serial_spi_enable(SerialSpi *spi, Sim *sim, SimInstance *instance, int enabled) {
    LogicValue s = input_level(sim, instance, spi->pins->sclk);

    spi->enabled = enabled;
    spi->active = 0;
    spi->out_bit = -1;
    spi->sclk = s;
    spi->cs = serial_level(sim, instance, spi->pins->cs, LOGIC_1);
    spi_drive(spi, sim, instance, LOGIC_Z, 0);
}

void serial_spi_pin(SerialSpi *spi, Sim *sim, SimInstance *instance, size_t pin,
                    const SerialSpiEvents *events) {
    const SerialSpiPins *p = spi->pins;

    if (pin == SERIAL_NO_PIN) {
        return;
    }
    if (!spi->enabled) {
        return;
    }
    if (pin == p->cs) {
        LogicValue c = serial_level(sim, instance, p->cs, LOGIC_1);

        if (c != spi->cs) {
            spi->cs = c;
            if (c == LOGIC_1) {
                if (spi->active) {
                    unsigned bits = spi->bits;

                    spi->active = 0;
                    spi->out_bit = -1;
                    spi_drive(spi, sim, instance, LOGIC_Z, spi->t_float);
                    if (events->end != NULL) {
                        events->end(events->context, bits);
                    }
                }
            } else if (!spi->active) {
                spi_begin(spi, sim, instance, events, c == LOGIC_X);
            } else if (c == LOGIC_X) {
                spi->x = 1;
                spi_drive(spi, sim, instance, spi_out_level(spi), 0);
            }
        }
    }
    if (pin == p->sclk) {
        LogicValue s = input_level(sim, instance, p->sclk);
        LogicValue idle = (p->mode >> 1) ? LOGIC_1 : LOGIC_0;

        if (s == LOGIC_X) {
            if (spi->active && !spi->x) {
                spi->x = 1;
                spi_drive(spi, sim, instance, spi_out_level(spi), 0);
            }
            return;
        }
        if (spi->sclk != LOGIC_X && s != spi->sclk && spi->active) {
            int leading = spi->sclk == idle;
            int sample = (p->mode & 1u) == 0 ? leading : !leading;

            spi->sclk = s;
            if (sample) {
                spi_sample(spi, sim, instance, events);
            } else {
                spi_shift(spi, sim, instance, events);
            }
            return;
        }
        spi->sclk = s;
    }
}

/* ---- I2C target ----------------------------------------------------------------- */

void serial_i2c_init(SerialI2c *i2c, const SerialI2cPins *pins) {
    unsigned i;

    memset(i2c, 0, sizeof *i2c);
    i2c->pins = pins;
    i2c->phase = SERIAL_I2C_IDLE;
    i2c->scl = LOGIC_X;
    i2c->sda = LOGIC_X;
    i2c->sda_out = LOGIC_Z;
    i2c->ack = LOGIC_0;
    for (i = 0; i < SERIAL_I2C_STRAPS; i++) {
        i2c->strap_idle[i] = i2c->strap_start[i] = i2c->strap_fall[i] = LOGIC_X;
    }
    i2c->enabled = 1;
}

static void i2c_drive(SerialI2c *i2c, Sim *sim, SimInstance *instance, LogicValue value) {
    /* Open drain: a 1 is released. */
    if (value == LOGIC_1) {
        value = LOGIC_Z;
    }
    if (i2c->sda_out == value) {
        return;
    }
    i2c->sda_out = value;
    sim_schedule_inertial_drive(sim, instance, i2c->pins->sda, value, DRIVE_STRONG,
                                i2c->t_out);
}

static void strap_sample(const SerialI2c *i2c, const Sim *sim, const SimInstance *instance,
                         LogicValue *out) {
    unsigned i;

    for (i = 0; i < i2c->pins->strap_count; i++) {
        out[i] = input_level(sim, instance, i2c->pins->strap[i]);
    }
}

int serial_i2c_strap(const SerialI2c *i2c, unsigned i) {
    LogicValue a;
    LogicValue b;
    LogicValue c;

    if (i >= i2c->pins->strap_count) {
        return -1;
    }
    a = i2c->strap_idle[i];
    b = i2c->strap_start[i];
    c = i2c->strap_fall[i];
    if (a == LOGIC_0 && b == LOGIC_0 && c == LOGIC_0) {
        return SERIAL_TIE_LOW;
    }
    if (a == LOGIC_1 && b == LOGIC_1 && c == LOGIC_1) {
        return SERIAL_TIE_HIGH;
    }
    if (a == LOGIC_1 && b == LOGIC_1 && c == LOGIC_0) {
        return SERIAL_TIE_SCL;
    }
    if (a == LOGIC_1 && b == LOGIC_0 && c == LOGIC_0) {
        return SERIAL_TIE_SDA;
    }
    return -1;
}

/* An X on the bus: drop the transaction, wait for a START. */
static void i2c_abandon(SerialI2c *i2c, Sim *sim, SimInstance *instance,
                        const SerialI2cEvents *events) {
    int selected = i2c->selected;

    i2c_drive(i2c, sim, instance, LOGIC_Z);
    i2c->selected = 0;
    if (i2c->phase != SERIAL_I2C_IDLE) {
        i2c->phase = SERIAL_I2C_IGNORE;
    }
    if (selected && events->end != NULL) {
        events->end(events->context, 0);
    }
}

static void i2c_start_condition(SerialI2c *i2c, Sim *sim, SimInstance *instance,
                                const SerialI2cEvents *events) {
    int repeated = i2c->phase != SERIAL_I2C_IDLE && i2c->selected;

    i2c_drive(i2c, sim, instance, LOGIC_Z);
    if (!repeated) {
        i2c->selected = 0;
    }
    i2c->phase = SERIAL_I2C_ADDRESS;
    i2c->bits = 0;
    i2c->in = 0;
    i2c->in_known = 0;
    i2c->index = 0;
    strap_sample(i2c, sim, instance, i2c->strap_start);
    i2c->after_start = 1;
    if (events->begin != NULL) {
        events->begin(events->context, repeated);
    }
}

static void i2c_stop_condition(SerialI2c *i2c, Sim *sim, SimInstance *instance,
                               const SerialI2cEvents *events) {
    int selected = i2c->selected;

    i2c_drive(i2c, sim, instance, LOGIC_Z);
    i2c->selected = 0;
    i2c->phase = SERIAL_I2C_IDLE;
    if (selected && events->end != NULL) {
        events->end(events->context, 1);
    }
}

static LogicValue out_bit(const SerialI2c *i2c, unsigned bit) {
    if (!((i2c->out_known >> bit) & 1u)) {
        return LOGIC_X;
    }
    return (i2c->out >> bit) & 1u ? LOGIC_1 : LOGIC_0;
}

static void i2c_begin_read_byte(SerialI2c *i2c, Sim *sim, SimInstance *instance,
                                const SerialI2cEvents *events) {
    i2c->out = 0;
    i2c->out_known = 0;
    if (events->next != NULL) {
        events->next(events->context, i2c->index, &i2c->out, &i2c->out_known);
    }
    i2c->bits = 0;
    i2c->phase = SERIAL_I2C_READ;
    i2c_drive(i2c, sim, instance, out_bit(i2c, 7));
}

static void i2c_rising(SerialI2c *i2c, Sim *sim, SimInstance *instance,
                       const SerialI2cEvents *events) {
    LogicValue sda = input_level(sim, instance, i2c->pins->sda);

    switch (i2c->phase) {
        case SERIAL_I2C_ADDRESS:
        case SERIAL_I2C_WRITE:
            if (i2c->bits >= 8) {
                break;
            }
            i2c->in = (uint8_t)((i2c->in << 1) | (sda == LOGIC_1 ? 1u : 0u));
            i2c->in_known = (uint8_t)((i2c->in_known << 1) | (sda == LOGIC_X ? 0u : 1u));
            if (++i2c->bits < 8) {
                break;
            }
            if (i2c->phase == SERIAL_I2C_ADDRESS) {
                LogicValue ack = events->address != NULL
                                     ? events->address(events->context, i2c->in, i2c->in_known)
                                     : LOGIC_0;

                if (ack == LOGIC_1 && !(i2c->in_known & 1u)) {
                    ack = LOGIC_X;   /* addressed, direction unknown */
                }
                i2c->ack = ack;
                i2c->read = i2c->in & 1u;
                if (ack == LOGIC_1) {
                    i2c->selected = 1;
                }
            } else {
                i2c->ack = events->received != NULL
                               ? events->received(events->context, i2c->index, i2c->in,
                                                  i2c->in_known)
                               : LOGIC_1;
                i2c->index++;
            }
            break;
        case SERIAL_I2C_READ:
            i2c->bits++;
            break;
        case SERIAL_I2C_ACK_IN: {
            LogicValue ack = sda == LOGIC_0 ? LOGIC_1 : sda == LOGIC_1 ? LOGIC_0 : LOGIC_X;

            i2c->ack = ack;
            if (events->acked != NULL) {
                events->acked(events->context, i2c->index, ack);
            }
            i2c->index++;
            break;
        }
        default:
            break;
    }
}

static void i2c_falling(SerialI2c *i2c, Sim *sim, SimInstance *instance,
                        const SerialI2cEvents *events) {
    if (i2c->after_start) {
        strap_sample(i2c, sim, instance, i2c->strap_fall);
        i2c->after_start = 0;
    }
    switch (i2c->phase) {
        case SERIAL_I2C_ADDRESS:
        case SERIAL_I2C_WRITE:
            if (i2c->bits == 8) {
                i2c_drive(i2c, sim, instance, i2c->ack == LOGIC_1   ? LOGIC_0
                                              : i2c->ack == LOGIC_0 ? LOGIC_Z
                                                                    : LOGIC_X);
                i2c->phase = SERIAL_I2C_ACK_OUT;
            }
            break;
        case SERIAL_I2C_ACK_OUT:
            i2c_drive(i2c, sim, instance, LOGIC_Z);
            if (i2c->ack != LOGIC_1) {
                i2c->phase = SERIAL_I2C_IGNORE;
            } else if (i2c->read) {
                i2c_begin_read_byte(i2c, sim, instance, events);
            } else {
                i2c->phase = SERIAL_I2C_WRITE;
                i2c->bits = 0;
                i2c->in = 0;
                i2c->in_known = 0;
            }
            break;
        case SERIAL_I2C_READ:
            if (i2c->bits < 8) {
                i2c_drive(i2c, sim, instance, out_bit(i2c, 7 - i2c->bits));
            } else {
                i2c_drive(i2c, sim, instance, LOGIC_Z);
                i2c->phase = SERIAL_I2C_ACK_IN;
            }
            break;
        case SERIAL_I2C_ACK_IN:
            if (i2c->ack == LOGIC_1) {
                i2c_begin_read_byte(i2c, sim, instance, events);
            } else {
                i2c_drive(i2c, sim, instance, LOGIC_Z);
                i2c->phase = SERIAL_I2C_IGNORE;
            }
            break;
        default:
            break;
    }
}

void serial_i2c_start(SerialI2c *i2c, Sim *sim, SimInstance *instance) {
    i2c->scl = input_level(sim, instance, i2c->pins->scl);
    i2c->sda = input_level(sim, instance, i2c->pins->sda);
    i2c->phase = SERIAL_I2C_IDLE;
    i2c->selected = 0;
    if (i2c->scl == LOGIC_1 && i2c->sda == LOGIC_1) {
        strap_sample(i2c, sim, instance, i2c->strap_idle);
    }
}

void serial_i2c_enable(SerialI2c *i2c, Sim *sim, SimInstance *instance, int enabled) {
    i2c->enabled = enabled;
    i2c->phase = SERIAL_I2C_IDLE;
    i2c->selected = 0;
    i2c->after_start = 0;
    i2c_drive(i2c, sim, instance, LOGIC_Z);
    i2c->scl = input_level(sim, instance, i2c->pins->scl);
    i2c->sda = input_level(sim, instance, i2c->pins->sda);
}

void serial_i2c_pin(SerialI2c *i2c, Sim *sim, SimInstance *instance, size_t pin,
                    const SerialI2cEvents *events) {
    const SerialI2cPins *p = i2c->pins;
    LogicValue scl;
    LogicValue sda;

    if (!i2c->enabled || pin == SERIAL_NO_PIN) {
        return;
    }
    scl = input_level(sim, instance, p->scl);
    sda = input_level(sim, instance, p->sda);
    if (pin == p->scl && scl != i2c->scl) {
        LogicValue was = i2c->scl;

        i2c->scl = scl;
        if (scl == LOGIC_X) {
            i2c_abandon(i2c, sim, instance, events);
        } else if (was == LOGIC_0 && scl == LOGIC_1) {
            i2c_rising(i2c, sim, instance, events);
        } else if (was == LOGIC_1 && scl == LOGIC_0) {
            i2c_falling(i2c, sim, instance, events);
        }
    }
    if (pin == p->sda && sda != i2c->sda) {
        LogicValue was = i2c->sda;

        i2c->sda = sda;
        if (i2c->scl == LOGIC_1) {
            if (sda == LOGIC_X) {
                i2c_abandon(i2c, sim, instance, events);
            } else if (was == LOGIC_1 && sda == LOGIC_0) {
                i2c_start_condition(i2c, sim, instance, events);
            } else if (was == LOGIC_0 && sda == LOGIC_1) {
                i2c_stop_condition(i2c, sim, instance, events);
            }
        }
    }
    if (scl == LOGIC_1 && sda == LOGIC_1) {
        strap_sample(i2c, sim, instance, i2c->strap_idle);
    }
}

/* ---- Loader ---------------------------------------------------------------------- */

static int load_mode(KindContext *ctx, const cJSON *object, unsigned *mode) {
    uint64_t m = 0;

    *mode = 0;
    if (cJSON_GetObjectItemCaseSensitive(object, "mode") == NULL) {
        return 0;
    }
    if (kind_uint(ctx, object, "mode", 3, &m) != 0) {
        return -1;
    }
    *mode = (unsigned)m;
    return 0;
}

static int load_spi(KindContext *ctx, const cJSON *object, SerialHostDevice *out) {
    if (kind_string(ctx, object, "cs", &out->cs) != 0 ||
        kind_string(ctx, object, "sclk", &out->sclk) != 0 ||
        kind_string(ctx, object, "mosi", &out->mosi) != 0 ||
        kind_string(ctx, object, "miso", &out->miso) != 0) {
        return -1;
    }
    return load_mode(ctx, object, &out->mode);
}

static int load_i2c(KindContext *ctx, const cJSON *object, SerialHostDevice *out) {
    const cJSON *table;
    const cJSON *item;
    const char *const *names;
    size_t n = 0;
    unsigned i = 0;
    unsigned want = 1;

    if (kind_string(ctx, object, "scl", &out->scl) != 0 ||
        kind_string(ctx, object, "sda", &out->sda) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(object, "address") != NULL) {
        if (kind_names(ctx, object, "address", &names, &n) != 0) {
            return -1;
        }
        if (n > 3) {
            return kind_error(ctx, "\"address\" has at most 3 pin names");
        }
        for (i = 0; i < n; i++) {
            out->strap[i] = names[i];
            want *= 4;
        }
    }
    out->strap_count = (unsigned)n;
    table = kind_member(ctx, object, "table", 1);
    if (table == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(table) || (unsigned)cJSON_GetArraySize(table) != want) {
        return kind_error(ctx, "\"table\" needs %u entries (4 per address pin tie)", want);
    }
    i = 0;
    cJSON_ArrayForEach(item, table) {
        if (cJSON_IsNull(item)) {
            out->table[i] = -1;
        } else if (cJSON_IsNumber(item) && item->valuedouble >= 0 && item->valuedouble <= 127 &&
                   item->valuedouble == (double)(int)item->valuedouble) {
            out->table[i] = (int)item->valuedouble;
        } else {
            return kind_error(ctx, "\"table\" entries are 7-bit addresses (0-127) or null");
        }
        i++;
    }
    out->table_count = want;
    return 0;
}

int serial_host_load(KindContext *ctx, const cJSON *bus, const char *const *extra,
                     SerialHostDevice *out) {
    static const char *const SPI_STYLE[] = {"style", "cs", "sclk", "mosi", "miso", "mode", NULL};
    static const char *const I2C_STYLE[] = {"style", "scl", "sda", "address", "table", NULL};
    static const char *const BOTH_STYLE[] = {"style", "selectPin", "spiLevel", "selectSampled",
                                             "spi", "i2c", NULL};
    static const char *const SPI_KEYS[] = {"cs", "sclk", "mosi", "miso", "mode", NULL};
    static const char *const I2C_KEYS[] = {"scl", "sda", "address", "table", NULL};
    const char *style;

    memset(out, 0, sizeof *out);
    if (!cJSON_IsObject(bus)) {
        return kind_error(ctx, "\"bus\" must be an object");
    }
    if (kind_string(ctx, bus, "style", &style) != 0) {
        return -1;
    }
    if (strcmp(style, "spi") == 0) {
        out->style = SERIAL_HOST_SPI;
        return kind_check_keys(ctx, bus, SPI_STYLE, extra) != 0 ? -1 : load_spi(ctx, bus, out);
    }
    if (strcmp(style, "i2c") == 0) {
        out->style = SERIAL_HOST_I2C;
        return kind_check_keys(ctx, bus, I2C_STYLE, extra) != 0 ? -1 : load_i2c(ctx, bus, out);
    }
    if (strcmp(style, "i2c-spi") == 0) {
        const cJSON *spi;
        const cJSON *i2c;
        uint64_t level = 1;

        out->style = SERIAL_HOST_I2C_SPI;
        out->select_live = 0;
        if (kind_check_keys(ctx, bus, BOTH_STYLE, extra) != 0 ||
            kind_string(ctx, bus, "selectPin", &out->select) != 0) {
            return -1;
        }
        if (cJSON_GetObjectItemCaseSensitive(bus, "spiLevel") != NULL &&
            kind_uint(ctx, bus, "spiLevel", 1, &level) != 0) {
            return -1;
        }
        out->spi_level = (int)level;
        if (cJSON_GetObjectItemCaseSensitive(bus, "selectSampled") != NULL) {
            const char *when;

            if (kind_string(ctx, bus, "selectSampled", &when) != 0) {
                return -1;
            }
            if (strcmp(when, "live") == 0) {
                out->select_live = 1;
            } else if (strcmp(when, "reset") != 0) {
                return kind_error(ctx, "\"selectSampled\" is \"live\" or \"reset\"");
            }
        }
        spi = kind_member(ctx, bus, "spi", 1);
        i2c = kind_member(ctx, bus, "i2c", 1);
        if (spi == NULL || i2c == NULL) {
            return -1;
        }
        kind_push(ctx, "spi");
        if (kind_check_keys(ctx, spi, SPI_KEYS, NULL) != 0 || load_spi(ctx, spi, out) != 0) {
            return -1;
        }
        kind_pop(ctx);
        kind_push(ctx, "i2c");
        if (kind_check_keys(ctx, i2c, I2C_KEYS, NULL) != 0 || load_i2c(ctx, i2c, out) != 0) {
            return -1;
        }
        kind_pop(ctx);
        return 0;
    }
    return kind_error(ctx, "unknown bus style \"%s\"", style);
}

int serial_host_bind(KindContext *ctx, const SerialHostDevice *dev, const PartPin *pins,
                     size_t pin_count, SerialHostPins *out) {
    unsigned i;

    memset(out, 0, sizeof *out);
    out->spi.cs = out->spi.sclk = out->spi.mosi = out->spi.miso = SERIAL_NO_PIN;
    out->i2c.scl = out->i2c.sda = SERIAL_NO_PIN;
    for (i = 0; i < SERIAL_I2C_STRAPS; i++) {
        out->i2c.strap[i] = SERIAL_NO_PIN;
    }
    out->select = SERIAL_NO_PIN;
    if (dev->style != SERIAL_HOST_I2C) {
        if (serial_bind_pin(ctx, pins, pin_count, "bus.cs", dev->cs, &out->spi.cs) != 0 ||
            serial_bind_pin(ctx, pins, pin_count, "bus.sclk", dev->sclk, &out->spi.sclk) != 0 ||
            serial_bind_pin(ctx, pins, pin_count, "bus.mosi", dev->mosi, &out->spi.mosi) != 0 ||
            serial_bind_pin(ctx, pins, pin_count, "bus.miso", dev->miso, &out->spi.miso) != 0) {
            return -1;
        }
        out->spi.mode = dev->mode;
    }
    if (dev->style != SERIAL_HOST_SPI) {
        if (serial_bind_pin(ctx, pins, pin_count, "bus.scl", dev->scl, &out->i2c.scl) != 0 ||
            serial_bind_pin(ctx, pins, pin_count, "bus.sda", dev->sda, &out->i2c.sda) != 0) {
            return -1;
        }
        for (i = 0; i < dev->strap_count; i++) {
            if (serial_bind_pin(ctx, pins, pin_count, "bus.address", dev->strap[i],
                                &out->i2c.strap[i]) != 0) {
                return -1;
            }
        }
        out->i2c.strap_count = dev->strap_count;
    }
    if (dev->style == SERIAL_HOST_I2C_SPI &&
        serial_bind_pin(ctx, pins, pin_count, "bus.selectPin", dev->select, &out->select) != 0) {
        return -1;
    }
    return 0;
}

int serial_host_has_pin(const SerialHostDevice *dev, const SerialHostPins *pins, size_t pin) {
    unsigned i;

    (void)dev;
    if (pin == SERIAL_NO_PIN) {
        return 0;
    }
    if (pin == pins->spi.cs || pin == pins->spi.sclk || pin == pins->spi.mosi ||
        pin == pins->spi.miso || pin == pins->i2c.scl || pin == pins->i2c.sda ||
        pin == pins->select) {
        return 1;
    }
    for (i = 0; i < pins->i2c.strap_count; i++) {
        if (pin == pins->i2c.strap[i]) {
            return 1;
        }
    }
    return 0;
}

int serial_host_open_drain(const SerialHostDevice *dev, const SerialHostPins *pins, size_t pin) {
    return dev->style != SERIAL_HOST_SPI && pin != SERIAL_NO_PIN &&
           (pin == pins->i2c.sda || pin == pins->i2c.scl);
}

void serial_host_init(SerialHost *host, const SerialHostDevice *dev, const SerialHostPins *pins) {
    memset(host, 0, sizeof *host);
    host->dev = dev;
    host->pins = pins;
    serial_spi_init(&host->spi, &pins->spi);
    serial_i2c_init(&host->i2c, &pins->i2c);
    host->mode = SERIAL_HOST_NONE;
    host->sampled = LOGIC_X;
    host->spi.enabled = 0;
    host->i2c.enabled = 0;
}

static void set_mode(SerialHost *host, Sim *sim, SimInstance *instance, SerialHostMode mode) {
    if (mode == host->mode) {
        return;
    }
    if (host->mode == SERIAL_HOST_ON_SPI) {
        serial_spi_enable(&host->spi, sim, instance, 0);
    }
    if (host->mode == SERIAL_HOST_ON_I2C) {
        serial_i2c_enable(&host->i2c, sim, instance, 0);
    }
    host->mode = mode;
    if (mode == SERIAL_HOST_ON_SPI) {
        serial_spi_enable(&host->spi, sim, instance, 1);
    }
    if (mode == SERIAL_HOST_ON_I2C) {
        serial_i2c_enable(&host->i2c, sim, instance, 1);
    }
}

/* The front end the style and the select pin (live or sampled) pick now. */
static void update_mode(SerialHost *host, Sim *sim, SimInstance *instance) {
    SerialHostMode mode = SERIAL_HOST_NONE;

    if (!host->held) {
        switch (host->dev->style) {
            case SERIAL_HOST_SPI:
                mode = SERIAL_HOST_ON_SPI;
                break;
            case SERIAL_HOST_I2C:
                mode = SERIAL_HOST_ON_I2C;
                break;
            case SERIAL_HOST_I2C_SPI: {
                LogicValue level = host->dev->select_live
                                       ? input_level(sim, instance, host->pins->select)
                                       : host->sampled;

                if (level != LOGIC_X) {
                    mode = (int)(level == LOGIC_1) == host->dev->spi_level ? SERIAL_HOST_ON_SPI
                                                                           : SERIAL_HOST_ON_I2C;
                }
                break;
            }
        }
    }
    set_mode(host, sim, instance, mode);
}

void serial_host_start(SerialHost *host, Sim *sim, SimInstance *instance) {
    serial_spi_start(&host->spi, sim, instance);
    serial_i2c_start(&host->i2c, sim, instance);
    host->spi.enabled = 0;
    host->i2c.enabled = 0;
    host->mode = SERIAL_HOST_NONE;
    host->sampled = input_level(sim, instance, host->pins->select);
    update_mode(host, sim, instance);
}

void serial_host_hold(SerialHost *host, Sim *sim, SimInstance *instance, int held) {
    if (host->held && !held) {
        host->sampled = input_level(sim, instance, host->pins->select);
    }
    host->held = held;
    update_mode(host, sim, instance);
}

void serial_host_pin(SerialHost *host, Sim *sim, SimInstance *instance, size_t pin,
                     const SerialSpiEvents *spi, const SerialI2cEvents *i2c) {
    if (pin == SERIAL_NO_PIN) {
        return;
    }
    if (host->dev->style == SERIAL_HOST_I2C_SPI && host->dev->select_live &&
        pin == host->pins->select) {
        update_mode(host, sim, instance);
    }
    if (host->mode == SERIAL_HOST_ON_SPI) {
        serial_spi_pin(&host->spi, sim, instance, pin, spi);
    } else if (host->mode == SERIAL_HOST_ON_I2C) {
        serial_i2c_pin(&host->i2c, sim, instance, pin, i2c);
    }
}

int serial_host_i2c_address(const SerialHost *host) {
    unsigned index = 0;
    unsigned scale = 1;
    unsigned i;

    for (i = 0; i < host->i2c.pins->strap_count; i++) {
        int tie = serial_i2c_strap(&host->i2c, i);

        if (tie < 0) {
            return -2;
        }
        index += (unsigned)tie * scale;
        scale *= 4;
    }
    if (index >= host->dev->table_count) {
        return -1;
    }
    return host->dev->table[index];
}
