# avr8js 0.21.1 (the oracle)

The JavaScript of [avr8js](https://github.com/wokwi/avr8js), an AVR
simulator by Uri Shaked and contributors (MIT, see `LICENSE`).

- Version: 0.21.1
- Tarball: `https://registry.npmjs.org/avr8js/-/avr8js-0.21.1.tgz`
- sha256: `febfe96e73a840cdb6cc6e583433af2badb8c85421f38545cf5d802802ed35e8`
- Source commit: `wokwi/avr8js` `bee6f0a94e0e27786f6bc21aee3c775849fb50fd`
- Contents: the 18 `.js` files of the tarball's `package/dist/cjs/`, at
  their paths under `dist/cjs/`, and the tarball's `LICENSE`. Unmodified.
  The `.d.ts` files and `dist/esm/` are left out.

The simulator's AVR core (`backend/src/models/avr_*.c`) is a C port of this
code. This copy is what the port is checked against: only
`backend/tools/avr8js-oracle/oracle.js` loads it, from the tests
(`avr_core_test`, `avr_periph_test`) when CMake finds `node`.

It sits outside `backend/src/`, so the app does not bundle it.
