/*
 * erc_more_test (plan-68-L): the ERC's order of problems across nets, a No
 * Connect pin among the pins it compares, and a supply part's pin that is
 * neither power nor ground; the memory image reader on a hex line with
 * trailing spaces and on a refusal with nowhere to put the message.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "erc.h"
#include "memory_image.h"
#include "parts.h"

static int failures;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static ErcPin pin(size_t net, ErcPinType type) {
    ErcPin p;

    memset(&p, 0, sizeof p);
    p.net = net;
    p.type = type;
    return p;
}

static void check_erc(void) {
    /* Net 1's two outputs come first in the pins, net 0's after a No
       Connect pin on it: problems are ordered by net all the same. */
    ErcPin pins[5] = {pin(1, ERC_OUTPUT), pin(1, ERC_OUTPUT), pin(0, ERC_NO_CONNECT), pin(0, ERC_OUTPUT),
                      pin(0, ERC_OUTPUT)};
    ErcProblem out[8];
    size_t count = erc_check(pins, 5, 2, NULL, ERC_DEFAULT_PIN_MAP, out, 8);
    size_t i, pin_to_pin = 0;

    expect(count >= 2 && count != ERC_CHECK_OUT_OF_MEMORY, "two output-to-output problems (and the No Connect's)");
    for (i = 1; i < count && i < 8; i++) {
        expect(out[i - 1].net <= out[i].net, "ordered by net");
    }
    for (i = 0; i < count && i < 8; i++) {
        if (out[i].kind == ERC_PIN_TO_PIN) {
            pin_to_pin++;
            expect(out[i].pin != 2 && out[i].other != 2, "the No Connect pin is compared with nothing");
        }
    }
    expect(pin_to_pin == 2, "one output-to-output problem on each net");
}

static void check_supply_pin(void) {
    static const char JSON[] =
        "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"supply\",\"devices\":[{\"id\":\"S\","
        "\"function\":\"S\",\"description\":\"d\",\"drawer\":[],\"pinouts\":{\"p2\":[{\"number\":1,\"name\":\"V\","
        "\"role\":\"input\"}]},\"volts\":5,"
        "\"parts\":[{\"name\":\"S\",\"package\":\"\",\"pinout\":\"p2\"}]}]}";
    PartLibrary *library = part_library_create();
    char *error = NULL;
    ErcPinType type = ERC_OUTPUT;

    if (load_parts(library, JSON, "s.json", &error) != 0) {
        printf("FAIL: the supply fixture: %s\n", error != NULL ? error : "?");
        failures++;
    } else {
        expect(erc_pin_type(part_library_find(library, "S"), 0, &type) == 1 && type == ERC_PASSIVE,
               "a supply's pin that is neither power nor ground is passive");
    }
    free(error);
    part_library_destroy(library);
}

static void check_memory_image(void) {
    static const char HEX[] = ":0100000041BE   \r\n:00000001FF\n";
    static const char BAD[] = ":zz\n";
    uint32_t values[4];

    expect(memory_image_parse((const unsigned char *)HEX, strlen(HEX), MEMORY_IMAGE_INTEL_HEX, 4, 8, values, NULL) == 0 &&
               values[0] == 0x41 && values[1] == 0xFF,
           "a hex line with trailing spaces reads");
    expect(memory_image_parse((const unsigned char *)BAD, strlen(BAD), MEMORY_IMAGE_INTEL_HEX, 4, 8, values, NULL) == -1,
           "a refusal with no place for its message still refuses");
}

int main(void) {
    check_erc();
    check_supply_pin();
    check_memory_image();
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
