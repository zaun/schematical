#ifndef CONVERTER_INT_H
#define CONVERTER_INT_H

/* The converter engine's instance state, shared by converter.c (protocol,
   SPI, I2C), converter_par.c (parallel and clocked buses) and rtc.c. */

#include "converter.h"

enum {
    CV_MAX_AT = 64,             /* registers in one @ read */
    CV_TM_KIND = 0xF0000000u,
    CV_TM_GEN = 0x0FFFFFFFu,
    CV_TM_CONV = 0x10000000u,
    CV_TM_READY = 0x20000000u,
    CV_TM_PAR = 0x30000000u,    /* converter_par.c */
    CV_TM_RTC = 0x40000000u     /* rtc.c */
};

enum { CV_PAY_NONE, CV_PAY_WREG, CV_PAY_POINTER };

/* An output segment: a pattern being shifted out. */
typedef struct {
    const CvPattern *pat;
    uint32_t caps[CV_CAPS];
    int at[CV_MAX_AT];
    size_t at_count;
    unsigned at_word;
    size_t pos;
    size_t at_index;
    unsigned at_bit;
    int sdio;                   /* reply bits: driven on a shared SDIO pin */
    int active;
} CvSeg;

/* One transaction (CS frame, I2C transfer, parallel write). */
typedef struct {
    int open;
    int reading;
    int xed;                    /* an unknown bit arrived */
    int done;                   /* fixed frame: a command fired */
    uint8_t bits[CV_MAX_BITS];
    size_t n;
    uint8_t *alive;             /* per command: can still match */
    int payload;
    int pay_regs[CV_MAX_AT];
    size_t pay_count, pay_index;
    unsigned pay_bits, pay_word;
    uint32_t pay_value;
    const CvCommand *pay_cmd;
    uint32_t pay_caps[CV_CAPS];
    const CvCommand *deferred;
    uint32_t deferred_caps[CV_CAPS];
} CvXfer;

typedef struct CvState {
    const Part *part;
    const CvDevice *dev;
    const CvBound *b;
    const CvPart *pd;
    Sim *sim;
    SimInstance *inst;
    uint32_t *regs;
    LogicValue *drv;
    uint8_t *alive_store;
    uint8_t *read_mark;         /* per register: its bits went out in this read */
    int read_marked;
    /* protocol */
    CvXfer x;
    CvSeg seg, last, next;
    int has_next;
    int seg_from;
    int idle_default;
    int at[CV_MAX_AT];
    size_t at_count;
    unsigned at_word;
    uint32_t pointer;
    int pointer_space;
    int pointer_inc;
    int read_reg;
    uint32_t read_key;          /* parallel reads: the address (or the byte select) */
    int read_noted;
    /* conversion */
    int converting;
    uint32_t conv_gen;
    int conv_channel;
    int out_channel;
    int result_fresh;
    int powered_down;
    int powered_down_bus;
    int in_reset;
    int start_level;
    uint8_t *ready_on;
    uint32_t *ready_gen;
    LogicValue ctl_level[CVP_COUNT];
    /* SPI */
    LogicValue sclk;
    int spi_selected;
    int spi_shifting;
    unsigned spi_bits;
    LogicValue spi_out;
    SimTime spi_delay;
    int spi_off;
    LogicValue pending_first;
    int has_pending_first;
    /* I2C */
    int has_i2c;
    SerialI2c i2c;
    SerialI2cPins i2c_pins;
    SerialI2cEvents i2c_ev;
    int i2c_general;
    /* parallel / clocked (converter_par.c) */
    int par_reading;
    int par_writing;
    LogicValue par_wr_level;
    uint32_t par_latched;           /* ALE address */
    int par_has_latched;
    uint32_t par_data_last;         /* unlatched DACs: the data pins last seen */
    LogicValue par_clock;
    CvSeg clk_seg;
    uint32_t clk_count;
    /* rtc (rtc.c) */
    void *rtc;
} CvState;

size_t cv_pin(const CvState *s, int name);
LogicValue cv_level(const CvState *s, size_t pin, LogicValue z_reads);
void cv_drive(CvState *s, size_t pin, LogicValue v, SimTime delay);
uint32_t cv_field_get(const CvState *s, const CvField *f);
int cv_conds_hold(const CvState *s, const CvConds *c);
uint32_t cv_choice(const CvState *s, const CvChoice *c);
uint32_t cv_code(const CvState *s);
LogicValue cv_ready_level(const CvState *s, int pin_name);
void cv_ready_refresh(CvState *s);
void cv_start_conversion(CvState *s, int channel);
void cv_result_read(CvState *s);
void cv_reg_write(CvState *s, int reg, uint32_t value);
void cv_reg_read(CvState *s, int reg);
void cv_reset(CvState *s);
void cv_bit_in(CvState *s, LogicValue v);
LogicValue cv_bit_out(CvState *s, int *sdio);
LogicValue cv_seg_bit(CvState *s, CvSeg *g, int idle, int *sdio);
/* Sets a segment to shift a pattern (caps/@ list optional). */
void cv_seg_set(CvSeg *seg, const CvPattern *pat);
/* The read pattern for the current read key (parallel/clocked reads). */
const CvPattern *cv_read_pattern(CvState *s);
void cv_xfer_begin(CvState *s, int reading);
void cv_xfer_end(CvState *s);
void cv_read_begin(CvState *s);
void cv_spi_refresh(CvState *s);
void cv_spi_conversion_done(CvState *s);
void cv_free_state(CvState *s);
int cv_reg_at(const CvState *s, int space, uint32_t address);
/* A read ended: clear-on-read bits of the registers it shifted out. */
void cv_reads_done(CvState *s);

/* rtc.c (plan-24-F) */
void rtc_start(CvState *s);
void rtc_timer(CvState *s, uint32_t id);
void rtc_after_write(CvState *s, int reg);
void rtc_pin(CvState *s, size_t pin);
void rtc_refresh(CvState *s);
void rtc_free(CvState *s);

/* converter_par.c (plan-24-E) */
void cv_par_start(CvState *s);
void cv_par_pin(CvState *s, size_t pin);
void cv_par_timer(CvState *s, uint32_t id);

#endif
