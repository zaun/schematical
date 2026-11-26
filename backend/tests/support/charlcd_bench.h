#ifndef CHARLCD_BENCH_H
#define CHARLCD_BENCH_H

/*
 * charlcd_test: the charlcd kind (plan-11-B section 6). An inline fixture
 * device copies the NHD-0216K1Z-NSW-BBW-L facts (plan-11-C section 4.1):
 * 16x2, the Newhaven timing and exec values, a fixture footprint. The bus is
 * driven through tests/support/bench.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "cJSON.h"
#include "circuit.h"
#include "charlcd_font.h"
#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"
#include "voltage.h"

/* ---- Fixture -------------------------------------------------------------- */

#define PINOUT \
    "'pinouts':{'p16':[{'number':1,'name':'VSS','role':'ground'}," \
    "{'number':2,'name':'VDD','role':'power'},{'number':3,'name':'V0','role':'input'}," \
    "{'number':4,'name':'RS','role':'input'},{'number':5,'name':'R/W','role':'input'}," \
    "{'number':6,'name':'E','role':'input'}," \
    "{'number':7,'name':'DB0','role':'inout','group':'data'}," \
    "{'number':8,'name':'DB1','role':'inout','group':'data'}," \
    "{'number':9,'name':'DB2','role':'inout','group':'data'}," \
    "{'number':10,'name':'DB3','role':'inout','group':'data'}," \
    "{'number':11,'name':'DB4','role':'inout','group':'data'}," \
    "{'number':12,'name':'DB5','role':'inout','group':'data'}," \
    "{'number':13,'name':'DB6','role':'inout','group':'data'}," \
    "{'number':14,'name':'DB7','role':'inout','group':'data'}," \
    "{'number':15,'name':'LED+','role':'input'},{'number':16,'name':'LED-','role':'input'}]}"

#define DATA "['DB0','DB1','DB2','DB3','DB4','DB5','DB6','DB7']"
#define TIMING "{'tc':1200,'tpw':140,'tas':0,'tah':10,'tdsw':40,'th':10,'tddr':100,'tdhr':10}"
#define EXEC "{'instruction':37000,'data':37000,'clearHome':1520000,'powerOn':40000000}"
#define BACKLIGHT "'backlight':{'anode':'LED+','cathode':'LED-','color':'White'},"
#define VCC "'vcc':[4.5,5.5],"

/* plan-35-D fixtures. p18-dual: the C404ALB 40x4 pin table (E1 pin 9 for
   the top two lines, E2 pin 15 for the bottom two). p18-rgb: the Midas
   MD42004A6W-FPTLRGB table (LED+ 15, K(B) 16, K(G) 17, K(R) 18). */
#define PINOUT_P18_DUAL \
    "'pinouts':{'p18-dual':[" \
    "{'number':1,'name':'DB7','role':'inout','group':'data'}," \
    "{'number':2,'name':'DB6','role':'inout','group':'data'}," \
    "{'number':3,'name':'DB5','role':'inout','group':'data'}," \
    "{'number':4,'name':'DB4','role':'inout','group':'data'}," \
    "{'number':5,'name':'DB3','role':'inout','group':'data'}," \
    "{'number':6,'name':'DB2','role':'inout','group':'data'}," \
    "{'number':7,'name':'DB1','role':'inout','group':'data'}," \
    "{'number':8,'name':'DB0','role':'inout','group':'data'}," \
    "{'number':9,'name':'E1','role':'input'},{'number':10,'name':'R/W','role':'input'}," \
    "{'number':11,'name':'RS','role':'input'},{'number':12,'name':'V0','role':'input'}," \
    "{'number':13,'name':'VSS','role':'ground'},{'number':14,'name':'VDD','role':'power'}," \
    "{'number':15,'name':'E2','role':'input'},{'number':16,'name':'NC','role':'nc'}," \
    "{'number':17,'name':'LED+','role':'input'},{'number':18,'name':'LED-','role':'input'}]}"

#define PINOUT_P18_RGB \
    "'pinouts':{'p18-rgb':[{'number':1,'name':'VSS','role':'ground'}," \
    "{'number':2,'name':'VDD','role':'power'},{'number':3,'name':'V0','role':'input'}," \
    "{'number':4,'name':'RS','role':'input'},{'number':5,'name':'R/W','role':'input'}," \
    "{'number':6,'name':'E','role':'input'}," \
    "{'number':7,'name':'DB0','role':'inout','group':'data'}," \
    "{'number':8,'name':'DB1','role':'inout','group':'data'}," \
    "{'number':9,'name':'DB2','role':'inout','group':'data'}," \
    "{'number':10,'name':'DB3','role':'inout','group':'data'}," \
    "{'number':11,'name':'DB4','role':'inout','group':'data'}," \
    "{'number':12,'name':'DB5','role':'inout','group':'data'}," \
    "{'number':13,'name':'DB6','role':'inout','group':'data'}," \
    "{'number':14,'name':'DB7','role':'inout','group':'data'}," \
    "{'number':15,'name':'LED+','role':'input'},{'number':16,'name':'K(B)','role':'input'}," \
    "{'number':17,'name':'K(G)','role':'input'},{'number':18,'name':'K(R)','role':'input'}]}"

#define RGB_BACKLIGHT \
    "'backlight':[{'anode':'LED+','cathode':'K(R)','color':'Red'}," \
    "{'anode':'LED+','cathode':'K(G)','color':'Green'}," \
    "{'anode':'LED+','cathode':'K(B)','color':'Blue'}],"

/* ---- Bus helpers (section 6) ---------------------------------------------- */

#define NS(v) ((SimTime)(v) * 1000ull)
#define US(v) ((SimTime)(v) * 1000000ull)
#define MS(v) ((SimTime)(v) * 1000000000ull)
/* ---- plan-11-C section 4.6: the CPU board ---- */

enum { BOARD_NETS = 64 };

extern int failures;
extern PartLibrary *library;
void expect(int ok, const char *what);
extern const char FOOTPRINTS[];
char *dequote(const char *text);
char *device_file(const char *name, const char *columns, const char *rows,
                         const char *rom, const char *panel, const char *data,
                         const char *timing, const char *exec, const char *extra);
char *large_file(const char *name, const char *pinouts, const char *pinout,
                        const char *footprint, const char *columns, const char *rows,
                        const char *panel, const char *e, const char *extra);
char *load_file(PartLibrary *lib, char *json, const char *source);
PartLibrary *fixture_library(void);
int glyph_is(unsigned code, const uint8_t rows[8]);
void check_rom(void);
void print_font(void);

typedef struct {
    Bench *b;
    const Part *part;
    int four;            /* 4-bit mode: bytes go as two nibbles */
    int db_low_z;        /* every 4-bit read saw DB0-3 at Z */
    const char *e;       /* the enable the accesses pulse: "E", or "E1" / "E2" on a 40x4 */
    /* The bus pins' names, from the part's bus_pins (RS, R/W, E or E1 and
       E2, DB0-DB7); the bench finds a pin by its name. */
    const char *rs, *rw, *en[2], *db[8];
    unsigned enables;
    int pins_ok;         /* each bus pin is the first pin of its name */
    /* One access: RS, R/W and DB set `setup` before E rises, E high `high`,
       then `after` with E low before the next change. */
    SimTime setup, high, after;
} Lcd;
extern const char *const DB[8];
void lcd_pins(Lcd *l);
Lcd lcd_open(const char *name);
SimTime now_ps(Lcd *l);
void at(Lcd *l, SimTime t);
void lv(Lcd *l, const char *pin, int bit);
void put_db(Lcd *l, uint8_t v, uint8_t mask);
void access_write(Lcd *l, int rs, uint8_t v, uint8_t mask);
void access_read(Lcd *l, int rs, uint8_t *v, uint8_t *k);
void wr(Lcd *l, int rs, uint8_t byte);
void cmd(Lcd *l, uint8_t byte, SimTime gap);
void text(Lcd *l, const char *s);
void rd(Lcd *l, int rs, uint8_t *v, uint8_t *k);
int read_ac(Lcd *l);
void lcd_state(Lcd *l, LcdState *st);
void init8(Lcd *l);
void clear(Lcd *l);
uint8_t dot(const LcdState *st, unsigned row, unsigned column, unsigned y, unsigned x);
int cell_is_rom(const LcdState *st, unsigned row, unsigned column, unsigned code);
int cell_rows_are(const LcdState *st, unsigned row, unsigned column, const uint8_t rows[8]);
int cell_all(const LcdState *st, unsigned row, unsigned column, uint8_t value);
int row_all(const LcdState *st, unsigned row, unsigned columns, uint8_t value);
int image_all(const LcdState *st, uint8_t value);
SimTime blink_time(SimTime from, int on);
int db_all(Lcd *l, LogicValue v);
extern PartLibrary *real_library;

/* A part's timing and exec times (ps), read from its device in charlcd.json
   (the library keeps them private to the model). */
typedef struct {
    SimTime tc, tpw, tas, tah, tdsw, th, tddr, tdhr;
    SimTime instruction, data, clear_home, power_on;
} RealTiming;
int cells_show(const LcdState *st, unsigned row, unsigned column, const char *s);

typedef struct {
    Circuit *c;
    const char *names[BOARD_NETS];
    size_t nets[BOARD_NETS];
    size_t count;
    int failed;
} Board;

/* What a board carries: the LCD part, its decode, and the EEPROM image
   (from offset 0, address 8000) with NMI and IRQ pointing at `halt`. */
typedef struct {
    const char *lcd;
    int two;                 /* two enables: E1 at 4000 (A13 = 0), E2 at 6000 (A13 = 1) */
    const uint8_t *program;
    size_t size;
    unsigned halt;           /* the program's JMP-to-itself address */
} BoardSpec;

/* ---- plan-35-E section 4.3: programs for the 20x4 and 40x4 boards ---- */

/* An EEPROM image being assembled at `pc` (address 8000 = offset 0). */
typedef struct {
    uint8_t bytes[256];
    unsigned pc;
} Asm;

#endif
