import Foundation

extension PartKinds {
    /// Kind → model → one plain sentence for "Which chip does yours behave
    /// like?" (plan-46-M): what the model does, with a familiar part where
    /// there is one. Each follows the model's section of
    /// docs/parts-format.md. `KindGuideTests` wants one for every model.
    static let variantBlurbs: [String: [String: String]] = [
        "arith": [
            "adder": "Adds two binary numbers and a carry, like the 74HC283.",
            "alu181": "The 74181 arithmetic logic unit: 4 bits, 16 logic and 16 arithmetic functions picked by its select pins.",
            "compare": "Compares two binary numbers: equal, greater or less, like the 74HC85 and 74HC688.",
            "parity": "Tells whether an odd or even number of its inputs are high, like the 74HC280.",
            "wallace": "A Wallace-tree slice of full adders for fast multipliers, like the 74LS275.",
        ],
        "buffer": [
            "tristate": "Passes its inputs to its outputs while enabled and lets go of the bus otherwise, like the 74HC125, 74HC244 and 74HC541.",
            "transceiver": "A two-way bus buffer with a direction pin, like the 74HC245.",
            "registered": "A two-way bus buffer that can also store the data in latches or flip-flops, like the 74HC646 and 74HC543.",
            "translator": "Shifts logic levels between two supply voltages, like the 74LVC1T45 and TXU0104.",
        ],
        "capacitor": [
            "ceramic": "A ceramic (MLCC) capacitor: small, not polarized, the usual 100 nF decoupling part.",
            "aluminum": "An aluminum electrolytic capacitor: large values, polarized, a can with a marked negative side.",
            "aluminum-polymer": "A polymer aluminum capacitor: polarized, with lower resistance than a wet electrolytic.",
            "tantalum": "A tantalum capacitor: polarized, compact, with a marked positive end.",
            "tantalum-polymer": "A polymer tantalum capacitor: polarized, with lower resistance than a plain tantalum.",
            "niobium-oxide": "A niobium oxide capacitor: polarized, used in place of tantalum.",
        ],
        "converter": [
            "adc": "An analog-to-digital converter: measures a voltage and reports it as a number over SPI, I2C or a parallel bus.",
            "dac": "A digital-to-analog converter: takes a number over its bus and puts out the matching voltage.",
        ],
        "counter": [
            "sync": "A counter whose bits all change on the same clock edge, with load, enable and carry, like the 74HC161 and 74HC193.",
            "ripple": "A counter made of divide-by-two stages that clock one another, like the 74HC393, 74HC4020 and 74HC4040.",
            "johnson": "A counter that turns on one output at a time in turn, like the CD4017.",
            "seg7": "A decade counter that drives a 7-segment digit directly, like the CD4026 and CD4033.",
            "divn": "A divider whose ratio is set on its pins, like the CD4059.",
            "rate": "A rate multiplier: passes a chosen fraction of its clock pulses, like the 7497 and CD4089.",
        ],
        "decoder": [
            "binary": "Turns a binary address into one active output, like the 74HC138, 74HC139 and 74HC154.",
            "priority": "Reports the number of the highest active input, like the 74HC148 and 74HC147.",
            "seg7": "Turns a BCD digit into 7-segment drive, like the 7447 and CD4511's decoder.",
        ],
        "diode": [
            "rectifier": "A general-purpose rectifier for power, like the 1N4007.",
            "fast": "A fast-recovery rectifier for switching supplies, like the UF4007.",
            "switching": "A small, fast signal diode, like the 1N4148.",
            "schottky": "A Schottky diode: low forward drop and fast, like the 1N5819 and BAT54.",
            "zener": "A Zener diode: holds a set voltage in reverse, like the 1N4733 or BZX84.",
            "bridge": "A bridge rectifier: four diodes in one package that turn AC into DC.",
        ],
        "fuse": [
            "fuse": "A fuse: a one-shot link that blows when too much current flows, like a 5 x 20 mm glass fuse.",
            "ptc": "A PTC resettable fuse: a polymer link that opens on overcurrent and recovers when it cools, like the Bourns MF series.",
        ],
        "latch": [
            "transparent": "Follows its data input while its latch-enable is active and holds it otherwise, like the 74HC373 and 74HC573.",
            "addressable": "Eight single-bit latches, one picked by an address, like the 74HC259.",
            "sr": "Set-reset latches: one input sets the output, another clears it, like the 74LS279 and CD4043.",
            "bcd7seg": "Latches a BCD digit and drives a 7-segment display, like the 74HC4511.",
            "bus-mux": "A 12-bit bus exchanger with a latch on each of its three ports, like the 74ALVCH16260.",
        ],
        "mux": [
            "select": "Passes the one data input its select pins name, like the 74HC151, 74HC153 and 74HC157.",
            "stored": "A data selector with a latch or register on its inputs or its output, like the 74HC354 and 74LS298.",
            "regfile": "A small memory with separate write and read addresses, like the 74HC670 4-by-4 register file.",
        ],
        "pio": [
            "pia6521": "The 6521 / 6821 peripheral interface adapter: two 8-bit ports with handshake lines.",
            "via6522": "The 6522 versatile interface adapter: two 8-bit ports, two timers and a shift register.",
        ],
        "register": [
            "parallel": "A row of D flip-flops loaded together on a clock edge, like the 74HC273, 74HC374 and 74HC574.",
        ],
        "crystal": [
            "crystal": "A quartz crystal: two terminals that give a clock input its frequency, like an HC-49 crystal.",
            "resonator": "A ceramic resonator with built-in load capacitors and a third, common pin.",
        ],
        "powerswitch": [
            "load": "A load switch that connects its input to its output while enabled, like the TPS22918.",
            "limited": "A current-limited power switch, like a USB power switch.",
            "ideal-diode": "An integrated ideal diode that passes its input to the output, like the LM66100.",
        ],
        "serial": [
            "ns16550": "The 16450 / 16550 UART family and the I2C and SPI bridges built on it: the PC's serial port chip.",
            "max3100": "Maxim's MAX3100 and MAX3140: a UART you talk to over SPI.",
            "max310x": "Maxim's MAX3107, MAX3108, MAX3109 and MAX14830: SPI or I2C UARTs with 128-word FIFOs.",
            "sc18is604": "NXP's SC18IS604: an SPI-to-I2C bridge (no UART).",
            "usbuart": "A USB-to-UART bridge such as the CP2102 or MCP2200; the USB side is the app's terminal.",
            "ay1013": "A pin-parallel UART with no register bus, like the AY-5-1013A, COM8017 and HD-6402.",
            "i8251": "Intel's 8251A USART.",
            "i82510": "Intel's 82510 asynchronous serial controller.",
            "acia6551": "The 6551 ACIA, the 6502 family's serial chip.",
            "hs82c52": "The 82C52 serial controller interface, and the HD-6406.",
            "com78c80x": "SMC's COM78C804 and COM78C802 multi-line UARTs.",
        ],
        "serialmem": [
            "eeprom": "A serial EEPROM you read and write over SPI a page at a time, like the AT25040B, 25LC256 and M95320.",
            "fram": "A serial F-RAM: non-volatile, written at bus speed with no write delay, like the FM25L16B and CY15B102Q.",
            "flash": "A serial NOR flash: erased by sector or chip, then programmed a page at a time, like the AT25DF021A and AT25XE021A.",
            "dataflash": "An Adesto / Atmel DataFlash with page buffers, like the AT45DB; listed for its facts, not simulated.",
            "pageeeprom": "A serial EEPROM organized in large pages, listed for its facts, not simulated.",
        ],
        "shift": [
            "shift": "A shift register: serial in or out, parallel in or out, like the 74HC595, 74HC165 and 74HC164.",
            "ls673": "The 74LS673 / 74LS674 16-bit shift registers with a serial data pin.",
            "tap64": "The CD4517B: two 64-stage shift registers with taps at 16, 32, 48 and 64.",
            "bus-register": "The CD4034B 8-bit bidirectional bus register.",
            "fifo": "A first-in first-out buffer, 4 bits wide and 16 words deep, like the CD40105B.",
            "serial-in": "The SN74LV8153 serial-to-parallel interface: a UART-style input that sets 8 outputs.",
        ],
    ]
}
