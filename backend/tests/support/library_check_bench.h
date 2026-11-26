#ifndef LIBRARY_CHECK_BENCH_H
#define LIBRARY_CHECK_BENCH_H

/*
 * library_check: load_parts accepts well-formed part files and rejects every
 * malformed case with a path-style message, adding nothing on failure
 * (plan-01-B section 3.7; C, D, E add their kinds' cases).
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "library_internal.h"
#include "model_pins.h"
#include "parts.h"
#include "serial_fixture.h"

#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            printf("%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
            failures++;                                                   \
        }                                                                 \
    } while (0)

/*
 * Footprints the fixture parts name ("footprint" is required on every part
 * but a supply): T<n> is an n-pin dual, BGA2 holds balls A1 B2, BGA4 the
 * 2 x 2 grid.
 */
#define FIXTURE_DUAL(ID, PINS, EXTRA) \
    "{'id':'" ID "','description':'d','style':'dual','pins':" PINS EXTRA ",'body':{'width':1,'height':1}," \
    "'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"
#define FIXTURE_FOOTPRINTS \
    "{'format':'schematical-footprints','version':1,'footprints':[" \
    FIXTURE_DUAL("T1", "1", ",'perSide':1") "," FIXTURE_DUAL("T2", "2", "") "," \
    FIXTURE_DUAL("T4", "4", "") "," FIXTURE_DUAL("T5", "5", ",'perSide':3") "," \
    FIXTURE_DUAL("T6", "6", "") "," FIXTURE_DUAL("T26", "26", "") "," \
    FIXTURE_DUAL("T40", "40", "") "," \
    "{'id':'BGA2','description':'d','style':'grid','pins':2,'rows':2,'columns':2,'pitch':0.5," \
    "'balls':['A1','B2'],'body':{'width':1,'height':1},'pad':{'width':0.2,'height':0.2,'shape':'round'}}," \
    "{'id':'BGA4','description':'d','style':'grid','pins':4,'rows':2,'columns':2,'pitch':0.5," \
    "'balls':['A1','A2','B1','B2'],'body':{'width':1,'height':1},'pad':{'width':0.2,'height':0.2,'shape':'round'}}]}"

#define HEAD(KIND) "{'format':'schematical-parts','version':1,'kind':'" KIND "','devices':["
#define TAIL "]}"

#define VCC_DEVICE \
    "{'id':'5V','function':'5V','description':'d','drawer':[]," \
    "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5," \
    "'parts':[{'name':'5V','package':'','pinout':'pin'}]}"
#define GND_DEVICE \
    "{'id':'GND','function':'GND','description':'d','drawer':[],'note':'n'," \
    "'pinouts':{'pin':[{'number':1,'name':'GND','role':'ground','group':'default'}]},'volts':0," \
    "'parts':[{'name':'GND','package':'','pinout':'pin','flags':['obsolete','military']}]}"

#define LED_DEVICE(ANODE2) \
    "{'id':'led-red','function':'LED','description':'d','drawer':['Indicators','LED']," \
    "'pinouts':{'two-pin':[{'number':1,'name':'K','role':'input'},{'number':2,'name':'A','role':'input'}]," \
    "'dip4':[{'number':1,'name':'K','role':'input'},{'number':2,'name':'" ANODE2 "','role':'input'}," \
    "{'number':3,'name':'NC','role':'nc'},{'number':4,'name':'NC','role':'nc'}]}," \
    "'colors':['Red'],'anode':'A','cathode':'K'," \
    "'parts':[{'name':'LED-RED-0603','package':'0603','pinout':'two-pin','footprint':'T2'}," \
    "{'name':'LED-RED-DIP','package':'DIP','pinout':'dip4','footprint':'T4'}]}"
#define FF_PINS \
    "'pinouts':{'p6':[{'number':1,'name':'D','role':'input'},{'number':2,'name':'CLK','role':'input'}," \
    "{'number':3,'name':'Q','role':'output'},{'number':4,'name':'QN','role':'output'}," \
    "{'number':5,'name':'GND','role':'ground'},{'number':6,'name':'VCC','role':'power'}]},"
#define SRAM_PINS_DEVICE(GRADE, DQ_EXTRA, VCC_EXTRA) \
    HEAD("sram") "{'id':'m','function':'1 KB','description':'d','drawer':['Memory','SRAM']," \
    "'pinouts':{'p6':[{'number':1,'name':'A0','role':'input','group':'address'}," \
    "{'number':2,'name':'DQ0','role':'inout'" DQ_EXTRA "},{'number':3,'name':'CE#','role':'input'}," \
    "{'number':4,'name':'OE#','role':'input'},{'number':5,'name':'WE#','role':'input'}," \
    "{'number':6,'name':'VCC','role':'power'" VCC_EXTRA "}]}," \
    "'address':['A0'],'data':['DQ0'],'chipEnable':['CE#'],'chipEnableHigh':[]," \
    "'outputEnable':'OE#','writeEnable':'WE#','byteEnable':[]," \
    "'grades':{'55':{'trc':55,'taa':55,'twpOeLow':45,'note':'n','vcc':[2.7,5.5]}}," \
    "'parts':[{'name':'M1','package':'DIP','pinout':'p6','footprint':'T6','grade':'" GRADE "'}]}" TAIL
#define SRAM_DEVICE(GRADE) SRAM_PINS_DEVICE(GRADE, "", "")
#define BALL_LED_PINS "{'ball':'A1','name':'K','role':'input'},{'ball':'B2','name':'A','role':'input'}"

extern int failures;
char *json(const char *text);
void load_fixture_footprints(PartLibrary *library);
void expect_fail(const char *name, const char *text, const char *expect);
int load(PartLibrary *library, const char *text, char **error);

#endif
