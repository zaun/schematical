# Backend

The backend is a C library: an event-driven logic simulator plus a part
library loaded from JSON. The app (and the tests) use two public headers in
`backend/include/`:

- `parts.h` — the part library: load the part files, look parts up, read
  their facts as C structs.
- `circuit.h` — build a circuit from library parts, run it, read nets,
  lights and memory back.

Part data lives in `schematical/schematical/Parts/<kind>.json`, one file per
kind; the format is in [parts-format.md](parts-format.md).

## Using it

```c
#include "parts.h"
#include "circuit.h"

/* 1. Load the part files (the app reads its bundle resources; tests use
      tests/support/read_file.c). Each call takes one file's text. */
PartLibrary *library = part_library_create();
char *error = NULL;
if (load_parts(library, gate_json_text, "gate.json", &error) != 0) {
    fprintf(stderr, "%s\n", error);   /* e.g. gate.json: devices[3] (74_00): ... */
    free(error);
}

/* 2. Read a part's facts. */
const Part *part = part_library_find(library, "SN74HC00N");
/* part->kind == PART_GATE, part->logic->family == "HC", part->pins[...] */

/* 3. Build and run a circuit. Pins are named by designator ("1", "A1"). */
Circuit *c = circuit_create(library);
size_t vcc, gnd, led, anode, cathode;
circuit_add_part(c, "VCC", &vcc);
circuit_add_part(c, "GND", &gnd);
circuit_add_part(c, "LED-RED-0603", &led);
circuit_add_net(c, &anode);
circuit_add_net(c, &cathode);
circuit_connect(c, vcc, "1", anode);
circuit_connect(c, led, "2", anode);    /* LED pin 2 = A */
circuit_connect(c, led, "1", cathode);  /* LED pin 1 = K */
circuit_connect(c, gnd, "1", cathode);
circuit_start(c);
circuit_advance(c, 1000000);            /* 1 us, in picoseconds */

/* 4. Read it back. */
CircuitValue v = circuit_net_value(c, anode);          /* CIRCUIT_1 */
unsigned char lit;
circuit_part_lights(c, led, &lit, 1);                  /* 1 */

circuit_destroy(c);
part_library_destroy(library);         /* the library outlives its circuits */
```

Notes:

- Building calls (`circuit_add_part`, `_add_net`, `_connect`) work only
  before `circuit_start`; `circuit_set_switch` works before and after.
- `circuit_advance` returns 0 once the run has stopped (a short circuit is
  `"smoke: ..."`, and `circuit_shorted_net` names the net).
- Lights (`circuit_part_lights`) are LEDs (one light) and 7-segment
  displays (`PartDisplay.leds` order); a light counts as on if it was on at
  any time during the last `circuit_advance`, so multiplexed digits show.
- SRAM contents: `circuit_memory_size`, `circuit_memory_read` (a 0 bit in
  `known` is X).

## Layout

```
backend/
  include/                 public API: parts.h, circuit.h
  src/
    engine/                sim.c sim.h net.c net.h (the event engine)
    library/               library.c (load_parts), kinds.c (KINDS[]),
                           kind.h + kind_util.c (the kind interface)
    models/                one file per kind: gate.c dff.c jkff.c sram.c
                           switch.c led.c display7.c oscillator.c supply.c,
                           and lights.c (on-time tracking for lights)
    circuit.c
    third_party/cjson/     cJSON v1.7.18
  tests/                   CTest programs; golden/ holds the regression
                           fixtures (one listing line and one trace hash
                           per part)
```

The app compiles `backend/src/` directly (an Xcode synchronized folder).

## Adding a kind

A new kind of part is added the same way every time:

1. `backend/include/parts.h`: a `PART_<KIND>` value and, if the app needs
   facts about it, an info struct and its pointer on `Part`.
2. `backend/src/models/<kind>.c`: the `PartKindOps` (device and part keys,
   `load_device`, `bind_pinout`, `load_part`) and the engine callbacks
   (`SimComponentType`; the engine config is the `const Part *`, model data
   through `part_internal(part)`). Use the `KindContext` helpers in
   `kind.h` for keys, pins by name, times and errors.
3. One line in `KINDS[]` (`backend/src/library/kinds.c`).
4. `schematical/schematical/Parts/<kind>.json`, and its section in
   `docs/parts-format.md`.
5. The app picks a symbol for the new `PartKind`.

Nothing else: the loader, the listing, the circuit API and the app's
drawer work from the kind table and the `Part` structs.

## Tests

`cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build && ctest --test-dir /tmp/logic-build`

- `library_check` — the loader's rules, every rejection case.
- `listing_check`, `trace_check` — every part's facts and a hash of its
  simulated behavior against `tests/golden/` (recorded from the original C
  tables before the move to JSON). A change to a part file that changes
  facts or behavior, or adds a part, shows up here. For an intended change,
  regenerate the goldens and review the diff:

  ```
  /tmp/logic-build/listing_check --print > backend/tests/golden/parts.txt
  /tmp/logic-build/trace_check --print > backend/tests/golden/traces.txt
  git diff --stat backend/tests/golden
  ```
- `switch_test`, `sram_test`, `display7_test` — model benches.
- `circuit_check` — the circuit API.
