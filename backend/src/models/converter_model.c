/* models/converter_model.c: part of the model in models/converter.c (split by plan-68-I/J). */
#include "converter_core_int.h"
#include "logic_alloc.h"
/* ---- The model -------------------------------------------------------------------------------- */

static size_t cv_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *cv_create(const void *config) {
    const Part *part = config;
    const PartInternal *in = part_internal(part);
    CvState *s = logic_calloc(1, sizeof *s);
    size_t i;

    if (s == NULL) {
        return NULL;
    }
    s->part = part;
    s->dev = in->device;
    s->b = in->bound;
    s->pd = in->data;
    s->regs = logic_calloc(s->dev->reg_count ? s->dev->reg_count : 1, sizeof(*s->regs));
    s->drv = logic_malloc(part->pin_count * sizeof(*s->drv));
    s->ready_on = logic_calloc(s->dev->ready_count ? s->dev->ready_count : 1, 1);
    s->ready_gen = logic_calloc(s->dev->ready_count ? s->dev->ready_count : 1, sizeof(*s->ready_gen));
    s->x.alive = logic_calloc(s->dev->protocol.command_count ? s->dev->protocol.command_count : 1, 1);
    /* cv_free_state frees the block through alive_store: set it before any
       failure below can send us there (plan-68-K found the leak). */
    s->alive_store = s->x.alive;
    s->read_mark = logic_calloc(s->dev->reg_count ? s->dev->reg_count : 1, 1);
    if (s->regs == NULL || s->drv == NULL || s->ready_on == NULL || s->ready_gen == NULL ||
        s->x.alive == NULL || s->read_mark == NULL) {
        cv_free_state(s);
        return NULL;
    }
    for (i = 0; i < part->pin_count; i++) {
        s->drv[i] = LOGIC_Z;
    }
    s->read_reg = CV_NONE;
    s->sclk = LOGIC_X;
    s->idle_default = '0';
    if (cvc_i2c_bus(s) != NULL) {
        const CvBus *b = cvc_i2c_bus(s);
        unsigned k;

        s->i2c_pins.scl = cv_pin(s, b->scl);
        s->i2c_pins.sda = cv_pin(s, b->sda);
        s->i2c_pins.strap_count = 0;
        if (s->pd->has_bus_i2c) {
            for (k = 0; k < s->pd->strap_count; k++) {
                s->i2c_pins.strap[s->i2c_pins.strap_count++] = (size_t)s->pd->straps[k];
            }
        } else {
            for (k = 0; k < b->strap_count; k++) {
                if (cv_pin(s, b->straps[k]) != SIZE_MAX) {
                    s->i2c_pins.strap[s->i2c_pins.strap_count++] = cv_pin(s, b->straps[k]);
                }
            }
        }
        serial_i2c_init(&s->i2c, &s->i2c_pins);
        s->i2c.t_out = s->dev->timing.t_out;
        s->i2c_ev.context = s;
        s->i2c_ev.begin = cvc_i2c_begin;
        s->i2c_ev.address = cvc_i2c_address;
        s->i2c_ev.received = cvc_i2c_received;
        s->i2c_ev.next = cvc_i2c_next;
        s->i2c_ev.end = cvc_i2c_end;
        s->has_i2c = 1;
    }
    return s;
}

void cv_free_state(CvState *s) {
    if (s == NULL) {
        return;
    }
    logic_free(s->regs);
    logic_free(s->drv);
    logic_free(s->ready_on);
    logic_free(s->ready_gen);
    logic_free(s->alive_store);
    logic_free(s->read_mark);
    rtc_free(s);
    disp_free(s);
    logic_free(s);
}

static void cv_destroy(void *state) {
    cv_free_state(state);
}

static void control_pins_start(CvState *s) {
    const CvPins *p = &s->dev->pins;
    size_t i;

    for (i = 0; i < CVP_COUNT; i++) {
        if (p->has[i]) {
            s->ctl_level[i] = cv_level(s, cv_pin(s, p->ctl[i].pin),
                                       p->ctl[i].active ? LOGIC_0 : LOGIC_1);
        }
    }
    s->in_reset = p->has[CVP_RESET] &&
                  s->ctl_level[CVP_RESET] == (p->ctl[CVP_RESET].active ? LOGIC_1 : LOGIC_0);
    s->powered_down_bus = p->has[CVP_POWERDOWN] &&
                          s->ctl_level[CVP_POWERDOWN] ==
                              (p->ctl[CVP_POWERDOWN].active ? LOGIC_1 : LOGIC_0);
    s->start_level = p->has[CVP_START] &&
                     s->ctl_level[CVP_START] == (p->ctl[CVP_START].active ? LOGIC_1 : LOGIC_0);
}

static void cv_start(Sim *sim, SimInstance *instance, void *state) {
    CvState *s = state;
    const CvBus *b;

    s->sim = sim;
    s->inst = instance;
    control_pins_start(s);
    cv_reset(s);
    if (s->has_i2c) {
        serial_i2c_start(&s->i2c, sim, instance);
        s->idle_default = 'r';
    }
    b = cvc_spi_bus(s);
    if (b != NULL) {
        s->sclk = cv_level(s, cv_pin(s, b->sclk), LOGIC_X);
        if (b->cs != CV_NONE) {
            LogicValue v = cv_level(s, cv_pin(s, b->cs), b->cs_level ? LOGIC_0 : LOGIC_1);

            if (v == (b->cs_level ? LOGIC_1 : LOGIC_0) && !s->in_reset) {
                cvc_spi_select(s, b);
            }
        }
    }
    if (s->dev->bus.style == CVB_I2C_SPI) {
        LogicValue v = cv_level(s, cv_pin(s, s->dev->bus.select), LOGIC_0);

        s->spi_off = (v == LOGIC_1) != (s->dev->bus.spi_level == 1);
        serial_i2c_enable(&s->i2c, sim, instance, s->spi_off);
    }
    cv_par_start(s);
    if ((s->dev->rtc && rtc_start(s) != 0) || (s->dev->disp != NULL && disp_start(s) != 0)) {
        sim_fail(sim, "out of memory");
        return;
    }
    cv_ready_refresh(s);
}

static int is_level_active(const CvPinCtl *c, LogicValue v) {
    return v == (c->active ? LOGIC_1 : LOGIC_0);
}

static void control_pin(CvState *s, size_t pin) {
    const CvPins *p = &s->dev->pins;
    size_t i;

    for (i = 0; i < p->level_count; i++) {
        if (cv_pin(s, p->levels[i].pin) == pin) {
            cvc_field_put(s, &p->levels[i].field, cv_level(s, pin, LOGIC_0) == LOGIC_1);
        }
    }
    for (i = 0; i < CVP_COUNT; i++) {
        const CvPinCtl *c = &p->ctl[i];
        LogicValue v, old;
        int rising, falling, edge_hit;

        if (!p->has[i] || cv_pin(s, c->pin) != pin) {
            continue;
        }
        v = cv_level(s, pin, c->active ? LOGIC_0 : LOGIC_1);
        old = s->ctl_level[i];
        if (v == old) {
            continue;
        }
        s->ctl_level[i] = v;
        rising = old == LOGIC_0 && v == LOGIC_1;
        falling = old == LOGIC_1 && v == LOGIC_0;
        edge_hit = c->edge == 1 ? rising : c->edge == 0 ? falling : 0;
        switch (i) {
        case CVP_RESET:
            if (is_level_active(c, v)) {
                s->in_reset = 1;
                if (s->x.open) {
                    s->x.open = 0;
                }
                s->spi_selected = 0;
                cv_reset(s);
            } else if (s->in_reset) {
                s->in_reset = 0;
                cv_reset(s);
            }
            break;
        case CVP_CONVST:
            if (edge_hit && (s->dev->conversion.starts & CVS_PIN)) {
                cv_start_conversion(s, -1);
            }
            break;
        case CVP_START:
            if (c->edge >= 0) {
                if (edge_hit) {
                    cv_start_conversion(s, -1);
                }
            } else {
                s->start_level = is_level_active(c, v);
                if (s->start_level) {
                    cv_start_conversion(s, -1);
                } else if (s->converting && cvc_continuous(s) == 0 && s->dev->conversion.mode != 1) {
                    /* the conversion in progress completes */
                }
            }
            break;
        case CVP_POWERDOWN:
            s->powered_down_bus = is_level_active(c, v);
            s->powered_down = s->powered_down_bus;
            if (s->powered_down_bus) {
                cvc_conv_stop(s);
            }
            break;
        case CVP_LDAC:
            if (edge_hit || (c->edge < 0 && is_level_active(c, v))) {
                cvc_dac_update(s, -1);
            }
            break;
        case CVP_CLEAR:
            if (edge_hit || (c->edge < 0 && is_level_active(c, v))) {
                cvc_dac_clear(s);
            }
            break;
        default:
            break;
        }
    }
    cv_ready_refresh(s);
}

static void cv_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    CvState *s = state;
    const CvBus *b = cvc_spi_bus(s);

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->inst = instance;
    control_pin(s, pin);
    if (s->dev->bus.style == CVB_I2C_SPI && pin == cv_pin(s, s->dev->bus.select)) {
        LogicValue v = cv_level(s, pin, LOGIC_0);

        s->spi_off = (v == LOGIC_1) != (s->dev->bus.spi_level == 1);
        serial_i2c_enable(&s->i2c, sim, instance, s->spi_off);
    }
    if (s->dev->rtc) {
        rtc_pin(s, pin);
    }
    if (s->dev->disp != NULL) {
        disp_pin(s, pin);
    }
    if (s->in_reset || s->powered_down_bus) {
        return;
    }
    if (b != NULL && !s->spi_off) {
        cvc_spi_pin(s, b, pin);
    }
    if (s->has_i2c) {
        serial_i2c_pin(&s->i2c, sim, instance, pin, &s->i2c_ev);
    }
    cv_par_pin(s, pin);
}

static void cv_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    CvState *s = state;

    s->sim = sim;
    s->inst = instance;
    if ((id & CV_TM_KIND) == CV_TM_CONV) {
        if ((id & CV_TM_GEN) == (s->conv_gen & CV_TM_GEN) && s->converting) {
            cvc_conv_done(s);
        }
    } else if ((id & CV_TM_KIND) == CV_TM_READY) {
        size_t i = (id >> 16) & 0xFFFu;

        if (i < s->dev->ready_count && (id & 0xFFFFu) == (s->ready_gen[i] & 0xFFFFu)) {
            s->ready_on[i] = 0;
            cv_ready_refresh(s);
        }
    } else if ((id & CV_TM_KIND) == CV_TM_RTC) {
        rtc_timer(s, id);
    } else if ((id & CV_TM_KIND) == CV_TM_DISP) {
        disp_timer(s, id);
    } else {
        cv_par_timer(s, id);
    }
}

/* The register window (plan-24-I): every register's current value. */
static size_t cv_registers(const void *state, ConverterRegister *out, size_t cap) {
    const CvState *s = state;
    size_t i;

    for (i = 0; i < s->dev->reg_count && i < cap && out != NULL; i++) {
        const CvRegister *r = &s->dev->regs[i];

        out[i].name = r->name;
        out[i].address = r->address;
        out[i].space = r->space;
        out[i].width = r->width;
        out[i].value = s->regs[i];
    }
    return s->dev->reg_count;
}

static int cv_rtc_time(const void *state, RtcTime *out) {
    return rtc_snapshot(state, out);
}

const SimComponentType CONVERTER_COMPONENT = {
    "converter", cv_pin_count, cv_create, cv_destroy, cv_start, cv_pin_changed, cv_timer,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, cv_registers, cv_rtc_time
};

