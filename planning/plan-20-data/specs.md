# plan-20 device specs

One section per device: what the datasheet reading (plan-20-A Phase 2) found. The full
machine-readable spec is `specs/<id>.json` (pinouts, pinout rules per package code, model
keys, timing per family, part list); the readers' notes, "does not fit" and "not this plan"
lists and the function tables of registered devices are in `reports/G<nn>.md`.

## 29FCT52

- Kind / model: `buffer` / `registered`; function "Transceiver"; 8-bit registered transceiver, 3-state outputs (two back-to-back registers with clock enables)
- Report: reports/G08.md; parts added: 2
- PDFs: cy29fct52t
- Pinouts: `p24` (24 pins) for DBQ, DW
- Source: Am2952 equivalent. Pinout cy29fct52t p.1 (Q = DBQ QSOP and SO = DW SOIC, same 24-pin layout); pin description p.1; function tables and logic diagram p.2 (register: CE low + CP rising = load, CE high = hold; OE low = outputs enabled, high = Z). The A register takes A0-A7 and drives B0-B7 (OEA), the B register takes B0-B7 and drives A0-A7 (OEB); no real-time path. Timing p.5, VCC p.3.

```json
{"units": [{"a": ["A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"], "b": ["B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7"], "ab": {"store": "register", "clk": {"pin": "CPA", "edge": "rising"}, "ce": {"pin": "CEA", "active": "low"}, "oe": {"pin": "OEA", "active": "low"}}, "ba": {"store": "register", "clk": {"pin": "CPB", "edge": "rising"}, "ce": {"pin": "CEB", "active": "low"}, "oe": {"pin": "OEB", "active": "low"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 6.3 | 6.3 | [4.75, 5.25] |  |  |  | cy29fct52t max tPLH/tPHL CPA,CPB -> A,B, -40 to 85 C (no typ printed) (p.5). VCC 4.75-5.25 V (p.3) |

## 74_162543

- Kind / model: `buffer` / `registered`; function "Transceiver"; 16-bit latched transceiver, 3-state outputs, series damping resistors
- Report: reports/G07.md; parts added: 0
- PDFs: cy74fct162543t
- Pinouts: `p56` (56 pins) for CY-PA, CY-PV
- Source: Pinout cy74fct162543t p.2 (SSOP/TSSOP 56). Function table p.2: per direction, CEAB low and LEAB low = latch transparent, LEAB high (or CEAB high) = storing; B drives the latch output only while CEAB and OEAB are both low, else high Z. All of CEAB/LEAB/OEAB (and BA counterparts) active low. CY74FCT162543T has 24-mA balanced outputs with series resistors (not modeled).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "ab": {"store": "latch", "le": {"pin": "1LEAB", "active": "low"}, "ce": {"pin": "1CEAB", "active": "low"}, "oe": {"pin": "1OEAB", "active": "low"}}, "ba": {"store": "latch", "le": {"pin": "1LEBA", "active": "low"}, "ce": {"pin": "1CEBA", "active": "low"}, "oe": {"pin": "1OEBA", "active": "low"}}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "ab": {"store": "latch", "le": {"pin": "2LEAB", "active": "low"}, "ce": {"pin": "2CEAB", "active": "low"}, "oe": {"pin": "2OEAB", "active": "low"}}, "ba": {"store": "latch", "le": {"pin": "2LEBA", "active": "low"}, "ce": {"pin": "2CEBA", "active": "low"}, "oe": {"pin": "2OEBA", "active": "low"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 8.5 | 8.5 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B transparent mode, CY74FCT162543T base grade, no typ printed (cy74fct162543t p.5; LE->B 12.5). VCC 5 V +-10% (p.1-2) |

## 74_162646

- Kind / model: `buffer` / `registered`; function "Transceiver"; 16-bit registered bus transceiver, 3-state outputs with series resistors (balanced 24 mA drivers)
- Report: reports/G08.md; parts added: 0
- PDFs: cy74fct162646t
- Pinouts: `p56` (56 pins) for CY-PV, CY-PA, DL, DGG
- Source: CY74FCT162646T ('646 function, two 8-bit units). Pinout cy74fct162646t p.2 (SSOP O56 = PV / DL and TSSOP Z56 = PA / DGG share it); pin description p.2; function table p.3 (OE high = isolation, A and B stored on CLK rising; OE low + DIR low = B side to A, SBA selects stored (H) or real-time (L); OE low + DIR high = A side to B, SAB selects stored (H) / real-time (L); data is clocked in on every CLK rising edge whatever OE/DIR). Logic block diagram p.1. Timing p.6, VCC 5 V +-10% p.4.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": {"pin": "1OE", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "1CLKAB", "edge": "rising"}, "select": {"pin": "1SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "1CLKBA", "edge": "rising"}, "select": {"pin": "1SBA", "active": "high"}}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": {"pin": "2OE", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "2CLKAB", "edge": "rising"}, "select": {"pin": "2SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "2CLKBA", "edge": "rising"}, "select": {"pin": "2SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 6.3 | 6.3 | [4.5, 5.5] |  |  |  | cy74fct162646t max tPLH/tPHL clock -> bus, AT speed grade (CT 5.7; bus-to-bus AT 6.3, CT 5.4), -40 to 85 C (no typ printed) (p.6). VCC 5 V +-10% (Operating Range, p.4) |

## 74_162952

- Kind / model: `buffer` / `registered`; function "Transceiver"; 16-bit registered bus transceiver, 3-state outputs, balanced 24-mA drivers with series resistors
- Report: reports/G06.md; parts added: 0
- PDFs: sccs065c
- Pinouts: `tssop56` (56 pins) for CY-PA
- Source: Pinout sccs065c p.1 (SSOP/TSSOP-56; PA = TSSOP, TI DGG): 1OEAB 1, 1CLKAB 2, 1CEAB 3, ... 2CEAB 26, 2CLKAB 27, 2OEAB 28, 2OEBA 29, 2CLKBA 30, 2CEBA 31, ... 1CEBA 54, 1CLKBA 55, 1OEBA 56; VCC 7/22/35/50, GND 4/11/18/25/32/39/46/53. Pin description p.2: OEAB, OEBA, CEAB, CEBA all active low. Function table p.2 (A-to-B; B-to-A symmetric): CEAB L and CLKAB rising stores A; CEAB H or no edge holds; OEAB L drives B with the stored data, OEAB H = Z. 162H952 has bus hold on A/B (not modeled). Two independent 8-bit units.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "ab": {"store": "register", "clk": {"pin": "1CLKAB", "edge": "rising"}, "ce": {"pin": "1CEAB", "active": "low"}, "oe": {"pin": "1OEAB", "active": "low"}}, "ba": {"store": "register", "clk": {"pin": "1CLKBA", "edge": "rising"}, "ce": {"pin": "1CEBA", "active": "low"}, "oe": {"pin": "1OEBA", "active": "low"}}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "ab": {"store": "register", "clk": {"pin": "2CLKAB", "edge": "rising"}, "ce": {"pin": "2CEAB", "active": "low"}, "oe": {"pin": "2OEAB", "active": "low"}}, "ba": {"store": "register", "clk": {"pin": "2CLKBA", "edge": "rising"}, "ce": {"pin": "2CEBA", "active": "low"}, "oe": {"pin": "2OEBA", "active": "low"}}}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 10 | 10 | [4.5, 5.5] |  |  |  | max tPLH/tPHL CLKAB/CLKBA -> B/A, CY74FCT162H952AT (sccs065c p.5; no typical printed; the E speed grade of the orderables is not in the table, 162H952 CT grade is 6.3 max); VCC 5 V +-10% (p.2) |

## 74_16543

- Kind / model: `buffer` / `registered`; function "Transceiver"; 16-bit latched (registered) bus transceiver, 3-state outputs
- Report: reports/G08.md; parts added: 16
- PDFs: 74act16543, sn74abt16543, sn74alvch16543, sn74lvch16543a, sn74lvt16543, sn74lvth16543, sn74lvth16543-ep
- Pinouts: `p56` (56 pins) for DL, DGG
- Source: Standard Widebus 56-pin '16543 pinout, identical in 74act16543 p.1, sn74abt16543 p.1, sn74alvch16543 p.1, sn74lvch16543a p.1, sn74lvt16543 p.1, sn74lvth16543 p.1, sn74lvth16543-ep p.1. Function table (each 8-bit section, A to B shown; B to A uses CEBA/LEBA/OEBA) sn74abt16543 p.2: CEAB high = Z; OEAB high = Z; CEAB low + LEAB high + OEAB low = B holds B0 (latched); CEAB low + LEAB low + OEAB low = B follows A. Description p.1: CEAB must be low both to enter data from A and to output data at B; LEAB low (with CEAB low) = transparent, LEAB rising = store. Model: ce gates both the latch and the output (the destination drives only while CE and OE are both low). Timing A or B -> B or A.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "ab": {"store": "latch", "le": {"pin": "1LEAB", "active": "low"}, "ce": {"pin": "1CEAB", "active": "low"}, "oe": {"pin": "1OEAB", "active": "low"}}, "ba": {"store": "latch", "le": {"pin": "1LEBA", "active": "low"}, "ce": {"pin": "1CEBA", "active": "low"}, "oe": {"pin": "1OEBA", "active": "low"}}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "ab": {"store": "latch", "le": {"pin": "2LEAB", "active": "low"}, "ce": {"pin": "2CEAB", "active": "low"}, "oe": {"pin": "2OEAB", "active": "low"}}, "ba": {"store": "latch", "le": {"pin": "2LEBA", "active": "low"}, "ce": {"pin": "2CEBA", "active": "low"}, "oe": {"pin": "2OEBA", "active": "low"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ACT | 6.9 | 7.3 | [4.5, 5.5] |  |  |  | 74act16543 typ tPLH/tPHL A or B -> B or A at 5 V, 25 C, 50 pF (p.7). VCC 4.5-5.5 V (p.5) |
| ABT | 2.5 | 2.7 | [4.5, 5.5] |  |  |  | sn74abt16543 typ tPLH/tPHL A or B -> B or A at 5 V, 25 C, 50 pF, SN74 table (p.7). VCC 4.5-5.5 V (p.5) |
| ALVC | 4.3 | 4.3 | [1.65, 3.6] |  |  |  | sn74alvch16543 max tpd A or B -> B or A at 3.3 V +-0.3 V, 50 pF (no typ printed) (p.7). VCC 1.65-3.6 V, VI to VCC (p.5) |
| LVC | 5.4 | 5.4 | [1.65, 3.6] | 5.5 |  |  | sn74lvch16543a max tpd A or B -> B or A at 3.3 V +-0.3 V, 50 pF (no typ printed) (p.7). VCC 1.65-3.6 V, VI to 5.5 V (p.5) |
| LVT | 2.7 | 2.9 | [2.7, 3.6] | 5.5 |  |  | sn74lvt16543 typ tPLH/tPHL A or B -> B or A at 3.3 V +-0.3 V, 25 C, 50 pF, SN74 columns (p.6); sn74lvth16543 2.3/2.1 (p.7), sn74lvth16543-ep 2.3/2.1 (p.6). VCC 2.7-3.6 V, VI to 5.5 V (sn74lvt16543 p.4, sn74lvth16543 p.5, -ep p.4) |

## 74_16646

- Kind / model: `buffer` / `registered`; function "Transceiver"; 16-bit bus transceiver and register, 3-state outputs
- Report: reports/G07.md; parts added: 17
- PDFs: 74act16646, cy74fct16646t, sn74abt16646, sn74alvch16646, sn74avc16646, sn74lvc16646a, sn74lvch16646a, sn74lvt16646, sn74lvth16646
- Pinouts: `p56` (56 pins) for DGG, DGV, DL, CY-PV
- Source: 56-pin pinout identical in 74act16646 p.1, sn74abt16646 p.1, sn74alvch16646 p.1, sn74lvc16646a p.1, sn74lvch16646a p.1, sn74lvt16646 p.1, sn74lvth16646 p.1, sn74avc16646 p.2, cy74fct16646t p.2. Function table sn74lvc16646a p.2 (also 74act16646 p.3, sn74abt16646 p.2, sn74avc16646 p.2, cy74fct16646t p.3): two 8-bit units; registers clock on CLKAB/CLKBA rising edge regardless of OE/DIR; OE low enables, DIR high = A to B, low = B to A; SAB/SBA high = stored data, low = real time.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": {"pin": "1OE", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "1CLKAB", "edge": "rising"}, "select": {"pin": "1SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "1CLKBA", "edge": "rising"}, "select": {"pin": "1SBA", "active": "high"}}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": {"pin": "2OE", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "2CLKAB", "edge": "rising"}, "select": {"pin": "2SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "2CLKBA", "edge": "rising"}, "select": {"pin": "2SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ACT | 8.9 | 9 | [4.5, 5.5] |  |  |  | typ CLKAB/CLKBA -> B/A at 5 V, 25 C (74act16646 p.8). VCC 4.5-5.5 V (p.6) |
| ABT | 3.1 | 3.2 | [4.5, 5.5] |  |  |  | typ CLK -> A/B at 5 V, 25 C, 50 pF (sn74abt16646 p.9). VCC 4.5-5.5 V (p.6) |
| ALVC | 4.5 | 4.5 | [1.65, 3.6] |  |  |  | max tpd CLK -> A/B at 3.3 V, no typ printed (sn74alvch16646 p.8). VCC 1.65-3.6 V, VI 0-VCC (p.6) |
| LVC | 6 | 6 | [1.65, 3.6] | 5.5 |  |  | max tpd CLK -> A/B at 3.3 V, no typ printed (sn74lvc16646a p.7). VCC 1.65-3.6 V, VI 0-5.5 V (p.5); sn74lvch16646a p.5 same range |
| LVT | 3.8 | 3.9 | [2.7, 3.6] | 5.5 |  |  | typ CLK -> A/B at 3.3 V (sn74lvt16646 p.7; sn74lvth16646 p.7 is faster, 2.8/2.8). VCC 2.7-3.6 V, VI max 5.5 V (p.5) |
| AVC | 4 | 4 | [1.2, 3.6] |  |  |  | max tpd CLK -> A/B at 2.5 V, no typ printed (sn74avc16646 p.10). VCC 1.2-3.6 V operating, VI 0-3.6 V (p.8) |
| FCT | 9 | 9 | [4.5, 5.5] |  |  |  | max tPLH/tPHL clock to bus, CY74FCT16646T base grade, no typ printed (cy74fct16646t p.6). VCC 5 V +-10% (p.1) |

## 74_16651

- Kind / model: `buffer` / `registered`; function "Transceiver"; 16-bit bus transceiver and register, inverting, 3-state outputs
- Report: reports/G06.md; parts added: 1
- PDFs: 74act16651
- Pinouts: `dl56` (56 pins) for DL
- Source: Pinout 74act16651 p.1 (DL, SSOP-56): 1OEAB 1, 1CLKAB 2, 1SAB 3, ... 2SAB 26, 2CLKAB 27, 2OEAB 28, 2OEBA 29, 2CLKBA 30, 2SBA 31, ... 1SBA 54, 1CLKBA 55, 1OEBA 56; VCC 7/22/35/50, GND 4/11/18/25/32/39/46/53. Function table p.2 (per 8-bit half): OEAB H enables B outputs, OEBA L enables A outputs; SAB/SBA L real-time, H stored; data stored on CLKAB/CLKBA rising edge regardless of enables. Inverting data paths (p.1 features). Two independent 8-bit units.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "ab": {"store": "register", "clk": {"pin": "1CLKAB", "edge": "rising"}, "oe": {"pin": "1OEAB", "active": "high"}, "select": {"pin": "1SAB", "active": "high"}, "invert": true}, "ba": {"store": "register", "clk": {"pin": "1CLKBA", "edge": "rising"}, "oe": {"pin": "1OEBA", "active": "low"}, "select": {"pin": "1SBA", "active": "high"}, "invert": true}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "ab": {"store": "register", "clk": {"pin": "2CLKAB", "edge": "rising"}, "oe": {"pin": "2OEAB", "active": "high"}, "select": {"pin": "2SAB", "active": "high"}, "invert": true}, "ba": {"store": "register", "clk": {"pin": "2CLKBA", "edge": "rising"}, "oe": {"pin": "2OEBA", "active": "low"}, "select": {"pin": "2SBA", "active": "high"}, "invert": true}}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ACT | 9.1 | 9.1 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL CLKAB/CLKBA -> B/A at 5 V, 25 C (74act16651 p.8; A/B -> B/A 6.6/8 typ); VCC 4.5-5.5 V (p.6) |

## 74_16652

- Kind / model: `buffer` / `registered`; function "Transceiver"; 16-bit bus transceiver and register, 3-state outputs
- Report: reports/G16.md; parts added: 8
- PDFs: 74ac16652, sn74abt16652, sn74lvch16652a, sn74lvth16652
- Pinouts: `p56` (56 pins) for DL, DGG, DGV
- Source: Pinout same in 74ac16652 p.1 (DL), sn74abt16652 p.1 (DL), sn74lvch16652a p.3-4 (DGG/DGV/DL), sn74lvth16652 p.1 (DGG/DL). Two 8-bit sections. Function table (sn74abt16652 p.2; same in 74ac16652 p.2, sn74lvch16652a p.13, sn74lvth16652 p.2): OEAB active high, OEBA active low, SAB/SBA high = stored data, low = real time; data stored on CLKAB/CLKBA rising edge regardless of select/enable.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "ab": {"store": "register", "clk": {"pin": "1CLKAB", "edge": "rising"}, "oe": {"pin": "1OEAB", "active": "high"}, "select": {"pin": "1SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "1CLKBA", "edge": "rising"}, "oe": {"pin": "1OEBA", "active": "low"}, "select": {"pin": "1SBA", "active": "high"}}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "ab": {"store": "register", "clk": {"pin": "2CLKAB", "edge": "rising"}, "oe": {"pin": "2OEAB", "active": "high"}, "select": {"pin": "2SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "2CLKBA", "edge": "rising"}, "oe": {"pin": "2OEBA", "active": "low"}, "select": {"pin": "2SBA", "active": "high"}}}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 7.8 | 7.8 | [3, 5.5] |  |  |  | Typical tPLH/tPHL CLKAB/CLKBA -> B/A at 5 V and 25 C (50 pF) (74ac16652 p.8). VCC 3-5.5 V (74ac16652 p.6) |
| ABT | 3.1 | 3.2 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL CLK -> B/A at 5 V and 25 C (50 pF) (sn74abt16652 p.9). VCC 4.5-5.5 V (sn74abt16652 p.6) |
| LVC | 6.4 | 6.4 | [1.65, 3.6] | 5.5 |  |  | Max tpd CLKAB/CLKBA -> A/B at 3.3 V, -40 to 85 C; no typical given (sn74lvch16652a p.8). VCC 1.65-3.6 V, inputs to 5.5 V (sn74lvch16652a p.6) |
| LVT | 2.7 | 2.8 | [2.7, 3.6] | 5.5 |  |  | Typical tPLH/tPHL CLK -> B/A at 3.3 V and 25 C (50 pF) (sn74lvth16652 p.8). VCC 2.7-3.6 V, VI to 5.5 V (sn74lvth16652 p.6) |

## 74_16952

- Kind / model: `buffer` / `registered`; function "Transceiver"; 16-bit registered transceiver, 3-state outputs
- Report: reports/G09.md; parts added: 9
- PDFs: sn74abt16952, sn74alvch16952, sn74lvt16952, sn74lvth16952
- Pinouts: `p56` (56 pins) for DGG, DL, DGV
- Source: Pinout sn74abt16952 p.1, sn74alvch16952 p.1, sn74lvt16952 p.1, sn74lvth16952 p.1 (DGG/DL/DGV, all the same: nOEAB, nCLKAB, nCLKENAB ... nCLKENBA, nCLKBA, nOEBA). Function table sn74abt16952 p.2: CLKENAB L and CLKAB rising stores A; OEAB L drives B from the register; OEAB H = Z; B-to-A uses CLKENBA, CLKBA, OEBA. Two independent 8-bit units.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "ab": {"store": "register", "clk": {"pin": "1CLKAB", "edge": "rising"}, "ce": {"pin": "1CLKENAB", "active": "low"}, "oe": {"pin": "1OEAB", "active": "low"}}, "ba": {"store": "register", "clk": {"pin": "1CLKBA", "edge": "rising"}, "ce": {"pin": "1CLKENBA", "active": "low"}, "oe": {"pin": "1OEBA", "active": "low"}}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "ab": {"store": "register", "clk": {"pin": "2CLKAB", "edge": "rising"}, "ce": {"pin": "2CLKENAB", "active": "low"}, "oe": {"pin": "2OEAB", "active": "low"}}, "ba": {"store": "register", "clk": {"pin": "2CLKBA", "edge": "rising"}, "ce": {"pin": "2CLKENBA", "active": "low"}, "oe": {"pin": "2OEBA", "active": "low"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.6 | 2.6 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL CLK -> A or B at 5 V, 50 pF (sn74abt16952 p.6); VCC 4.5-5.5 V (p.5) |
| ALVC | 3.9 | 3.9 | [1.65, 3.6] |  |  |  | max tpd CLK -> A or B at 3.3 V (no typical printed) (sn74alvch16952 p.7); VCC 1.65-3.6 V, VI 0-VCC (p.5) |
| LVT | 3.4 | 3.4 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL CLK -> A or B at 3.3 V, 50 pF, SN74LVT16952 (sn74lvt16952 p.6; sn74lvth16952 p.7 gives typ 2.7); VCC 2.7-3.6 V, VI to 5.5 V (sn74lvt16952 p.4, sn74lvth16952 p.5) |

## 74_16952_CE

- Kind / model: `buffer` / `registered`; function "Transceiver"; 16-bit registered transceiver, 3-state outputs
- Report: reports/G09.md; parts added: 4
- PDFs: cy74fct16952t, sn74lvch16952a
- Pinouts: `p56` (56 pins) for DGG, DL, DGV, CY-PV
- Source: Same function and pin positions as 74_16952 but the clock enables are named CEAB/CEBA: sn74lvch16952a p.1 (DGG/DGV/DL) and cy74fct16952t p.1 (SSOP/TSSOP). Function table sn74lvch16952a p.2 / cy74fct16952t p.2: CEAB L and CLKAB rising stores A; OEAB L drives B from the register; OEAB H = Z.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "ab": {"store": "register", "clk": {"pin": "1CLKAB", "edge": "rising"}, "ce": {"pin": "1CEAB", "active": "low"}, "oe": {"pin": "1OEAB", "active": "low"}}, "ba": {"store": "register", "clk": {"pin": "1CLKBA", "edge": "rising"}, "ce": {"pin": "1CEBA", "active": "low"}, "oe": {"pin": "1OEBA", "active": "low"}}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "ab": {"store": "register", "clk": {"pin": "2CLKAB", "edge": "rising"}, "ce": {"pin": "2CEAB", "active": "low"}, "oe": {"pin": "2OEAB", "active": "low"}}, "ba": {"store": "register", "clk": {"pin": "2CLKBA", "edge": "rising"}, "ce": {"pin": "2CEBA", "active": "low"}, "oe": {"pin": "2OEBA", "active": "low"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVC | 6.6 | 6.6 | [1.65, 3.6] | 5.5 |  |  | max tpd CLKAB/CLKBA -> B/A at 3.3 V (no typical printed) (sn74lvch16952a p.6); VCC 1.65-3.6 V, VI 0-5.5 V (p.4) |
| FCT | 6.3 | 6.3 | [4.5, 5.5] |  |  |  | max tPLH/tPHL CLKAB/CLKBA -> B/A, CY74FCT16952CT (fastest grade printed; the E grade of the placed part is not in the PDF) (cy74fct16952t p.5); VCC 5 V +-10% (p.2) |

## 74_2543

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal latched transceiver, 3-state outputs, 25-ohm series output resistors
- Report: reports/G05.md; parts added: 4
- PDFs: cy74fct2543t
- Pinouts: `p24` (24 pins) for DBQ, DW
- Source: Pinout cy74fct2543t p.1 (Q = DBQ QSOP, SO = DW SOIC). Pin description p.2: all of OEAB/OEBA/CEAB/CEBA/LEAB/LEBA active low. Function table p.2 (A->B; B->A uses CEBA/LEBA/OEBA): CEAB H -> storing, B Z; LEAB H -> storing; OEAB H -> B Z; CEAB L, LEAB L, OEAB L -> transparent, B = current A; CEAB L, LEAB H, OEAB L -> B = previous A (latched on LEAB rising).

```json
{"units": [{"a": ["A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"], "b": ["B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7"], "ab": {"store": "latch", "le": {"pin": "LEAB", "active": "low"}, "ce": {"pin": "CEAB", "active": "low"}, "oe": {"pin": "OEAB", "active": "low"}}, "ba": {"store": "latch", "le": {"pin": "LEBA", "active": "low"}, "ce": {"pin": "CEBA", "active": "low"}, "oe": {"pin": "OEBA", "active": "low"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 8.5 | 8.5 | [4.75, 5.25] |  |  |  | max tPLH/tPHL A->B (latch transparent), CY74FCT2543T grade, 50 pF (cy74fct2543t p.5; no typ printed; AT 6.5, CT 5.5; LE->B 12.5); VCC 4.75-5.25 V (p.3) |

## 74_2652

- Kind / model: `buffer` / `registered`; function "Transceiver"; 8-bit registered bus transceiver, 3-state outputs, 25-ohm series output resistors
- Report: reports/G06.md; parts added: 1
- PDFs: cy74fct2652t
- Pinouts: `q24` (24 pins) for DBQ, CY-QC
- Source: Pinout CPAB 1, SAB 2, GAB 3, A1-A8 4-11, GND 12, B8-B1 13-20, GBA 21 (active low), SBA 22, CPBA 23, VCC 24 (cy74fct2652t p.1, Q = QSOP-24 = TI DBQ). Function table cy74fct2652t p.2: GAB H enables B outputs, GBA L enables A outputs; SAB/SBA L = real-time, H = stored; data stored on CPAB/CPBA rising edge regardless of enables. Same function as '652; pins keep the FCT names (CP = clock, G = output enable).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "ab": {"store": "register", "clk": {"pin": "CPAB", "edge": "rising"}, "oe": {"pin": "GAB", "active": "high"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CPBA", "edge": "rising"}, "oe": {"pin": "GBA", "active": "low"}, "select": {"pin": "SBA", "active": "high"}}}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 6.3 | 6.3 | [4.75, 5.25] |  |  |  | max tPLH/tPHL CPAB/CPBA -> B/A, CY74FCT2652AT (cy74fct2652t p.7; no typical printed; CT grade 5.7 max); VCC 4.75-5.25 V (p.5) |

## 74_2952

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and registers, clock enables, 3-state outputs
- Report: reports/G13.md; parts added: 6
- PDFs: sn74abt2952a, sn74lvc2952a, sn74lvth2952
- Pinouts: `p24` (24 pins) for DB, DW, PW
- Source: Pinout: sn74abt2952a p.1 (DB/DW/PW/NT), sn74lvc2952a p.1 (DB/PW), sn74lvth2952 p.1 (DB/DGV/DW/NS/PW): identical 24-pin. Two back-to-back 8-bit registers; A->B: B stores A on CLKAB rising while CLKENAB low, OEAB low drives B with the register (no real-time path); B->A the same with CLKBA/CLKENBA/OEBA (function table sn74abt2952a p.2, sn74lvc2952a p.2, sn74lvth2952 p.2).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "ab": {"store": "register", "clk": {"pin": "CLKAB", "edge": "rising"}, "ce": {"pin": "CLKENAB", "active": "low"}, "oe": {"pin": "OEAB", "active": "low"}}, "ba": {"store": "register", "clk": {"pin": "CLKBA", "edge": "rising"}, "ce": {"pin": "CLKENBA", "active": "low"}, "oe": {"pin": "OEBA", "active": "low"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 3.3 | 4 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL CLKAB/CLKBA->B/A at 5 V, 25 C, 50 pF (sn74abt2952a p.6). VCC 4.5-5.5 V (sn74abt2952a p.4) |
| LVC | 8.2 | 8.2 | [1.65, 3.6] | 5.5 |  |  | max tpd CLK->B/A at 3.3 V +-0.3 V, 50 pF; no typical printed (sn74lvc2952a p.5). VCC 1.65-3.6 V, inputs to 5.5 V (sn74lvc2952a p.3) |
| LVT | 2.9 | 3.1 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL CLK->B/A at 3.3 V, 25 C, 50 pF, SN74LVTH2952 (sn74lvth2952 p.6). VCC 2.7-3.6 V, inputs to 5.5 V (sn74lvth2952 p.4) |

## 74_32543

- Kind / model: `buffer` / `registered`; function "Transceiver"; 36-bit registered bus transceiver (two 18-bit sections, latched), 3-state outputs
- Report: reports/G11.md; parts added: 1
- PDFs: sn74abth32543
- Pinouts: `p100` (100 pins) for PZ
- Source: p100 = PZ LQFP-100 from sn74abth32543 p.1 (read from the drawing: 1-25 left side top to bottom, 26-50 bottom left to right, 51-75 right side bottom to top, 76-100 top right to left). Each 18-bit section is a '543: function table p.3 (A to B shown; B to A same with CEBA/LEBA/OEBA): CEAB H -> B Z; OEAB H -> B Z; CEAB L, LEAB H, OEAB L -> B holds B0 (latched); CEAB L, LEAB L, OEAB L -> B = A (transparent). Description p.2: CEAB must be low to enter data from A or to output data to B; LEAB low = transparent, low-to-high stores. 5962-9557801NXD is the SMD of this PZ part (packaging table p.9). Bus hold not modeled.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "1A9", "1A10", "1A11", "1A12", "1A13", "1A14", "1A15", "1A16", "1A17", "1A18"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "1B9", "1B10", "1B11", "1B12", "1B13", "1B14", "1B15", "1B16", "1B17", "1B18"], "ab": {"store": "latch", "le": {"pin": "1LEAB", "active": "low"}, "ce": {"pin": "1CEAB", "active": "low"}, "oe": {"pin": "1OEAB", "active": "low"}}, "ba": {"store": "latch", "le": {"pin": "1LEBA", "active": "low"}, "ce": {"pin": "1CEBA", "active": "low"}, "oe": {"pin": "1OEBA", "active": "low"}}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "2A9", "2A10", "2A11", "2A12", "2A13", "2A14", "2A15", "2A16", "2A17", "2A18"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8", "2B9", "2B10", "2B11", "2B12", "2B13", "2B14", "2B15", "2B16", "2B17", "2B18"], "ab": {"store": "latch", "le": {"pin": "2LEAB", "active": "low"}, "ce": {"pin": "2CEAB", "active": "low"}, "oe": {"pin": "2OEAB", "active": "low"}}, "ba": {"store": "latch", "le": {"pin": "2LEBA", "active": "low"}, "ce": {"pin": "2CEBA", "active": "low"}, "oe": {"pin": "2OEBA", "active": "low"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 3.5 | 3.5 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->B (transparent) at 5 V, 25 C, 50 pF, SN74ABTH32543 (sn74abth32543 p.7); LE->B typ 4.6/4.3. VCC 4.5-5.5 V, VI 0-VCC (sn74abth32543 p.5) |

## 74_543

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal registered (latched) transceiver, 3-state outputs
- Report: reports/G12.md; parts added: 20
- PDFs: sn54bct543, sn74abt543a, sn74bct543, sn74f543, sn74lvc543a, sn74lvt543, sn74lvth543
- Pinouts: `p24` (24 pins) for DB, DBL, DW, NS, PW; `lccc28` (28 pins) for FK
- Source: Pinout from sn74abt543a p.1 (DB, DW, NT, PW, JT, W) and sn74bct543 p.1, sn74f543 p.1, sn74lvc543a p.1, sn74lvt543 p.1, sn74lvth543 p.1 (DB, DGV, DW, NS, PW) - all the same 24-pin assignment; FK 28-pin from sn54bct543 p.1 (NC 1, 8, 15, 22). Function table sn74abt543a p.2 (same in sn74bct543 p.1, sn74f543 p.1, sn74lvc543a p.2, sn74lvt543 p.2, sn74lvth543 p.2): CEAB low enables both the A-to-B latch and the B outputs; LEAB low = transparent; OEAB low (with CEAB low) drives B. B-to-A identical with CEBA/LEBA/OEBA. Timing is A->B transparent propagation.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "ab": {"store": "latch", "le": {"pin": "LEAB", "active": "low"}, "ce": {"pin": "CEAB", "active": "low"}, "oe": {"pin": "OEAB", "active": "low"}}, "ba": {"store": "latch", "le": {"pin": "LEBA", "active": "low"}, "ce": {"pin": "CEBA", "active": "low"}, "oe": {"pin": "OEBA", "active": "low"}}}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 4.4 | 4.4 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 50 pF (sn74abt543a p.6; LE->B typ 4.1/4.6); VCC 4.5-5.5 V (p.4) |
| BCT | 5.7 | 6.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 50 pF (sn74bct543 p.5; LE->B typ 8.2/8.5); VCC 4.5-5.5 V SN54 and SN74 (p.3) |
| F | 5.1 | 4.6 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 50 pF (sn74f543 p.5; LE->B typ 8.1); VCC 4.5-5.5 V (p.3) |
| LVC | 7 | 7 | [1.65, 3.6] | 5.5 |  |  | max tpd A->B at 3.3 V, 50 pF (sn74lvc543a p.4, no typ printed); VCC 1.65-3.6 V, VI to 5.5 V (p.3) |
| LVT | 2.9 | 3.3 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->B at 3.3 V, 50 pF (sn74lvt543 p.6; sn74lvth543 p.6 typ 2.5/2.5); VCC 2.7-3.6 V, VI to 5.5 V (sn74lvt543 p.4, sn74lvth543 p.4) |

## 74_543_FCT

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal latched registered transceiver, 3-state outputs (FCT, A0-A7/B0-B7 naming)
- Report: reports/G12.md; parts added: 5
- PDFs: cy74fct543t
- Pinouts: `p24` (24 pins) for DBQ, DW, CY-QC
- Source: Separate device only because cy74fct543t names the data pins A0-A7/B0-B7 (74_543 uses A1-A8/B1-B8); same pin positions. Pinout cy74fct543t p.1 (Q = QSOP/DBQ, SO = SOIC/DW); pin description and function table p.2. Timing is A->B transparent propagation.

```json
{"units": [{"a": ["A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"], "b": ["B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7"], "ab": {"store": "latch", "le": {"pin": "LEAB", "active": "low"}, "ce": {"pin": "CEAB", "active": "low"}, "oe": {"pin": "OEAB", "active": "low"}}, "ba": {"store": "latch", "le": {"pin": "LEBA", "active": "low"}, "ce": {"pin": "CEBA", "active": "low"}, "oe": {"pin": "OEBA", "active": "low"}}}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 8.5 | 8.5 | [4.75, 5.25] |  |  |  | max tPLH/tPHL A->B, CY74FCT543T grade, 50 pF (cy74fct543t p.6; AT grade 6.5, CT grade 5.3; no typ printed); VCC 4.75-5.25 V (p.3) |

## 74_646

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and registers, 3-state outputs
- Report: reports/G11.md; parts added: 22
- PDFs: sn54abt646a, sn54lvth646, sn74abt646, sn74abt646a, sn74als646a, sn74as646, sn74hc646, sn74hct646, sn74lvc646a, sn74lvth646
- Pinouts: `p24` (24 pins) for DB, DBL, DGV, DW, NS, NT, PW, JT, W; `lccc28` (28 pins) for FK
- Source: p24 from sn74abt646a p.1 (DB/DGV/DW/NS/NT/PW and SN54 JT/W; 74ABT646ADBLE/SN74ABT646ADBLE are DB SSOP-24 orderables not in the packaging table), sn74abt646 p.1, sn74als646a p.1 (DW/NT/JT), sn74hc646 p.1, sn74hct646 p.1, sn74lvc646a p.1, sn74lvth646 p.1: all identical. lccc28 (FK) from sn74abt646a p.1, sn74lvth646 p.1 (5962-9674801Q3A), sn74hc646 p.1, sn74als646a p.1: NC on 1, 8, 15, 22. Function table (all 646 PDFs): OE H = isolation (both ports read, registers still clock); OE L + DIR L: SBA L = real-time B to A, SBA H = stored B to A; OE L + DIR H: SAB L = real-time A to B, SAB H = stored A to B; CLKAB/CLKBA rising edge stores the A/B bus whatever OE/DIR are. (sn74abt646a p.4, sn74als646a p.3, sn74hc646 p.2, sn74lvc646a p.2, sn74lvth646 p.2).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": {"pin": "OE", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "CLKAB", "edge": "rising"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CLKBA", "edge": "rising"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 4 | 4 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL CLKAB/CLKBA -> B/A at 5 V, 25 C, 50 pF, SN74ABT646A (sn74abt646a p.8); A->B typ 3/3.3. VCC 4.5-5.5 V, SN54 and SN74 (sn74abt646a p.5); SN74ABT646 (sn74abt646) same function and pinout |
| ALS | 30 | 17 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL CLK -> A/B at 4.5-5.5 V, 0-70 C, 50 pF, SN74ALS646A; no typical printed (sn74als646a p.8). VCC 4.5-5.5 V (sn74als646a p.6) |
| AS | 8.5 | 9 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL CLK -> A/B at 4.5-5.5 V, 0-70 C, 50 pF, SN74AS646; no typical printed (sn74als646a p.14, same PDF as sn74as646). VCC 4.5-5.5 V (p.12) |
| HC | 18 | 18 | [2, 6] |  |  |  | Typical tpd CLK -> A/B at 4.5 V, 25 C, 50 pF (sn74hc646 p.7); A->B typ 14. VCC 2-6 V (sn74hc646 p.5) |
| HCT | 18 | 18 | [4.5, 5.5] |  |  |  | Typical tpd CLK -> A/B at 4.5 V, 25 C, 50 pF (sn74hct646 p.6). VCC 4.5-5.5 V (sn74hct646 p.5) |
| LVC | 8.4 | 8.4 | [1.65, 3.6] | 5.5 |  |  | Max tpd CLK -> A/B at 3.3 V, -40 to 85 C, 50 pF, SN74LVC646A; no typical printed (sn74lvc646a p.7). VCC 1.65-3.6 V, VI to 5.5 V (sn74lvc646a p.5) |
| LVT | 3.1 | 3.1 | [2.7, 3.6] | 5.5 |  |  | Typical tPLH/tPHL CLKAB/CLKBA -> B/A at 3.3 V, 25 C, 50 pF, SN74LVTH646 (sn74lvth646 p.7). VCC 2.7-3.6 V, VI to 5.5 V, SN54 and SN74 (sn74lvth646 p.5) |

## 74_646_AC

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and registers, 3-state outputs
- Report: reports/G11.md; parts added: 1
- PDFs: cd74ac646
- Pinouts: `p24` (24 pins) for DW
- Source: CD74AC646 (Harris data, scanned): clocks CAB/CBA, output enable OE, bits A0-A7/B0-B7 (cd74ac646 p.1 terminal assignment: 1 CAB, 2 SAB, 3 DIR, 4-11 A0-A7, 12 GND, 13 B7 ... 20 B0, 21 OE, 22 SBA, 23 CBA, 24 VCC); same positions as 74_646. Function table cd74ac646 p.2 (646 column): Function table (all 646 PDFs): OE H = isolation (both ports read, registers still clock); OE L + DIR L: SBA L = real-time B to A, SBA H = stored B to A; OE L + DIR H: SAB L = real-time A to B, SAB H = stored A to B; CAB/CBA rising edge stores the A/B bus whatever OE/DIR are. Timing cd74ac646 p.5, VCC p.2.

```json
{"units": [{"a": ["A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"], "b": ["B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7"], "dir": {"pin": "DIR", "ab": "high"}, "oe": {"pin": "OE", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "CAB", "edge": "rising"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CBA", "edge": "rising"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 12.3 | 12.3 | [1.5, 5.5] |  |  |  | Max tPLH/tPHL store A data to B bus (clock -> bus) at 5 V, -40 to 85 C, 50 pF; no typical in the table (cd74ac646 p.5; cover typ 5.3 ns is the data path). VCC 1.5-5.5 V AC types (cd74ac646 p.2) |

## 74_646_FCT

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal registered bus transceiver, 3-state outputs
- Report: reports/G11.md; parts added: 7
- PDFs: cy54fct646t, cy74fct646t
- Pinouts: `p24` (24 pins) for DBQ, DW, D; `lccc28` (28 pins) for FK, L
- Source: CY54/74FCT646T print CPAB/CPBA for the clocks and G for the output enable (cy74fct646t p.1-2; same PDF as cy54fct646t); same positions as 74_646 CLKAB/CLKBA/OE. p24 from cy74fct646t p.1 (Q = DBQ, SO = DW, CY54 D). lccc28 = CY54FCT646T L package p.1 (5962-9222301M3A, FK): NC on 1, 8, 15, 22. Function table (all 646 PDFs): G H = isolation (both ports read, registers still clock); G L + DIR L: SBA L = real-time B to A, SBA H = stored B to A; G L + DIR H: SAB L = real-time A to B, SAB H = stored A to B; CPAB/CPBA rising edge stores the A/B bus whatever G/DIR are. (cy74fct646t p.2).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": {"pin": "G", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "CPAB", "edge": "rising"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CPBA", "edge": "rising"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 9 | 9 | [4.75, 5.25] |  |  |  | Max tPLH/tPHL CPAB/CPBA -> A/B, CY74FCT646T (slowest grade; AT 6.3, CT 5.7), -40 to 85 C, 50 pF; no typical printed (cy74fct646t p.8). VCC 4.75-5.25 V CY74, 4.5-5.5 V CY54 (cy74fct646t p.5) |

- Part range: names starting `5962-9222301` (FCT): vcc [4.5, 5.5]

## 74_646_LS

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and registers, 3-state outputs
- Report: reports/G11.md; parts added: 1
- PDFs: sn74ls646
- Pinouts: `p24` (24 pins) for DW, NT, JT
- Source: SN74LS646 (scanned SDLS190A, same file as sdls190a): clocks CAB/CBA and output enable G (sn74ls646 p.1 pinout, DW/NT and SN54 JT); same positions as 74_646. Function table sn74ls646 p.2 (LS646 column): Function table (all 646 PDFs): G H = isolation (both ports read, registers still clock); G L + DIR L: SBA L = real-time B to A, SBA H = stored B to A; G L + DIR H: SAB L = real-time A to B, SAB H = stored A to B; CAB/CBA rising edge stores the A/B bus whatever G/DIR are. SN74LS646NT3 is an NT orderable not in the packaging table (only SN74LS646DW). Timing p.8, VCC p.7.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": {"pin": "G", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "CAB", "edge": "rising"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CBA", "edge": "rising"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 16 | 23 | [4.75, 5.25] |  |  |  | Typical tPLH/tPHL CAB/CBA -> A/B at 5 V, 25 C, 45 pF, RL 667 ohm (sn74ls646 p.8). VCC 4.75-5.25 V SN74, 4.5-5.5 V SN54 (sn74ls646 p.7) |

## 74_647

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and register, open-collector outputs
- Report: reports/G04.md; parts added: 0
- PDFs: dl2:sdls190a
- Pinouts: `p24` (24 pins) for NT
- Source: Image-only PDF sdls190a (SN74LS646 thru SN74LS649). Pinout p.1 (SN74LS' DW or NT, SN54LS' JT): 1 CAB, 2 SAB, 3 DIR, 4-11 A1-A8, 12 GND, 13-20 B8-B1, 21 G (active low), 22 SBA, 23 CBA, 24 VCC. Function table p.2 (LS646/LS647: true data). 'LS647 has open-collector outputs on both the A and B ports (p.1 table, p.6 output schematic, p.9). Registers clock on the low-to-high transition of CAB/CBA whatever the enables.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": {"pin": "G", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "CAB", "edge": "rising"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CBA", "edge": "rising"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": false, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 22 | 28 | [4.75, 5.25] |  |  |  | Typical CAB/CBA -> B/A at 5 V, 25 C (RL 667 ohm, 45 pF), 'LS647 (sdls190a p.10); A->B real time 17/18. VCC 4.75-5.25 V SN74, 4.5-5.5 V SN54; off-state A/B output voltage 5.5 V max (sdls190a p.9) |

## 74_649

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and registers, inverting, open-collector outputs
- Report: reports/G11.md; parts added: 0
- PDFs: sdls190a
- Pinouts: `p24` (24 pins) for DW, NT, JT
- Source: SN74LS649 from sdls190a (SN54/74LS646 thru LS649, scanned): pinout p.1 (CAB, SAB, DIR, A1-A8, GND, B8-B1, G, SBA, CBA, VCC; DW/NT/JT), 'LS649 = open-collector, inverting (p.1 table). Function table p.2 (LS648/LS649 column): same as 646 with complemented data: OE(G) L + DIR H: SAB L = real-time A-bar to B, SAB H = stored A-bar to B; DIR L: SBA L = real-time B-bar to A, SBA H = stored B-bar to A; G H = isolation; CAB/CBA rising stores. Both A and B outputs open collector. SN74LS649NT is not in a packaging table (the PDF lists only SN74LS646DW). Timing p.10, VCC p.9.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": {"pin": "G", "active": "low"}, "ab": {"store": "register", "clk": {"pin": "CAB", "edge": "rising"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CBA", "edge": "rising"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": true, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 17 | 28 | [4.75, 5.25] |  |  |  | Typical tPLH/tPHL CAB/CBA -> A/B at 5 V, 25 C, 45 pF, RL 667 ohm, 'LS649 (sdls190a p.10). VCC 4.75-5.25 V SN74, 4.5-5.5 V SN54 (sdls190a p.9). Open-collector outputs, VOH max 5.5 V |

## 74_651

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal inverting bus transceiver and register, 3-state outputs
- Report: reports/G12.md; parts added: 2
- PDFs: sn74abt651
- Pinouts: `p24` (24 pins) for DW, DBL, DB, PW
- Source: Pinout sn74abt651 p.1 (DB, DW, NT, PW; JT). Function table p.2 and bus-management figure p.3: OEAB high enables B outputs, OEBA low enables A outputs; SAB/SBA high = stored data, low = real time; data stored on CLKAB/CLKBA rising edge regardless of enables. Inverting data paths (p.1 features). Timing is CLK->B.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "ab": {"store": "register", "clk": {"pin": "CLKAB", "edge": "rising"}, "oe": {"pin": "OEAB", "active": "high"}, "select": {"pin": "SAB", "active": "high"}, "invert": true}, "ba": {"store": "register", "clk": {"pin": "CLKBA", "edge": "rising"}, "oe": {"pin": "OEBA", "active": "low"}, "select": {"pin": "SBA", "active": "high"}, "invert": true}}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 4 | 4 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL CLKAB->B at 5 V, 50 pF (sn74abt651 p.8; A->B typ 4/3.3); VCC 4.5-5.5 V (p.6) |

## 74_652

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and registers, real-time or stored data, 3-state outputs
- Report: reports/G13.md; parts added: 19
- PDFs: sn54als652, sn54as652, sn74abt652a, sn74als652a, sn74as652, sn74bct652, sn74hct652, sn74lvc652a, sn74lvth652
- Pinouts: `p24` (24 pins) for DB, DW, PW, NT, JT
- Source: Pinout (24-pin DB/DW/NT/PW/JT): sn74abt652a p.1, sn54als652 / sn74als652a / sn54as652 / sn74as652 (one combined PDF) p.1, sn74bct652 p.1, sn74hct652 p.1, sn74lvc652a p.1, sn74lvth652 p.1 - identical names and numbers. OEAB active high, OEBA active low; SAB/SBA low = real-time, high = stored; registers clock on CLKAB/CLKBA rising regardless of select/enable (function table sn74abt652a p.2, sn74als652a p.2, sn74hct652 p.2, sn74lvth652 p.2). The LS (CAB/GAB names), CD74AC/ACT (CAB, A0-A7 names) and CY74FCT (CPAB/GAB names) versions are separate devices 74_652_LS, 74_652_CD, 74_652_FCT.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "ab": {"store": "register", "clk": {"pin": "CLKAB", "edge": "rising"}, "oe": {"pin": "OEAB", "active": "high"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CLKBA", "edge": "rising"}, "oe": {"pin": "OEBA", "active": "low"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 4 | 4 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL CLK->B/A at 5 V, 25 C, 50 pF, SN74ABT652A (sn74abt652a p.9). VCC 4.5-5.5 V (sn74abt652a p.6) |
| ALS | 30 | 17 | [4.5, 5.5] |  |  |  | max tPLH/tPHL CLKAB/CLKBA->B/A, VCC 4.5-5.5 V, 50 pF, SN74ALS652A; no typical printed (sn74als652a p.11; SN54ALS652 max 35/20). VCC 4.5-5.5 V for SN54ALS652 and SN74ALS652A (sn74als652a p.7) |
| AS | 8.5 | 9 | [4.5, 5.5] |  |  |  | max tPLH/tPHL CLKAB/CLKBA->B/A, VCC 4.5-5.5 V, 50 pF, SN74AS652; no typical printed (sn74as652 p.20). VCC 4.5-5.5 V (sn74as652 p.17) |
| BCT | 6.9 | 6.8 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL CLKAB->B at 5 V, 25 C (sn74bct652 p.6). VCC 4.5-5.5 V (sn74bct652 p.5) |
| HCT | 18 | 18 | [4.5, 5.5] |  |  |  | typ tpd CLKAB/CLKBA->B/A at 4.5 V, 25 C, 50 pF (16 ns at 5.5 V) (sn74hct652 p.6). VCC 4.5-5.5 V (sn74hct652 p.5) |
| LVC | 8 | 8 | [1.65, 3.6] | 5.5 |  |  | max tpd CLK->A/B at 3.3 V +-0.3 V, SN74LVC652A; no typical printed (sn74lvc652a p.7). VCC 1.65-3.6 V, inputs to 5.5 V (sn74lvc652a p.5) |
| LVT | 3.1 | 3.1 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL CLKAB/CLKBA->B/A at 3.3 V, 25 C, 50 pF, SN74LVTH652 (sn74lvth652 p.7). VCC 2.7-3.6 V, inputs to 5.5 V (sn74lvth652 p.5) |

## 74_652_CD

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and registers, real-time or stored data, 3-state outputs (CD74 pin names)
- Report: reports/G13.md; parts added: 1
- PDFs: cd74ac652, cd74act652
- Pinouts: `p24` (24 pins) for M, DW
- Source: Separate from 74_652 because the Harris/RCA datasheet prints different names (CAB/CBA clocks, A0-A7 / B0-B7). cd74ac652 and cd74act652 are the same scanned PDF (CD54/74AC651/652, CD54/74ACT651/652). Pinout: p.1 Terminal Assignment (24-pin, M = SOIC). OEAB active high, OEBA active low, SAB/SBA low = real-time, high = stored; CAB/CBA rising edge stores (function table p.2, '652 column). Timing p.4 (AC) and p.5 (ACT); ROC p.2.

```json
{"units": [{"a": ["A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"], "b": ["B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7"], "ab": {"store": "register", "clk": {"pin": "CAB", "edge": "rising"}, "oe": {"pin": "OEAB", "active": "high"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CBA", "edge": "rising"}, "oe": {"pin": "OEBA", "active": "low"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 12.3 | 12.3 | [1.5, 5.5] |  |  |  | max tPLH/tPHL store A data to B bus (clock->B), 5 V, -40 to 85 C, 50 pF; no typical in the table (cd74ac652 p.4; p.1 quotes 5.3 ns typical propagation delay at 5 V). VCC 1.5-5.5 V (cd74ac652 p.2) |
| ACT | 14.1 | 14.1 | [4.5, 5.5] |  |  |  | max tPLH/tPHL store A data to B bus (clock->B), 5 V, -40 to 85 C, 50 pF; no typical in the table (cd74act652 p.5). VCC 4.5-5.5 V (cd74act652 p.2) |

## 74_652_FCT

- Kind / model: `buffer` / `registered`; function "Transceiver"; 8-bit registered transceiver, real-time or stored data, 3-state outputs (FCT pin names)
- Report: reports/G13.md; parts added: 6
- PDFs: cy74fct652t
- Pinouts: `p24` (24 pins) for DBQ, DW
- Source: Separate from 74_652 because cy74fct652t prints different control names (CPAB/CPBA clocks, GAB/GBA enables). Pinout: cy74fct652t p.1 (Q = QSOP/DBQ, SO = SOIC/DW). GAB active high, GBA active low, SAB/SBA low = real-time, high = stored; CPAB/CPBA rising edge stores regardless of select/enable (function table p.2). Speed grades T/AT/CT share one FCT entry (base T numbers).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "ab": {"store": "register", "clk": {"pin": "CPAB", "edge": "rising"}, "oe": {"pin": "GAB", "active": "high"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CPBA", "edge": "rising"}, "oe": {"pin": "GBA", "active": "low"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 9 | 9 | [4.75, 5.25] |  |  |  | max tPLH/tPHL CPAB/CPBA->B/A, CY74FCT652T grade, -40 to 85 C; no typical printed (AT 6.3, CT 5.7) (cy74fct652t p.7). VCC 4.75-5.25 V (cy74fct652t p.5) |

## 74_652_LS

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and registers, real-time or stored data, 3-state outputs (LS pin names)
- Report: reports/G13.md; parts added: 1
- PDFs: sn74ls652
- Pinouts: `p24` (24 pins) for DW, NT, JT
- Source: Separate from 74_652 only because the LS datasheet prints different control names (CAB/CBA clocks, GAB/GBA enables). Pinout: sn74ls652 p.1 (scanned; JT/DW/NT top view): pin 22 is printed 'SA' in the pinout drawings but SBA in the logic diagram/symbol (p.3-4), named SBA here. GAB active high, GBA active low, SAB/SBA low = real-time, high = stored, CAB/CBA rising edge stores regardless of select/enable (function table p.2, 'LS652 column). Timing p.6, ROC p.5.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "ab": {"store": "register", "clk": {"pin": "CAB", "edge": "rising"}, "oe": {"pin": "GAB", "active": "high"}, "select": {"pin": "SAB", "active": "high"}}, "ba": {"store": "register", "clk": {"pin": "CBA", "edge": "rising"}, "oe": {"pin": "GBA", "active": "low"}, "select": {"pin": "SBA", "active": "high"}}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 15 | 24 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL clock->bus at 5 V, 25 C, 45 pF, RL 667 ohm, 'LS652 (sn74ls652 p.6). VCC 4.75-5.25 V (SN74LS652, sn74ls652 p.5) |

## 74_653

- Kind / model: `buffer` / `registered`; function "Transceiver"; Octal bus transceiver and register, inverting, open-collector A outputs, 3-state B outputs
- Report: reports/G15.md; parts added: 1
- PDFs: sn54als653, sn74als653
- Pinouts: `p24` (24 pins) for DW, NT, JT, W
- Source: sn74als653 = sn54als653 (SDAS066G). Pinout p.1 (JT, DW/NT; W not drawn, assumed = JT). Function table p.4 ('ALS653 table); A outputs open-collector, B outputs 3-state (p.1 device table); inverting data paths. Timing p.15, ROC p.12. The model takes one output style per device: A side is really open-collector, B side 3-state (see _report.md).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "ab": {"store": "register", "clk": {"pin": "CLKAB", "edge": "rising"}, "oe": {"pin": "OEAB", "active": "high"}, "select": {"pin": "SAB", "active": "high"}, "invert": true}, "ba": {"store": "register", "clk": {"pin": "CLKBA", "edge": "rising"}, "oe": {"pin": "OEBA", "active": "low"}, "select": {"pin": "SBA", "active": "high"}, "invert": true}}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 30 | 17 | [4.5, 5.5] |  |  |  | max tPLH/tPHL CLKAB->B, SN74ALS653, 4.5-5.5 V, 50 pF, no typ printed (sn74als653 p.15); A->B real time max 18/15, CLKBA->A (open-collector) max 64/22; VCC 4.5-5.5 V SN54 and SN74 (p.12) |

## 74_11245

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, 3-state outputs, center-pin VCC/GND
- Report: reports/G10.md; parts added: 3
- PDFs: 74ac11245, 74act11245, scas031c, sn54act11
- Pinouts: `p24` (24 pins) for DB, DBL, DW, PW, NT, N, NS
- Source: 74ac11245 (SCAS010B) p.1 DB/DW/NT/PW pinout and function table; 74act11245 (= scas031c, SCAS031C) p.1 same. OE low: DIR high = A to B, DIR low = B to A; OE high = isolation. SN74ACT11245DBLE was mapped by the census to sn54act11 (SN74ACT11 triple AND, wrong sheet); placed here by its name (DB SSOP-24, LE reel).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 4.8 | 4.1 | [3, 5.5] |  |  |  | typ tPLH/tPHL A or B -> B or A at 5 V +-0.5 V, 25 C, 50 pF (74ac11245 p.4); VCC 3-5.5 V (p.3) |
| ACT | 6.2 | 5.4 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A or B -> B or A at 5 V +-0.5 V, 25 C, 50 pF (74act11245 p.4); VCC 4.5-5.5 V (p.3) |

## 74_1245

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, 3-state outputs (low power)
- Report: reports/G07.md; parts added: 1
- PDFs: sn54als1245a, sn74als1245a
- Pinouts: `p20` (20 pins) for FK, N, DW
- Source: Pinout sn54als1245a p.1 (J/DW/N and FK; the 20-pin FK numbers match the DIP). Function table p.1: OE low + DIR low = B to A, DIR high = A to B, OE high = isolation.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 13 | 13 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B SN74ALS1245A, no typ printed, CL 50 pF (sn54als1245a p.3; SN54 max 19/15). VCC 4.5-5.5 V for SN54 and SN74 (p.2) |

## 74_162245

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 16-bit bus transceiver, 3-state outputs, series damping resistors
- Report: reports/G10.md; parts added: 16
- PDFs: cy74fct162245t, dl2:sn74alvchr162245, sn54abt162245, sn74abt162245, sn74abth162245, sn74lvcr162245, sn74lvt162245a, sn74lvth162245, sn74lvth162245-ep
- Pinouts: `p48` (48 pins) for DGG, DL, WD, DGV, CY-PA
- Source: Standard 48-pin Widebus pinout, same in sn74abt162245 (= sn54abt162245) p.1 (DGG/DL/WD), sn74abth162245 p.1, sn74lvcr162245 p.1, sn74lvt162245a p.1, sn74lvth162245 p.1, sn74lvth162245-ep p.1, sn74alvchr162245 p.1, cy74fct162245t p.1 (SSOP/TSSOP; CY-PA = 48-pin TSSOP). Function table (each 8-bit section): OE low, DIR high = A to B, DIR low = B to A, OE high = isolation. Series output resistors (B port on ABT/ABTH/LVT/LVTH; all outputs on LVCR/ALVCHR/FCT162245T) are not modeled.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.2 | 2.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C, 50 pF (sn74abt162245 p.5; sn74abth162245 p.6 same); VCC 4.5-5.5 V SN54 and SN74 (p.3) |
| ALVC | 4.2 | 4.2 | [1.65, 3.6] |  |  |  | max tpd A or B -> B or A at 3.3 V +-0.3 V, no typical printed, SN74ALVCHR162245 (sn74alvchr162245 p.5); VCC 1.65-3.6 V, VI 0-VCC (p.4) |
| FCT | 4.1 | 4.1 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B, 74FCT162245CT grade (fastest printed; the E grade of CY74FCT162245ETPAC is not in the sheet), VCC 5 V +-10% (cy74fct162245t p.4-5) |
| LVC | 7.5 | 7.5 | [2.7, 3.6] |  |  |  | max tpd A or B -> B or A at 3.3 V +-0.3 V, 50 pF, no typical printed, SN74LVCR162245 (sn74lvcr162245 p.5); VCC 2.7-3.6 V, VI 0-VCC (p.4) |
| LVT | 2.3 | 2.2 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->B at 3.3 V, 50 pF (sn74lvt162245a p.6; sn74lvth162245 p.6 same); VCC 2.7-3.6 V, VI to 5.5 V (p.4) |

## 74_16245

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 16-bit bus transceiver, 3-state outputs
- Report: reports/G08.md; parts added: 51
- PDFs: 74ac16245, 74act16245, cy74fct16245t, sn74abt16245a, sn74abt16245a-ep, sn74abth16245, sn74act16245-ep, sn74ahct16245, sn74alvch16245, sn74alvch16245-ep, sn74alvchr16245, sn74auc16245, sn74avc16245, sn74lvc16245a, sn74lvch16245a, sn74lvchr16245a, sn74lvcr16245a, sn74lvcz16245a, sn74lvt16245b, sn74lvth16245a, sn74lvth16245a-ep, sn74lvth16245a-q1
- Pinouts: `p48` (48 pins) for DL, DGG, DGV, CY-PA, CY-PV
- Source: Standard Widebus 48-pin '16245 pinout, identical in every PDF read: 74ac16245 p.1, 74act16245 p.1, sn74act16245-ep p.1, sn74abt16245a p.1, sn74abt16245a-ep p.1, sn74abth16245 p.1, sn74ahct16245 p.3, sn74alvch16245 p.3, sn74alvch16245-ep p.1, sn74alvchr16245 p.1, sn74auc16245 p.1, sn74avc16245 p.3, sn74lvc16245a p.3, sn74lvch16245a p.3, sn74lvchr16245a p.3, sn74lvcr16245a p.3, sn74lvcz16245a p.1, sn74lvt16245b p.1, sn74lvth16245a p.1, sn74lvth16245a-ep p.1, sn74lvth16245a-q1 p.1, cy74fct16245t p.1 (SSOP/TSSOP; PV = SSOP DL, PA = TSSOP DGG). Function table (each PDF): OE low + DIR low = B to A, OE low + DIR high = A to B, OE high = isolation (e.g. 74ac16245 p.1, cy74fct16245t p.4).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 4.6 | 5.2 | [3, 5.5] |  |  |  | 74ac16245 typ tPLH/tPHL A or B -> B or A at 5 V +-0.5 V, 25 C, 50 pF (p.4; 3.3 V row 7.6/9). VCC 3-5.5 V (p.3) |
| ACT | 6.9 | 6.4 | [4.5, 5.5] |  |  |  | 74act16245 typ tPLH/tPHL A or B -> B or A at 5 V, 25 C, 50 pF (p.4); sn74act16245-ep same (p.4). VCC 4.5-5.5 V (74act16245 p.3, sn74act16245-ep p.3) |
| ABT | 2.2 | 2.3 | [4.5, 5.5] |  |  |  | sn74abt16245a typ tPLH/tPHL A or B -> B or A at 5 V, 25 C, 50 pF (p.6); sn74abth16245 p.5 and sn74abt16245a-ep p.5 same typ. VCC 4.5-5.5 V (sn74abt16245a p.4, sn74abth16245 p.3, -ep p.3) |
| AHCT | 7 | 5.3 | [4.5, 5.5] |  |  |  | sn74ahct16245 typ tPLH/tPHL A or B -> B or A at 5 V, 25 C, 50 pF (p.7; 15 pF 4.5/4.5). VCC 4.5-5.5 V, VI to 5.5 V (p.5) |
| ALVC | 3 | 3 | [1.65, 3.6] |  |  |  | sn74alvch16245 max tpd A or B -> B or A at 3.3 V +-0.3 V, 50 pF (no typ printed) (p.8); sn74alvch16245-ep 3 (p.4), sn74alvchr16245 4.2 (p.5). VCC 1.65-3.6 V, VI to VCC (sn74alvch16245 p.7; alvchr p.4; -ep p.3) |
| AUC | 1.5 | 1.5 | [0.8, 2.7] | 3.6 |  |  | sn74auc16245 typ tpd A or B -> B or A at 1.8 V +-0.15 V, 25 C, 30 pF (p.4). VCC 0.8-2.7 V, VI to 3.6 V (p.3) |
| AVC | 1.9 | 1.9 | [1.4, 3.6] |  |  |  | sn74avc16245 max tpd A or B -> B or A at 2.5 V +-0.2 V (no typ printed above 1.2 V) (p.6). VCC 1.4-3.6 V, VI to 3.6 V (p.5) |
| LVC | 4 | 4 | [1.65, 3.6] | 5.5 |  |  | sn74lvc16245a max tpd A or B -> B or A at 3.3 V +-0.3 V, 50 pF (no typ printed) (p.9); sn74lvch16245a 4 (p.9), sn74lvcr16245a 4.8 (p.7), sn74lvchr16245a 4.8 (p.9), sn74lvcz16245a 3.7 (p.4). VCC 1.65-3.6 V, VI to 5.5 V (sn74lvc16245a p.7; lvch p.7, lvcr p.6, lvchr p.7); LVCZ is 2.7-3.6 V (partVcc) |
| LVT | 2.3 | 2.1 | [2.7, 3.6] | 5.5 |  |  | sn74lvt16245b typ tPLH/tPHL A or B -> B or A at 3.3 V +-0.3 V, 25 C, 50 pF (p.7); sn74lvth16245a p.6, -ep p.6, -q1 p.5 same. VCC 2.7-3.6 V, VI to 5.5 V (sn74lvt16245b p.5, sn74lvth16245a p.4, -ep p.4, -q1 p.3) |
| FCT | 4.1 | 4.1 | [4.5, 5.5] |  |  |  | cy74fct16245t max tPLH/tPHL A to B, B to A, CT speed grade (T 7.0, AT 4.5; E grade not in the PDF), -40 to 85 C (no typ printed) (p.5). VCC 5 V +-10% (Operating Range, p.4) |

- Part range: names starting `SN74LVCZ` (LVC): vcc [2.7, 3.6], inputMax 5.5

## 74_1645

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, 3-state outputs (low-power '645)
- Report: reports/G02.md; parts added: 1
- PDFs: sn74als1645a
- Pinouts: `p20` (20 pins) for N, DW
- Source: sn74als1645a p.1: DW or N pinout = standard '245 (1 DIR, 2-9 A1-A8, 10 GND, 11-18 B8-B1, 19 OE, 20 VCC); function table p.1 (SN74ALS1645A column, noninverting): OE low + DIR high = A to B, DIR low = B to A, OE high = isolation. The PDF also covers the inverting SN74ALS1640A (not in this group).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 13 | 13 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B SN74ALS1645A, 4.5-5.5 V, 50 pF (sn74als1645a p.3; no typical printed); VCC 4.5-5.5 V (p.2) |

## 74_16623

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 16-bit bus transceiver, separate A-to-B and B-to-A enables, 3-state outputs
- Report: reports/G04.md; parts added: 2
- PDFs: sn74abt16623
- Pinouts: `p48` (48 pins) for DL
- Source: Pinout sn74abt16623 p.1 (DGG or DL; WD for SN54): 1 1OEAB, 2-3 1B1-1B2, 5-6 1B3-1B4, 8-9 1B5-1B6, 11-12 1B7-1B8, 13-14 2B1-2B2, 16-17 2B3-2B4, 19-20 2B5-2B6, 22-23 2B7-2B8, 24 2OEAB, 25 2OEBA, 26-27 2A8-2A7, 29-30 2A6-2A5, 32-33 2A4-2A3, 35-36 2A2-2A1, 37-38 1A8-1A7, 40-41 1A6-1A5, 43-44 1A4-1A3, 46-47 1A2-1A1, 48 1OEBA; GND 4,10,15,21,28,34,39,45; VCC 7,18,31,42. Function table p.2 (each 8-bit section): OEBA L/OEAB L B->A; L/H both directions (bus latch, outputs reinforce inputs); H/L isolation; H/H A->B. OEAB active high, OEBA active low.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "oeab": {"pin": "1OEAB", "active": "high"}, "oeba": {"pin": "1OEBA", "active": "low"}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "oeab": {"pin": "2OEAB", "active": "high"}, "oeba": {"pin": "2OEBA", "active": "low"}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2 | 2.2 | [4.5, 5.5] |  |  |  | Typical A->B or B->A at 5 V, 25 C, 50 pF, SN74ABT16623 (sn74abt16623 p.5). VCC 4.5-5.5 V (sn74abt16623 p.3) |

## 74_16640

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 16-bit inverting bus transceiver, 3-state outputs
- Report: reports/G05.md; parts added: 3
- PDFs: sn74abt16640
- Pinouts: `p48` (48 pins) for DGG, DL
- Source: Pinout sn74abt16640 p.1 (DGG/DL). Function table p.1 (each 8-bit section): OE L, DIR L -> B data to A; OE L, DIR H -> A data to B; OE H -> isolation. Inverting (p.1 description).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.5 | 2.8 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A or B -> B or A at 5 V, 50 pF, SN74ABT16640 (sn74abt16640 p.5); VCC 4.5-5.5 V (p.3) |

## 74_16861

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 20-bit bus transceiver, 3-state outputs (two 10-bit sections, OEAB/OEBA)
- Report: reports/G10.md; parts added: 1
- PDFs: 74act16861
- Pinouts: `p56` (56 pins) for DL, WD
- Source: 74act16861 (SCAS197B) p.1 DL/WD pinout; p.2 function table (each 10-bit section): OEAB low = A to B, OEBA low = B to A, both low = latch A and B (A = B), both high = isolation.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "1A9", "1A10"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "1B9", "1B10"], "oeab": {"pin": "1OEAB", "active": "low"}, "oeba": {"pin": "1OEBA", "active": "low"}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "2A9", "2A10"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8", "2B9", "2B10"], "oeab": {"pin": "2OEAB", "active": "low"}, "oeba": {"pin": "2OEBA", "active": "low"}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ACT | 6.5 | 7.5 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A or B -> B or A at 5 V, 25 C, 50 pF (74act16861 p.4); VCC 4.5-5.5 V (p.3) |

## 74_16863

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 18-bit bus transceiver, 3-state outputs
- Report: reports/G16.md; parts added: 4
- PDFs: 74act16863, sn74abt16863, sn74alvch16863
- Pinouts: `p56` (56 pins) for DL, DGG
- Source: Pinout same in sn74abt16863 p.1 (DL), 74act16863 p.1 (DL), sn74alvch16863 p.1 (DGG/DL). Two 9-bit sections. Function table (each 9-bit section; sn74abt16863 p.2, 74act16863 p.1, sn74alvch16863 p.1): OEAB H + OEBA L = B to A, OEAB L + OEBA H = A to B, both H = isolation; both enables are active low; the both-low row is not listed.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "1A9"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "1B9"], "oeab": {"pin": "1OEAB", "active": "low"}, "oeba": {"pin": "1OEBA", "active": "low"}}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "2A9"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8", "2B9"], "oeab": {"pin": "2OEAB", "active": "low"}, "oeba": {"pin": "2OEBA", "active": "low"}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ACT | 7 | 6.4 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->B at 5 V and 25 C (50 pF) (74act16863 p.4). VCC 4.5-5.5 V (74act16863 p.3) |
| ABT | 2.2 | 2.2 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->B at 5 V and 25 C (50 pF) (sn74abt16863 p.5). VCC 4.5-5.5 V (sn74abt16863 p.3) |
| ALVC | 3.4 | 3.4 | [1.65, 3.6] |  |  |  | Max tpd A->B at 3.3 V; no typical given (sn74alvch16863 p.4). VCC 1.65-3.6 V, VI 0-VCC (sn74alvch16863 p.3) |

## 74_2245

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver with series damping resistors, 3-state outputs
- Report: reports/G11.md; parts added: 28
- PDFs: sn54abt2245, sn74abt2245, sn74abtr2245, sn74bct2245, sn74lvcr2245a, sn74lvth2245
- Pinouts: `p20` (20 pins) for DB, DBQ, DGV, DW, N, NS, PW, RGY, J, W
- Source: Standard '245 pinout: sn74abt2245 p.1 (DB/DW/N/PW, SN54 J/W; 5962-9560601QRA = SNJ54ABT2245J), sn74abtr2245 p.1, sn74bct2245 p.1 (DB/DW/N/NS), sn74lvcr2245a p.3 (DB/DBQ/DGV/DW/NS/PW, and RGY VQFN with the same numbers 1-20, thermal pad not a pin), sn74lvth2245 p.1. Function table OE L + DIR L = B data to A, DIR H = A data to B, OE H = isolation (sn74abt2245 p.1, sn74lvcr2245a p.10). B-port (ABT/ABTR/BCT) or all-port (LVCR) series resistors not modeled. SN74BCT2245NS is an NS orderable not in the packaging table.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.5 | 3.2 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->B at 5 V, 25 C, 50 pF, SN74ABT2245 (sn74abt2245 p.6; SN74ABTR2245 identical, sn74abtr2245 p.6). VCC 4.5-5.5 V, SN54 and SN74 (sn74abt2245 p.4) |
| BCT | 3.3 | 5.1 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->B at 5 V, 25 C, 50 pF (sn74bct2245 p.4). VCC 4.5-5.5 V (sn74bct2245 p.3) |
| LVC | 6.3 | 6.3 | [1.65, 3.6] | 5.5 |  |  | Max tpd A/B->B/A at 3.3 V, -40 to 85 C, 50 pF, SN74LVCR2245A; no typical printed (sn74lvcr2245a p.7). VCC 1.65-3.6 V, VI to 5.5 V (sn74lvcr2245a p.6) |
| LVT | 2.9 | 2.6 | [2.7, 3.6] | 5.5 |  |  | Typical tPLH/tPHL A->B at 3.3 V, 25 C, 50 pF, SN74LVTH2245 (sn74lvth2245 p.5). VCC 2.7-3.6 V, VI to 5.5 V (sn74lvth2245 p.3) |

## 74_2245_FCT

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver with 25-ohm series resistors, 3-state outputs
- Report: reports/G11.md; parts added: 9
- PDFs: cy74fct2245t
- Pinouts: `p20` (20 pins) for N, PW, DBQ, DW, CY-SOC
- Source: CY74FCT2245T prints T/R for the direction pin and bits A0-A7/B0-B7 (cy74fct2245t p.1: 1 T/R, 2-9 A0-A7, 10 GND, 11 B7 ... 18 B0, 19 OE, 20 VCC; P = N, Q = DBQ, SO = DW; PW from the packaging table). Function table p.2: OE L + T/R L = bus B to bus A, T/R H = bus A to bus B, OE H = Z. CY74FCT2245SOCT (CY-SOC) is a DW orderable not in the packaging table. Series resistors not modeled.

```json
{"units": [{"a": ["A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"], "b": ["B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7"], "dir": {"pin": "T/R", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 7 | 7 | [4.75, 5.25] |  |  |  | Max tPLH/tPHL A->B, CY74FCT2245T (slowest grade; AT 4.6, CT 4.1), -40 to 85 C, 50 pF; no typical printed (cy74fct2245t p.4). VCC 4.75-5.25 V (cy74fct2245t p.2) |

## 74_243

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Quad bus transceiver, 3-state outputs
- Report: reports/G09.md; parts added: 1
- PDFs: sn74ls243
- Pinouts: `p14` (14 pins) for N, NS, D, J, W
- Source: Pinout sn74ls243 p.1 (D/N/NS top view; GAB active low, GBA active high). Function table p.1: GAB L GBA L = A to B; H H = B to A; H L = isolation; L H = latch A and B (A = B), the model's both-enabled bus latch. Timing p.4.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4"], "b": ["B1", "B2", "B3", "B4"], "oeab": {"pin": "GAB", "active": "low"}, "oeba": {"pin": "GBA", "active": "high"}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 12 | 12 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL A->B or B->A, SN74LS243, 5 V, 45 pF, RL 667 ohm (sn74ls243 p.4); VCC 4.75-5.25 V (p.3) |

## 74_243_CD

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Quad bus transceiver, 3-state outputs
- Report: reports/G09.md; parts added: 4
- PDFs: cd74hc243, cd74hct243
- Pinouts: `p14` (14 pins) for N, D, F, J
- Source: Pinout cd74hc243 / cd74hct243 (same PDF) p.3 (J/N/D: 1=OEB active low, 13=OEA active high, NC 2 and 12). Logic diagram p.8: OEB low enables A->B, OEA high enables B->A. Truth table p.8 rows: OEB H OEA H = B to A; OEB L OEA L = A to B; OEB H OEA L = Z Z; OEB L OEA H printed as Z Z (the logic diagram and the classic '243 give both drivers on / bus latch there; see _report.md). Timing p.6.

```json
{"units": [{"a": ["A0", "A1", "A2", "A3"], "b": ["B0", "B1", "B2", "B3"], "oeab": {"pin": "OEB", "active": "low"}, "oeba": {"pin": "OEA", "active": "high"}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 7 | 7 | [2, 6] |  |  |  | typ tpd data to outputs at 5 V, 15 pF (footnote 1) (cd74hc243 p.6); VCC 2-6 V (p.4) |
| HCT | 9 | 9 | [4.5, 5.5] |  |  |  | typ tpd data to outputs at 5 V, 15 pF (footnote 1) (cd74hct243 p.6); VCC 4.5-5.5 V (p.4) |

## 74_245

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, 3-state outputs
- Report: reports/G02.md; parts added: 182
- PDFs: cd54act245, cd54hct245, cd74ac245, cd74act245, cd74fct245, cd74hc245, cd74hct245, cy54fct245t, cy74fct245t, sn54abt245a, sn54ahct245, sn54bct245, sn54f245, sn54hc245-sp, sn54hct245-sp, sn54lvch245a-sp, sn54lvth245a-sp, sn64bct245, sn74abt245b, sn74abth245, sn74ac245, sn74ac245-q1, sn74act245, sn74act245-q1, sn74ahc245, sn74ahc245-ep, sn74ahc245-q1, sn74ahct245, sn74ahct245-ep, sn74ahct245-q1, sn74als245a, sn74alvc245, sn74alvch245, sn74as245, sn74auc245, sn74auch245, sn74bct245, sn74f245, sn74hc245, sn74hcs245, sn74hcs245-q1, sn74hct245, sn74ls245, sn74lv245a, sn74lvc245a, sn74lvc245a-ep, sn74lvch245a, sn74lvcz245a, sn74lvt245b, sn74lvth245a, sn74lvth245a-ep
- Pinouts: `p20` (20 pins) for CY-QC, DB, DBQ, DGS, DGV, DW, FK, J, M, N, NS, PW, RGY, RKS, W
- Source: Standard '245 pinout (1 DIR, 2-9 A1-A8, 10 GND, 11-18 B8-B1, 19 OE, 20 VCC) in every PDF: sn74hc245 p.3 (DB/DGV/DW/N/J/W/PW and FK, same numbers), sn74ahc245 p.3 (RKS VQFN same numbers, thermal pad), sn74ahct245 p.3 (RGY/RKS), sn74abt245b p.1 (RGY, FK), sn74lvc245a p.3 (RGY), sn74auc245 p.1 (RGY), sn74ac245 p.3 (DGS, RKS), sn74hcs245 p.3, sn74lv245a p.3, cy74fct245t p.1, cd74fct245 p.1, sn74ls245 p.3, sn74als245a p.1 (ALS and AS), sn74f245 p.1, sn74bct245 p.1, sn64bct245 p.1, sn74alvc245 p.1, sn74lvt245b p.1. Name aliases (same positions): CY74FCT245T names DIR 'T/R' and numbers the bits A0-A7/B0-B7; CD54/74AC(T)245 (p.3) and CD54/74HC(T)245 (p.3, drawing only) number the bits A0-A7/B0-B7. Function table identical in all PDFs: OE low + DIR low = B to A, OE low + DIR high = A to B, OE high = isolation (sn74hc245 p.9, cd74ac245 p.11, cy74fct245t p.2, sn74lvc245a p.9 Table 1, ...). LS/ALS/AS/F/BCT parts have 3-state push-pull outputs like the rest.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 15 | 15 | [2, 6] |  |  |  | typ tpd A->B at 4.5 V, 50 pF, 25 C (sn74hc245 p.6); VCC 2-6 V SN54 and SN74 (p.4); CD74HC245 same range (cd74hc245 p.4) |
| HCT | 16 | 16 | [4.5, 5.5] |  |  |  | typ tpd A->B at 4.5 V, 50 pF, 25 C (sn74hct245 p.6); VCC 4.5-5.5 V (p.4); CD54/74HCT245 same (cd74hct245 p.4) |
| HCS | 8 | 8 | [2, 6] |  |  |  | typ tpd A->B at 4.5 V, 50 pF (sn74hcs245 p.5); VCC 2-6 V (p.4); -Q1 same (sn74hcs245-q1) |
| AC | 3.5 | 3.5 | [2, 6] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C, 50 pF (sn74ac245 p.7); VCC 2-6 V SN54/SN74 (p.5). CD74AC245 1.5-5.5 V (cd74ac245 p.4) and SN74AC245-Q1 1.5-6 V (sn74ac245-q1) in partVcc |
| ACT | 4 | 4 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C, 50 pF (sn74act245 p.6); VCC 4.5-5.5 V (p.5); CD54/74ACT245 same (cd74act245 p.4) |
| AHC | 5.5 | 5.5 | [2, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 50 pF (sn74ahc245 p.7); VCC 2-5.5 V (p.5); only OE/DIR take 5.5 V regardless of VCC, A/B to VCC, so no inputMax |
| AHCT | 5.3 | 5.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 50 pF (sn74ahct245 p.6); VCC 4.5-5.5 V (p.4) |
| LS | 8 | 8 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL at 5 V, 25 C, 45 pF/667 ohm (sn74ls245 p.5); VCC SN74LS245 4.75-5.25 V, SN54LS245 4.5-5.5 V (p.4) |
| ALS | 10 | 10 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B SN74ALS245A, 4.5-5.5 V, 50 pF (sn74als245a p.4; no typical printed); VCC 4.5-5.5 V (p.3); -1 version same timing (higher IOL) |
| AS | 7.5 | 7 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B SN74AS245, 4.5-5.5 V, 50 pF (sn74as245 p.5; no typical printed); VCC 4.5-5.5 V (p.4) |
| F | 3.8 | 4.2 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C (sn74f245 p.4); VCC 4.5-5.5 V SN54/SN74 (p.3) |
| BCT | 4.4 | 4.8 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C (sn74bct245 p.4); VCC 4.5-5.5 V SN54/SN74 (p.3); SN64BCT245 same VCC (sn64bct245 p.2) |
| ABT | 2 | 2.6 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C, 50 pF (sn74abt245b p.5); VCC 4.5-5.5 V SN54/SN74 (p.3); ABTH245 identical (sn74abth245 p.3, p.5) |
| LV | 7.9 | 7.9 | [2, 5.5] |  |  |  | typ tpd A->B at 3.3 V, 50 pF (sn74lv245a p.7; 5.9 ns at 15 pF); VCC 2-5.5 V, VI 0-5.5 V (p.5) |
| LVC | 3.8 | 3.8 | [1.65, 3.6] | 5.5 |  |  | typ tpd A->B at 3.3 V, 50 pF (sn74lvc245a p.6); VCC 1.65-3.6 V, VI 0-5.5 V (p.5). LVCH245A same for SN74 (sn74lvch245a p.8); SN54LVCH245A 2-3.6 V (p.9) and LVCZ245A 2.7-3.6 V (sn74lvcz245a p.3) in partVcc |
| LVT | 2.3 | 2.1 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->B at 3.3 V, 50 pF (sn74lvt245b p.4); VCC 2.7-3.6 V, VI to 5.5 V (p.3); LVTH245A same (sn74lvth245a p.3, p.5) |
| ALVC | 3.4 | 3.4 | [1.65, 3.6] |  |  |  | max tpd A->B at 3.3 V, 50 pF (sn74alvc245 p.4; no typical printed); VCC 1.65-3.6 V, VI 0-VCC (p.3); ALVCH245 same range |
| AUC | 1 | 1 | [0.8, 2.7] | 3.6 |  |  | typ tpd A->B at 1.8 V, 15 pF (sn74auc245 p.4); VCC 0.8-2.7 V, VI to 3.6 V (p.3); AUCH245 same (sn74auch245) |
| FCT | 7 | 7 | [4.75, 5.25] |  |  |  | max tPLH/tPHL A->B, CY74FCT245T base grade, 50 pF (cy74fct245t p.6; no typical; AT 4.6, CT 4.1, DT 3.8 ns); VCC 4.75-5.25 V CY74 (p.3), CY54 4.5-5.5 V; CD74FCT245 max 7 ns, 4.75-5.25 V (cd74fct245 p.3-4) |

- Part range: names starting `CD74AC245` (AC): vcc [1.5, 5.5]

- Part range: names starting `SN74AC245DGSRQ1` (AC): vcc [1.5, 6]

- Part range: names starting `SN74AC245PWRQ1` (AC): vcc [1.5, 6]

- Part range: names starting `SN74LVCZ245A` (LVC): vcc [2.7, 3.6]

- Part range: names starting `5962-9754301` (LVC): vcc [2, 3.6]

- Part range: names starting `5962-9221405` (FCT): vcc [4.5, 5.5]

## 74_25245

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 25-ohm octal bus transceiver, 3-state outputs
- Report: reports/G11.md; parts added: 4
- PDFs: sn64bct25245, sn74abth25245, sn74bct25245
- Pinouts: `p24` (24 pins) for DW, NT, JT
- Source: p24 from sn74abth25245 p.1 (DW/NT, SN54 JT), sn74bct25245 p.1 (DW/NT), sn64bct25245 p.1 (DW/NT): identical, distributed GND on 2/5/8/11 and VCC on 16/21. Function table OE L + DIR L = B data to A, DIR H = A data to B, OE H = isolation (sn74abth25245 p.2, sn74bct25245 p.1, sn64bct25245 p.1). SN64BCT25245 is the -40 to 85 C BCT part.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.3 | 2.4 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->B at 5 V, 25 C, 50 pF, SN74ABTH25245 (sn74abth25245 p.6). VCC 4.5-5.5 V (sn74abth25245 p.4) |
| BCT | 3.3 | 4.3 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->B at 5 V, 25 C (sn74bct25245 p.4; SN64BCT25245 same typ, sn64bct25245 p.4). VCC 4.5-5.5 V (sn74bct25245 p.3, sn64bct25245 p.2) |

## 74_29863

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 9-bit bus transceiver, 3-state outputs (dual output enables per direction)
- Report: reports/G08.md; parts added: 2
- PDFs: sn74als29863, sn74bct29863b
- Pinouts: `p24` (24 pins) for DW
- Source: Am29863 equivalent. Pinout sn74als29863 p.1 (DW/NT), sn74bct29863b p.1 (DW/NT, SN54 JT/W), same 24-pin layout. Function table sn74als29863 p.1 / sn74bct29863b p.1: OEAB1 and OEAB2 both low with an OEBA high = A to B; OEBA1 and OEBA2 both low with an OEAB high = B to A; all four low = latch A and B; otherwise isolation. Logic diagram p.2 (AND of the two enables). Timing sn74als29863 p.3, sn74bct29863b p.4.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9"], "oeab": [{"pin": "OEAB1", "active": "low"}, {"pin": "OEAB2", "active": "low"}], "oeba": [{"pin": "OEBA1", "active": "low"}, {"pin": "OEBA2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 8 | 8 | [4.75, 5.25] |  |  |  | sn74als29863 max tPLH/tPHL A or B -> B or A, 50 pF, VCC 4.75-5.25 V (no typ printed; 300 pF 15) (p.3). VCC 4.75-5.25 V (p.2) |
| BCT | 3 | 4.8 | [4.5, 5.5] |  |  |  | sn74bct29863b typ tPLH/tPHL A or B -> B or A at 5 V, 25 C, 50 pF (p.4). VCC 4.5-5.5 V (p.3) |

## 74_32245

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 36-bit bus transceiver, 3-state outputs
- Report: reports/G09.md; parts added: 1
- PDFs: sn74abth32245
- Pinouts: `lqfp100` (100 pins) for PZ
- Source: Pinout sn74abth32245 p.1 (PZ 100-pin LQFP top view, checked on the rendered page: four 9-bit groups, nDIR and nOE per group, 16 GND, 4 VCC). Function table p.2: OE L DIR L = B data to A bus; OE L DIR H = A data to B bus; OE H = isolation. Bus hold not modeled.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "1A9"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "1B9"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "2A9"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8", "2B9"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}, {"a": ["3A1", "3A2", "3A3", "3A4", "3A5", "3A6", "3A7", "3A8", "3A9"], "b": ["3B1", "3B2", "3B3", "3B4", "3B5", "3B6", "3B7", "3B8", "3B9"], "dir": {"pin": "3DIR", "ab": "high"}, "oe": [{"pin": "3OE", "active": "low"}]}, {"a": ["4A1", "4A2", "4A3", "4A4", "4A5", "4A6", "4A7", "4A8", "4A9"], "b": ["4B1", "4B2", "4B3", "4B4", "4B5", "4B6", "4B7", "4B8", "4B9"], "dir": {"pin": "4DIR", "ab": "high"}, "oe": [{"pin": "4OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 3.2 | 3.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A or B -> B or A at 5 V, 50 pF (sn74abth32245 p.5); VCC 4.5-5.5 V (p.3) |

## 74_620

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, inverting, 3-state outputs
- Report: reports/G16.md; parts added: 6
- PDFs: sn54bct620a, sn74abt620, sn74als620a
- Pinouts: `p20` (20 pins) for J, DW, N, NS
- Source: Pinout same in sn74abt620 p.1 (DB/DW/N/PW/J), sn74als620a p.1 (DW/N), sn54bct620a p.1 (J/W/DW/N). Function table (sn74abt620 p.2, sn74als620a p.1, sn54bct620a p.1): OEBA L = B to A, OEAB H = A to B, both enabled = bus latch (outputs reinforce inputs), OEBA H + OEAB L = isolation. Inverting (complementary data).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "oeab": {"pin": "OEAB", "active": "high"}, "oeba": {"pin": "OEBA", "active": "low"}}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 4.1 | 4.3 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL A->B at 5 V and 25 C (50 pF); SN74ABT620 gives no typical (sn74abt620 p.5). VCC 4.5-5.5 V (sn74abt620 p.3) |
| ALS | 10 | 10 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL A->B at 4.5-5.5 V, 0 to 70 C (50 pF); no typical given (sn74als620a p.5). VCC 4.5-5.5 V (sn74als620a p.4) |
| BCT | 3.4 | 1.9 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->B at 5 V and 25 C (50 pF) (sn54bct620a p.4). VCC 4.5-5.5 V, SN54 and SN74 alike (sn54bct620a p.2) |

## 74_621

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, open-collector outputs (OEAB/OEBA)
- Report: reports/G10.md; parts added: 2
- PDFs: sn74als621a
- Pinouts: `p20` (20 pins) for N, DW, NS
- Source: sn74als621a (SDAS226A) p.1 DW/N pinout and function table: OEAB high = A to B, OEBA low = B to A, both = bus latch (A = B), OEAB low/OEBA high = isolation; open-collector outputs on both ports (p.1 device table, p.6 IOH/VOH 5.5 V). NS is not in the sheet; the SOP-20 takes the same pinout.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "oeab": {"pin": "OEAB", "active": "high"}, "oeba": {"pin": "OEBA", "active": "low"}}], "invert": false, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 33 | 20 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B, VCC 4.5-5.5 V, CL 50 pF, RL 680 ohm, no typical printed (sn74als621a p.6); VCC 4.5-5.5 V (p.5) |

## 74_623

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, 3-state outputs, separate A-to-B and B-to-A enables
- Report: reports/G15.md; parts added: 16
- PDFs: cd54act623, cd74ac623, cd74act623, cd74fct623, sn54abt623a, sn54bct623, sn74abt623, sn74als623a, sn74hc623, sn74hct623, sn74ls623
- Pinouts: `p20` (20 pins) for J, FK, N, DW, DB, DBL, NS, PW, W
- Source: Pinout: sn74abt623 p.1 (DB/DW/N/PW, JT/W, FK numbered 1-20 same as DIP), sn54bct623 p.1 (J/W, FK), sn74hc623 p.1, sn74hct623 p.1, sn74als623a p.1, sn74ls623 p.1 (scan), cd74fct623 p.1, cd74ac623 p.2 (scan; CD sheet numbers bits A0-A7/B0-B7, same pins). Function table sn74abt623 p.2 / sn74hc623 p.1: OEAB high enables A->B, OEBA low enables B->A, both enabled = bus latch. LS sheet names OEAB/OEBA as GAB/GBA (same pins).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "oeab": {"pin": "OEAB", "active": "high"}, "oeba": {"pin": "OEBA", "active": "low"}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.6 | 2.6 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C, 50 pF (sn74abt623 p.5); VCC 4.5-5.5 V SN54 and SN74 (p.3) |
| BCT | 3.1 | 4.9 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C, 50 pF (sn54bct623 p.4); VCC 4.5-5.5 V (p.2) |
| AC | 4.5 | 4.5 | [1.5, 5.5] |  |  |  | typ 4.5 ns at 5 V, 50 pF (cd74ac623 p.1 features; table p.5 gives max 8.7 ns at 5 V); VCC 1.5-5.5 V (p.2) |
| ACT | 4.5 | 4.5 | [4.5, 5.5] |  |  |  | typ 4.5 ns at 5 V, 50 pF (cd74ac623 p.1 features, covers AC/ACT; table p.5 gives ACT max 9.6 ns); VCC 4.5-5.5 V (p.2) |
| FCT | 5.3 | 5.3 | [4.75, 5.25] |  |  |  | typ tpd A->B at 25 C, 50 pF (cd74fct623 p.4); VCC 4.75-5.25 V (p.3) |
| ALS | 13 | 11 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B, SN74ALS623A, 4.5-5.5 V, 50 pF, no typ printed (sn74als623a p.4); VCC 4.5-5.5 V (p.4) |
| HC | 10 | 10 | [2, 6] |  |  |  | typ tpd at 4.5 V, 50 pF (sn74hc623 p.4); VCC 2-6 V (p.3) |
| HCT | 15 | 15 | [4.5, 5.5] |  |  |  | typ tpd at 4.5 V, 50 pF (sn74hct623 p.3); VCC 4.5-5.5 V (p.3) |
| LS | 8 | 11 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C, 45 pF, SN74LS623 (sn74ls623 p.4, scanned); VCC 4.75-5.25 V (p.4) |

## 74_638

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, inverting, open-collector A outputs and 3-state B outputs
- Report: reports/G11.md; parts added: 1
- PDFs: sn74als638a
- Pinouts: `p20` (20 pins) for N, NS, DW
- Source: p20 from sn74als638a p.1 (DW or N package; NS orderable uses the same 20-pin numbering). Function table sn74als638a p.1: OE L + DIR L = B data to A bus, OE L + DIR H = A data to B bus, OE H = isolation; SN74ALS638A is the inverting part (logic column 'Inverting', p.1). A-bus outputs are open collector, B-bus outputs 3-state (p.1); the model has one output type, so it is given as push-pull (see _report.md). Timing sn74als638a p.4, VCC p.3.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 12 | 12 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL A->B at 4.5-5.5 V, 0-70 C, 50 pF; no typical printed (sn74als638a p.4). VCC 4.5-5.5 V (sn74als638a p.3). B->A (open-collector side) max 25/30 ns |

## 74_639

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, open-collector A outputs, 3-state B outputs
- Report: reports/G14.md; parts added: 2
- PDFs: sn74als639a
- Pinouts: `p20` (20 pins) for DW, N
- Source: Pinout and function table from sn74als639a p.1 (SDAS123A, DW or N; same file as dl2 sdas123a). OE low + DIR high = A to B, DIR low = B to A, OE high = isolation. A-bus outputs are open-collector, B-bus 3-state (p.1); output is given as push-pull (device-wide key) - see _report.md.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 12 | 12 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL A->B, VCC 4.5-5.5 V, 50 pF, TA min to max (sn74als639a p.4; no typical given). VCC 4.5-5.5 V (p.3) |

## 74_640

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, inverting, 3-state outputs
- Report: reports/G15.md; parts added: 30
- PDFs: cd74hc640, cd74hct640, sn54als640b, sn54bct640, sn54hc640, sn54ls640, sn74abt640, sn74als640b, sn74as640, sn74bct640, sn74hc640, sn74ls640
- Pinouts: `p20` (20 pins) for J, FK, W, N, E, DW, DB, NS, PW
- Source: Pinout: sn74hc640 p.3 (J/DW/N/NS/PW), sn74abt640 p.1 (DB/DW/N/PW, J, FK numbered same as DIP), sn74als640b p.1 (J, DW/N, FK), sn74bct640 p.1 (J/W, DW/N, FK), sn74ls640 p.1 (scan; J, DW/N, FK), cd74hc640 p.3 (CD sheet numbers bits A0-A7/B0-B7, same pins). Function table sn74hc640 p.8: OE low + DIR high = A->B, DIR low = B->A, OE high = isolation; outputs invert. LS sheet names OE as G (same pin). W flatpack: sn54bct640 p.1 draws W with the J pinout; assumed same for SNJ54ALS640BW.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 10 | 10 | [2, 6] |  |  |  | typ tpd at 4.5 V, 50 pF (sn74hc640 p.5; cd74hc640 p.6 prints typ 7 ns); VCC 2-6 V (sn74hc640 p.4, cd74hc640 p.4) |
| HCT | 9 | 9 | [4.5, 5.5] |  |  |  | typ tpd at 4.5 V, 50 pF (cd74hct640 p.6); VCC 4.5-5.5 V (p.4) |
| ABT | 2.7 | 2.7 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL at 5 V, 25 C, 50 pF (sn74abt640 p.4); VCC 4.5-5.5 V (p.3) |
| ALS | 11 | 10 | [4.5, 5.5] |  |  |  | max tPLH/tPHL SN74ALS640B, 4.5-5.5 V, 50 pF, no typ printed (sn74als640b p.3); VCC 4.5-5.5 V SN54/SN74 (p.2) |
| AS | 7 | 6 | [4.5, 5.5] |  |  |  | max tPLH/tPHL SN74AS640, 4.5-5.5 V, 50 pF, no typ printed (sn74as640 = sn74als640b p.5); VCC 4.5-5.5 V (p.4) |
| BCT | 3.6 | 1.9 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL at 5 V, 25 C, 50 pF (sn74bct640 p.4); VCC 4.5-5.5 V (p.3) |
| LS | 6 | 8 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C, 45 pF (sn74ls640 p.5, scanned); VCC 4.75-5.25 V SN74LS, 4.5-5.5 V SN54LS (p.4) |

- Part range: names starting `SNJ54LS` (LS): vcc [4.5, 5.5]

- Part range: names starting `8416101` (LS): vcc [4.5, 5.5]

## 74_641

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, open-collector outputs
- Report: reports/G07.md; parts added: 12
- PDFs: sn74als641a, sn74as641, sn74ls641
- Pinouts: `p20` (20 pins) for N, DW, NS, DB
- Source: Pinout: sn74als641a p.1 (DW/N; same file covers SN74AS641), sn74ls641 p.1 (scanned; J/DW/N, enable pin printed G-bar, named OE here). Function table sn74als641a p.1 / sn74ls641 p.1: OE low + DIR low = B to A, DIR high = A to B, OE high = isolation. Open-collector outputs on both A and B ports (sn74als641a p.3 VOH 5.5 V, IOH at VOH 5.5 V; sn74ls641 p.7 title 'with open-collector outputs').

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 17 | 16 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL A->B at 5 V, 25 C, CL 45 pF, RL 667 ohm (sn74ls641 p.8). VCC 4.75-5.25 V SN74LS641 (p.7) |
| ALS | 25 | 18 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B, no typ printed, CL 50 pF, RL 680 ohm (sn74als641a p.4). VCC 4.5-5.5 V (p.3) |
| AS | 21 | 7.5 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->B, no typ printed, CL 50 pF, RL 680 ohm (sn74as641 p.6). VCC 4.5-5.5 V (p.5) |

## 74_642

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, inverting, open-collector outputs
- Report: reports/G06.md; parts added: 5
- PDFs: sdas300, sn74als641a-1, sn74as641, sn74ls642
- Pinouts: `p20` (20 pins) for DW, N, NE, DB
- Source: Pinout DIR 1, A1-A8 2-9, GND 10, B8-B1 11-18, OE 19, VCC 20: sn74as641 (= sn74als641a-1 = sdas300, SDAS300) p.1 (DW, N); sn74ls642 (SDLS189, scanned) p.1 (DW, J, N; enable printed G-bar there, named OE here). Function table OE L DIR L: B->A, OE L DIR H: A->B, OE H: isolation (sn74as641 p.1, sn74ls642 p.1). '642 is the inverting open-collector member (sn74as641 p.1 device table, sn74ls642 p.1). Both A and B outputs are open-collector.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": true, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 30 | 22 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A or B -> B or A, SN74ALS642A, 50 pF, 0-70 C (sn74as641 p.4; no typical printed); VCC 4.5-5.5 V (p.3) |
| LS | 19 | 14 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL A->B at 5 V, 45 pF, 'LS642 (sn74ls642 p.8); VCC 4.75-5.25 V (p.7) |

## 74_645

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver, 3-state outputs
- Report: reports/G06.md; parts added: 16
- PDFs: sn54as645, sn54hc645, sn74als645a, sn74as645, sn74hc645, sn74hct645, sn74ls645
- Pinouts: `p20` (20 pins) for J, N, NS, DW, PW, FK, NE, DB
- Source: Pinout DIR 1, A1-A8 2-9, GND 10, B8-B1 11-18, OE 19, VCC 20: sn74as645 (= sn74als645a = sn54as645, SDAS278) p.1 (J, DW, N; FK same numbering), sn74hc645 (= sn54hc645) p.1 (DW, N, NS, J, W; FK same numbering), sn74hct645 p.1 (DW, N, NS, PW), sn74ls645 (SDLS189, scanned) p.1 (DW, J, N; enable printed G-bar there, named OE here). Function table OE L DIR L: B->A, OE L DIR H: A->B, OE H: isolation (sn74as645 p.1, sn74hct645 p.2).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AS | 9.5 | 9 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A or B -> B or A, SN74AS645, 50 pF (sn74as645 p.5; no typical printed); VCC 4.5-5.5 V for SN54 and SN74 (p.4) |
| ALS | 10 | 10 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A or B -> B or A, SN74ALS645A, 50 pF (sn74als645a p.3; no typical printed); VCC 4.5-5.5 V (p.2) |
| HC | 15 | 15 | [2, 6] |  |  |  | typ tpd A or B -> B or A at 4.5 V, 50 pF (sn74hc645 p.3); VCC 2-6 V for SN54 and SN74 (p.2) |
| HCT | 16 | 16 | [4.5, 5.5] |  |  |  | typ tpd A or B -> B or A at 4.5 V, 50 pF (sn74hct645 p.3); VCC 4.5-5.5 V (p.3) |
| LS | 8 | 11 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL A->B at 5 V, 45 pF, 'LS645 (sn74ls645 p.5); VCC 4.75-5.25 V (p.4) |

## 74_861

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 10-bit bus transceiver, 3-state outputs
- Report: reports/G08.md; parts added: 6
- PDFs: sn74abt861, sn74lvc861a
- Pinouts: `p24` (24 pins) for DW, DB, DGV, PW
- Source: Pinout sn74abt861 p.1 (DW/NT), sn74lvc861a p.1 (DB/DGV/DW/NS/PW), same 24-pin layout. Function table sn74abt861 p.2, sn74lvc861a p.2: OEAB low = A to B, OEBA low = B to A, both high = isolation, both low = latch A and B (A = B). Timing sn74abt861 p.5, sn74lvc861a p.4.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "B10"], "oeab": {"pin": "OEAB", "active": "low"}, "oeba": {"pin": "OEBA", "active": "low"}}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 3.4 | 3.2 | [4.5, 5.5] |  |  |  | sn74abt861 typ tPLH/tPHL A or B -> B or A at 5 V, 25 C, 50 pF (p.5). VCC 4.5-5.5 V (p.3) |
| LVC | 6.4 | 6.4 | [1.65, 3.6] | 5.5 |  |  | sn74lvc861a max tpd A or B -> B or A at 3.3 V, 50 pF (no typ printed) (p.4). VCC 1.65-3.6 V, VI to 5.5 V (p.3) |

## 74_863

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; 9-bit bus transceiver, 3-state outputs
- Report: reports/G09.md; parts added: 4
- PDFs: sn74abt863, sn74lvc863a
- Pinouts: `p24` (24 pins) for DW, DGV, PW, DB, NS, NT
- Source: Pinout sn74lvc863a p.1 and sn74abt863 p.1 (same: 1 OEBA1, 2-10 A1-A9, 11 OEBA2, 12 GND, 13 OEAB1, 14 OEAB2, 15-23 B9-B1, 24 VCC). Function table p.2 (both PDFs): OEAB1 and OEAB2 both low = A to B, OEBA1 and OEBA2 both low = B to A, all four low = latch A and B; uses the array-of-controls form of oeab/oeba.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9"], "oeab": [{"pin": "OEAB1", "active": "low"}, {"pin": "OEAB2", "active": "low"}], "oeba": [{"pin": "OEBA1", "active": "low"}, {"pin": "OEBA2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.6 | 2.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A or B -> B or A at 5 V, 50 pF (sn74abt863 p.5); VCC 4.5-5.5 V (p.3) |
| LVC | 6.1 | 6.1 | [1.65, 3.6] | 5.5 |  |  | max tpd A or B -> B or A at 3.3 V (no typical printed) (sn74lvc863a p.4); VCC 1.65-3.6 V, VI 0-5.5 V (p.3) |

## 74_8T245_LV

- Kind / model: `buffer` / `transceiver`; function "Transceiver"; Octal bus transceiver with level-shifting (LVxT) inputs, single supply, 3-state outputs
- Report: reports/G12.md; parts added: 5
- PDFs: sn74lv8t245, sn74lv8t245-q1
- Pinouts: `p20` (20 pins) for DGS, PW, RKS
- Source: SN74LV8T245 is a single-supply part ('245 pinout, one VCC; up/down translation comes from its low-threshold, 5.5 V-tolerant inputs), so it is a transceiver, not a dual-supply translator. Pinout sn74lv8t245 p.3 Table 5-1 (RKS, DGS, PW; RKS thermal pad not a pin); sn74lv8t245-q1 p.3 same. DIR: L = B to A, H = A to B; OE active low (p.3).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 8.8 | 8.8 | [1.6, 5.5] |  |  |  | typ tpd A->B at 3.3 V, 50 pF (sn74lv8t245 p.7); VCC 1.6-5.5 V, VI to 5.5 V (p.5) |

## 1T34_DUAL

- Kind / model: `buffer` / `translator`; function "Level shifter"; 1-bit unidirectional dual-supply voltage-level translator (A to B)
- Report: reports/G07.md; parts added: 1
- PDFs: sn74aup1t34, sn74aup1t34-q1
- Pinouts: `p5` (5 pins) for DCK in sn74aup1t34, sn74aup1t34-q1; `p6` (6 pins) for DRY, DSF in sn74aup1t34, sn74aup1t34-q1
- Source: Pinout sn74aup1t34 p.4 (DCK: 1 VCCA, 2 A, 3 GND, 4 B, 5 VCCB; DRY/DSF: 1 VCCA, 2 A, 3 GND, 4 B, 5 NC, 6 VCCB); sn74aup1t34-q1 p.3 (DCK, same). A is the input referenced to VCCA, B the output referenced to VCCB; no enable.

```json
{"units": [{"a": ["A"], "y": ["B"], "oe": []}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A"]}, {"power": "VCCB", "pins": ["B"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUP | 5.07 | 5.07 | [0.9, 3.6] |  | [0.9, 3.6] |  | sn74aup1t34 max tPLH/tPHL A->B, VCCA 1.65 V to VCCB 3 V, 15 pF (p.9; nearest row to 1.8 V -> 3.3 V, the part tops out at 3.6 V; no typ printed at these supplies). VCCA and VCCB 0.9-3.6 V (p.6); ROC gives no VI row (abs max 4.6 V), so no inputMax |

## 1T45

- Kind / model: `buffer` / `translator`; function "Level shifter"; Single-bit dual-supply bus transceiver / level shifter, direction control
- Report: reports/G14.md; parts added: 41
- PDFs: sn74avc1t45, sn74avc1t45-q1, sn74avch1t45, sn74axc1t45, sn74axc1t45-q1, sn74axch1t45, sn74lvc1t45, sn74lvc1t45-ep, sn74lvc1t45-q1, sn74lxc1t45, sn74lxc1t45-q1, sn74lxch1t45
- Pinouts: `dsbga` (6 pins) for YZP; `p6` (6 pins) for DBV, DCK, DRL, DRY, DTQ, DEA
- Source: Pinout VCCA 1, GND 2, A 3, B 4, DIR 5, VCCB 6, identical in sn74avc1t45 p.4 (DBV, DCK, DRL), sn74avch1t45 p.3, sn74axc1t45 p.3 (DBV, DCK, DRL, DEA, DTQ), sn74axch1t45 p.3 (DBV, DCK, DTQ, DRY), sn74lvc1t45 p.2-3, sn74lxc1t45 p.3 (DCK, DRL, DRY, DTQ), sn74lxch1t45 p.3, and the -Q1/-EP data sheets (DCK, DRY). DSBGA (YZP) balls: sn74avc1t45 p.5 and sn74lvc1t45 p.3 (A1 VCCA, A2 VCCB, B1 GND, B2 DIR, C1 A, C2 B), listed in p6 order. DIR high = A to B, low = B to A (sn74avc1t45 p.15 function table, same in all); no OE. A and DIR referenced to VCCA, B to VCCB.

```json
{"units": [{"a": ["A"], "b": ["B"], "dir": {"pin": "DIR", "ab": "high"}, "oe": []}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A", "DIR"]}, {"power": "VCCB", "pins": ["B"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AVC | 2.8 | 2.8 | [1.2, 3.6] |  | [1.2, 3.6] |  | Max tPLH/tPHL A->B at VCCA = VCCB = 3.3 V (nearest row to 3.3<->5 V; no typical) (sn74avc1t45 p.11). VCCA/VCCB 1.2-3.6 V, VI to 3.6 V (p.7). sn74avc1t45-q1 lists 1.08-3.6 V (main PDF used) |
| AXC | 3 | 3 | [0.65, 3.6] |  | [0.65, 3.6] |  | Max tpd A->B at VCCA = VCCB = 3.3 V, -40 to 85 C (nearest row to 3.3<->5 V; no typical) (sn74axc1t45 p.10). VCCA/VCCB 0.65-3.6 V, VI to 3.6 V (p.5) |
| LVC | 4.4 | 4 | [1.65, 5.5] |  | [1.65, 5.5] |  | Max tPLH/tPHL A->B at VCCA = 3.3 V, VCCB = 5 V (no typical) (sn74lvc1t45 p.8). VCCA/VCCB 1.65-5.5 V, VI to 5.5 V (p.4-5) |
| LXC | 12 | 12 | [1.1, 5.5] |  | [1.1, 5.5] |  | Max tpd A->B at VCCA = 3.3 V, VCCB = 5 V, -40 to 85 C (no typical) (sn74lxc1t45 p.13). VCCA/VCCB 1.1-5.5 V, VI to 5.5 V (p.5) |

## 2T245

- Kind / model: `buffer` / `translator`; function "Level shifter"; Dual-bit dual-supply bus transceiver, per-bit direction, common output enable, 3-state outputs
- Report: reports/G04.md; parts added: 3
- PDFs: sn74avc2t245, sn74avc2t245-q1, sn74axc2t245
- Pinouts: `rsw10` (10 pins) for RSW
- Source: Pinout sn74axc2t245 p.2 Table 4-1 (RSW: 1 DIR2, 2 OE, 3 GND, 4 B2, 5 B1, 6 VCCB, 7 VCCA, 8 A1, 9 A2, 10 DIR1; DIR1 controls A1/B1, DIR2 controls A2/B2); sn74avc2t245 p.3 Figure 4-1/Table 4-1 and sn74avc2t245-q1 p.3 same numbers. Function table sn74avc2t245 p.12 / sn74axc2t245 p.20 (each transceiver): OE L, DIRx L -> B to A; OE L, DIRx H -> A to B; OE H -> isolation. DIRx and OE referenced to VCCA.

```json
{"units": [{"a": ["A1"], "b": ["B1"], "dir": {"pin": "DIR1", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}, {"a": ["A2"], "b": ["B2"], "dir": {"pin": "DIR2", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "DIR1", "DIR2", "OE"]}, {"power": "VCCB", "pins": ["B1", "B2"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AVC | 2.4 | 2.4 | [1.2, 3.6] |  | [1.2, 3.6] |  | Max A->B at VCCA = 3.3 V, VCCB = 3.3 V (nearest row to 3.3<->5 V; no 5 V supply; no typical printed), SN74AVC2T245 (sn74avc2t245 p.9). VCCA and VCCB 1.2-3.6 V, VI 0-3.6 V (sn74avc2t245 p.4-5) |
| AXC | 4 | 4 | [0.65, 3.6] |  | [0.65, 3.6] |  | Max tpd A->B at VCCA = 3.3 V, VCCB = 3.3 V, -40 to 85 C (nearest row to 3.3<->5 V; no typical printed), SN74AXC2T245 (sn74axc2t245 p.13). VCCA and VCCB 0.65-3.6 V, VI 0-3.6 V (sn74axc2t245 p.4) |

## 2T45

- Kind / model: `buffer` / `translator`; function "Level shifter"; Dual-bit dual-supply bus transceiver with configurable voltage translation, 3-state outputs
- Report: reports/G16.md; parts added: 26
- PDFs: sn74avc2t45, sn74avc2t45-q1, sn74avch2t45, sn74axc2t45, sn74axc2t45-q1, sn74axch2t45, sn74lvc2t45, sn74lvc2t45-ep, sn74lvc2t45-q1, sn74lxc2t45, sn74lxc2t45-q1
- Pinouts: `p8` (8 pins) for DCT, DCU, DDF, DTM, DTT; `yzp` (8 pins) for YZP
- Source: Numbered pinout 1 VCCA, 2 A1, 3 A2, 4 GND, 5 DIR, 6 B2, 7 B1, 8 VCCB, same in every PDF and package: sn74lvc2t45 p.2 (DCT/DCU), sn74lvc2t45-q1 p.4 (DCU), sn74lvc2t45-ep p.1 (DCT/DCU), sn74avc2t45 p.3 (DCT/DCU; DDF not drawn, assumed the same numbering), sn74avc2t45-q1 p.4 (DCU/DTT), sn74avch2t45 p.3, sn74axc2t45 p.2 (DCT/DCU/DTM), sn74axch2t45 p.3 (DCU/DTM), sn74lxc2t45 p.3 (DCT/DCU/DTT/DTM), sn74lxc2t45-q1 p.3 (DCU/DTT). YZP balls (bottom view): A1 VCCA, A2 VCCB, B1 A1, B2 B1, C1 A2, C2 B2, D1 GND, D2 DIR (sn74lvc2t45 p.3, sn74avc2t45 p.3). Function table: DIR L = B to A, H = A to B (sn74lvc2t45 p.15); no OE. DIR referenced to VCCA.

```json
{"units": [{"a": ["A1", "A2"], "b": ["B1", "B2"], "dir": {"pin": "DIR", "ab": "high"}, "oe": []}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "DIR"]}, {"power": "VCCB", "pins": ["B1", "B2"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVC | 4.4 | 4.0 | [1.65, 5.5] | 5.5 | [1.65, 5.5] | 5.5 | Max tPLH/tPHL A->B at VCCA 3.3 V, VCCB 5 V, -40 to 85 C; no typical given (sn74lvc2t45 p.8). VCCA/VCCB 1.65-5.5 V, VI 0-5.5 V (sn74lvc2t45 p.4-5) |
| AVC | 2.4 | 2.4 | [1.2, 3.6] | 3.6 | [1.2, 3.6] | 3.6 | Max tPLH/tPHL A->B at VCCA 3.3 V, VCCB 3.3 V (nearest row to 3.3<->5 V; parts top out at 3.6 V), -40 to 85 C (sn74avc2t45 p.9; sn74avch2t45 p.10 identical). VCCA/VCCB 1.2-3.6 V, VI 0-3.6 V (sn74avc2t45 p.5) |
| AXC | 4 | 4 | [0.65, 3.6] | 3.6 | [0.65, 3.6] | 3.6 | Max tpd A->B at VCCA 3.3 V, VCCB 3.3 V (nearest row to 3.3<->5 V), -40 to 85 C (sn74axc2t45 p.13; sn74axch2t45 same 4 ns). VCCA/VCCB 0.65-3.6 V, VI 0-3.6 V (sn74axc2t45 p.4) |
| LXC | 11 | 11 | [1.1, 5.5] | 5.5 | [1.1, 5.5] | 5.5 | Max tpd A->B at VCCA 3.3 V, VCCB 5 V, -40 to 85 C (sn74lxc2t45 p.14). VCCA/VCCB 1.1-5.5 V, VI 0-5.5 V (sn74lxc2t45 p.5) |

## 40109B

- Kind / model: `buffer` / `translator`; function "Level shifter"; Quad low-to-high voltage level shifter, 3-state outputs
- Report: reports/G11.md; parts added: 5
- PDFs: cd40109b, cd40109b-q1
- Pinouts: `p16` (16 pins) for N, NS, PW, J
- Source: p16 from cd40109b (scanned Harris data) p.4 terminal assignment: 1 VCC, 2 ENABLE A, 3 A, 4 E, 5 F, 6 B, 7 ENABLE B, 8 VSS, 9 ENABLE C, 10 C, 11 G, 12 NC, 13 H, 14 D, 15 ENABLE D, 16 VDD; ENABLE A-D shortened to ENA-END. Logic diagram p.1: A (pin 3) -> E (pin 4), enable A on pin 2, both inputs through VCC-side level shifters, output on VDD. Truth table p.1 (cd40109b-q1 p.2): ENABLE 1 -> output = input, ENABLE 0 -> Z. Inputs and enables are in the VCC (low) domain, E-H in the VDD (high) domain. VCC may exceed VDD (then it shifts high to low).

```json
{"units": [{"a": ["A"], "y": ["E"], "oe": [{"pin": "ENA", "active": "high"}]}, {"a": ["B"], "y": ["F"], "oe": [{"pin": "ENB", "active": "high"}]}, {"a": ["C"], "y": ["G"], "oe": [{"pin": "ENC", "active": "high"}]}, {"a": ["D"], "y": ["H"], "oe": [{"pin": "END", "active": "high"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCC", "pins": ["A", "B", "C", "D", "ENA", "ENB", "ENC", "END"]}, {"power": "VDD", "pins": ["E", "F", "G", "H"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| C | 130 | 300 | [3, 18] | 18 | [3, 18] |  | Typical tPLH/tPHL data input -> output, L-H mode, VCC 5 V -> VDD 10 V (nearest row to 3.3 <-> 5 V), 25 C, 50 pF (cd40109b p.3). Supply range 3-18 V (cd40109b p.1, recommended operating boundary Fig. 10 p.3; cd40109b-q1 p.3); no separate VCC range is printed, so both domains take 3-18 V. Inputs may exceed VCC and VDD (p.1), IIN tested at 18 V (p.2), hence inputMax 18 |

## 4010B

- Kind / model: `buffer` / `translator`; function "Level shifter"; CMOS hex non-inverting buffer/converter (high-to-low level)
- Report: reports/G12.md; parts added: 7
- PDFs: cd4010b, cd4010b-q1
- Pinouts: `p16` (16 pins) for N, D, NS, PW
- Source: Pinout cd4010b p.1 terminal assignments (scan) and cd4010b-q1 p.1 (image): VCC pin 1 = output-stage supply, VDD pin 16 = input/logic supply, VSS 8, NC 13; outputs G..L (G = A ... L = F), as 4009UB in gate.json. Recommended VDD 3-18 V, VCC 3 V to VDD, VI VCC to VDD (cd4010b p.2, cd4010b-q1 p.3): converts high-to-low only. Timing from cd4010b-q1 p.5 (main PDF is a scan).

```json
{"units": [{"a": ["A", "B", "C", "D", "E", "F"], "y": ["G", "H", "I", "J", "K", "L"], "oe": []}], "invert": false, "output": "push-pull", "domains": [{"power": "VDD", "pins": ["A", "B", "C", "D", "E", "F"]}, {"power": "VCC", "pins": ["G", "H", "I", "J", "K", "L"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| C | 100 | 65 | [3, 18] |  | [3, 18] |  | typ tPLH/tPHL at VDD = VI = VCC = 5 V, 50 pF, 25 C (cd4010b-q1 p.5; VDD 10 V / VCC 5 V: 50/30); VDD (domain A) 3-18 V, VCC (domain B) 3 V to VDD (cd4010b p.2) |

## 4504B

- Kind / model: `buffer` / `translator`; function "Level shifter"; Hex voltage-level shifter, TTL-to-CMOS or CMOS-to-CMOS
- Report: reports/G05.md; parts added: 7
- PDFs: cd4504b, cd4504b-ep
- Pinouts: `p16` (16 pins) for D, N, PW
- Source: Pinout cd4504b p.1 terminal assignment (scanned; VCC=1, VSS=8, SELECT=13, VDD=16), cd4504b-ep p.1 same. Inputs referenced to VCC, outputs to VDD (p.1 Fig. 1: 'shift input signals from the VCC logic level to the VDD logic level'). SELECT picks TTL (H) or CMOS (L) input thresholds; no logic effect, no enable. Y = A, non-inverting.

```json
{"units": [{"a": ["AIN", "BIN", "CIN", "DIN", "EIN", "FIN"], "y": ["AOUT", "BOUT", "COUT", "DOUT", "EOUT", "FOUT"], "oe": []}], "invert": false, "output": "push-pull", "domains": [{"power": "VCC", "pins": ["AIN", "BIN", "CIN", "DIN", "EIN", "FIN", "SELECT"]}, {"power": "VDD", "pins": ["AOUT", "BOUT", "COUT", "DOUT", "EOUT", "FOUT"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| C | 140 | 140 | [5, 18] |  | [5, 18] |  | typ tPLH/tPHL TTL-to-CMOS, VCC 5 V -> VDD 10 V, 50 pF (cd4504b p.3, the nearest row to 3.3<->5 V); recommended supply 5-18 V (cd4504b p.3, listed for VDD only; VCC taken as the same range - the datasheet characterises VCC 5-15 V and lets VCC exceed VDD) |

## 74_164245

- Kind / model: `buffer` / `translator`; function "Level shifter"; 16-bit dual-supply bus transceiver, 3-state outputs, controls on VCCA
- Report: reports/G10.md; parts added: 10
- PDFs: sn74alvc164245, sn74alvc164245-ep, sn74avca164245, sn74avcah164245
- Pinouts: `p48` (48 pins) for DGG, DL, DGV
- Source: sn74alvc164245 p.3 Pin Functions (DGG/DL; VCCB pins 7, 18; VCCA pins 31, 42); sn74alvc164245-ep p.1 same; sn74avca164245 p.1 (DGG/DGV); sn74avcah164245 p.1 same. Function table (each section): OE low, DIR high = A to B, DIR low = B to A, OE high = isolation. Control pins 1DIR, 2DIR, 1OE, 2OE are powered by VCCA (sn74alvc164245 p.1; sn74avca164245 p.1).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["1DIR", "2DIR", "1OE", "2OE", "1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"]}, {"power": "VCCB", "pins": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALVC | 5.8 | 5.8 | [2.3, 3.6] |  | [3, 5.5] |  | max tpd A->B at VCCA 3.3 V +-0.3 V, VCCB 5 V +-0.5 V, no typical printed (sn74alvc164245 p.10); VCCA 2.3-3.6 V (p.8), VCCB 3-5.5 V (p.7); inputs 0-VCCA / 0-VCCB |
| AVC | 3.1 | 3.1 | [1.4, 3.6] |  | [1.4, 3.6] |  | max tpd A->B at VCCA 3.3 V +-0.3 V, VCCB 3.3 V +-0.3 V (the nearest row to 3.3/5 V; no 5 V side), no typical printed (sn74avca164245 p.8; sn74avcah164245 p.8 same); VCCA and VCCB 1.4-3.6 V (p.5) |

## 74_164245_B

- Kind / model: `buffer` / `translator`; function "Level shifter"; 16-bit dual-supply bus transceiver, 3-state outputs, controls on VCCB
- Report: reports/G10.md; parts added: 3
- PDFs: sn74avcb164245, sn74avcbh164245
- Pinouts: `p48` (48 pins) for DGG, DGV, DL
- Source: sn74avcb164245 p.1 (DGG/DGV) and sn74avcbh164245 p.1 (pin 41 misprinted 1BA5 = 1A5): same pinout and function table as 74_164245, but the control pins 1DIR, 2DIR, 1OE, 2OE are supplied by VCCB (sn74avcb164245 p.1 description), so the domains differ.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"]}, {"power": "VCCB", "pins": ["1DIR", "2DIR", "1OE", "2OE", "1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AVC | 3.1 | 3.1 | [1.4, 3.6] |  | [1.4, 3.6] |  | max tpd A->B at VCCA 3.3 V +-0.3 V, VCCB 3.3 V +-0.3 V (nearest row to 3.3/5 V), no typical printed (sn74avcb164245 p.7; sn74avcbh164245 p.7 same); VCCA and VCCB 1.4-3.6 V (p.5) |

## 74_16T245

- Kind / model: `buffer` / `translator`; function "Level shifter"; 16-bit dual-supply bus transceiver with configurable voltage translation, 3-state outputs
- Report: reports/G13.md; parts added: 15
- PDFs: sn74avc16t245, sn74avc16t245-q1, sn74avch16t245, sn74lvc16t245, sn74lvc16t245-ep, sn74lvch16t245, sn74lvch16t245-ep
- Pinouts: `p48` (48 pins) for DGG, DGV, DL
- Source: Pinout: standard 48-pin, identical in sn74lvc16t245 p.4, sn74lvc16t245-ep p.1, sn74lvch16t245 p.1, sn74lvch16t245-ep p.1, sn74avc16t245 p.4, sn74avc16t245-q1 p.3, sn74avch16t245 p.1. Two 8-bit units; OE low + DIR high = A->B, DIR low = B->A, OE high = isolation (function table sn74lvc16t245 p.14; same in the AVC PDFs). Control pins (1DIR, 2DIR, 1OE, 2OE) are supplied by VCCA (sn74lvc16t245 p.3, sn74avc16t245 p.1).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["1DIR", "2DIR", "1OE", "2OE", "1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"]}, {"power": "VCCB", "pins": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVC | 4.4 | 4.4 | [1.65, 5.5] | 5.5 | [1.65, 5.5] | 5.5 | max tPLH/tPHL A->B at VCCA 3.3 V, VCCB 5 V; no typical printed (sn74lvc16t245 p.10). VCCA/VCCB 1.65-5.5 V, control inputs 0-5.5 V (sn74lvc16t245 p.7; -ep, lvch16t245, lvch16t245-ep same ranges) |
| AVC | 2.7 | 2.7 | [1.2, 3.6] | 3.6 | [1.2, 3.6] | 3.6 | max tPLH/tPHL A->B at VCCA 3.3 V, VCCB 3.3 V (nearest row to 3.3<->5 V; AVC is limited to 3.6 V); no typical printed (sn74avc16t245 p.10). VCCA/VCCB 1.2-3.6 V, VI 0-3.6 V (sn74avc16t245 p.7; -q1 and avch16t245 same ranges) |

## 74_20T245

- Kind / model: `buffer` / `translator`; function "Level shifter"; 20-bit dual-supply bus transceiver / level shifter, 3-state (two 10-bit sections)
- Report: reports/G14.md; parts added: 4
- PDFs: sn74avc20t245, sn74avch20t245
- Pinouts: `p56` (56 pins) for DGG, DGV
- Source: Pinout from sn74avc20t245 p.1 (DGG, DGV) and sn74avch20t245 p.1 (same). Function table p.2 (each 10-bit section): OE low + DIR high = A to B, DIR low = B to A, OE high = isolation. Control inputs 1DIR, 2DIR, 1OE, 2OE are supplied by VCCA (p.2). GQL/ZQL BGA not in this group.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "1A9", "1A10"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "1B9", "1B10"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "2A9", "2A10"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8", "2B9", "2B10"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "1A9", "1A10", "2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "2A9", "2A10", "1DIR", "2DIR", "1OE", "2OE"]}, {"power": "VCCB", "pins": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "1B9", "1B10", "2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8", "2B9", "2B10"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AVC | 2.9 | 2.9 | [1.2, 3.6] |  | [1.2, 3.6] |  | Max tPLH/tPHL A->B at VCCA = VCCB = 3.3 V (nearest row to 3.3<->5 V; no typical) (sn74avc20t245 p.8). VCCA/VCCB 1.2-3.6 V, VI to 3.6 V (p.4) |

## 74_24T245

- Kind / model: `buffer` / `translator`; function "Level shifter"; 24-bit dual-supply bus transceiver, configurable voltage translation, 3-state outputs (six 4-bit sections)
- Report: reports/G15.md; parts added: 2
- PDFs: sn74avc24t245, sn74avch24t245
- Pinouts: `nmu83` (83 pins) for NMU
- Source: Ball map sn74avc24t245 p.3 Table 5-1 (NMU 83-ball, G3 unpopulated), same in sn74avch24t245 p.3; function table sn74avc24t245 p.17 (each 4-bit section: OE low + DIR high = A->B, DIR low = B->A, OE high = isolation). DIR and OE referenced to VCCA (p.1, p.3).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "b": ["1B1", "1B2", "1B3", "1B4"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "b": ["2B1", "2B2", "2B3", "2B4"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}, {"a": ["3A1", "3A2", "3A3", "3A4"], "b": ["3B1", "3B2", "3B3", "3B4"], "dir": {"pin": "3DIR", "ab": "high"}, "oe": [{"pin": "3OE", "active": "low"}]}, {"a": ["4A1", "4A2", "4A3", "4A4"], "b": ["4B1", "4B2", "4B3", "4B4"], "dir": {"pin": "4DIR", "ab": "high"}, "oe": [{"pin": "4OE", "active": "low"}]}, {"a": ["5A1", "5A2", "5A3", "5A4"], "b": ["5B1", "5B2", "5B3", "5B4"], "dir": {"pin": "5DIR", "ab": "high"}, "oe": [{"pin": "5OE", "active": "low"}]}, {"a": ["6A1", "6A2", "6A3", "6A4"], "b": ["6B1", "6B2", "6B3", "6B4"], "dir": {"pin": "6DIR", "ab": "high"}, "oe": [{"pin": "6OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["1A1", "1A2", "1A3", "1A4", "2A1", "2A2", "2A3", "2A4", "3A1", "3A2", "3A3", "3A4", "4A1", "4A2", "4A3", "4A4", "5A1", "5A2", "5A3", "5A4", "6A1", "6A2", "6A3", "6A4", "1DIR", "2DIR", "3DIR", "4DIR", "5DIR", "6DIR", "1OE", "2OE", "3OE", "4OE", "5OE", "6OE"]}, {"power": "VCCB", "pins": ["1B1", "1B2", "1B3", "1B4", "2B1", "2B2", "2B3", "2B4", "3B1", "3B2", "3B3", "3B4", "4B1", "4B2", "4B3", "4B4", "5B1", "5B2", "5B3", "5B4", "6B1", "6B2", "6B3", "6B4"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AVC | 2.7 | 2.7 | [1.2, 3.6] |  | [1.2, 3.6] |  | max tPLH/tPHL A->B at VCCA 3.3 V, VCCB 3.3 V (nearest row to 3.3<->5 V; the parts stop at 3.6 V), no typ printed (sn74avc24t245 p.12; sn74avch24t245 p.12 same); VCCA and VCCB 1.2-3.6 V, VI 0-3.6 V (p.7) |

## 74_3245

- Kind / model: `buffer` / `translator`; function "Level shifter"; Octal dual-supply bus transceiver, 3-state outputs (A 2.3-3.6 V, B 3-5.5 V)
- Report: reports/G10.md; parts added: 8
- PDFs: sn74lvcc3245a
- Pinouts: `p24` (24 pins) for DB, DBQ, DW, NS, PW
- Source: sn74lvcc3245a (SCAS585R) p.3 Figure 4-1 and Table 4-1 (DB/DBQ/DW/NS/PW, same 24-pin pinout; pin 23 NC); function table p.13 Table 7-1: OE low, DIR high = A to B, DIR low = B to A, OE high = isolation; control inputs referenced to VCCA (p.1, p.5).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["DIR", "OE", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"]}, {"power": "VCCB", "pins": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVC | 5.3 | 6 | [2.3, 3.6] |  | [3, 5.5] |  | max tPLH/tPHL A->B at VCCA 2.7-3.6 V, VCCB 5 V +-0.5 V, no typical printed (sn74lvcc3245a p.7); VCCA 2.3-3.6 V, VCCB 3-5.5 V, inputs 0-VCCA / 0-VCCB (p.4-5) |

## 74_32T245

- Kind / model: `buffer` / `translator`; function "Level shifter"; 32-bit dual-supply bus transceiver / level shifter (four 8-bit sections), 3-state outputs
- Report: reports/G12.md; parts added: 1
- PDFs: sn74avc32t245
- Pinouts: `nfbga96` (96 pins) for NMJ
- Source: Ball map sn74avc32t245 p.4-5 Table 6-1 Pin Assignments (GKE/ZKE/ZRL/NMJ share it; NMJ 96-ball nFBGA used), Pin Functions p.5-7. Function table p.18 (each 8-bit section): OE low enables, DIR high = A to B. A ports and nOE referenced to VCCA, B ports to VCCB; nDIR put in the VCCA domain (as in the 8T245).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "b": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "b": ["2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}, {"a": ["3A1", "3A2", "3A3", "3A4", "3A5", "3A6", "3A7", "3A8"], "b": ["3B1", "3B2", "3B3", "3B4", "3B5", "3B6", "3B7", "3B8"], "dir": {"pin": "3DIR", "ab": "high"}, "oe": [{"pin": "3OE", "active": "low"}]}, {"a": ["4A1", "4A2", "4A3", "4A4", "4A5", "4A6", "4A7", "4A8"], "b": ["4B1", "4B2", "4B3", "4B4", "4B5", "4B6", "4B7", "4B8"], "dir": {"pin": "4DIR", "ab": "high"}, "oe": [{"pin": "4OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "3A1", "3A2", "3A3", "3A4", "3A5", "3A6", "3A7", "3A8", "4A1", "4A2", "4A3", "4A4", "4A5", "4A6", "4A7", "4A8", "1DIR", "2DIR", "3DIR", "4DIR", "1OE", "2OE", "3OE", "4OE"]}, {"power": "VCCB", "pins": ["1B1", "1B2", "1B3", "1B4", "1B5", "1B6", "1B7", "1B8", "2B1", "2B2", "2B3", "2B4", "2B5", "2B6", "2B7", "2B8", "3B1", "3B2", "3B3", "3B4", "3B5", "3B6", "3B7", "3B8", "4B1", "4B2", "4B3", "4B4", "4B5", "4B6", "4B7", "4B8"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AVC | 2.7 | 2.7 | [1.2, 3.6] |  | [1.2, 3.6] |  | max tPLH/tPHL A->B at VCCA 3.3 V, VCCB 3.3 V (nearest row to 3.3<->5 V; device max 3.6 V) (sn74avc32t245 p.13, no typ printed); VCCA/VCCB 1.2-3.6 V, VI to 3.6 V (p.9) |

## 74_4245

- Kind / model: `buffer` / `translator`; function "Level shifter"; Octal bus transceiver and 3.3-V to 5-V level shifter, 3-state outputs
- Report: reports/G06.md; parts added: 6
- PDFs: sn74lvc4245a, sn74lvc4245a-ep
- Pinouts: `p24` (24 pins) for DB, DW, PW
- Source: Pinout sn74lvc4245a p.3 (DB, DW, PW): VCCA 1, DIR 2, A1-A8 3-10, GND 11-13, B8-B1 14-21, OE 22, VCCB 23-24 (figure prints pin 23 'NC,VCCB'; the pin table and sn74lvc4245a-ep p.1 say VCCB). Function table p.11: OE L DIR L B->A, OE L DIR H A->B, OE H isolation. A port and DIR/OE referenced to VCCA (5-V side, p.1), B port to VCCB (3.3-V side).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "DIR", "OE"]}, {"power": "VCCB", "pins": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVC | 6.7 | 6.3 | [4.5, 5.5] |  | [2.7, 3.6] |  | max tPLH/tPHL A->B at VCCA 5 V +-0.5 V, VCCB 2.7-3.6 V, 50 pF (sn74lvc4245a p.7; no typical printed); VCCA 4.5-5.5 V, VCCB 2.7-3.6 V, inputs 0..own VCC (p.5); -ep same ranges |

## 74_4245_LVCC

- Kind / model: `buffer` / `translator`; function "Level shifter"; Octal dual-supply bus transceiver, 3-state outputs (B port 2.7-5.5 V)
- Report: reports/G06.md; parts added: 6
- PDFs: sn74lvcc4245a
- Pinouts: `p24` (24 pins) for DB, DW, PW, NS
- Source: Pinout sn74lvcc4245a p.3 (DB, DW, NS, PW): VCCA 1, DIR 2, A1-A8 3-10, GND 11-13, B8-B1 14-21, OE 22, NC 23, VCCB 24. Function table: OE L DIR L B->A, OE L DIR H A->B, OE H isolation. DIR/OE powered by VCCA (p.1). Separate device from 74_4245: pin 23 is NC and the VCCB range differs (2.7-5.5 V vs 2.7-3.6 V) while both are family LVC.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "DIR", "OE"]}, {"power": "VCCB", "pins": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVC | 7 | 7 | [4.5, 5.5] |  | [2.7, 5.5] |  | max tPLH/tPHL A->B at VCCA 5 V +-0.5 V, VCCB 2.7-3.6 V, 50 pF (sn74lvcc4245a p.7; no typical printed); VCCA 4.5-5.5 V, VCCB 2.7-5.5 V, inputs 0..own VCC (p.5) |

## 74_4T245

- Kind / model: `buffer` / `translator`; function "Level shifter"; 4-bit dual-supply bus transceiver with configurable voltage translation, 3-state outputs
- Report: reports/G16.md; parts added: 25
- PDFs: sn74avc4t245, sn74avc4t245-q1, sn74avch4t245, sn74avch4t245-ep, sn74avch4t245-q1, sn74axc4t245, sn74axc4t245-q1, sn74axch4t245
- Pinouts: `p16` (16 pins) for D, DGV, PW, RGY, DYY; `rsv16` (16 pins) for RSV
- Source: D/DGV/PW/RGY/DYY pinout and RSV pinout from sn74avc4t245 p.4-5 (Figures 4-1..4-5, Table 4-1); same in sn74avch4t245 pin table, sn74axc4t245 (PW/RSV), sn74axc4t245-q1 (PW/RSV), sn74axch4t245 (PW/RSV), sn74avc4t245-q1 (PW/DYY), sn74avch4t245-q1 (PW), sn74avch4t245-ep p.1 (RSV). Two 2-bit sections. Function table (sn74avc4t245 p.16): OE L + DIR L = B to A, OE L + DIR H = A to B, OE H = isolation. DIR and OE referenced to VCCA.

```json
{"units": [{"a": ["1A1", "1A2"], "b": ["1B1", "1B2"], "dir": {"pin": "1DIR", "ab": "high"}, "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2"], "b": ["2B1", "2B2"], "dir": {"pin": "2DIR", "ab": "high"}, "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["1A1", "1A2", "2A1", "2A2", "1DIR", "2DIR", "1OE", "2OE"]}, {"power": "VCCB", "pins": ["1B1", "1B2", "2B1", "2B2"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AVC | 2.9 | 2.9 | [1.2, 3.6] | 3.6 | [1.2, 3.6] | 3.6 | Max tPLH/tPHL A->B at VCCA 3.3 V, VCCB 3.3 V (nearest row to 3.3<->5 V; parts top out at 3.6 V), -40 to 85 C (sn74avc4t245 p.12; sn74avch4t245 p.12 same). VCCA/VCCB 1.2-3.6 V, VI 0-3.6 V (sn74avc4t245 p.7) |
| AXC | 4 | 4 | [0.65, 3.6] | 3.6 | [0.65, 3.6] | 3.6 | Max tpd A->B at VCCA 3.3 V, VCCB 3.3 V (nearest row to 3.3<->5 V), -40 to 85 C (sn74axc4t245 p.12; sn74axch4t245 p.15 same). VCCA/VCCB 0.65-3.6 V, VI 0-3.6 V (sn74axc4t245 p.6) |

## 74_4T774

- Kind / model: `buffer` / `translator`; function "Level shifter"; 4-bit dual-supply bus transceiver / level shifter, per-bit direction control, 3-state
- Report: reports/G14.md; parts added: 9
- PDFs: sn74avc4t774, sn74avc4t774-q1, sn74axc4t774, sn74axc4t774-q1
- Pinouts: `rsv16` (16 pins) for RSV; `p16` (16 pins) for PW, RGY, BQB, DYY
- Source: Pinout from sn74avc4t774 p.4-5 Table 4-1 (PW, RGY, BQB, DYY: DIR1 1 ... VCCA 16; RSV UQFN renumbered: A1 1 ... DIR2 16) and sn74axc4t774 p.3 (PW, BQB, RSV, same); -Q1 data sheets PW only, same. RGY/BQB thermal pad is not a pin. Function table sn74avc4t774 p.15 / sn74axc4t774 p.21: OE low + DIRn high = An to Bn, DIRn low = Bn to An, OE high = isolation (OE common to all 4 bits). DIR1-4 and OE referenced to VCCA.

```json
{"units": [{"a": ["A1"], "b": ["B1"], "dir": {"pin": "DIR1", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}, {"a": ["A2"], "b": ["B2"], "dir": {"pin": "DIR2", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}, {"a": ["A3"], "b": ["B3"], "dir": {"pin": "DIR3", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}, {"a": ["A4"], "b": ["B4"], "dir": {"pin": "DIR4", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "A3", "A4", "DIR1", "DIR2", "DIR3", "DIR4", "OE"]}, {"power": "VCCB", "pins": ["B1", "B2", "B3", "B4"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AVC | 2.9 | 2.9 | [1.1, 3.6] |  | [1.1, 3.6] |  | Max tPLH/tPHL A->B at VCCA = VCCB = 3.3 V (nearest row to 3.3<->5 V; no typical) (sn74avc4t774 p.11). VCCA/VCCB 1.1-3.6 V (p.6), VI to 3.6 V (p.7). sn74avc4t774-q1 lists 1.08-3.6 V (main PDF used) |
| AXC | 5 | 5 | [0.65, 3.6] |  | [0.65, 3.6] |  | Max tpd A->B at VCCA = VCCB = 3.3 V, -40 to 85 C (nearest row to 3.3<->5 V; no typical) (sn74axc4t774 p.14). VCCA/VCCB 0.65-3.6 V, VI to 3.6 V (p.5) |

## 74_8T245

- Kind / model: `buffer` / `translator`; function "Level shifter"; 8-bit dual-supply bus transceiver / level shifter, 3-state outputs
- Report: reports/G12.md; parts added: 27
- PDFs: sn74avc8t245, sn74avc8t245-q1, sn74avch8t245, sn74lvc8t245, sn74lvc8t245-ep, sn74lvc8t245-q1, sn74lvch8t245, sn74lxc8t245, sn74lxc8t245-q1, sn74lxch8t245
- Pinouts: `p24` (24 pins) for DB, DGS, DGV, DW, NS, PW, RHL
- Source: Pinout sn74avc8t245 p.3 Pin Functions (DGV, PW, RHL); sn74avc8t245-q1 p.3 (PW, RHL figures); sn74avch8t245 p.3; sn74lvc8t245 p.3 (DB, DW, NS, PW); sn74lvch8t245 p.3 (DB, DGV, PW, RHL); sn74lxc8t245 p.3 (PW, DGS, RHL numbering; RJW differs, not used); sn74lxc8t245-q1 p.3; sn74lxch8t245 p.3 - all the same 24-pin assignment. DIR high = A to B, OE low enables (sn74lxc8t245 p.22 function table; same in the others). DIR and OE referenced to VCCA. Bus hold (AVCH, LVCH, LXCH) not modeled.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "DIR", "OE"]}, {"power": "VCCB", "pins": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AVC | 2.5 | 2.5 | [1.2, 3.6] |  | [1.2, 3.6] |  | max tPLH/tPHL A->B at VCCA 3.3 V, VCCB 3.3 V (nearest row to 3.3<->5 V; device max 3.6 V), -40 to 85 C (sn74avc8t245 p.10, no typ printed); VCCA/VCCB 1.2-3.6 V, VI to 3.6 V (p.5) |
| LVC | 4.4 | 4.4 | [1.65, 5.5] |  | [1.65, 5.5] |  | max tPLH/tPHL A->B at VCCA 3.3 V, VCCB 5 V (sn74lvc8t245 p.11, no typ printed); VCCA/VCCB 1.65-5.5 V, control VI to 5.5 V (p.7) |
| LXC | 6.5 | 6.5 | [1.08, 5.5] |  | [1.08, 5.5] |  | max tpd A->B at VCCA 3.3 V, VCCB 5 V, -40 to 85 C (sn74lxc8t245 p.13, no typ printed); VCCA/VCCB 1.08-5.5 V, VI to 5.5 V (p.5) |

## TXU0101

- Kind / model: `buffer` / `translator`; function "Level shifter"; 1-bit fixed-direction dual-supply level translator, Schmitt-trigger input, 3-state output
- Report: reports/G06.md; parts added: 6
- PDFs: txu0101, txu0101-q1
- Pinouts: `p6` (6 pins) for DBV, DCK, DRY, DTQ
- Source: Pinout txu0101 p.3 (DBV, DCK, DRY, DTQ all the same): VCCA 1, GND 2, A 3, BY 4, OE 5, VCCB 6; txu0101-q1 p.3 same (DCK, DRY). A -> BY when OE high, OE low = BY Z (p.1, p.3). A referenced to VCCA, BY to VCCB; OE works from either supply (VCC(MIN) logic), put in the VCCA domain. Inputs have weak pull-downs (not modeled).

```json
{"units": [{"a": ["A"], "y": ["BY"], "oe": [{"pin": "OE", "active": "high"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A", "OE"]}, {"power": "VCCB", "pins": ["BY"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TXU | 8 | 8 | [1.08, 5.5] |  | [1.08, 5.5] |  | max tpd A->B at VCCA 3.3 V +-0.3 V, VCCB 5 V +-0.5 V, -40 to 85 C (txu0101 p.13; no typical printed); VCCA and VCCB 1.08-5.5 V, VI 0-5.5 V on every input (p.5); -q1 p.5 same |

## TXU0102

- Kind / model: `buffer` / `translator`; function "Level shifter"; Dual-bit fixed-direction voltage-level translator, Schmitt-trigger inputs, 3-state outputs
- Report: reports/G13.md; parts added: 5
- PDFs: txu0102, txu0102-q1
- Pinouts: `p8` (8 pins) for DCU, DTM, DTT
- Source: Pinout: txu0102 p.4 Figures 6-1..6-3 + Table 6-1 (DCU, DTT, DTM: same numbering); txu0102-q1 p.3 (DCU, DTT, same). Both bits A->B (A1->B1Y, A2->B2Y); OE high enables, low = all outputs Hi-Z (function table txu0102 p.23). OE may be driven from either VCCA or VCCB logic levels (VCC(MIN) circuitry); placed in the VCCA domain. Inputs and OE have weak internal pull-downs.

```json
{"units": [{"a": ["A1", "A2"], "y": ["B1Y", "B2Y"], "oe": [{"pin": "OE", "active": "high"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "OE"]}, {"power": "VCCB", "pins": ["B1Y", "B2Y"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TXU | 8 | 8 | [1.08, 5.5] | 5.5 | [1.08, 5.5] | 5.5 | max tpd A->B at VCCA 3.3 V, VCCB 5.0 V, -40 to 85 C; no typical printed (txu0102 p.14). VCCA and VCCB 1.08-5.5 V, VI 0-5.5 V (txu0102 p.6; txu0102-q1 same) |

## TXU0104

- Kind / model: `buffer` / `translator`; function "Level shifter"; 4-bit fixed-direction (A to B) dual-supply level shifter, Schmitt-trigger inputs, 3-state outputs
- Report: reports/G16.md; parts added: 1
- PDFs: txu0104-q1
- Pinouts: `p14` (14 pins) for PW
- Source: Pinout from txu0104-q1 p.4 (PW; WBQA same numbering). Function table (txu0104-q1 p.24): OE H -> outputs follow inputs, OE L -> Hi-Z. OE may be driven from either supply's levels (datasheet: VCC(MIN) control logic); placed in the VCCA domain. Inputs have static pull-downs (not modeled).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4"], "y": ["B1Y", "B2Y", "B3Y", "B4Y"], "oe": [{"pin": "OE", "active": "high"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "A3", "A4", "OE"]}, {"power": "VCCB", "pins": ["B1Y", "B2Y", "B3Y", "B4Y"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TXU | 8 | 8 | [1.08, 5.5] | 5.5 | [1.08, 5.5] | 5.5 | Max tpd A->B at VCCA 3.3 V, VCCB 5 V, -40 to 85 C (txu0104-q1 p.14). VCCA/VCCB 1.08-5.5 V, VI 0-5.5 V (txu0104-q1 p.6) |

## TXU0202

- Kind / model: `buffer` / `translator`; function "Level shifter"; Dual-bit fixed-direction level shifter (one A to B, one B to A), Schmitt inputs, 3-state
- Report: reports/G14.md; parts added: 4
- PDFs: txu0202, txu0202-q1
- Pinouts: `p8` (8 pins) for DCU, DTT
- Source: Pinout from txu0202 p.4 (DCU, DTT; DTM not in this group) and txu0202-q1 p.3 (same): B2 1, GND 2, VCCA 3, A2Y 4, A1 5, OE 6, VCCB 7, B1Y 8. Channel 1 A1 -> B1Y, channel 2 B2 -> A2Y. OE high = enabled, low = all outputs Hi-Z (p.23 function table). OE may be driven from either VCCA or VCCB levels (p.3); put in the VCCA domain.

```json
{"units": [{"a": ["A1"], "y": ["B1Y"], "oe": [{"pin": "OE", "active": "high"}]}, {"a": ["B2"], "y": ["A2Y"], "oe": [{"pin": "OE", "active": "high"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2Y", "OE"]}, {"power": "VCCB", "pins": ["B2", "B1Y"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TXU | 8 | 8 | [1.08, 5.5] |  | [1.08, 5.5] |  | Max tpd A->B at VCCA = 3.3 V, VCCB = 5 V, -40 to 85 C (B->A 10 ns; no typical) (txu0202 p.14). VCCA/VCCB 1.08-5.5 V, VI to 5.5 V (p.6) |

## TXU0204

- Kind / model: `buffer` / `translator`; function "Level shifter"; 4-bit fixed-direction dual-supply level shifter (2 A-to-B, 2 B-to-A), Schmitt-trigger inputs, 3-state outputs
- Report: reports/G15.md; parts added: 5
- PDFs: txu0204, txu0204-q1
- Pinouts: `p14` (14 pins) for PW, BQA; `p12` (12 pins) for RUT, DTR
- Source: Pinouts txu0204 p.4 Table 6-1 (PW/BQA 14-pin, NC on 6 and 9; RUT/DTR 12-pin), txu0204-q1 p.4 (PW/WBQA same as PW). OE high (to VCCA or VCCB level, VCC(MIN) logic) enables all outputs, low = high impedance (p.1, p.4). A1/A2 -> B1Y/B2Y, B3/B4 -> A3Y/A4Y. OE placed in the VCCA domain (sheet: either supply). Inputs have weak pulldowns (not modeled).

```json
{"units": [{"a": ["A1", "A2"], "y": ["B1Y", "B2Y"], "oe": [{"pin": "OE", "active": "high"}]}, {"a": ["B3", "B4"], "y": ["A3Y", "A4Y"], "oe": [{"pin": "OE", "active": "high"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "A3Y", "A4Y", "OE"]}, {"power": "VCCB", "pins": ["B1Y", "B2Y", "B3", "B4"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TXU | 8 | 8 | [1.08, 5.5] |  | [1.08, 5.5] |  | max tpd A->B at VCCA 3.3 V, VCCB 5 V, -40 to 85 C, no typ printed (txu0204 p.14; B->A max 10); VCCA and VCCB 1.08-5.5 V, VI 0-5.5 V (p.6); -Q1 same (txu0204-q1 p.6) |

## TXU0304

- Kind / model: `buffer` / `translator`; function "Level shifter"; 4-bit fixed-direction (3 A to B, 1 B to A) dual-supply level shifter, Schmitt-trigger inputs, 3-state outputs
- Report: reports/G16.md; parts added: 3
- PDFs: txu0304, txu0304-q1
- Pinouts: `p14` (14 pins) for PW; `rut12` (12 pins) for RUT
- Source: PW pinout and RUT pinout from txu0304 p.4 (Table 6-1: PW/BQA and RUT/DTR columns); txu0304-q1 p.4 PW the same. A1-A3 in (VCCA) -> B1Y-B3Y out (VCCB); B4 in (VCCB) -> A4Y out (VCCA). Function table (txu0304 p.23): OE H -> outputs follow inputs, OE L -> Hi-Z. OE may be driven from either supply's levels; placed in the VCCA domain. Inputs have static pull-downs (not modeled).

```json
{"units": [{"a": ["A1", "A2", "A3"], "y": ["B1Y", "B2Y", "B3Y"], "oe": [{"pin": "OE", "active": "high"}]}, {"a": ["B4"], "y": ["A4Y"], "oe": [{"pin": "OE", "active": "high"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "A3", "A4Y", "OE"]}, {"power": "VCCB", "pins": ["B1Y", "B2Y", "B3Y", "B4"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TXU | 8 | 8 | [1.08, 5.5] | 5.5 | [1.08, 5.5] | 5.5 | Max tpd A->B at VCCA 3.3 V, VCCB 5 V, -40 to 85 C (txu0304 p.14). VCCA/VCCB 1.08-5.5 V, VI 0-5.5 V (txu0304 p.6) |

## TXV0106

- Kind / model: `buffer` / `translator`; function "Level shifter"; 6-bit fixed-direction dual-supply voltage translator, 3-state outputs
- Report: reports/G02.md; parts added: 1
- PDFs: txv0106
- Pinouts: `bqb16` (16 pins) for BQB
- Source: txv0106 p.3: BQB WQFN-16 pinout 1 VCCA, 2-7 A1-A6, 8 GND, 9 OE, 10-15 B6-B1, 16 VCCB (+ thermal pad, not a pin); A pins and OE referenced to VCCA, B pins to VCCB. Function table p.17: OE low = A data to B bus, OE high = isolation (both ports Hi-Z). Fixed direction A->B. Integrated 10-ohm damping resistor on outputs (not modeled).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6"], "y": ["B1", "B2", "B3", "B4", "B5", "B6"], "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["A1", "A2", "A3", "A4", "A5", "A6", "OE"]}, {"power": "VCCB", "pins": ["B1", "B2", "B3", "B4", "B5", "B6"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TXV | 2.3 | 2.3 | [1.14, 3.6] |  | [1.14, 3.6] |  | max tpd A->B at VCCA 3.3 V, VCCB 3.3 V, 15 pF (txv0106 p.10; no typical printed); VCCA and VCCB 1.14-3.6 V, VI 0-3.6 V (p.5) |

## TXV0108

- Kind / model: `buffer` / `translator`; function "Level shifter"; 8-bit dual-supply direction-controlled voltage translator, 3-state, 1.14-3.6 V each port
- Report: reports/G03.md; parts added: 0
- PDFs: txv0108
- Pinouts: `rgy24` (24 pins) for RGY
- Source: Pinout from txv0108 p.3 (RGY 24-pin VQFN; thermal pad not a pin): VCCA 1, GND 11-13, VCCB 23 and 24. Function table p.17: OE L, DIR L -> B to A; OE L, DIR H -> A to B; OE H -> both ports Hi-Z. DIR and OE referenced to VCCA (p.3). 10-ohm damping output resistors (not modeled).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "b": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"], "dir": {"pin": "DIR", "ab": "high"}, "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull", "domains": [{"power": "VCCA", "pins": ["DIR", "OE", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"]}, {"power": "VCCB", "pins": ["B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"]}]}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TXV | 2.3 | 2.3 | [1.14, 3.6] | 3.6 | [1.14, 3.6] | 3.6 | max tpd A->B at VCCA = 3.3 V, VCCB = 3.3 V, 15 pF (txv0108 p.10; device tops out at 3.6 V so this is the row nearest 3.3 <-> 5 V; no typical printed; B->A 2.3 max; VCCA 1.8 V, VCCB 3.3 V row: A->B 3.1, B->A 3.9 max, txv0108 p.8); VCCA, VCCB 1.14-3.6 V, VI 0-3.6 V on every port (txv0108 p.5) |

## 1G125

- Kind / model: `buffer` / `tristate`; function "Buffer"; Single bus buffer gate, 3-state output
- Report: reports/G11.md, reports/G13.md; parts added: 39
- PDFs: sn74ahc1g125, sn74ahc1g125-q1, sn74ahct1g125, sn74ahct1g125-q1, sn74auc1g125, sn74auc1g125-ep, sn74aup1g125, sn74lvc1g125, sn74lvc1g125-ep, sn74lvc1g125-q1, sn74lvc1g125b
- Pinouts: `p5` (5 pins) for DBV, DCK, DRL, DPW, DTX; `p6` (6 pins) for DRY, DSF; `dsbga` (5 pins) for YZP; `dsbga6` (6 pins) for YFP; `p5` (5 pins) for DBV, DCK; `p6` (6 pins) for DRY
- Source: p5 from sn74lvc1g125 p.3 Pin Functions (DBV/DCK/DRL/DPW: OE 1, A 2, GND 3, Y 4, VCC 5), sn74aup1g125 p.3, sn74ahc1g125, sn74ahct1g125, sn74auc1g125, sn74lvc1g125b p.3; DTX: sn74ahct1g125-q1 p.3 Figure 4-2 + Table 4-1 (same numbers). p6 (DRY/DSF SON): sn74lvc1g125 p.3, sn74aup1g125 p.3 (N.C. on 5, VCC 6). DSBGA (row-major): YZP 5-ball A1 OE, A2 VCC, B1 A, C1 GND, C2 Y (sn74lvc1g125 p.3, sn74aup1g125 p.3, sn74auc1g125 p.3 Table 5-2); YFP 6-ball adds B2 DNU (sn74aup1g125 p.3). Function: OE L -> Y = A, OE H -> Z. | Only the automotive 1P1G125Q* orderables of sn74lvc1g125-q1 (they belong to the 1G125 device). Pinout: sn74lvc1g125-q1 p.3 Figures 4-1..4-3 + Pin Functions (DBV/DCK 5-pin; DRY 6-pin, NC on 5; OE active low). Timing sn74lvc1g125-q1 p.6.

```json
{"units": [{"a": ["A"], "y": ["Y"], "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AHC | 5.3 | 5.3 | [2, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (sn74ahc1g125 p.6). VCC 2-5.5 V, VI 0-5.5 V (sn74ahc1g125 p.4; -Q1 p.4 same) |
| AHCT | 5.3 | 5.3 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (sn74ahct1g125 p.6). VCC 4.5-5.5 V (sn74ahct1g125 p.4); CAHCT1G125 (-Q1) VCC 3-5.5 V (sn74ahct1g125-q1 p.4) via partVcc |
| AUC | 1 | 1 | [0.8, 2.7] | 3.6 |  |  | Typical tpd A->Y at 1.8 V, 25 C, 15 pF (sn74auc1g125 p.6). VCC 0.8-2.7 V, VI to 3.6 V (sn74auc1g125 p.4; -EP same) |
| AUP | 3.2 | 3.2 | [0.8, 3.6] |  |  |  | Typical tpd A->Y at 3.3 V, 25 C, 15 pF (sn74aup1g125 p.10). VCC 0.8-3.6 V, inputs 0-3.6 V (sn74aup1g125 p.5) |
| LVC | 4.5 | 4.5 | [1.65, 5.5] |  |  |  | Max tpd A->Y at 3.3 V, -40 to 85 C, 50 pF; no typical printed (sn74lvc1g125 p.6). VCC 1.65-5.5 V, VI 0-5.5 V (sn74lvc1g125 p.5; -EP same); SN74LVC1G125B VCC 1.1-5.5 V (sn74lvc1g125b p.4) via partVcc |

- Part range: names starting `CAHCT1G125` (AHCT): vcc [3, 5.5]

- Part range: names starting `SN74LVC1G125B` (LVC): vcc [1.1, 5.5]

## 1G126

- Kind / model: `buffer` / `tristate`; function "Buffer"; Single buffer, 3-state output (active-high enable)
- Report: reports/G14.md, reports/G15.md; parts added: 30
- PDFs: sn74ahc1g126, sn74ahct1g126, sn74ahct1g126-q1, sn74auc1g126, sn74aup1g126, sn74lvc1g126, sn74lvc1g126-ep, sn74lvc1g126-q1
- Pinouts: `son6` (6 pins) for DRY; `p5` (5 pins) for DBV, DCK; `p5` (5 pins) for DBV, DCK, DRL, DPW; `p6` (6 pins) for DRY, DSF; `dsbga` (5 pins) for YZP; `dsbga6` (6 pins) for YFP
- Source: Pinout from sn74lvc1g126-q1 p.3: DBV OE 1, A 2, GND 3, Y 4, VCC 5; DRY OE 1, A 2, GND 3, Y 4, N.C. 5, VCC 6. Function table p.10: OE high = Y follows A, OE low = Z. MERGE with group G15's 1G126 device (same function; parts here are the 1P1G126...Q1 orderables). | Pinouts: sn74lvc1g126 p.3 (DBV/DCK/DRL, DRY/DSF with pin 5 N.C., YZP), sn74aup1g126 p.3 (DPW same numbering as DBV; YFP 6-ball with B2 DNU per the drawing), sn74auc1g126 p.3, sn74ahc1g126 p.3, sn74ahct1g126 p.3, sn74ahct1g126-q1 p.3, sn74lvc1g126-ep p.2. DSBGA balls listed in SOT-23 pin order. OE high = Y drives A.

```json
{"units": [{"a": ["A"], "y": ["Y"], "oe": [{"pin": "OE", "active": "high"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVC | 5.8 | 5.8 | [1.65, 5.5] |  |  |  | Max tpd A->Y at 3.3 V (no typical) (sn74lvc1g126-q1 p.7). VCC 1.65-5.5 V, VI to 5.5 V (p.5) |
| AHC | 5.3 | 5.3 | [2, 5.5] |  |  |  | typ at 5 V, 50 pF (sn74ahc1g126 p.6); VCC 2-5.5 V, VI to 5.5 V (p.4) |
| AHCT | 5.3 | 5.3 | [4.5, 5.5] |  |  |  | typ at 5 V, 50 pF (sn74ahct1g126 p.5); VCC 4.5-5.5 V (p.4); the -Q1 sheet allows 3-5.5 V (sn74ahct1g126-q1 p.4, partVcc) |
| AUC | 1 | 1 | [0.8, 2.7] | 3.6 |  |  | typ tpd at 1.8 V, 15 pF (sn74auc1g126 p.7); VCC 0.8-2.7 V, VI to 3.6 V (p.5) |
| AUP | 3.2 | 3.2 | [0.8, 3.6] |  |  |  | typ tpd at 3.3 V, 25 C, 15 pF (sn74aup1g126 p.9); VCC 0.8-3.6 V (p.4) |

## 1G240

- Kind / model: `buffer` / `tristate`; function "Buffer"; Single inverting buffer/driver, 3-state output
- Report: reports/G07.md; parts added: 17
- PDFs: sn74auc1g240, sn74aup1g240, sn74lvc1g240
- Pinouts: `p5` (5 pins) for DBV, DCK, DPW; `p6` (6 pins) for DRY, DSF; `dsbga` (5 pins) for YZP
- Source: Pinouts sn74aup1g240 p.3 Pin Functions (DBV/DCK/DPW 1 OE, 2 A, 3 GND, 4 Y, 5 VCC; DRY/DSF NC at 5, VCC 6; YZP A1 OE, B1 A, C1 GND, C2 Y, A2 VCC); sn74lvc1g240 p.3 (DBV/DCK, DRY/DSF; its YZP figure numbers pins 1-5 but draws pin 4 Y top-right / 5 VCC bottom-right, balls taken from sn74aup1g240 and sn74auc1g240 p.1); sn74auc1g240 p.1 (DBV/DCK/YZP). OE active low, inverting.

```json
{"units": [{"a": ["A"], "y": ["Y"], "oe": [{"pin": "OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 1 | 1 | [0.8, 2.7] | 3.6 |  |  | sn74auc1g240 typ tpd at 1.8 V, 15 pF (p.4). VCC 0.8-2.7 V, VI 0-3.6 V (p.3) |
| AUP | 3.6 | 3.6 | [0.8, 3.6] |  |  |  | sn74aup1g240 typ tpd at 3.3 V, 15 pF, 25 C (p.9). VCC 0.8-3.6 V (p.5) |
| LVC | 4.5 | 4.5 | [1.65, 5.5] |  |  |  | sn74lvc1g240 max tpd at 3.3 V, 50 pF, -40 to 85 C, no typ printed (p.6). VCC 1.65-5.5 V, VI 0-5.5 V (p.5) |

## 1T125

- Kind / model: `buffer` / `tristate`; function "Buffer"; Single buffer, 3-state output, active-low enable, with level shifting
- Report: reports/G06.md; parts added: 3
- PDFs: sn74lv1t125, sn74lv1t125-q1
- Pinouts: `p5` (5 pins) for DBV, DCK
- Source: Pinout sn74lv1t125 p.4 (DBV, DCK): OE 1 (active low), A 2, GND 3, Y 4, VCC 5; sn74lv1t125-q1 p.3 same. Single-supply translator: reduced input thresholds, inputs 5.5-V tolerant.

```json
{"units": [{"a": ["A"], "y": ["Y"], "oe": [{"pin": "OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 4.9 | 4.9 | [1.6, 5.5] |  |  |  | typ tpd at 3.3 V, 30 pF (sn74lv1t125 p.7); VCC 1.6-5.5 V, VI to 5.5 V (p.5); -q1 p.4 same |

## 1T126

- Kind / model: `buffer` / `tristate`; function "Buffer"; Single bus buffer, 3-state output, active-high enable, level-shifting input
- Report: reports/G13.md; parts added: 3
- PDFs: sn74lv1t126, sn74lv1t126-q1
- Pinouts: `p5` (5 pins) for DBV, DCK
- Source: Pinout: sn74lv1t126 p.4 Figure 5-1 / Table 5-1 (DBV/DCK, OE active high); sn74lv1t126-q1 p.3 (same). Timing sn74lv1t126 p.7.

```json
{"units": [{"a": ["A"], "y": ["Y"], "oe": [{"pin": "OE", "active": "high"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 4.0 | 4.0 | [1.6, 5.5] |  |  |  | typ tpd A->Y at 3.3 V, 15 pF, 25 C (4.9 at 30 pF) (sn74lv1t126 p.7). VCC 1.6-5.5 V, VI 0-5.5 V (sn74lv1t126 p.5; -q1 p.5 same) |

## 2G125

- Kind / model: `buffer` / `tristate`; function "Buffer"; Dual bus buffer gate, 3-state outputs
- Report: reports/G09.md; parts added: 10
- PDFs: sn74auc2g125, sn74aup2g125, sn74lvc2g125, sn74lvc2g125-q1
- Pinouts: `p8` (8 pins) for DCT, DCU, DQE; `rse8` (8 pins) for RSE; `dsbga` (8 pins) for YZP, YFP
- Source: Pinout sn74lvc2g125 p.4 Pin Functions (DCT/DCU, YZP balls A1=1OE B1=1A C1=2Y D1=GND D2=2A C2=1Y B2=2OE A2=VCC); sn74auc2g125 p.1 (DCT/DCU/YZP same); sn74aup2g125 p.1 (DCU/DQE same; YFP/YZP same balls; RSE UQFN top view 1=2OE 2=1Y 3=2A 4=GND 5=2Y 6=1A 7=1OE 8=VCC). Function table OE low = Y=A, OE high = Z (sn74lvc2g125 p.11).

```json
{"units": [{"a": ["1A"], "y": ["1Y"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A"], "y": ["2Y"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 1 | 1 | [0.8, 2.7] | 3.6 |  |  | typ tpd A->Y at 1.8 V, 15 pF (sn74auc2g125 p.4); VCC 0.8-2.7 V, VI to 3.6 V (p.3) |
| AUP | 3.1 | 3.1 | [0.8, 3.6] |  |  |  | typ tpd A->Y at 3.3 V, 15 pF, 25 C (sn74aup2g125 p.7); VCC 0.8-3.6 V, VIH to 3.6 V (p.4) |
| LVC | 4.3 | 4.3 | [1.65, 5.5] |  |  |  | max tpd A->Y at 3.3 V, 30/50 pF, -40 to 85 C (no typical printed) (sn74lvc2g125 p.7); VCC 1.65-5.5 V, VI 0-5.5 V (p.6); -q1 p.3-4 same |

## 2G126

- Kind / model: `buffer` / `tristate`; function "Buffer"; Dual bus buffer gate, 3-state outputs (active-high enables)
- Report: reports/G11.md; parts added: 12
- PDFs: sn74auc2g126, sn74aup2g126, sn74lvc2g126, sn74lvc2g126-ep
- Pinouts: `p8` (8 pins) for DCT, DCU, DQE; `rse8` (8 pins) for RSE; `dsbga` (8 pins) for YZP, YFP
- Source: p8 from sn74lvc2g126 p.3 Figure 5-1 + Pin Functions (DCT/DCU), sn74auc2g126 p.1 (DCT/DCU), sn74aup2g126 p.1 (DCU and DQE X2SON, same numbers). rse8 = SN74AUP2G126 RSE UQFN, different numbering (sn74aup2g126 p.1 and logic diagram p.3: 1 2OE, 2 1Y, 3 2A, 4 GND, 5 2Y, 6 1A, 7 1OE, 8 VCC). DSBGA (row-major) YZP/YFP: A1 1OE, A2 VCC, B1 1A, B2 2OE, C1 2Y, C2 1Y, D1 GND, D2 2A (sn74lvc2g126 p.3 Figure 5-2 + Pin Functions; sn74aup2g126 p.1). Function: OE H -> Y = A, OE L -> Z (sn74aup2g126 p.2).

```json
{"units": [{"a": ["1A"], "y": ["1Y"], "oe": [{"pin": "1OE", "active": "high"}]}, {"a": ["2A"], "y": ["2Y"], "oe": [{"pin": "2OE", "active": "high"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 1.1 | 1.1 | [0.8, 2.7] | 3.6 |  |  | Typical tpd A->Y at 1.8 V, 25 C, 15 pF (sn74auc2g126 p.4). VCC 0.8-2.7 V, VI to 3.6 V (sn74auc2g126 p.3) |
| AUP | 3.7 | 3.7 | [0.8, 3.6] |  |  |  | Typical tpd A->Y at 3.3 V, 25 C, 15 pF (sn74aup2g126 p.7). VCC 0.8-3.6 V (sn74aup2g126 p.4) |
| LVC | 4 | 4 | [1.65, 5.5] |  |  |  | Max tpd A->Y at 3.3 V, -40 to 85 C, 50 pF; no typical printed (sn74lvc2g126 p.6). VCC 1.65-5.5 V, VI 0-5.5 V (sn74lvc2g126 p.5; -EP same) |

## 2G240

- Kind / model: `buffer` / `tristate`; function "Buffer"; Dual inverting buffer/driver, 3-state outputs
- Report: reports/G15.md; parts added: 7
- PDFs: sn74auc2g240, sn74aup2g240, sn74lvc2g240
- Pinouts: `p8` (8 pins) for DCT, DCU, DQE; `rse8` (8 pins) for RSE; `dsbga` (8 pins) for YZP
- Source: Pinouts: sn74lvc2g240 p.1 (DCT/DCU; YZP bottom view + p.18 outline: A1=1OE, B1=1A, C1=2Y, D1=GND, D2=2A, C2=1Y, B2=2OE, A2=VCC; balls listed in DCT pin order), sn74auc2g240 p.1 (DCT/DCU/YZP), sn74aup2g240 p.1 (DCU/DQE same numbering; RSE: 1 2OE, 2 1Y, 3 2A, 4 GND, 5 2Y, 6 1A, 7 1OE, 8 VCC). Function table sn74auc2g240 p.2: OE low, Y = not A.

```json
{"units": [{"a": ["1A"], "y": ["1Y"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A"], "y": ["2Y"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 1.2 | 1.2 | [0.8, 2.7] | 3.6 |  |  | typ tpd at 1.8 V, 15 pF (sn74auc2g240 p.4); VCC 0.8-2.7 V, VI to 3.6 V (p.3) |
| AUP | 3.4 | 3.4 | [0.8, 3.6] |  |  |  | typ tpd at 3.3 V, 25 C, 15 pF (sn74aup2g240 p.6); VCC 0.8-3.6 V, VI 0-3.6 V (p.4) |
| LVC | 4.6 | 4.6 | [1.65, 5.5] |  |  |  | max tpd at 3.3 V, 50 pF, -40 to 85 C, no typ printed (sn74lvc2g240 p.5); VCC 1.65-5.5 V, VI to 5.5 V (p.3) |

## 2G241

- Kind / model: `buffer` / `tristate`; function "Buffer"; Dual buffer/driver, 3-state outputs (1OE active low, 2OE active high)
- Report: reports/G10.md; parts added: 11
- PDFs: sn74auc2g241, sn74aup2g241, sn74lvc2g241
- Pinouts: `p8` (8 pins) for DCU, DCT, DQE; `rse8` (8 pins) for RSE; `dsbga8` (8 pins) for YFP, YZP
- Source: sn74aup2g241 p.1 (DCU/DQE same order; RSE UQFN: 1 2OE, 2 1Y, 3 2A, 4 GND, 5 2Y, 6 1A, 7 1OE, 8 VCC; YFP balls A1-D2 = pins 1-8); sn74auc2g241 p.1 (DCT/DCU, YZP); sn74lvc2g241 p.3 Pin Functions (DCT/DCU, YZP; 1OE active low, 2OE active high). YZP balls are given only as pins 1-8, same positions as YFP.

```json
{"units": [{"a": ["1A"], "y": ["1Y"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A"], "y": ["2Y"], "oe": [{"pin": "2OE", "active": "high"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 1.2 | 1.2 | [0.8, 2.7] | 3.6 |  |  | typ tpd at 1.8 V +-0.15 V, 25 C, CL 15 pF (sn74auc2g241 p.5); VCC 0.8-2.7 V, VI to 3.6 V (p.4) |
| AUP | 3.7 | 3.7 | [0.8, 3.6] |  |  |  | typ tpd at 3.3 V +-0.3 V, 25 C, CL 15 pF (sn74aup2g241 p.7); VCC 0.8-3.6 V, VI to 3.6 V (p.4) |
| LVC | 4.3 | 4.3 | [1.65, 5.5] |  |  |  | max tpd at 3.3 V +-0.3 V, -40 to 85 C, 50 pF, no typical printed (sn74lvc2g241 p.6); VCC 1.65-5.5 V, VI to 5.5 V (p.4) |

## 4503B

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex non-inverting buffer, 3-state outputs (two disable controls)
- Report: reports/G16.md; parts added: 5
- PDFs: cd4503b
- Pinouts: `p16` (16 pins) for N, D, NS, PW
- Source: Scanned Harris datasheet (no text layer); read from the images. Terminal assignment cd4503b p.1: 1 DIS A, 2 D1, 3 Q1, 4 D2, 5 Q2, 6 D3, 7 Q3, 8 VSS, 9 Q4, 10 D4, 11 Q5, 12 D5, 13 Q6, 14 D6, 15 DIS B, 16 VDD. Functional diagram p.1: DIS A gates buffers 1-4, DIS B buffers 5-6. Truth table p.1: DIS 0 -> Q = D, DIS 1 -> high Z.

```json
{"units": [{"a": ["D1", "D2", "D3", "D4"], "y": ["Q1", "Q2", "Q3", "Q4"], "oe": [{"pin": "DISA", "active": "low"}]}, {"a": ["D5", "D6"], "y": ["Q5", "Q6"], "oe": [{"pin": "DISB", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| C | 75 | 55 | [3, 18] |  |  |  | Typical tPLH/tPHL at VDD 5 V and 25 C (50 pF) (cd4503b p.3). VDD 3-18 V (cd4503b p.2) |

## 74_11240

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, inverting 3-state outputs, center-pin VCC/GND
- Report: reports/G13.md; parts added: 4
- PDFs: 74ac11240, 74act11240
- Pinouts: `p24` (24 pins) for DB, DW, PW, NT
- Source: Pinout: 74ac11240 p.1 (DB/DW/NT, top view), 74act11240 p.1 (same); PW (74ac11240 packaging table only, TSSOP-24) assumed same pinout. Two 4-bit groups, 1OE/2OE active low, inverting (function table 74ac11240 p.1). Timing 74ac11240 p.4, 74act11240 p.3.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 5.4 | 4.6 | [3, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (74ac11240 p.4). VCC 3-5.5 V (74ac11240 p.3) |
| ACT | 6.5 | 6 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (74act11240 p.3). VCC 4.5-5.5 V (74act11240 p.2) |

## 74_11244

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, 3-state outputs, center-pin VCC/GND
- Report: reports/G10.md; parts added: 6
- PDFs: 74ac11244, 74act11244, scas171b
- Pinouts: `p24` (24 pins) for DB, DW, PW, NT, N, NS
- Source: 74ac11244 (= scas171b, SCAS171B) p.1 DB/DW/NT/PW pinout and function table; 74act11244 (SCAS006C) p.1 same pinout. Both OEs active low.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 4.9 | 4.5 | [3, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V +-0.5 V, 25 C, 50 pF (74ac11244 p.4); VCC 3-5.5 V (p.3) |
| ACT | 6 | 5.4 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V +-0.5 V, 25 C, 50 pF (74act11244 p.3); VCC 4.5-5.5 V (p.2) |

## 74_125

- Kind / model: `buffer` / `tristate`; function "Buffer"; Quad bus buffer gate, 3-state outputs
- Report: reports/G04.md; parts added: 121
- PDFs: cd54hc125, cd74hc125, cd74hc125-q1, cd74hct125, sn54abt125, sn54ahc125, sn54ahct125, sn54hc125, sn54ls125a, sn64bct125a, sn74abt125, sn74act125, sn74act125-q1, sn74ahc125, sn74ahc125-ep, sn74ahc125-q1, sn74ahct125, sn74ahct125-ep, sn74ahct125-q1, sn74alvc125, sn74auc125, sn74bct125a, sn74f125, sn74hc125, sn74hc125-q1, sn74hcs125, sn74hcs125-q1, sn74hct125, sn74ls125a, sn74lv125a, sn74lv125a-q1, sn74lvc125a, sn74lvc125a-ep, sn74lvc125a-q1, sn74lvt125, sn74lvt125-ep, sn74lvt125-q1, sn74lvth125, sn74lvth125-ep
- Pinouts: `p14` (14 pins) for J, N, D, PW, DB, RGY, BQA, NS, DGV, W, DYY
- Source: Standard '125 pinout (1OE 1A 1Y 2OE 2A 2Y GND 3Y 3A 3OE 4Y 4A 4OE VCC), same in every PDF: sn74lvc125a p.3 Table 4-1, sn74hc125 p.4, cd74hc125 p.3, cd74hct125 p.3, sn74ahc125 p.3 (D/DB/DGV/N/NS/J/W/PW and RGY/BQA same numbers), sn74ahct125 p.3, sn74act125 p.3 (PW, BQA), sn74hcs125 p.3 (D/PW/DYY, BQA), sn74abt125/sn74lvth125/sn74lv125a p.1 (incl. RGY), sn74ls125a/sn74f125/sn74bct125a/sn64bct125a/sn74alvc125/sn74auc125/sn74lvt125 p.1. QFN thermal pad is not a pin. Function table: OE low -> Y = A, OE high -> Z (every PDF). sn74ahc125-q1 p.3 Table 4-1 and its BQA figure put 3OE on 8 and 3Y on 10 while its D/PW figure on the same page shows the standard 8 = 3Y, 10 = 3OE; treated as a datasheet error (same die as SN74AHC125, whose BQA is standard).

```json
{"units": [{"a": ["1A"], "y": ["1Y"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A"], "y": ["2Y"], "oe": [{"pin": "2OE", "active": "low"}]}, {"a": ["3A"], "y": ["3Y"], "oe": [{"pin": "3OE", "active": "low"}]}, {"a": ["4A"], "y": ["4Y"], "oe": [{"pin": "4OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 14 | 14 | [2, 6] |  |  |  | Typical tpd A->Y at 4.5 V, 25 C, 50 pF, SN74HC125 (sn74hc125 p.6). VCC 2-6 V (sn74hc125 p.4; CD74HC125 cd74hc125 p.4 same) |
| HCT | 11 | 11 | [4.5, 5.5] |  |  |  | Typical tpd A->Y at 4.5 V, 25 C, 50 pF, SN74HCT125 (sn74hct125 p.5). VCC 4.5-5.5 V (sn74hct125 p.4; CD74HCT125 same) |
| HCS | 6 | 6 | [2, 6] |  |  |  | Typical tpd A->Y at 4.5 V, 25 C, 50 pF, SN74HCS125 (sn74hcs125 p.6). Schmitt-trigger inputs. VCC 2-6 V (sn74hcs125 p.4) |
| LS | 9 | 7 | [4.75, 5.25] |  |  |  | Typical at 5 V, 25 C (RL 667 ohm, 45 pF), SN74LS125A (sn74ls125a p.6). VCC 4.75-5.25 V SN74, 4.5-5.5 V SN54 (sn74ls125a p.6) |
| F | 3.6 | 5.1 | [4.5, 5.5] |  |  |  | Typical at 5 V, 25 C, 50 pF, SN74F125 (sn74f125 p.3). VCC 4.5-5.5 V (sn74f125 p.2) |
| ABT | 3.2 | 2.5 | [4.5, 5.5] |  |  |  | Typical at 5 V, 25 C, 50 pF, SN74ABT125 (sn74abt125 p.5). VCC 4.5-5.5 V SN54 and SN74 (sn74abt125 p.3) |
| BCT | 3.5 | 5 | [4.5, 5.5] |  |  |  | Typical at 5 V, 25 C, SN74BCT125A (sn74bct125a p.4; SN64BCT125A same, sn64bct125a p.3). VCC 4.5-5.5 V (sn74bct125a p.3) |
| LVT | 2.7 | 2.9 | [2.7, 3.6] | 5.5 |  |  | Typical at 3.3 V, 25 C, 50 pF, SN74LVT125 (sn74lvt125 p.4); LVTH125 typ 2/2.1 (sn74lvth125 p.5). VCC 2.7-3.6 V, inputs to 5.5 V (sn74lvt125 p.3) |
| LV | 6.1 | 6.1 | [2, 5.5] |  |  |  | Typical tpd at 3.3 V, 25 C, 50 pF, SN74LV125A (sn74lv125a p.6). VCC 2-5.5 V, VI 0-5.5 V (sn74lv125a p.5) |
| LVC | 2.5 | 2.5 | [1.65, 3.6] | 5.5 |  |  | Typical tpd at 3.3 V, 25 C, 50 pF, SN74LVC125A (sn74lvc125a p.6). VCC 1.65-3.6 V, inputs to 5.5 V (sn74lvc125a p.5) |
| AHC | 5.3 | 5.3 | [2, 5.5] |  |  |  | Typical at 5 V, 25 C, 50 pF, SN74AHC125 (sn74ahc125 p.9). VCC 2-5.5 V, VI 0-5.5 V (sn74ahc125 p.5) |
| AHCT | 5.3 | 5.3 | [4.5, 5.5] |  |  |  | Typical at 5 V, 25 C, 50 pF, SN74AHCT125 (sn74ahct125 p.6). VCC 4.5-5.5 V (sn74ahct125 p.5) |
| ACT | 5.2 | 5.7 | [4.5, 5.5] |  |  |  | Typical at 4.5 V (no 5 V row), 50 pF, SN74ACT125 (sn74act125 p.6). VCC 4.5-5.5 V (sn74act125 p.4) |
| ALVC | 2.8 | 2.8 | [1.65, 3.6] |  |  |  | Max tpd at 3.3 V (no typical printed), SN74ALVC125 (sn74alvc125 p.3). VCC 1.65-3.6 V, VI to 3.6 V (sn74alvc125 p.2) |
| AUC | 1 | 1 | [0.8, 2.7] | 3.6 |  |  | Typical tpd at 1.8 V, 25 C, 15 pF, SN74AUC125 (sn74auc125 p.4). VCC 0.8-2.7 V, inputs to 3.6 V (sn74auc125 p.3) |

- Part range: names starting `JM38510/32301` (LS): vcc [4.5, 5.5]

## 74_126

- Kind / model: `buffer` / `tristate`; function "Buffer"; Quadruple bus buffer gates, 3-state outputs, active-high enables
- Report: reports/G06.md; parts added: 88
- PDFs: cd74hc126, cd74hct126, sdls044a, sn54ahc126, sn54bct126a, sn64bct126a, sn74abt126, sn74ac126, sn74ac126-q1, sn74act126, sn74act126-q1, sn74ahc126, sn74ahc126-q1, sn74ahct126, sn74ahct126-q1, sn74alvc126, sn74auc126, sn74f126, sn74hc126, sn74hcs126, sn74hcs126-q1, sn74ls126a, sn74lv126a, sn74lvc126a, sn74lvc126a-q1, sn74lvth126
- Pinouts: `p14` (14 pins) for N, D, J, DB, NS, PW, RGY, BQA, DGV, DYY
- Source: Standard '126 pinout (1OE 1, 1A 2, 1Y 3, 2OE 4, 2A 5, 2Y 6, GND 7, 3Y 8, 3A 9, 3OE 10, 4Y 11, 4A 12, 4OE 13, VCC 14), the same in every PDF and package: sn74hc126 p.3, cd74hc126 p.3 (D, N, J), cd74hct126 p.3, sn54bct126a p.1, sn64bct126a p.1, sn74abt126 p.1 (D/DB/N/NS/PW and RGY), sn74ac126 p.3 (PW, BQA), sn74act126 p.3, sn74ahc126 = sn54ahc126 p.3 (PW, BQA), sn74ahct126 p.3, sn74alvc126 p.1, sn74auc126 p.1 (RGY), sn74f126 p.1, sn74hcs126 p.3 (D, DYY, PW), sn74ls126a = sdls044a p.1 (SN74126 N and SN74LS126A D/N/NS; enables printed 1G..4G there, named 1OE..4OE here), sn74lv126a p.2, sn74lvc126a p.3 (D/DB/DGV/NS/PW, BQA/RGY), sn74lvth126 p.1, and the -Q1 PDFs p.3. RGY/BQA thermal pad is not a pin. Function table (OE H: Y = A; OE L: Z): sn74hc126 p.11, sn74auc126 p.1, sn74abt126 p.2.

```json
{"units": [{"a": ["1A"], "y": ["1Y"], "oe": [{"pin": "1OE", "active": "high"}]}, {"a": ["2A"], "y": ["2Y"], "oe": [{"pin": "2OE", "active": "high"}]}, {"a": ["3A"], "y": ["3Y"], "oe": [{"pin": "3OE", "active": "high"}]}, {"a": ["4A"], "y": ["4Y"], "oe": [{"pin": "4OE", "active": "high"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 14 | 14 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hc126 p.6); VCC 2-6 V (p.4). cd74hc126: 8 ns typ at 5 V, 15 pF (p.6), VCC 2-6 V (p.4) |
| HCT | 9 | 9 | [4.5, 5.5] |  |  |  | typ tpd A->Y at 5 V, 15 pF (cd74hct126 p.5; no typical at 50 pF); VCC 4.5-5.5 V (p.4) |
| BCT | 3.6 | 5.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn64bct126a p.3; sn54bct126a p.3 same); VCC 4.5-5.5 V (sn64bct126a p.2; sn54bct126a p.2 same for SN54) |
| ABT | 2.9 | 2.5 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V (sn74abt126 p.4); VCC 4.5-5.5 V, VI max VCC (p.3) |
| AC | 4.7 | 4.7 | [1.5, 6] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74ac126 p.9); VCC 1.5-6 V (p.5); -q1 p.5 same range |
| ACT | 5.2 | 5.7 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 4.5 V, 50 pF (sn74act126 p.6; no 5 V row); VCC 4.5-5.5 V (p.4); -q1 p.4 same |
| AHC | 5.3 | 5.3 | [2, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74ahc126 p.7); VCC 2-5.5 V, VI to 5.5 V (p.5); -q1 p.5 same |
| AHCT | 5.3 | 5.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74ahct126 p.7); VCC 4.5-5.5 V, VI to 5.5 V (p.5); -q1 p.4 same |
| ALVC | 3.1 | 3.1 | [1.65, 3.6] |  |  |  | max tpd A->Y at 3.3 V +-0.3 V (sn74alvc126 p.3; no typical printed); VCC 1.65-3.6 V, VI max 3.6 V (p.2) |
| AUC | 1 | 1 | [0.8, 2.7] | 3.6 |  |  | typ tpd A->Y at 1.8 V, 15 pF (sn74auc126 p.4); VCC 0.8-2.7 V, VI to 3.6 V (p.3) |
| F | 4 | 5.5 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74f126 p.3); VCC 4.5-5.5 V (p.2) |
| HCS | 8 | 8 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V (sn74hcs126 p.5); VCC 2-6 V (p.4); -q1 p.4 same |
| LS | 9 | 8 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL at 5 V, 45 pF, SN74LS126A (sn74ls126a p.6); VCC 4.75-5.25 V (p.6) |
| TTL | 8 | 12 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL at 5 V, 50 pF, SN74126 (sdls044a = sn74ls126a p.5); VCC 4.75-5.25 V (p.5) |
| LV | 6.4 | 6.4 | [2, 5.5] |  |  |  | typ tpd A->Y at 3.3 V, 50 pF (sn74lv126a p.5); VCC 2-5.5 V, VI to 5.5 V (p.3) |
| LVC | 2.5 | 2.5 | [1.65, 3.6] | 5.5 |  |  | typ tpd A->Y at 3.3 V (sn74lvc126a p.7); VCC 1.65-3.6 V, VI to 5.5 V (p.4); -q1 same |
| LVT | 2.3 | 2.4 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->Y at 3.3 V, 50 pF, SN74LVTH126 (sn74lvth126 p.5); VCC 2.7-3.6 V, VI to 5.5 V (p.3) |

## 74_162240

- Kind / model: `buffer` / `tristate`; function "Buffer"; 16-bit inverting buffer/driver, 3-state outputs, 22-ohm series output resistors
- Report: reports/G14.md; parts added: 5
- PDFs: sn74lvt162240, sn74lvth162240
- Pinouts: `p48` (48 pins) for DGG, DGV, DL
- Source: Pinout from sn74lvt162240 p.1 (DGG, DGV, DL) and sn74lvth162240 p.1 (same). Function table p.2 (each 4-bit buffer): OE low = Y is not A, OE high = Z. Four 4-bit units, each with its own active-low nOE.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}, {"a": ["3A1", "3A2", "3A3", "3A4"], "y": ["3Y1", "3Y2", "3Y3", "3Y4"], "oe": [{"pin": "3OE", "active": "low"}]}, {"a": ["4A1", "4A2", "4A3", "4A4"], "y": ["4Y1", "4Y2", "4Y3", "4Y4"], "oe": [{"pin": "4OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVT | 2.5 | 2.9 | [2.7, 3.6] | 5.5 |  |  | Typical tPLH/tPHL A->Y at 3.3 V, 25 C (sn74lvt162240 p.6). VCC 2.7-3.6 V, VI to 5.5 V (p.5) |

## 74_162241

- Kind / model: `buffer` / `tristate`; function "Buffer"; 16-bit buffer/driver, 3-state outputs, 22-ohm series output resistors
- Report: reports/G09.md; parts added: 1
- PDFs: sn74lvth162241
- Pinouts: `p48` (48 pins) for DL, DGG
- Source: Pinout sn74lvth162241 p.1 (DGG/DL top view). Function tables p.2: 1OE, 4OE active low; 2OE, 3OE active high; four 4-bit groups. Series output resistors and bus hold not modeled.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "high"}]}, {"a": ["3A1", "3A2", "3A3", "3A4"], "y": ["3Y1", "3Y2", "3Y3", "3Y4"], "oe": [{"pin": "3OE", "active": "high"}]}, {"a": ["4A1", "4A2", "4A3", "4A4"], "y": ["4Y1", "4Y2", "4Y3", "4Y4"], "oe": [{"pin": "4OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVT | 3 | 2.4 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->Y at 3.3 V, 50 pF, SN74LVTH162241 (sn74lvth162241 p.6); VCC 2.7-3.6 V, VI to 5.5 V (p.5) |

## 74_162244

- Kind / model: `buffer` / `tristate`; function "Buffer"; 16-bit buffer/driver, 3-state outputs with series damping resistors
- Report: reports/G12.md; parts added: 25
- PDFs: cy74fct162244t, sn54lvth162244, sn74abt162244, sn74alvch162244, sn74lvc162244a, sn74lvch162244a, sn74lvt162244a, sn74lvth162244, sn74lvth162244-ep
- Pinouts: `p48` (48 pins) for DGG, DGV, DL, WD, CY-PA
- Source: Pinout sn74abt162244 p.1 (DGG, DGV, DL); same in sn74alvch162244 p.1, sn74lvc162244a p.1, sn74lvch162244a p.1, sn74lvt162244a p.1, sn74lvth162244 p.1, sn74lvth162244-ep p.1, sn54lvth162244 p.1 (WD), cy74fct162244t p.1 (SSOP/TSSOP). Four 4-bit groups, each with its own active-low nOE. Output series resistors and bus hold not modeled.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}, {"a": ["3A1", "3A2", "3A3", "3A4"], "y": ["3Y1", "3Y2", "3Y3", "3Y4"], "oe": [{"pin": "3OE", "active": "low"}]}, {"a": ["4A1", "4A2", "4A3", "4A4"], "y": ["4Y1", "4Y2", "4Y3", "4Y4"], "oe": [{"pin": "4OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.5 | 3.1 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74abt162244 p.5); VCC 4.5-5.5 V (p.3) |
| ALVC | 4.2 | 4.2 | [1.65, 3.6] |  |  |  | max tpd A->Y at 3.3 V (sn74alvch162244 p.5, no typ printed); VCC 1.65-3.6 V, VI to VCC (p.4) |
| FCT | 4.8 | 4.8 | [4.5, 5.5] |  |  |  | max tPLH/tPHL data->output, CY74FCT162244AT grade (cy74fct162244t p.5; base T grade 6.5, CT grade 4.1; no typ printed); VCC 5 V +-10% (p.2) |
| LVC | 4.4 | 4.4 | [1.65, 3.6] | 5.5 |  |  | max tpd A->Y at 3.3 V (sn74lvc162244a p.8, no typ printed); VCC 1.65-3.6 V, VI to 5.5 V (p.7) |
| LVT | 3.4 | 2.9 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->Y at 3.3 V (sn74lvt162244a p.6; sn74lvth162244 p.6 same); VCC 2.7-3.6 V, VI to 5.5 V (sn74lvt162244a p.5; SN54LVTH162244 same, sn54lvth162244 p.5) |

## 74_162344

- Kind / model: `buffer` / `tristate`; function "Buffer"; 1-bit to 4-bit address driver (8 x 1:4 fan-out), 3-state outputs, series damping resistors
- Report: reports/G10.md; parts added: 2
- PDFs: sn74alvch162344
- Pinouts: `p56` (56 pins) for DGG, DL, DGV
- Source: sn74alvch162344 (SCES085I) p.1 DGG/DGV/DL pinout; p.2 function table (OE low: Bn = A; OE high: Z); p.3 logic diagram: OE1 enables 1A->1B1-1B4 and 2A->2B1-2B4, OE2 3A/4A, OE3 5A/6A, OE4 7A/8A. Each A drives four B outputs, so a unit lists each A pin four times in "a". 26-ohm output resistors and bus hold are not modeled.

```json
{"units": [{"a": ["1A", "1A", "1A", "1A", "2A", "2A", "2A", "2A"], "y": ["1B1", "1B2", "1B3", "1B4", "2B1", "2B2", "2B3", "2B4"], "oe": [{"pin": "OE1", "active": "low"}]}, {"a": ["3A", "3A", "3A", "3A", "4A", "4A", "4A", "4A"], "y": ["3B1", "3B2", "3B3", "3B4", "4B1", "4B2", "4B3", "4B4"], "oe": [{"pin": "OE2", "active": "low"}]}, {"a": ["5A", "5A", "5A", "5A", "6A", "6A", "6A", "6A"], "y": ["5B1", "5B2", "5B3", "5B4", "6B1", "6B2", "6B3", "6B4"], "oe": [{"pin": "OE3", "active": "low"}]}, {"a": ["7A", "7A", "7A", "7A", "8A", "8A", "8A", "8A"], "y": ["7B1", "7B2", "7B3", "7B4", "8B1", "8B2", "8B3", "8B4"], "oe": [{"pin": "OE4", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALVC | 4.4 | 4.4 | [1.65, 3.6] |  |  |  | max tpd A->B at 3.3 V +-0.3 V, no typical printed (sn74alvch162344 p.5); VCC 1.65-3.6 V, VI 0-VCC (p.4) |

## 74_16240

- Kind / model: `buffer` / `tristate`; function "Buffer"; 16-bit buffer/driver, inverting 3-state outputs
- Report: reports/G13.md; parts added: 23
- PDFs: 74act16240, sn74abt16240a, sn74ahc16240, sn74ahct16240, sn74alvch16240, sn74auc16240, sn74lvch16240a, sn74lvcz16240a, sn74lvt16240, sn74lvth16240
- Pinouts: `p48` (48 pins) for DGG, DGV, DL
- Source: Pinout: standard Widebus 48-pin, identical in 74act16240 p.1, sn74abt16240a p.1 (DGG/DGV/DL), sn74ahc16240 p.1, sn74ahct16240 p.1, sn74alvch16240 p.1, sn74auc16240 p.1 (DGG), sn74lvch16240a p.1, sn74lvcz16240a p.1, sn74lvt16240 p.1, sn74lvth16240 p.1. Four 4-bit groups, 1OE..4OE active low, inverting (function tables p.1-2 of each).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}, {"a": ["3A1", "3A2", "3A3", "3A4"], "y": ["3Y1", "3Y2", "3Y3", "3Y4"], "oe": [{"pin": "3OE", "active": "low"}]}, {"a": ["4A1", "4A2", "4A3", "4A4"], "y": ["4Y1", "4Y2", "4Y3", "4Y4"], "oe": [{"pin": "4OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ACT | 5 | 6.7 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (74act16240 p.5). VCC 4.5-5.5 V (74act16240 p.4) |
| ABT | 2.7 | 3.1 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, SN74ABT16240A (sn74abt16240a p.5). VCC 4.5-5.5 V (sn74abt16240a p.4) |
| AHC | 5.1 | 5.1 | [2, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (sn74ahc16240 p.5). VCC 2-5.5 V, VI to 5.5 V (sn74ahc16240 p.4) |
| AHCT | 7 | 5.9 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (sn74ahct16240 p.5). VCC 4.5-5.5 V, VI to 5.5 V (sn74ahct16240 p.4) |
| ALVC | 3.9 | 3.9 | [1.65, 3.6] |  |  |  | max tpd A->Y at 3.3 V +-0.3 V, 30 pF; no typical printed (sn74alvch16240 p.5). VCC 1.65-3.6 V, VI 0-VCC (sn74alvch16240 p.4) |
| AUC | 1.4 | 1.4 | [0.8, 2.7] | 3.6 |  |  | typ tpd A->Y at 1.8 V, 30 pF (sn74auc16240 p.5). VCC 0.8-2.7 V, inputs to 3.6 V (sn74auc16240 p.4) |
| LVC | 4.2 | 4.2 | [1.65, 3.6] | 5.5 |  |  | max tpd A->Y at 3.3 V +-0.3 V, 50 pF; no typical printed (sn74lvch16240a p.4; sn74lvcz16240a p.4 max 4.2/4.1, VCC 2.7-3.6 V). VCC 1.65-3.6 V, inputs to 5.5 V (sn74lvch16240a p.3) |
| LVT | 2.2 | 2.7 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->Y at 3.3 V, 25 C, 50 pF, SN74LVT16240 (sn74lvt16240 p.6; sn74lvth16240 p.6 same). VCC 2.7-3.6 V, inputs to 5.5 V (sn74lvt16240 p.4) |

- Part range: names starting `SN74LVCZ` (LVC): vcc [2.7, 3.6]

## 74_16241

- Kind / model: `buffer` / `tristate`; function "Buffer"; 16-bit buffer/driver, 3-state outputs, complementary output enables
- Report: reports/G15.md; parts added: 6
- PDFs: sn74abt16241a, sn74lvth16241
- Pinouts: `p48` (48 pins) for DGG, DGV, DL, WD
- Source: Pinout sn74abt16241a p.1 (DGG/DGV/DL, WD), sn74lvth16241 p.1 (DGG/DL, WD). Function tables sn74abt16241a p.2 / sn74lvth16241 p.2: 1OE and 4OE active low, 2OE and 3OE active high.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "high"}]}, {"a": ["3A1", "3A2", "3A3", "3A4"], "y": ["3Y1", "3Y2", "3Y3", "3Y4"], "oe": [{"pin": "3OE", "active": "high"}]}, {"a": ["4A1", "4A2", "4A3", "4A4"], "y": ["4Y1", "4Y2", "4Y3", "4Y4"], "oe": [{"pin": "4OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.7 | 2.7 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (sn74abt16241a p.5); VCC 4.5-5.5 V (p.4) |
| LVT | 2.6 | 2.2 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL at 3.3 V, 25 C, 50 pF (sn74lvth16241 p.7); VCC 2.7-3.6 V, VI to 5.5 V (p.5) |

## 74_16244

- Kind / model: `buffer` / `tristate`; function "Buffer"; 16-bit buffer/line driver, 3-state outputs
- Report: reports/G07.md; parts added: 52
- PDFs: 74ac16244, 74act16244, cy74fct16244t, sn54lvth16244a-sp, sn74abt16244a, sn74abth16244, sn74ahc16244, sn74ahct16244, sn74alb16244, sn74alvc16244a, sn74alvch16244, sn74auc16244, sn74auch16244, sn74avc16244, sn74lvc16244a, sn74lvc16244a-ep, sn74lvc16244a-q1, sn74lvch16244a, sn74lvcz16244a, sn74lvt16244b, sn74lvth16244a, sn74lvth16244a-ep
- Pinouts: `p48` (48 pins) for DGG, DGV, DL, WD, CY-PV
- Source: Widebus 48-pin pinout, identical in every PDF: 74ac16244 p.1, 74act16244 p.1, sn74abt16244a p.1, sn74abth16244 p.1, sn74ahc16244 p.3, sn74ahct16244 p.3, sn74alb16244 p.1, sn74alvc16244a p.1, sn74alvch16244 p.1, sn74auc16244 p.1, sn74auch16244 p.1, sn74avc16244 p.2, sn74lvc16244a p.3, -ep p.1, -q1 p.1, sn74lvch16244a p.3, sn74lvcz16244a p.1, sn74lvt16244b p.1, sn74lvth16244a p.2, -ep p.1, sn54lvth16244a-sp p.2 (WD), cy74fct16244t p.1 (SSOP/TSSOP). Four 4-bit units, each OE active low, non-inverting.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}, {"a": ["3A1", "3A2", "3A3", "3A4"], "y": ["3Y1", "3Y2", "3Y3", "3Y4"], "oe": [{"pin": "3OE", "active": "low"}]}, {"a": ["4A1", "4A2", "4A3", "4A4"], "y": ["4Y1", "4Y2", "4Y3", "4Y4"], "oe": [{"pin": "4OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 4.6 | 5.3 | [3, 5.5] |  |  |  | typ A->Y at 5 V, 25 C, 50 pF (74ac16244 p.5). VCC 3-5.5 V (p.4) |
| ACT | 6.5 | 6.3 | [4.5, 5.5] |  |  |  | typ A->Y at 5 V, 25 C, 50 pF (74act16244 p.5). VCC 4.5-5.5 V (p.4) |
| ABT | 2.3 | 2.6 | [4.5, 5.5] |  |  |  | typ A->Y at 5 V, 25 C, 50 pF (sn74abt16244a p.5; sn74abth16244 p.5 same). VCC 4.5-5.5 V, VI 0-VCC (p.4) |
| AHC | 5.4 | 5.4 | [2, 5.5] |  |  |  | typ A->Y at 5 V, 25 C, 50 pF (sn74ahc16244 p.7). VCC 2-5.5 V, VI 0-5.5 V (p.5) |
| AHCT | 7 | 5.9 | [4.5, 5.5] |  |  |  | typ A->Y at 5 V, 25 C, 50 pF (sn74ahct16244 p.6). VCC 4.5-5.5 V, VI 0-5.5 V (p.5) |
| ALB | 1.3 | 1.3 | [3, 3.6] |  |  |  | typ tpd A->Y at 3.3 V, 25 C (sn74alb16244 p.4). VCC 3-3.6 V (p.4) |
| ALVC | 3 | 3 | [1.65, 3.6] |  |  |  | max tpd A->Y at 3.3 V, no typ printed (sn74alvc16244a p.5; sn74alvch16244 p.5 same). VCC 1.65-3.6 V, data VI 0-3.6 V (sn74alvc16244a p.4) |
| AUC | 1.3 | 1.3 | [0.8, 2.7] | 3.6 |  |  | typ tpd A->Y at 1.8 V, 15 pF (sn74auc16244 p.5; sn74auch16244 p.6 same). VCC 0.8-2.7 V, VI 0-3.6 V (p.4) |
| AVC | 1.9 | 1.9 | [1.4, 3.6] |  |  |  | max tpd A->Y at 2.5 V, no typ printed (sn74avc16244 p.6). VCC 1.4-3.6 V operating, VI 0-3.6 V (p.5) |
| LVC | 4.1 | 4.1 | [1.65, 3.6] | 5.5 |  |  | max tpd A->Y at 3.3 V, -40 to 85 C, no typ printed (sn74lvc16244a p.9). VCC 1.65-3.6 V, VI 0-5.5 V (p.7); LVCH, -ep, -q1 same range; LVCZ 2.7-3.6 V (partVcc) |
| LVT | 2.3 | 2 | [2.7, 3.6] | 5.5 |  |  | typ A->Y at 3.3 V, 50 pF (sn74lvt16244b p.6; sn74lvth16244a p.7 same). VCC 2.7-3.6 V, VI max 5.5 V (sn74lvt16244b p.4); -sp and -ep same |
| FCT | 6.5 | 6.5 | [4.5, 5.5] |  |  |  | max tPLH/tPHL data->output, CY74FCT16244T base grade, no typ printed (cy74fct16244t p.5). VCC 5 V +-10% (p.1-2) |

- Part range: names starting `SN74LVCZ` (LVC): vcc [2.7, 3.6]

## 74_162541

- Kind / model: `buffer` / `tristate`; function "Buffer"; 16-bit buffer/driver with 22-ohm series resistors, 3-state outputs
- Report: reports/G11.md; parts added: 3
- PDFs: sn74lvth162541
- Pinouts: `p48` (48 pins) for DGG, DL
- Source: p48 from sn74lvth162541 p.1 (DGG/DL, SN54 WD) and logic symbol p.3. Two 8-bit sections; both OE inputs of a section must be low (function table p.2: OE1 L, OE2 L -> Y = A; either H -> Z). Series output resistors and bus hold not modeled. N74LVTH162541DL left out (typo of SN74LVTH162541DL, not in the packaging table).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "y": ["1Y1", "1Y2", "1Y3", "1Y4", "1Y5", "1Y6", "1Y7", "1Y8"], "oe": [{"pin": "1OE1", "active": "low"}, {"pin": "1OE2", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "y": ["2Y1", "2Y2", "2Y3", "2Y4", "2Y5", "2Y6", "2Y7", "2Y8"], "oe": [{"pin": "2OE1", "active": "low"}, {"pin": "2OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVT | 2.9 | 2.4 | [2.7, 3.6] | 5.5 |  |  | Typical tPLH/tPHL A->Y at 3.3 V, 25 C, 50 pF, SN74LVTH162541 (sn74lvth162541 p.6). VCC 2.7-3.6 V, VI to 5.5 V, SN54 and SN74 (sn74lvth162541 p.4) |

## 74_162827

- Kind / model: `buffer` / `tristate`; function "Buffer"; 20-bit buffer/driver, series-damping output resistors, 3-state outputs
- Report: reports/G04.md; parts added: 7
- PDFs: cy74fct162827t, sn74abt162827a, sn74alvch162827
- Pinouts: `p56` (56 pins) for DGG, DL, DGV, CY-PA
- Source: Pinout sn74abt162827a p.1 (DGG or DL), sn74alvch162827 p.1 (DGG, DGV, DL), cy74fct162827t p.1 (SSOP/TSSOP) - all the same: 1 1OE1, 2-3 1Y1-1Y2, 5-6 1Y3-1Y4, 8-10 1Y5-1Y7, 12-14 1Y8-1Y10, 15-17 2Y1-2Y3, 19-21 2Y4-2Y6, 23-24 2Y7-2Y8, 26-27 2Y9-2Y10, 28 2OE1, 29 2OE2, 30-31 2A10-2A9, 33-34 2A8-2A7, 36-38 2A6-2A4, 40-42 2A3-2A1, 43-45 1A10-1A8, 47-49 1A7-1A5, 51-52 1A4-1A3, 54-55 1A2-1A1, 56 1OE2; GND 4,11,18,25,32,39,46,53; VCC 7,22,35,50. Function table (each 10-bit buffer): OE1 and OE2 both low -> Y = A, either high -> Z (cy74fct162827t p.2; sn74alvch162827 p.1). Outputs have series damping resistors.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "1A9", "1A10"], "y": ["1Y1", "1Y2", "1Y3", "1Y4", "1Y5", "1Y6", "1Y7", "1Y8", "1Y9", "1Y10"], "oe": [{"pin": "1OE1", "active": "low"}, {"pin": "1OE2", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "2A9", "2A10"], "y": ["2Y1", "2Y2", "2Y3", "2Y4", "2Y5", "2Y6", "2Y7", "2Y8", "2Y9", "2Y10"], "oe": [{"pin": "2OE1", "active": "low"}, {"pin": "2OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.1 | 2.8 | [4.5, 5.5] |  |  |  | Typical A->Y at 5 V, 25 C, 50 pF, SN74ABT162827A (sn74abt162827a p.5). VCC 4.5-5.5 V, VI 0-VCC (sn74abt162827a p.3) |
| ALVC | 3.8 | 3.8 | [1.65, 3.6] |  |  |  | Max tpd A->Y at 3.3 V (no typical printed), SN74ALVCH162827 (sn74alvch162827 p.5). VCC 1.65-3.6 V, VI 0-VCC (sn74alvch162827 p.3) |
| FCT | 4.2 | 4.2 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL A->Y, 50 pF, CY74FCT162827CT grade (no typical printed; AT 8, BT 5) (cy74fct162827t p.4). VCC 5 V +/-10% (cy74fct162827t p.2) |

## 74_162830

- Kind / model: `buffer` / `tristate`; function "Buffer"; 1-bit to 2-bit address driver (18 inputs, 36 outputs), 3-state outputs, 26-ohm series output resistors
- Report: reports/G05.md; parts added: 3
- PDFs: sn74alvch162830, sn74alvchs162830, sn74alvchs162830a
- Pinouts: `p80` (80 pins) for DBB
- Source: Pinout sn74alvch162830 p.1 (DBB); sn74alvchs162830 p.1 and sn74alvchs162830a p.1 same. Function table sn74alvch162830 p.2: OE1 L -> 1Yn = A; OE2 L -> 2Yn = A; each H -> that bank Z. Each A input feeds two units (1Y bank, 2Y bank).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15", "A16", "A17", "A18"], "y": ["1Y1", "1Y2", "1Y3", "1Y4", "1Y5", "1Y6", "1Y7", "1Y8", "1Y9", "1Y10", "1Y11", "1Y12", "1Y13", "1Y14", "1Y15", "1Y16", "1Y17", "1Y18"], "oe": [{"pin": "OE1", "active": "low"}]}, {"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15", "A16", "A17", "A18"], "y": ["2Y1", "2Y2", "2Y3", "2Y4", "2Y5", "2Y6", "2Y7", "2Y8", "2Y9", "2Y10", "2Y11", "2Y12", "2Y13", "2Y14", "2Y15", "2Y16", "2Y17", "2Y18"], "oe": [{"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALVC | 3.5 | 3.5 | [1.65, 3.6] |  |  |  | max tpd A->Y at 3.3 V (sn74alvch162830 p.4; typ not printed; ALVCHS same 3.5 max, sn74alvchs162830 p.5, sn74alvchs162830a p.4); VCC 1.65-3.6 V, VI to VCC (sn74alvch162830 p.3). ALVCHS parts 2.3-3.6 V -> partVcc |

- Part range: names starting `SN74ALVCHS162830` (ALVC): vcc [2.3, 3.6]

- Part range: names starting `74ALVCHS162830` (ALVC): vcc [2.3, 3.6]

## 74_16344

- Kind / model: `buffer` / `tristate`; function "Buffer"; 1-bit to 4-bit address driver (8 x 1:4 fan-out), 3-state outputs
- Report: reports/G12.md; parts added: 2
- PDFs: sn74alvch16344
- Pinouts: `p56` (56 pins) for DGG, DL
- Source: Pinout sn74alvch16344 p.1 (DGG, DL). Each input nA drives four outputs nB1-nB4 (the outputs are named B in the datasheet). Logic diagram p.2: OE1 enables 1B1-2B4, OE2 3B1-4B4, OE3 5B1-6B4, OE4 7B1-8B4 (all active low); function table p.1. Each unit lists its A pin once per output it drives. Bus hold not modeled.

```json
{"units": [{"a": ["1A", "1A", "1A", "1A", "2A", "2A", "2A", "2A"], "y": ["1B1", "1B2", "1B3", "1B4", "2B1", "2B2", "2B3", "2B4"], "oe": [{"pin": "OE1", "active": "low"}]}, {"a": ["3A", "3A", "3A", "3A", "4A", "4A", "4A", "4A"], "y": ["3B1", "3B2", "3B3", "3B4", "4B1", "4B2", "4B3", "4B4"], "oe": [{"pin": "OE2", "active": "low"}]}, {"a": ["5A", "5A", "5A", "5A", "6A", "6A", "6A", "6A"], "y": ["5B1", "5B2", "5B3", "5B4", "6B1", "6B2", "6B3", "6B4"], "oe": [{"pin": "OE3", "active": "low"}]}, {"a": ["7A", "7A", "7A", "7A", "8A", "8A", "8A", "8A"], "y": ["7B1", "7B2", "7B3", "7B4", "8B1", "8B2", "8B3", "8B4"], "oe": [{"pin": "OE4", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALVC | 4 | 4 | [1.65, 3.6] |  |  |  | max tpd A->B at 3.3 V (sn74alvch16344 p.4, no typ printed); VCC 1.65-3.6 V, VI to VCC (p.3) |

## 74_16540

- Kind / model: `buffer` / `tristate`; function "Buffer"; 16-bit inverting buffer/driver, 3-state outputs
- Report: reports/G05.md; parts added: 9
- PDFs: sn74abt16540a, sn74ahc16540, sn74ahct16540, sn74lvch16540a
- Pinouts: `p48` (48 pins) for DGG, DL
- Source: Pinout sn74abt16540a p.1 (DGG/DGV/DL); same in sn74ahc16540 p.1, sn74ahct16540 p.1, sn74lvch16540a p.1. Function table sn74abt16540a p.2 (each 8-bit section): OE1 L and OE2 L -> Y = not A; either H -> Z.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "y": ["1Y1", "1Y2", "1Y3", "1Y4", "1Y5", "1Y6", "1Y7", "1Y8"], "oe": [{"pin": "1OE1", "active": "low"}, {"pin": "1OE2", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "y": ["2Y1", "2Y2", "2Y3", "2Y4", "2Y5", "2Y6", "2Y7", "2Y8"], "oe": [{"pin": "2OE1", "active": "low"}, {"pin": "2OE2", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.3 | 2.5 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74abt16540a p.4); VCC 4.5-5.5 V SN54/SN74 (p.3) |
| AHC | 5.2 | 5.2 | [2, 5.5] |  |  |  | typ A->Y at 5 V, 50 pF (sn74ahc16540 p.5); VCC 2-5.5 V, VI to 5.5 V (p.3) |
| AHCT | 6 | 6 | [4.5, 5.5] |  |  |  | typ A->Y at 5 V, 50 pF (sn74ahct16540 p.4); VCC 4.5-5.5 V (p.3) |
| LVC | 3.7 | 3.7 | [1.65, 3.6] | 5.5 |  |  | max tpd A->Y at 3.3 V (sn74lvch16540a p.4; no typ printed); VCC 1.65-3.6 V, VI to 5.5 V (p.3). LVCH (bus hold) folded into LVC |

## 74_16541

- Kind / model: `buffer` / `tristate`; function "Buffer"; 16-bit buffer/driver, 3-state outputs (two output enables per byte)
- Report: reports/G08.md; parts added: 16
- PDFs: 74act16541, sn74abt16541a, sn74ahc16541, sn74ahct16541, sn74lvch16541a, sn74lvth16541
- Pinouts: `p48` (48 pins) for DL, DGG, DGV
- Source: Standard Widebus 48-pin '16541 pinout, identical in 74act16541 p.1, sn74abt16541a p.1, sn74ahc16541 p.1, sn74ahct16541 p.1, sn74lvch16541a p.1, sn74lvth16541 p.1. Function table (each 8-bit section; e.g. 74act16541 p.1): OE1 and OE2 both low = Y follows A, either high = Z.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8"], "y": ["1Y1", "1Y2", "1Y3", "1Y4", "1Y5", "1Y6", "1Y7", "1Y8"], "oe": [{"pin": "1OE1", "active": "low"}, {"pin": "1OE2", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8"], "y": ["2Y1", "2Y2", "2Y3", "2Y4", "2Y5", "2Y6", "2Y7", "2Y8"], "oe": [{"pin": "2OE1", "active": "low"}, {"pin": "2OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ACT | 5.9 | 6.3 | [4.5, 5.5] |  |  |  | 74act16541 typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (p.4). VCC 4.5-5.5 V (p.3) |
| ABT | 2.1 | 2.5 | [4.5, 5.5] |  |  |  | sn74abt16541a typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (p.4). VCC 4.5-5.5 V (p.3) |
| AHC | 5 | 5 | [2, 5.5] |  |  |  | sn74ahc16541 typ tPLH/tPHL A->Y at 5 V +-0.5 V, 25 C, 50 pF (p.5; 3.3 V 7.5). VCC 2-5.5 V, VI to 5.5 V (p.3) |
| AHCT | 6.2 | 6 | [4.5, 5.5] |  |  |  | sn74ahct16541 typ tPLH/tPHL A->Y at 5 V +-0.5 V, 25 C, 50 pF (p.4). VCC 4.5-5.5 V, VI to 5.5 V (p.3) |
| LVC | 4.2 | 4.2 | [1.65, 3.6] | 5.5 |  |  | sn74lvch16541a max tpd A->Y at 3.3 V +-0.3 V, 50 pF (no typ printed) (p.4). VCC 1.65-3.6 V, VI to 5.5 V (p.3) |
| LVT | 2.4 | 2 | [2.7, 3.6] | 5.5 |  |  | sn74lvth16541 typ tPLH/tPHL A->Y at 3.3 V +-0.3 V, 25 C, 50 pF, SN74 columns (p.6). VCC 2.7-3.6 V, VI to 5.5 V (p.4) |

## 74_16825

- Kind / model: `buffer` / `tristate`; function "Buffer"; 18-bit buffer/driver, 3-state outputs (two 9-bit sections, each with two active-low output enables)
- Report: reports/G03.md; parts added: 3
- PDFs: sn74abt16825, sn74alvch16825
- Pinouts: `p56` (56 pins) for DL
- Source: Pinout from sn74abt16825 p.1 (DL/WD) and sn74alvch16825 p.1 (DGG/DL), identical: GND on 4, 11, 14, 15, 18, 25, 32, 39, 42, 43, 46, 53; VCC on 7, 22, 35, 50. Function table (each 9-bit section): nOE1 = nOE2 = L -> Y = A; either high -> Z (sn74abt16825 p.2, sn74alvch16825 p.1). ALVCH bus hold not modeled.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "1A9"], "y": ["1Y1", "1Y2", "1Y3", "1Y4", "1Y5", "1Y6", "1Y7", "1Y8", "1Y9"], "oe": [{"pin": "1OE1", "active": "low"}, {"pin": "1OE2", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "2A9"], "y": ["2Y1", "2Y2", "2Y3", "2Y4", "2Y5", "2Y6", "2Y7", "2Y8", "2Y9"], "oe": [{"pin": "2OE1", "active": "low"}, {"pin": "2OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 1.9 | 2.1 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74abt16825 p.4); VCC 4.5-5.5 V, VI 0-VCC (sn74abt16825 p.3) |
| ALVC | 3.4 | 3.4 | [1.65, 3.6] |  |  |  | max tpd A->Y at 3.3 V +-0.3 V, no typical printed (sn74alvch16825 p.4); VCC 1.65-3.6 V, VI 0-VCC (sn74alvch16825 p.3) |

## 74_16827

- Kind / model: `buffer` / `tristate`; function "Buffer"; 20-bit buffer/driver, 3-state outputs
- Report: reports/G16.md; parts added: 7
- PDFs: 74act16827, cy74fct16827t, sn74abt16827, sn74alvch16827, sn74avc16827
- Pinouts: `p56` (56 pins) for DL, DGG, DGV, CY-PA
- Source: Pinout same in sn74abt16827 p.1 (DL), 74act16827 p.1 (DL; pin 51 misprinted 'A14', it is 1A4), sn74alvch16827 p.1 (DGG/DL), sn74avc16827 p.2 (DGG/DGV), cy74fct16827t p.1 (SSOP/TSSOP; names printed '1A 1'). Two 10-bit sections, each with OE1 and OE2 (NANDed, both active low: outputs drive only when both are low) (sn74abt16827 p.1 function table).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4", "1A5", "1A6", "1A7", "1A8", "1A9", "1A10"], "y": ["1Y1", "1Y2", "1Y3", "1Y4", "1Y5", "1Y6", "1Y7", "1Y8", "1Y9", "1Y10"], "oe": [{"pin": "1OE1", "active": "low"}, {"pin": "1OE2", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4", "2A5", "2A6", "2A7", "2A8", "2A9", "2A10"], "y": ["2Y1", "2Y2", "2Y3", "2Y4", "2Y5", "2Y6", "2Y7", "2Y8", "2Y9", "2Y10"], "oe": [{"pin": "2OE1", "active": "low"}, {"pin": "2OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ACT | 7.4 | 7.4 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V and 25 C (50 pF) (74act16827 p.4). VCC 4.5-5.5 V (74act16827 p.3) |
| ABT | 1.9 | 2.1 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V and 25 C (50 pF) (sn74abt16827 p.4). VCC 4.5-5.5 V (sn74abt16827 p.3) |
| ALVC | 3.4 | 3.4 | [1.65, 3.6] |  |  |  | Max tpd A->Y at 3.3 V; no typical given (sn74alvch16827 p.4). VCC 1.65-3.6 V, VI 0-VCC (sn74alvch16827 p.3) |
| AVC | 1.9 | 1.9 | [1.2, 3.6] |  |  |  | Max tpd A->Y at 2.5 V; no typical at 2.5 V (sn74avc16827 p.5). VCC 1.2-3.6 V, VI 0-3.6 V (sn74avc16827 p.4) |
| FCT | 4.2 | 4.2 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL A->Y, 50 pF, CY74FCT16827CT grade (cy74fct16827t p.4); the ordered E grade (ETPAC) is not in the PDF. VCC 5 V +-10% (cy74fct16827t p.2) |

## 74_2240

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal inverting buffer/line/MOS driver, 3-state outputs with series damping resistors
- Report: reports/G08.md; parts added: 9
- PDFs: sn54abt2240a, sn54bct2240, sn74abt2240a
- Pinouts: `p20` (20 pins) for DB, DBL, DW, N, NS, PW, J
- Source: '240 pinout with 25-ohm (ABT) / 33-ohm (BCT) series output resistors. Pinout sn74abt2240a p.1 (SN74 DB/DW/N/PW, SN54 J/W; same text as sn54abt2240a), sn54bct2240 p.1 (SN54 J/W). Function table sn74abt2240a p.2, sn54bct2240 p.2 (OE low: Y = not A; OE high: Z). Timing sn74abt2240a p.5, sn54bct2240 p.4.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 3 | 4.1 | [4.5, 5.5] |  |  |  | sn74abt2240a typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, SN74 table (p.5; SN54 table typ 3/4.8). VCC 4.5-5.5 V, SN54 and SN74 (p.4) |
| BCT | 3.4 | 2.8 | [4.5, 5.5] |  |  |  | sn54bct2240 typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (p.4). VCC 4.5-5.5 V, SN54 and SN74 (p.4) |

## 74_2240_FCT

- Kind / model: `buffer` / `tristate`; function "Buffer"; 8-bit inverting buffer/line driver, 3-state outputs with 25-ohm series resistors
- Report: reports/G08.md; parts added: 1
- PDFs: cy74fct2240t
- Pinouts: `p20` (20 pins) for DBQ, CY-QC, CY-SOC
- Source: CY74FCT2240T: same pin positions and function as the '2240 but its own pin names (OEA/OEB, DAn/DBn inputs, OAn/OBn outputs, numbered 0-3), so its own device. Pinout cy74fct2240t p.1 (Q = DBQ QSOP, SO = SOIC); function table and logic diagram p.2 (OEA low: OAn = not DAn; OEB low: OBn = not DBn; high: Z). Timing p.5, VCC p.3.

```json
{"units": [{"a": ["DA0", "DA1", "DA2", "DA3"], "y": ["OA0", "OA1", "OA2", "OA3"], "oe": [{"pin": "OEA", "active": "low"}]}, {"a": ["DB0", "DB1", "DB2", "DB3"], "y": ["OB0", "OB1", "OB2", "OB3"], "oe": [{"pin": "OEB", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 4.1 | 4.1 | [4.75, 5.25] |  |  |  | cy74fct2240t max tPLH/tPHL D->O, CT speed grade (T 8, AT 4.8), -40 to 85 C (no typ printed) (p.5). VCC 4.75-5.25 V (p.3) |

## 74_2241

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, 3-state outputs, 25-ohm series output resistors
- Report: reports/G02.md; parts added: 4
- PDFs: sn74abt2241
- Pinouts: `p20` (20 pins) for DB, DW, PW
- Source: sn74abt2241 p.1: DB/DW/N/PW (and J) pinout = '241 (1 1OE, 2/4/6/8 1A1-1A4, 18/16/14/12 1Y1-1Y4, 11/13/15/17 2A1-2A4, 9/7/5/3 2Y1-2Y4, 19 2OE). Function tables p.2: 1OE active low, 2OE active high; noninverting. Outputs have equivalent 25-ohm series resistors (not modeled).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "high"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 3 | 4.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (sn74abt2241 p.5); VCC 4.5-5.5 V SN54/SN74, VI 0-VCC (p.4) |

## 74_2244

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver with series damping resistors, 3-state outputs
- Report: reports/G11.md; parts added: 18
- PDFs: sn54bct2244, sn74abt2244a, sn74bct2244, sn74lvc2244a
- Pinouts: `p20` (20 pins) for DB, DBQ, DGV, DW, N, NS, PW, J, W
- Source: Standard '244 pinout: sn74abt2244a p.1 (DB/DW/N/PW, SN54 J/W), sn74bct2244 p.1 (DW/N/NS, SN54 J/W), sn74lvc2244a p.3 (DB/DBQ/DGV/DW/NS/PW). Outputs have built-in 25/26/33-ohm series resistors (not modeled). Function table OE L: Y = A, OE H: Z (sn74abt2244a p.2, sn74bct2244 p.2). SN74BCT2244NS is an NS orderable not in the packaging table.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 3.4 | 4.5 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, SN74ABT2244A (sn74abt2244a p.6). VCC 4.5-5.5 V, SN54 and SN74 (sn74abt2244a p.4) |
| BCT | 3 | 4.6 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (sn74bct2244 p.4). VCC 4.5-5.5 V, SN54 and SN74 (sn74bct2244 p.3) |
| LVC | 5.5 | 5.5 | [1.65, 3.6] | 5.5 |  |  | Max tpd A->Y at 3.3 V, -40 to 85 C, 50 pF; no typical printed (sn74lvc2244a p.6). VCC 1.65-3.6 V, VI to 5.5 V (sn74lvc2244a p.5) |

## 74_2244_FCT

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver with 25-ohm series resistors, 3-state outputs
- Report: reports/G11.md; parts added: 6
- PDFs: cy74fct2244t
- Pinouts: `p20` (20 pins) for DBQ, DW, CY-QC, CY-SOC
- Source: CY74FCT2244T prints OEA/OEB, DA0-3/OA0-3, DB0-3/OB0-3 (cy74fct2244t p.1 pinout, Q = DBQ and SO = DW). Logic diagram p.2: OEA enables DAn -> OAn, OEB enables DBn -> OBn; function table p.2 (OE L: O = D, H: Z). Same positions as the '244 (OEA = 1OE, DAn = 1A(n+1), OEB = 2OE, DB0 at pin 17 = 2A4). CY-QC rows (CY74FCT2244ATQC-TI, CTQC, TQC) are tube QSOP orderables not in the packaging table, given the Q (DBQ) pinout. CY74FCT2244TS0C (digit zero) left out as a typo of CY74FCT2244TSOC. 25-ohm series output resistors not modeled.

```json
{"units": [{"a": ["DA0", "DA1", "DA2", "DA3"], "y": ["OA0", "OA1", "OA2", "OA3"], "oe": [{"pin": "OEA", "active": "low"}]}, {"a": ["DB0", "DB1", "DB2", "DB3"], "y": ["OB0", "OB1", "OB2", "OB3"], "oe": [{"pin": "OEB", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 6.5 | 6.5 | [4.75, 5.25] |  |  |  | Max tPLH/tPHL D->O, CY74FCT2244T (slowest grade; AT 4.6, CT 4.1), -40 to 85 C, 50 pF; no typical printed (cy74fct2244t p.5). VCC 4.75-5.25 V (cy74fct2244t p.3) |

## 74_240

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, inverting, 3-state outputs (two 4-bit groups, each with an active-low output enable)
- Report: reports/G03.md; parts added: 145
- PDFs: cd74ac240, cd74act240, cd74hc240, cd74hct240, cy54fct240t, cy74fct240t, sn54abt240, sn54ahc240, sn54ahct240, sn54als240a, sn54as240a, sn54bct240, sn54f240, sn54hc240, sn54hct240, sn54ls240, sn54lvth240, sn54s240, sn74abt240a, sn74ac240, sn74ac240-q1, sn74act240, sn74act240-q1, sn74ahc240, sn74ahc240-q1, sn74ahct240, sn74ahct240-q1, sn74als240a, sn74as240a, sn74auc240, sn74bct240, sn74f240, sn74hc240, sn74hcs240, sn74hcs240-q1, sn74hct240, sn74ls240, sn74lv240a, sn74lvc240a, sn74lvc240a-q1, sn74lvcz240a, sn74lvt240a, sn74lvth240, sn74s240
- Pinouts: `p20` (20 pins) for PW, N, DW, J, DBQ, DB, NS, RKS, DGS, RGY, DGV, FK, W, CY-QC, DBL
- Source: Standard '240 20-pin pinout, same numbering in every package (N, DW, DB, NS, PW, DGS, DGV, J, W, FK LCCC, RKS/RGY VQFN with exposed pad not a pin, CY QSOP/SOIC): sn74hc240 p.3 (incl. FK), sn74act240 p.3 (incl. RKS), sn74ls240 p.3 (incl. FK), sn74auc240 p.1 (RGY), sn74lvc240a p.3, sn74ahc240 p.3, sn74lv240a p.4, sn74abt240a p.1, cy74fct240t p.1 (incl. L/LCC). Name variants folded to the TI names: LS/S sheets print 1G/2G for 1OE/2OE (sn74ls240 p.3); CD74AC/ACT and CD74HC/HCT number bits from 0 (1A0-1A3, 2Y0-2Y3: cd74ac240 p.4 Pin Functions, cd74hc240 p.3 pinout drawing); CY FCT prints OEA/OEB, DA0-3/DB0-3, OA0-3/OB0-3 (cy74fct240t p.1, OEA=1OE, DAn=1A(n+1), OAn=1Y(n+1)). Function: OE low -> Y = not A, OE high -> Z (sn74hc240 p.9, cy74fct240t p.2).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 10 | 10 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hc240 p.5); VCC 2-6 V (sn74hc240 p.4; cd74hc240 p.4 same) |
| HCT | 13 | 13 | [4.5, 5.5] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hct240 p.5); VCC 4.5-5.5 V (sn74hct240 p.4; cd74hct240 p.4 same) |
| HCS | 7 | 7 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hcs240 p.5); VCC 2-6 V (sn74hcs240 p.4) |
| LS | 9 | 12 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL at 5 V, 45 pF, 667 ohm (sn74ls240 p.6); VCC 4.75-5.25 V SN74, 4.5-5.5 V SN54 (sn74ls240 p.4) |
| S | 4.5 | 4.5 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL SNx4S240 at 5 V, 50 pF, 90 ohm (sn74s240 p.6); VCC 4.75-5.25 V SN74, 4.5-5.5 V SN54 (sn74s240 p.4) |
| F | 4.7 | 3.1 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL at 5 V, 50 pF (sn74f240 p.4); VCC 4.5-5.5 V (sn74f240 p.3) |
| ALS | 9 | 9 | [4.5, 5.5] |  |  |  | max tPLH/tPHL SN74ALS240A, 4.5-5.5 V, 50 pF, no typical printed (sn74als240a p.5); VCC 4.5-5.5 V (sn74als240a p.4) |
| AS | 6.5 | 6.5 | [4.5, 5.5] |  |  |  | max tPLH/tPHL SN74AS240A, 4.5-5.5 V, 50 pF, no typical printed (sn74as240a p.6); VCC 4.5-5.5 V (sn74as240a p.4) |
| AC | 4.5 | 4.5 | [2, 6] |  |  |  | typ tPLH/tPHL at 5 V, 50 pF (sn74ac240 p.6); VCC 2-6 V (sn74ac240 p.4); CD74AC240 1.5-5.5 V (cd74ac240 p.5) |
| ACT | 6 | 5.5 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL at 5 V, 50 pF (sn74act240 p.6); VCC 4.5-5.5 V (sn74act240 p.5; cd74act240 p.5 same) |
| AHC | 5.1 | 5.1 | [2, 5.5] |  |  |  | typ tPLH/tPHL at 5 V, 50 pF (sn74ahc240 p.6); VCC 2-5.5 V, VI 0-5.5 V (sn74ahc240 p.4) |
| AHCT | 5.9 | 5.9 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL at 5 V, 50 pF (sn74ahct240 p.7); VCC 4.5-5.5 V, VI 0-5.5 V (sn74ahct240 p.5) |
| ABT | 2.9 | 3.1 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL SN74ABT240A at 5 V, 50 pF (sn74abt240a p.4); VCC 4.5-5.5 V (sn74abt240a p.3) |
| BCT | 3.3 | 1.8 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL at 5 V, 50 pF (sn74bct240 p.4); VCC 4.5-5.5 V (sn74bct240 p.3) |
| LVT | 2.2 | 2.6 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL at 3.3 V, 50 pF (sn74lvt240a p.4; sn74lvth240 p.6 same); VCC 2.7-3.6 V, VI to 5.5 V (sn74lvt240a p.3, sn74lvth240 p.4) |
| LV | 5.9 | 5.9 | [2, 5.5] |  |  |  | typ tpd at 3.3 V, 50 pF (sn74lv240a p.8); VCC 2-5.5 V, VI 0-5.5 V (sn74lv240a p.6) |
| LVC | 6.5 | 6.5 | [1.65, 3.6] | 5.5 |  |  | max tpd at 3.3 V, -40 to 85 C, no typical printed (sn74lvc240a p.6); VCC 1.65-3.6 V, VI to 5.5 V (sn74lvc240a p.4) |
| AUC | 1.1 | 1.1 | [0.8, 2.7] | 3.6 |  |  | typ tpd at 1.8 V, 15 pF (sn74auc240 p.4); VCC 0.8-2.7 V, VI to 3.6 V (sn74auc240 p.3) |
| FCT | 8 | 8 | [4.75, 5.25] |  |  |  | max tPLH/tPHL CY74FCT240T, no typical printed (cy74fct240t p.7; AT grade 4.8, CT grade 4.3); VCC 4.75-5.25 V CY74, 4.5-5.5 V CY54 (cy74fct240t p.4) |

- Part range: names starting `SNJ54LS` (LS): vcc [4.5, 5.5]

- Part range: names starting `SNJ54S` (S): vcc [4.5, 5.5]

- Part range: names starting `CY54` (FCT): vcc [4.5, 5.5]

- Part range: names starting `5962-92213` (FCT): vcc [4.5, 5.5]

- Part range: names starting `CD74AC240` (AC): vcc [1.5, 5.5]

- Part range: names starting `SN74LVCZ` (LVC): vcc [2.7, 3.6], inputMax 5.5

## 74_241

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, 3-state outputs (1OE active low, 2OE active high)
- Report: reports/G10.md; parts added: 35
- PDFs: sn54abt241, sn54act241, sn54as241a, sn74abt241a, sn74ac241, sn74act241, sn74als241c, sn74as241a, sn74f241, sn74hc241, sn74lvc241a, sn74lvth241
- Pinouts: `p20` (20 pins) for N, D, DW, NS, DB, DBL, PW, DGS, RKS, J, W, FK
- Source: Standard 20-pin pinout, identical in every sheet: sn74abt241a p.1 (J/W/DB/DW/N/PW and FK), sn74ac241 p.3 Table 3-1, sn74act241 p.3 (J/W/DB/DW/N/NS/PW and FK), sn74als241c p.1 (ALS and AS, J/DW/N and FK), sn74f241 p.1, sn74hc241 p.3 (J/DW/N/NS/PW and FK), sn74lvc241a p.3 (PW/DGS and RKS, same numbers), sn74lvth241 p.1 (DB/DW/NS/PW and FK). FK (20-pin LCCC) numbers are the same as the DIP, so it shares p20. Function tables: 1OE low enables 1Y=1A, 2OE high enables 2Y=2A (sn74abt241a p.2, sn74lvc241a p.14 Table 7-1/7-2; the lvc241a pin table calls 2OE active low, its function table says high). DBL/PWLE codes are the old DB/PW LE-reel suffixes.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "high"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.6 | 2.9 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, SN74ABT241A (sn74abt241a p.5); VCC 4.5-5.5 V SN54 and SN74 (p.3) |
| AC | 5 | 4.5 | [2, 6] |  |  |  | typ tPLH/tPHL A->Y at 5 V +-0.5 V, 25 C, 50 pF (sn74ac241 p.5 sec 4.6); VCC 2-6 V (p.4) |
| ACT | 6 | 5.5 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (sn74act241 p.5); VCC 4.5-5.5 V SN54 and SN74 (p.4) |
| ALS | 11 | 10 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->Y, SN74ALS241C, VCC 4.5-5.5 V, 50 pF, no typical printed (sn74als241c p.4); VCC 4.5-5.5 V SN54 and SN74 (p.3) |
| AS | 6.2 | 6.2 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->Y, SN74AS241A, VCC 4.5-5.5 V, 50 pF, no typical printed (sn74as241a p.5); VCC 4.5-5.5 V SN54 and SN74 (p.4) |
| F | 3.6 | 3.6 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C (sn74f241 p.4); VCC 4.5-5.5 V SN54 and SN74 (p.3) |
| HC | 12 | 12 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V, 25 C, 50 pF (sn74hc241 p.5); VCC 2-6 V (p.4) |
| LVC | 3.9 | 3.9 | [1.1, 3.6] | 5.5 |  |  | typ tpd A->Y at 3.3 V, 25 C, 50 pF (sn74lvc241a p.7); VCC 1.1-3.6 V, VI to 5.5 V (p.5) |
| LVT | 2.3 | 2.2 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->Y at 3.3 V, 50 pF, SN74LVTH241 (sn74lvth241 p.5); VCC 2.7-3.6 V, VI to 5.5 V (p.3) |

## 74_241_CD

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, 3-state outputs (1OE active low, 2OE active high)
- Report: reports/G10.md; parts added: 4
- PDFs: cd74act241, cd74hc241, cd74hct241
- Pinouts: `p20` (20 pins) for N, E, DW, M, PW, J, F
- Source: cd74hc241 (= cd74hct241, SCHS167G) p.3 pinout figure (J/N/DW/PW); cd74act241 (SCHS287C) p.3 Figure 3-2 and p.4 Table 3-1 (DW/N): bits numbered from 0 (1A0-1A3, 1Y0-1Y3, 2A0-2A3, 2Y0-2Y3), same positions as 74_241 (pin 3 = 2Y3 ... pin 18 = 1Y0). 2OE (pin 19) active high on the 241 (cd74hc241 p.1 description; cd74act241 p.9 note).

```json
{"units": [{"a": ["1A0", "1A1", "1A2", "1A3"], "y": ["1Y0", "1Y1", "1Y2", "1Y3"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A0", "2A1", "2A2", "2A3"], "y": ["2Y0", "2Y1", "2Y2", "2Y3"], "oe": [{"pin": "2OE", "active": "high"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 9 | 9 | [2, 6] |  |  |  | typ tPLH/tPHL data to output at 5 V, 25 C, CL 15 pF, CD74HC241 (cd74hc241 p.9 sec 5.8); VCC 2-6 V (p.4) |
| HCT | 10 | 10 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL data to output at 5 V, 25 C, CL 15 pF, CD74HCT241 (cd74hct241 p.9 sec 5.8); VCC 4.5-5.5 V (p.4) |
| ACT | 8.7 | 8.7 | [4.5, 5.5] |  |  |  | max tPLH/tPHL data to output at 5 V, -40 to 85 C, 50 pF, ACT241 (cd74act241 p.8 sec 4.7), no typical printed; VCC 4.5-5.5 V (p.5) |

## 74_241_G

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, 3-state outputs (1G active low, 2G active high)
- Report: reports/G10.md; parts added: 6
- PDFs: sn54ls241, sn54s241, sn74ls241, sn74s241
- Pinouts: `p20` (20 pins) for N, DW, NS, DB, J, W, FK
- Source: sn74ls241 (= sn54ls241 = sn74s241 = sn54s241, one sheet SDLS144D) p.3 Pin Functions: DB/DW/J/N/NS/W and FK use the same numbers; enables printed 1G (active low) and 2G (active high, footnote: 2G for SNx4LS241/SNx4S241). Same positions as 74_241 with OE named G.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1G", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2G", "active": "high"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 12 | 12 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL at 5 V, 25 C, RL 667 ohm, CL 45 pF, SNx4LS241 (sn74ls241 p.6 sec 6.7); VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (p.4) |
| S | 6 | 6 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL at 5 V, 25 C, RL 90 ohm, CL 50 pF, SNx4S241 (sn74s241 p.6 sec 6.8); VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (p.4) |

- Part range: names starting `SN54` (LS): vcc [4.5, 5.5]

- Part range: names starting `SNJ54` (LS): vcc [4.5, 5.5]

- Part range: names starting `SN54` (S): vcc [4.5, 5.5]

- Part range: names starting `SNJ54` (S): vcc [4.5, 5.5]

## 74_244

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, 3-state outputs
- Report: reports/G01.md; parts added: 215
- PDFs: cd74ac244, cd74act244, cd74fct244, cd74hc244, cd74hct244, cy54fct244t, cy74fct244t, sn54abt244, sn54act244, sn54act244-sp, sn54ahc244-sp, sn54als244c, sn54als244c-sp, sn54bct244, sn54f244, sn54hc244-sp, sn54hct244-sp, sn54s244, sn64bct244, sn74abt244a, sn74abt244a-ep, sn74ac244, sn74ac244-ep, sn74ac244-q1, sn74act244, sn74act244-ep, sn74act244-q1, sn74ahc244, sn74ahc244-q1, sn74ahct244, sn74ahct244-ep, sn74ahct244-q1, sn74als244c, sn74alvc244, sn74alvc244-ep, sn74alvch244, sn74as244a, sn74auc244, sn74auch244, sn74bct244, sn74f244, sn74hc244, sn74hc244-ep, sn74hc244-q1, sn74hcs244, sn74hcs244-q1, sn74hct244, sn74hct244-ep, sn74hct244-q1, sn74ls244, sn74lv244a, sn74lv244a-q1, sn74lv244b-ep, sn74lvc244a, sn74lvc244a-q1, sn74lvch244a, sn74lvcz244a, sn74lvt244b, sn74lvth244a, sn74lvth244a-ep, sn74lvtz244, sn74s244
- Pinouts: `p20` (20 pins) for DB, DBQ, DGS, DGV, DW, FK, J, N, NS, PW, RGY, RKS, W
- Source: p20 from sn74hc244 p.3, sn74lvc244a p.3-4 (DB/DGV/DW/N/NS/PW/RGY/DGS/RKS: same numbers 1-20, VQFN thermal pad is not a pin), sn74ls244 p.3 (J/W/DB/DW/N/NS and FK: same numbers), sn74f244 p.1 and sn54abt244 p.1 (FK same numbers), cy74fct244t p.1 (P/Q/SO/D and L LCC same numbers). Pin names normalized to the SN74 names: SN74LS/S244 print 1G/2G for 1OE/2OE (sn74ls244 p.3); CD74HC/HCT/AC/ACT244 number bits from 0 (1A0..1A3 = 1A1..1A4, 1Y0..1Y3 = 1Y1..1Y4, 2A0..2A3 = 2A1..2A4, 2Y0..2Y3 = 2Y1..2Y4, same pins; cd74hc244 p.3, cd74ac244 p.4); CY54/74FCT244T print OEA/OEB, DA0-3/OA0-3, DB0-3/OB0-3 (OEA=1OE, DAn=1A(n+1), OAn=1Y(n+1), OEB=2OE, DB0..DB3 at pins 17,15,13,11 = 2A4..2A1, OB0..OB3 at pins 3,5,7,9 = 2Y4..2Y1; cy74fct244t p.1, p.3). Function table: OE L -> Y = A, OE H -> Z (cd74fct244 p.1, cy74fct244t p.2).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.6 | 2.9 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, SN74ABT244A (sn74abt244a p.5). VCC 4.5-5.5 V, SN54 and SN74 (sn74abt244a p.3) |
| AC | 5 | 5 | [2, 6] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, SN74AC244 (sn74ac244 p.6). VCC 2-6 V (sn74ac244 p.4); CD74AC244 1.5-5.5 V (cd74ac244 p.5) and SN74AC244-Q1 1.5-6 V (sn74ac244-q1 p.5) via partVcc |
| ACT | 6.5 | 7 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, SN74ACT244 (sn74act244 p.6). VCC 4.5-5.5 V (sn74act244 p.4); CD74ACT244 also 4.5-5.5 V (cd74act244 p.5) |
| AHC | 5.4 | 5.4 | [2, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, SN74AHC244 (sn74ahc244 p.7). VCC 2-5.5 V, VI 0-5.5 V (sn74ahc244 p.5) |
| AHCT | 5.9 | 5.9 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, SN74AHCT244 (sn74ahct244 p.5). VCC 4.5-5.5 V (sn74ahct244 p.4) |
| ALS | 10 | 10 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL A->Y at 4.5-5.5 V, 0-70 C, 50 pF; SN74ALS244C gives no typical (sn74als244c p.4). VCC 4.5-5.5 V (sn74als244c p.3) |
| ALVC | 2.8 | 2.8 | [1.65, 3.6] |  |  |  | Max tpd A->Y at 3.3 V, -40 to 85 C, 50 pF; SN74ALVC244 gives no typical (sn74alvc244 p.4). VCC 1.65-3.6 V, VI 0-3.6 V (sn74alvc244 p.3); ALVCH VI 0-VCC (sn74alvch244 p.3) |
| AS | 6.2 | 6.2 | [4.5, 5.5] |  |  |  | Max tPLH/tPHL A->Y at 4.5-5.5 V, 0-70 C, 50 pF; SN74AS244A gives no typical (sn74as244a p.5). VCC 4.5-5.5 V (sn74as244a p.4) |
| AUC | 1.1 | 1.1 | [0.8, 2.7] | 3.6 |  |  | Typical tpd A->Y at 1.8 V, 25 C, 15 pF, SN74AUC244 (sn74auc244 p.4). VCC 0.8-2.7 V, inputs to 3.6 V (sn74auc244 p.3); AUCH same (sn74auch244) |
| BCT | 2.5 | 3.2 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, BCT244 (sn74bct244 p.4). VCC 4.5-5.5 V, SN54 and SN74 (sn74bct244 p.3); SN64BCT244 same (sn64bct244 p.3) |
| F | 3.6 | 3.6 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, 50 pF, F244 (sn74f244 p.4). VCC 4.5-5.5 V, SN54F and SN74F (sn74f244 p.3) |
| FCT | 4.5 | 4.5 | [4.75, 5.25] |  |  |  | Typical tpd A->Y at 5 V, 25 C, CD74FCT244 (cd74fct244 p.3; CD74FCT244AT 3.8). CY74FCT244T prints max only: 6.5 (T), 4.6 (AT), 4.1 (CT), 3.6 (DT) ns (cy74fct244t p.7). VCC 4.75-5.25 V (cd74fct244 p.2; CY74FCT244T/DT also, cy74fct244t p.4); CY54FCT244T 4.5-5.5 V via partVcc |
| HC | 13 | 13 | [2, 6] |  |  |  | Typical tpd A->Y at 4.5 V, 25 C, 50 pF, SN74HC244 (sn74hc244 p.7). VCC 2-6 V (sn74hc244 p.4); CD74HC244 also 2-6 V (cd74hc244 p.4) |
| HCS | 7 | 7 | [2, 6] |  |  |  | Typical tpd A->Y at 4.5 V, 25 C, 50 pF, SN74HCS244 (sn74hcs244 p.5). VCC 2-6 V (sn74hcs244 p.4) |
| HCT | 15 | 15 | [4.5, 5.5] |  |  |  | Typical tpd A->Y at 4.5 V, 25 C, 50 pF, SN74HCT244 (sn74hct244 p.7). VCC 4.5-5.5 V (sn74hct244 p.4); CD74HCT244 also 4.5-5.5 V (cd74hct244 p.4) |
| LS | 12 | 12 | [4.75, 5.25] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, CL 45 pF, RL 667 ohm, SNx4LS244 (sn74ls244 p.6). VCC 4.75-5.25 V for SN74LS (sn74ls244 p.4) |
| LV | 6.8 | 6.8 | [2, 5.5] |  |  |  | Typical tpd A->Y at 3.3 V, 25 C, 50 pF, SN74LV244A (sn74lv244a p.7). VCC 2-5.5 V, VI 0-5.5 V (sn74lv244a p.5) |
| LVC | 3.9 | 3.9 | [1.65, 3.6] | 5.5 |  |  | Typical tpd A->Y at 3.3 V, 25 C, 50 pF, SN74LVC244A (sn74lvc244a p.8). VCC 1.65-3.6 V, inputs to 5.5 V (sn74lvc244a p.6); LVCH same range (sn74lvch244a p.4); SN74LVCZ244A 2.7-3.6 V via partVcc (sn74lvcz244a p.4) |
| LVT | 2.3 | 2.1 | [2.7, 3.6] | 5.5 |  |  | Typical tPLH/tPHL A->Y at 3.3 V, 25 C, 50 pF, SN74LVT244B (sn74lvt244b p.4). VCC 2.7-3.6 V, inputs to 5.5 V (sn74lvt244b p.3); LVTH/LVTZ same (sn74lvth244a, sn74lvtz244) |
| S | 6 | 6 | [4.75, 5.25] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, CL 50 pF, RL 90 ohm, SNx4S244 (sn74s244 p.6, same document as sn74ls244). VCC 4.75-5.25 V for SN74S; SN54S 4.5-5.5 V via partVcc (sn74s244 p.4) |

- Part range: names starting `SN54S` (S): vcc [4.5, 5.5]

- Part range: names starting `CD74AC` (AC): vcc [1.5, 5.5]

- Part range: names starting `SN74AC244Q` (AC): vcc [1.5, 6]

- Part range: names starting `CY54` (FCT): vcc [4.5, 5.5]

- Part range: names starting `5962-92203` (FCT): vcc [4.5, 5.5]

- Part range: names starting `SN74LVCZ` (LVC): vcc [2.7, 3.6], inputMax 5.5

## 74_25244

- Kind / model: `buffer` / `tristate`; function "Buffer"; 25-ohm octal buffer/driver, 3-state outputs
- Report: reports/G16.md; parts added: 2
- PDFs: sn64bct25244, sn74bct25244
- Pinouts: `p24` (24 pins) for DW
- Source: Pinout from sn74bct25244 p.1 (DW/NT) and sn64bct25244 p.1 (DW/NT), identical; distributed GND/VCC pins. Function table p.1: OE L -> Y = A, OE H -> Z; 1OE gates 1A1-1A4, 2OE gates 2A1-2A4 (logic diagram p.2).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| BCT | 3.2 | 4 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V and 25 C (sn74bct25244 p.3; sn64bct25244 p.3 same). VCC 4.5-5.5 V (sn74bct25244 p.2, sn64bct25244 p.2) |

## 74_2541

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, 25-ohm series output resistors, 3-state outputs
- Report: reports/G04.md; parts added: 7
- PDFs: cy74fct2541t
- Pinouts: `p20` (20 pins) for DBQ, DW, CY-QC
- Source: Pinout cy74fct2541t p.1 (Q/QSOP = DBQ and SO = DW, same): 1 OEA, 2-9 D0-D7, 10 GND, 11-18 O7-O0, 19 OEB, 20 VCC. Function table p.2: OEA and OEB both low -> O = D, both high -> Z (the table omits mixed rows; the logic diagram ANDs the two active-low enables like the '541). Speed grades: FCT2541T 8 ns, AT 4.8 ns, CT 4.1 ns max (p.1, p.5); one FCT timing entry uses the slowest (T) grade.

```json
{"units": [{"a": ["D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"], "y": ["O0", "O1", "O2", "O3", "O4", "O5", "O6", "O7"], "oe": [{"pin": "OEA", "active": "low"}, {"pin": "OEB", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 8 | 8 | [4.75, 5.25] |  |  |  | Max tPLH/tPHL D->O, 50 pF, CY74FCT2541T grade (no typical printed; AT 4.8, CT 4.1) (cy74fct2541t p.5). VCC 4.75-5.25 V (cy74fct2541t p.3) |

## 74_2827

- Kind / model: `buffer` / `tristate`; function "Buffer"; 10-bit buffer/driver, 3-state outputs with 25-ohm series resistors
- Report: reports/G12.md; parts added: 2
- PDFs: sn74abt2827, sn74bct2827c
- Pinouts: `p24` (24 pins) for DW, NS
- Source: Pinout sn74abt2827 p.1 (DW, NT, JT) and sn74bct2827c p.1 (DW, NT) - identical. Function table sn74abt2827 p.2 (OE1 and OE2 both low = enabled). Output series resistors not modeled.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y10"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 3.3 | 2.7 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74abt2827 p.4); VCC 4.5-5.5 V (p.3) |
| BCT | 3.6 | 5.1 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74bct2827c p.4); VCC 4.5-5.5 V (p.3) |

## 74_2827_FCT

- Kind / model: `buffer` / `tristate`; function "Buffer"; 10-bit buffer, 3-state outputs with 25-ohm series resistors (FCT, D0-D9/Y0-Y9 naming)
- Report: reports/G12.md; parts added: 2
- PDFs: cy74fct2827t
- Pinouts: `p24` (24 pins) for DBQ, CY-QC
- Source: Separate device only because cy74fct2827t names the pins D0-D9/Y0-Y9 (74_2827 uses A1-A10/Y1-Y10); same positions. Pinout cy74fct2827t p.1 (Q package = QSOP/DBQ); function table p.2 (OE1 and OE2 both low = enabled). Output series resistors not modeled.

```json
{"units": [{"a": ["D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9"], "y": ["Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 8 | 8 | [4.75, 5.25] |  |  |  | max tPLH/tPHL D->Y, CY74FCT2827AT grade, 50 pF (cy74fct2827t p.4; CT grade 4.4; no typ printed); VCC 4.75-5.25 V (p.3) |

## 74_29827

- Kind / model: `buffer` / `tristate`; function "Buffer"; 10-bit buffer/bus driver, 3-state outputs
- Report: reports/G07.md; parts added: 2
- PDFs: sn74als29827, sn74bct29827b
- Pinouts: `p24` (24 pins) for DW
- Source: Pinout sn74als29827 p.1 (DW/NT), sn74bct29827b p.1 (DW/NT/JT/W). The 3-state control is a 2-input NOR: outputs drive only while OE1 and OE2 are both low (sn74als29827 p.1). Non-inverting.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y10"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 8 | 8 | [4.75, 5.25] |  |  |  | max tPLH/tPHL A->Y at CL 50 pF, no typ printed (sn74als29827 p.3; 15/15 at 300 pF). VCC 4.75-5.25 V (p.2) |
| BCT | 3.3 | 5.1 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C (sn74bct29827b p.3). VCC 4.5-5.5 V (p.2) |

## 74_29828

- Kind / model: `buffer` / `tristate`; function "Buffer"; 10-bit inverting buffer/bus driver, 3-state outputs
- Report: reports/G12.md; parts added: 0
- PDFs: sdas095b
- Pinouts: `p24` (24 pins) for DW, NT
- Source: Pinout sdas095b p.1 (DW, NT; shared with SN74ALS29827); OE1 and OE2 both low enable (2-input NOR, p.1); SN74ALS29828 inverts (p.1 description, logic symbol). The PDF's packaging table lists only SN74ALS29827DW; the 29828 rows come from the census.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y10"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 7 | 7.5 | [4.75, 5.25] |  |  |  | max tPLH/tPHL A->Y, SN74ALS29828, 50 pF (sdas095b p.3, no typ printed); VCC 4.75-5.25 V (p.2) |

## 74_365

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex buffer/line driver, 3-state outputs
- Report: reports/G09.md; parts added: 11
- PDFs: sn54hc365, sn74hc365, sn74hcs365, sn74hcs365-q1
- Pinouts: `p16` (16 pins) for D, N, NS, PW, J, DYY, BQB
- Source: Pinout sn74hc365 p.3 (J/D/N/NS/PW; sn54hc365 is the same PDF), sn74hcs365 p.3 (D/DYY/PW), sn74hcs365-q1 p.3 (D/PW, WBQB same numbering + thermal pad). OE1 and OE2 active low, both low = Y=A (sn74hc365 p.1, sn74hcs365 p.3).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 12 | 12 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hc365 p.5); VCC 2-6 V, SN54 and SN74 (p.4) |
| HCS | 9 | 9 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hcs365 p.5; -q1 p.5 same); VCC 2-6 V (p.4) |

## 74_365_CD

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex buffer/line driver, 3-state outputs
- Report: reports/G09.md; parts added: 3
- PDFs: cd74hc365, cd74hct365
- Pinouts: `p16` (16 pins) for N, D, M, J, F
- Source: Pinout cd74hc365 / cd74hct365 (same PDF) p.3 Figure 4-1 + Table 4-1 (J/N/D: OE1 and OE2 active low, 1A..6A, 1Y..6Y). Timing p.6.

```json
{"units": [{"a": ["1A", "2A", "3A", "4A", "5A", "6A"], "y": ["1Y", "2Y", "3Y", "4Y", "5Y", "6Y"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 8 | 8 | [2, 6] |  |  |  | typ tPLH/tPHL data to outputs at 5 V, 15 pF (cd74hc365 p.6); VCC 2-6 V (p.4) |
| HCT | 9 | 9 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL data to outputs at 5 V, 15 pF (cd74hct365 p.6); VCC 4.5-5.5 V (p.4) |

## 74_365_LS

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex bus driver, 3-state outputs
- Report: reports/G09.md; parts added: 6
- PDFs: sn54ls365a, sn74ls365a
- Pinouts: `p16` (16 pins) for D, N, NS, J, W; `lccc20` (20 pins) for FK
- Source: sn74ls365a = sn54ls365a (same scanned PDF, covers '365A-'368A). Pinout p.1 ('LS365A J/W/D/N top view; 'LS365A FK: 1 NC, 2 G1, 3 A1, 4 Y1, 5 A2, 6 NC, 7 Y2, 8 A3, 9 Y3, 10 GND, 11 NC, 12 Y4, 13 A4, 14 Y5, 15 A5, 16 NC, 17 Y6, 18 A6, 19 G2, 20 VCC). G1, G2 active low into an AND (logic diagram p.2). VCC and timing p.6-7 (rendered pages).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6"], "oe": [{"pin": "G1", "active": "low"}, {"pin": "G2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 10 | 9 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 45 pF, RL 667 ohm (sn74ls365a p.7); VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (p.6) |

- Part range: names starting `SNJ54LS365A` (LS): vcc [4.5, 5.5]

- Part range: names starting `JM38510/32201` (LS): vcc [4.5, 5.5]

## 74_366

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex inverting buffer/line driver, 3-state outputs
- Report: reports/G05.md; parts added: 2
- PDFs: cd74hc366, cd74hc366-q1
- Pinouts: `p16` (16 pins) for D, M, N
- Source: Pinout: cd74hc366 p.3 Table 4-1 (N, D/M SOIC); cd74hc366-q1 p.3 figure (D) same. Function table cd74hc366 p.8 Table 7-1: OE1 and OE2 NORed; both L -> Y = not A; either H -> Z (column header misprinted 'HCT 366').

```json
{"units": [{"a": ["1A", "2A", "3A", "4A", "5A", "6A"], "y": ["1Y", "2Y", "3Y", "4Y", "5Y", "6Y"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 9 | 9 | [2, 6] |  |  |  | typ tPLH/tPHL data->output, HC366, at 5 V, 15 pF (cd74hc366 p.6; only typ printed; max 22 ns at 4.5 V 50 pF); VCC 2-6 V (cd74hc366 p.4; -Q1 p.4 same) |

## 74_367

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex buffer/line driver, 3-state outputs (4 + 2 enables)
- Report: reports/G14.md; parts added: 21
- PDFs: sn54hc367, sn74ahc367, sn74ahct367, sn74hc367, sn74hcs367, sn74hcs367-q1, sn74lv367a
- Pinouts: `p16` (16 pins) for D, N, NS, PW, DB, DGV, J, BQB
- Source: Pinout: sn74hc367 p.3 (J, D, N, NS, PW; same file as sn54hc367), sn74ahc367 p.3 (D, DB, DGV, N, PW), sn74ahct367 p.3, sn74hcs367 p.3 and sn74hcs367-q1 p.3 (D, PW, WBQB with thermal pad, same numbering), sn74lv367a p.3 (D, DGV, NS, PW). 1OE drives 1A1-1A4, 2OE drives 2A1-2A2, both active low (sn74ahc367 p.8 function table). JM38510/65708BEA is SN54HC367J per the sn54hc367 packaging table.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2"], "y": ["2Y1", "2Y2"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 12 | 12 | [2, 6] |  |  |  | Typical tpd A->Y at 4.5 V, 50 pF, 25 C (sn74hc367 p.5). VCC 2-6 V for SN54HC367 and SN74HC367 (p.4) |
| AHC | 6.1 | 6.2 | [2, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 50 pF (sn74ahc367 p.5). VCC 2-5.5 V, VI to 5.5 V (p.4) |
| AHCT | 3.5 | 3.3 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 50 pF (sn74ahct367 p.5). VCC 4.5-5.5 V (p.4) |
| HCS | 6 | 6 | [2, 6] |  |  |  | Typical tpd A->Y at 4.5 V, 50 pF (sn74hcs367 p.5). VCC 2-6 V (p.4; sn74hcs367-q1 same) |
| LV | 6.2 | 6.2 | [2, 5.5] |  |  |  | Typical tpd A->Y at 3.3 V, 50 pF (sn74lv367a p.7). VCC 2-5.5 V, VI to 5.5 V (p.5) |

## 74_367_CD

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex buffer/line driver, 3-state outputs (4 + 2 enables)
- Report: reports/G14.md; parts added: 4
- PDFs: cd74hc367, cd74hct367
- Pinouts: `p16` (16 pins) for N, D, J
- Source: Pinout from cd74hc367 p.3 (one data sheet for CD54/74HC367, HCT367, HC368, HCT368; also filed as cd74hc367, cd74hct367): OE1 1, 1A 2, 1Y 3, 2A 4, 2Y 5, 3A 6, 3Y 7, GND 8, 4Y 9, 4A 10, 5Y 11, 5A 12, 6Y 13, 6A 14, OE2 15, VCC 16 (same positions as 74_367, different names; '368 Y pins printed with overbars). OE1 controls 4 gates, OE2 the other 2 (p.8 description).

```json
{"units": [{"a": ["1A", "2A", "3A", "4A"], "y": ["1Y", "2Y", "3Y", "4Y"], "oe": [{"pin": "OE1", "active": "low"}]}, {"a": ["5A", "6A"], "y": ["5Y", "6Y"], "oe": [{"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 8 | 8 | [2, 6] |  |  |  | Typical tpd data->outputs at 5 V, 15 pF (cd74hc367 p.6, note 3; at 4.5 V/50 pF only max 21 ns is given). VCC 2-6 V (p.4) |
| HCT | 9 | 9 | [4.5, 5.5] |  |  |  | Typical tpd data->outputs at 5 V, 15 pF (cd74hc367 p.6, note 3; max 25/30 ns at 4.5 V, 50 pF). VCC 4.5-5.5 V (p.4) |

## 74_367_LS

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex buffer/line driver, 3-state outputs (4 + 2 enables)
- Report: reports/G14.md; parts added: 5
- PDFs: sn54ls367a, sn74ls367a
- Pinouts: `p16` (16 pins) for D, N, NS, J, DB
- Source: Pinout from sn74ls367a (scanned SN54365A-SN74LS368A data sheet, Dec 1983 rev Mar 1988; identical file as sn54ls367a, sn74ls368a, sn54ls368a) p.1 (J, D, N packages; NS uses the same numbering); enables printed 1G/2G (active low), else same pins as 74_367. Logic diagram p.2: 1G drives 1A1-1A4, 2G drives 2A1-2A2.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1G", "active": "low"}]}, {"a": ["2A1", "2A2"], "y": ["2Y1", "2Y2"], "oe": [{"pin": "2G", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 10 | 9 | [4.75, 5.25] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, RL 667 ohm, CL 45 pF (sn74ls367a p.7, scanned). VCC 4.75-5.25 V SN74LS, 4.5-5.5 V SN54LS (p.6) |

- Part range: names starting `SN54` (LS): vcc [4.5, 5.5]

- Part range: names starting `SNJ54` (LS): vcc [4.5, 5.5]

## 74_368

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex inverting buffer/line driver, 3-state outputs (4 + 2 enables)
- Report: reports/G14.md; parts added: 6
- PDFs: sn54hc368, sn74hc368
- Pinouts: `p16` (16 pins) for D, N, NS, PW, J
- Source: Pinout: sn74hc368 p.3 (J, D, N, NS, PW; FK not in this group; same file as sn54hc368); outputs printed with overbars (1Y1...). Y = not A when OE low, Z when OE high (sn74hc368 p.9). 1OE drives 1A1-1A4, 2OE drives 2A1-2A2. 5962-8681201EA is SNJ54HC368J per the sn54hc368 packaging table.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2"], "y": ["2Y1", "2Y2"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 12 | 12 | [2, 6] |  |  |  | Typical tpd A->Y at 4.5 V, 50 pF, 25 C (sn74hc368 p.6). VCC 2-6 V for SN54HC368 and SN74HC368 (p.4) |

## 74_368_CD

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex inverting buffer/line driver, 3-state outputs (4 + 2 enables)
- Report: reports/G14.md; parts added: 3
- PDFs: cd74hc368, cd74hct368
- Pinouts: `p16` (16 pins) for N, D, J
- Source: Pinout from cd74hc367 p.3 (one data sheet for CD54/74HC367, HCT367, HC368, HCT368; also filed as cd74hc368, cd74hct368): OE1 1, 1A 2, 1Y 3, 2A 4, 2Y 5, 3A 6, 3Y 7, GND 8, 4Y 9, 4A 10, 5Y 11, 5A 12, 6Y 13, 6A 14, OE2 15, VCC 16 (same positions as 74_368, different names; '368 Y pins printed with overbars). OE1 controls 4 gates, OE2 the other 2 (p.8 description).

```json
{"units": [{"a": ["1A", "2A", "3A", "4A"], "y": ["1Y", "2Y", "3Y", "4Y"], "oe": [{"pin": "OE1", "active": "low"}]}, {"a": ["5A", "6A"], "y": ["5Y", "6Y"], "oe": [{"pin": "OE2", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 9 | 9 | [2, 6] |  |  |  | Typical tpd data->outputs at 5 V, 15 pF (cd74hc367 p.6, note 3; at 4.5 V/50 pF only max 21 ns is given). VCC 2-6 V (p.4) |
| HCT | 11 | 11 | [4.5, 5.5] |  |  |  | Typical tpd data->outputs at 5 V, 15 pF (cd74hc367 p.6, note 3; max 25/30 ns at 4.5 V, 50 pF). VCC 4.5-5.5 V (p.4) |

## 74_368_LS

- Kind / model: `buffer` / `tristate`; function "Buffer"; Hex inverting buffer/line driver, 3-state outputs (4 + 2 enables)
- Report: reports/G14.md; parts added: 4
- PDFs: sn54ls368a, sn74ls368a
- Pinouts: `p16` (16 pins) for D, N, NS, J, DB
- Source: Pinout from sn74ls367a (scanned SN54365A-SN74LS368A data sheet, Dec 1983 rev Mar 1988; identical file as sn54ls367a, sn74ls368a, sn54ls368a) p.1 (J, D, N packages; NS uses the same numbering); enables printed 1G/2G (active low), else same pins as 74_368. Logic diagram p.2: 1G drives 1A1-1A4, 2G drives 2A1-2A2, inverting outputs.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1G", "active": "low"}]}, {"a": ["2A1", "2A2"], "y": ["2Y1", "2Y2"], "oe": [{"pin": "2G", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 7 | 12 | [4.75, 5.25] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 25 C, RL 667 ohm, CL 45 pF (sn74ls367a p.9, scanned). VCC 4.75-5.25 V SN74LS, 4.5-5.5 V SN54LS (p.8) |

- Part range: names starting `SN54` (LS): vcc [4.5, 5.5]

- Part range: names starting `SNJ54` (LS): vcc [4.5, 5.5]

## 74_4T125

- Kind / model: `buffer` / `tristate`; function "Buffer"; Quadruple bus buffer, 3-state outputs, active-low enables, with level shifting
- Report: reports/G06.md; parts added: 3
- PDFs: sn74lv4t125, sn74lv4t125-ep
- Pinouts: `p14` (14 pins) for PW, RGY
- Source: Pinout sn74lv4t125 p.3 (PW, RGY): '125 layout, 1OE 1, 1A 2, 1Y 3, 2OE 4, 2A 5, 2Y 6, GND 7, 3Y 8, 3A 9, 3OE 10, 4Y 11, 4A 12, 4OE 13, VCC 14; sn74lv4t125-ep p.3 (PW) same. Function table p.13 (OE L: Y = A, OE H: Z). RGY thermal pad is not a pin.

```json
{"units": [{"a": ["1A"], "y": ["1Y"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A"], "y": ["2Y"], "oe": [{"pin": "2OE", "active": "low"}]}, {"a": ["3A"], "y": ["3Y"], "oe": [{"pin": "3OE", "active": "low"}]}, {"a": ["4A"], "y": ["4Y"], "oe": [{"pin": "4OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 5 | 5 | [1.6, 5.5] |  |  |  | typ tpd at 3.3 V, 30 pF (sn74lv4t125 p.7); VCC 1.6-5.5 V, VI to 5.5 V (p.5); -ep p.4 same |

## 74_540

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal inverting buffer/line driver, 3-state outputs
- Report: reports/G09.md; parts added: 57
- PDFs: sn54ahct540, sn74abt540, sn74ahc540, sn74ahc540-q1, sn74ahct540, sn74ahct540-q1, sn74als540, sn74bct540a, sn74hc540, sn74hct540, sn74lv540a, sn74lvc540a, sn74lvc540a-q1, sn74lvth540
- Pinouts: `p20` (20 pins) for DB, DGS, DGV, DW, N, NE, NS, PW, RKS, RGY, J, W, FK
- Source: Flow-through pinout, same in sn74abt540 p.1, sn74ahc540 p.3, sn74ahc540-q1 p.3 (RKS = same numbering + pad, DGS, PW), sn74ahct540 p.3 (= sn54ahct540), sn74ahct540-q1 p.3 (RKS/DGS/PW), sn74als540 p.1, sn74bct540a p.1, sn74hc540 p.3 (FK uses the same numbers), sn74hct540 p.1, sn74lv540a p.3 (RGY same numbering), sn74lvc540a p.3 (RKS same numbering), sn74lvc540a-q1 p.1, sn74lvth540 p.1. OE1, OE2 active low (2-input NOR), Y = not A (sn74abt540 p.1 function table).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.9 | 3.1 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74abt540 p.3); VCC 4.5-5.5 V (p.2) |
| AHC | 5.2 | 5.2 | [2, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74ahc540 p.6); VCC 2-5.5 V (p.4); -q1 same range (p.5) |
| AHCT | 6 | 6 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74ahct540 p.5); VCC 4.5-5.5 V (p.4); -q1 p.4 same |
| ALS | 12 | 9 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->Y, SN74ALS540, VCC 4.5-5.5 V, 50 pF (no typical printed) (sn74als540 p.4); VCC 4.5-5.5 V (p.3) |
| BCT | 4.1 | 1.9 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74bct540a p.4); VCC 4.5-5.5 V (p.3) |
| HC | 10 | 10 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hc540 p.5); VCC 2-6 V (p.4) |
| HCT | 13 | 13 | [4.5, 5.5] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hct540 p.3); VCC 4.5-5.5 V (p.2) |
| LV | 5.8 | 5.8 | [2, 5.5] |  |  |  | typ tpd A->Y at 3.3 V, 50 pF (sn74lv540a p.6); VCC 2-5.5 V, VI 0-5.5 V (p.5) |
| LVC | 5.3 | 5.3 | [1.65, 3.6] | 5.5 |  |  | max tpd A->Y at 3.3 V (no typical printed) (sn74lvc540a p.6); VCC 1.65-3.6 V, VI 0-5.5 V (p.5) |
| LVT | 2.4 | 2.7 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->Y at 3.3 V, 50 pF, SN74LVTH540 (sn74lvth540 p.5); VCC 2.7-3.6 V, VI to 5.5 V (p.4) |

- Part range: names starting `SN74LVC540ADGSRQ1` (LVC): vcc [2, 3.6], inputMax 5.5

- Part range: names starting `SN74LVC540AQ` (LVC): vcc [2, 3.6], inputMax 5.5

- Part range: names starting `SN74LVC540AW` (LVC): vcc [2, 3.6], inputMax 5.5

- Part range: names starting `CLVC540A` (LVC): vcc [2, 3.6], inputMax 5.5

## 74_5402

- Kind / model: `buffer` / `tristate`; function "Buffer"; 12-bit line/memory driver, 3-state outputs, 25-ohm series output resistors
- Report: reports/G02.md; parts added: 2
- PDFs: sn74abt5402a
- Pinouts: `p28` (28 pins) for DW
- Source: sn74abt5402a p.1: DW (and JT) pinout 1-6 Y1-Y6, 7 GND, 8-13 Y7-Y12, 14 OE1, 15 OE2, 16-20 D12-D8, 21 VCC, 22-28 D7-D1. Function table p.1: OE1 and OE2 both low = Y follows D; either high = Z (2-input AND of active-low enables); noninverting. Outputs have equivalent 25-ohm series resistors (not modeled).

```json
{"units": [{"a": ["D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "D11", "D12"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y10", "Y11", "Y12"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 4.5 | 3.7 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL D->Y at 5 V, 25 C, 50 pF (sn74abt5402a p.4); VCC 4.5-5.5 V SN54/SN74 (p.3) |

## 74_5403

- Kind / model: `buffer` / `tristate`; function "Buffer"; 12-bit inverting line/memory driver, 3-state outputs with 25-ohm series resistors
- Report: reports/G03.md; parts added: 1
- PDFs: sn74abt5403
- Pinouts: `p28` (28 pins) for DW
- Source: Pinout from sn74abt5403 p.1 (DW/JT): D1-D12 in, Y1-Y12 out, OE1 14, OE2 15 (both active low, 2-input AND enable). Function table p.1: OE1 = OE2 = L -> Y = not D; either OE high -> Z. 25-ohm series output resistors (not modeled). Datasheet marked PRODUCT PREVIEW (1997) though the DW part is Active (p.6).

```json
{"units": [{"a": ["D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "D11", "D12"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y10", "Y11", "Y12"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 4.5 | 4.4 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL D->Y at 5 V, 50 pF (sn74abt5403 p.4); VCC 4.5-5.5 V, VI 0-VCC (sn74abt5403 p.3) |

## 74_540_CD

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal inverting buffer/line driver, 3-state outputs
- Report: reports/G09.md; parts added: 8
- PDFs: cd54act540, cd74ac540, cd74act540, cd74hc540, cd74hct540
- Pinouts: `p20` (20 pins) for DW, N, J, F
- Source: Pinout cd74ac540 p.3 Figure 3-1 (= cd54act540, cd74act540; OE1, A0..A7, Y0..Y7, OE2; the Table 3-1 on that page belongs to another device and was ignored) and cd74hc540 p.3 (= cd74hct540; pin 1 printed 'OE', text p.1/p.9 calls it OE1). OE1 and OE2 active low, inverting (cd74hc540 p.9).

```json
{"units": [{"a": ["A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"], "y": ["Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 6.2 | 6.2 | [1.5, 5.5] |  |  |  | max tPLH/tPHL data to output AC540 at 5 V, 50 pF, -40 to 85 C (no typical printed) (cd74ac540 p.7); VCC 1.5-5.5 V (p.4) |
| ACT | 6.5 | 6.5 | [4.5, 5.5] |  |  |  | max tPLH/tPHL data to output ACT540 at 5 V, 50 pF, -40 to 85 C (no typical printed) (cd74act540 p.8); VCC 4.5-5.5 V (p.4) |
| HC | 9 | 9 | [2, 6] |  |  |  | typ tPLH/tPHL data to outputs (540) at 5 V, 15 pF (cd74hc540 p.6); VCC 2-6 V (p.4) |
| HCT | 9 | 9 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL data to outputs (540) at 5 V, 15 pF (cd74hct540 p.6); VCC 4.5-5.5 V (p.4) |

## 74_540_FCT

- Kind / model: `buffer` / `tristate`; function "Buffer"; 8-bit inverting buffer/line driver, 3-state outputs
- Report: reports/G09.md; parts added: 1
- PDFs: cy74fct540t
- Pinouts: `p20` (20 pins) for DBQ, CY-QC
- Source: Pinout cy74fct540t p.1 (Q = QSOP/DBQ top view: OEA, D0..D7, GND, O7..O0, OEB, VCC). Function table p.2: OEA L OEB L = O = not D; otherwise Z (both active low).

```json
{"units": [{"a": ["D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"], "y": ["O0", "O1", "O2", "O3", "O4", "O5", "O6", "O7"], "oe": [{"pin": "OEA", "active": "low"}, {"pin": "OEB", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| FCT | 4.1 | 4.1 | [4.75, 5.25] |  |  |  | max tPLH/tPHL D->O, CY74FCT540CT (no typical printed) (cy74fct540t p.5); VCC 4.75-5.25 V (CY74, p.2) |

## 74_540_LS

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal inverting buffer/line driver, 3-state outputs
- Report: reports/G09.md; parts added: 7
- PDFs: sn54ls540, sn74ls540
- Pinouts: `p20` (20 pins) for DB, DW, N, NS, J, W, FK
- Source: sn74ls540 = sn54ls540 (same scanned PDF). Pinout p.1 (J/W/DW/N top view; FK uses the same numbers: 1 G1, 2 A1, 3 A2 ... 19 G2, 20 VCC). G1, G2 active low (2-input NOR), Y = not A (p.1-2). VCC p.3, timing p.4 (rendered pages).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8"], "oe": [{"pin": "G1", "active": "low"}, {"pin": "G2", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LS | 9 | 9 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL 'LS540 at 5 V, 45 pF, RL 667 ohm (sn74ls540 p.4); VCC SN74 4.75-5.25 V, SN54 4.5-5.5 V (p.3) |

- Part range: names starting `8415501` (LS): vcc [4.5, 5.5]

- Part range: names starting `JM38510/32404` (LS): vcc [4.5, 5.5]

## 74_541

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, 3-state outputs
- Report: reports/G05.md; parts added: 118
- PDFs: cd54act541, cd54hc541, cd74ac541, cd74act541, cd74fct541, cd74hc541, cd74hct541, cy54fct541t, cy74fct541t, sn54abt541, sn54ahct541, sn54bct541, sn54hc541, sn54hct541, sn74abt541b, sn74abt541b-q1, sn74ac541, sn74ac541-q1, sn74ahc541, sn74ahc541-q1, sn74ahct541, sn74ahct541-q1, sn74als541, sn74bct541a, sn74f541, sn74hc541, sn74hcs541, sn74hct541, sn74ls541, sn74lv541a, sn74lv541a-q1, sn74lvc541a, sn74lvc541a-ep, sn74lvc541a-q1, sn74lvth541
- Pinouts: `p20` (20 pins) for DB, DBQ, DGS, DGV, DW, FK, J, N, NS, PW, RGY, RKS
- Source: Pinout: sn74hc541 p.3 Pin Functions (OE1=1, A1-A8=2-9, GND=10, Y8-Y1=11-18, OE2=19, VCC=20); same positions in sn74ahc541 p.3, sn74ahct541 p.3 (J/W/FK too), sn74lvc541a p.3-4 (RKS/RGY same numbering), sn74ac541 p.3 (DGS, RKS), sn74hcs541 p.3, sn74lv541a p.3 (DB/DGV/DW/NS/PW/DGS, RGY/RKS), sn74abt541b p.1 (J/W/DB/DW/N/PW, FK), sn74lvth541, sn74als541, sn74bct541a, sn74f541, sn74ls541 p.1 (FK same numbering), cd74fct541 p.1. 20-pin FK LCCC uses the same 1-20 numbering (no NC). Names differ only by datasheet style at the same positions: CD74HC/HCT541 and CD54/74AC/ACT541 print OE1, A0-A7, Y0-Y7, OE2 (cd74hc541 p.3 figure, cd74ac541 p.3 figure; the cd74ac541 Pin Functions table p.3 is a copy of a '273 table and is wrong); CY54/74FCT541T prints OEA, D0-D7, OEB, O0-O7 (cy74fct541t p.1); SN54/74LS541 prints G1, G2 (sn74ls541 p.1). Merged under the SN names (A1-A8/Y1-Y8). Function table (sn74hc541, cd74fct541 p.1): OE1 L and OE2 L -> Y = A; either OE high -> Z.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 12 | 12 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hc541 p.6); VCC 2-6 V (sn74hc541 p.4; cd74hc541 p.4 same). CD74HC541 prints no 50 pF typ |
| HCT | 13 | 13 | [4.5, 5.5] |  |  |  | typ tpd A->Y at 4.5 V, 50 pF (sn74hct541 p.5); VCC 4.5-5.5 V SN54/SN74 (sn74hct541 p.4; cd74hct541 p.4 same) |
| HCS | 7 | 7 | [2, 6] |  |  |  | typ tpd A->Y at 4.5 V (sn74hcs541 p.5); VCC 2-6 V (sn74hcs541 p.4) |
| AC | 5.3 | 7 | [1.5, 6] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74ac541 p.7); VCC 1.5-6 V (sn74ac541 p.4; -Q1 same). CD74AC541: max 7.1 ns at 5 V, VCC 1.5-5.5 V (cd74ac541 p.4, p.7) -> partVcc |
| ACT | 7.5 | 7.5 | [4.5, 5.5] |  |  |  | max tPLH/tPHL data->output at 5 V, 50 pF, -40 to 85 C (cd74ac541 p.8, no typ printed); VCC 4.5-5.5 V (cd74ac541 p.4) |
| AHC | 5 | 5 | [2, 5.5] |  |  |  | typ A->Y at 5 V, 50 pF (sn74ahc541 p.6); VCC 2-5.5 V, VI to 5.5 V (sn74ahc541 p.4) |
| AHCT | 6.2 | 6.2 | [4.5, 5.5] |  |  |  | typ A->Y at 5 V, 50 pF (sn74ahct541 p.5); VCC 4.5-5.5 V SN54/SN74 (sn74ahct541 p.4) |
| ABT | 2 | 2.6 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF, SN74ABT541B (sn74abt541b p.5; SN54ABT541 typ 2.6/2.9); VCC 4.5-5.5 V SN54/SN74 (sn74abt541b p.3) |
| ALS | 14 | 10 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->Y, SN74ALS541, 4.5-5.5 V, 50 pF (sn74als541 p.4, no typ printed); VCC 4.5-5.5 V SN54/SN74 (sn74als541 p.3) |
| BCT | 3.7 | 5.5 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74bct541a p.3); VCC 4.5-5.5 V SN54/SN74 (sn74bct541a p.2) |
| F | 3.3 | 2.7 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V (sn74f541 p.3); VCC 4.5-5.5 V (sn74f541 p.2) |
| LS | 9 | 10 | [4.75, 5.25] |  |  |  | typ tPLH/tPHL at 5 V, 45 pF (sn74ls541 p.4, scanned); VCC SN74LS 4.75-5.25 V (sn74ls541 p.3) |
| FCT | 6 | 6 | [4.75, 5.25] |  |  |  | typ tpd A->Y at 5 V (cd74fct541 p.3); VCC 4.75-5.25 V (cd74fct541 p.3; CY74FCT541T 4.75-5.25 V, cy74fct541t p.3). CY74FCT541T max 8 / AT 4.8 / CT 4.1 ns (cy74fct541t p.6) |
| LV | 6.1 | 6.1 | [2, 5.5] |  |  |  | typ tpd A->Y at 3.3 V, 50 pF (sn74lv541a p.6); VCC 2-5.5 V, VI to 5.5 V (sn74lv541a p.5) |
| LVC | 5.1 | 5.1 | [1.65, 3.6] | 5.5 |  |  | max tpd A->Y at 3.3 V, -40 to 85 C (sn74lvc541a p.8, no typ printed); VCC 1.65-3.6 V SN74, VI to 5.5 V (sn74lvc541a p.6). -Q1/-EP PDFs give VCC 2-3.6 V (sn74lvc541a-q1 p.3, sn74lvc541a-ep p.3); main PDF used |
| LVT | 2.4 | 2.4 | [2.7, 3.6] | 5.5 |  |  | typ tPLH/tPHL A->Y at 3.3 V, 50 pF, SN74LVTH541 (sn74lvth541 p.5); VCC 2.7-3.6 V, VI to 5.5 V (sn74lvth541 p.3) |

- Part range: names starting `CD74AC541` (AC): vcc [1.5, 5.5]

- Part range: names starting `CY54FCT541` (FCT): vcc [4.5, 5.5]

- Part range: names starting `5962-9223705` (FCT): vcc [4.5, 5.5]

## 74_7541

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/driver, open-drain outputs, dual output enables
- Report: reports/G15.md; parts added: 17
- PDFs: sn74ac7541, sn74ac7541-q1, sn74act7541, sn74act7541-q1, sn74ahc7541-q1, sn74ahct7541-q1
- Pinouts: `p20` (20 pins) for DGS, PW, RKS
- Source: Pinout: sn74ac7541 p.3 (DGS/PW, RKS: same numbering, thermal pad not a pin), sn74act7541 p.3, sn74ac7541-q1 p.3, sn74act7541-q1 p.3, sn74ahc7541-q1 p.3, sn74ahct7541-q1 p.3. OE1 and OE2 both low = outputs active (open-drain); either high = high impedance (sn74ac7541 p.1).

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 3.8 | 6.5 | [1.5, 6] |  |  |  | sn74ac7541 p.6 prints only OE->Y: typ tPLZ 3.8 (used as release/rise) and tPZL 6.5 (fall) at 5 V, 50 pF; p.1 max tpd 11 ns at 5 V; VCC 1.5-6 V (p.4) |
| ACT | 4.2 | 5 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74act7541 p.5); VCC 4.5-5.5 V (p.4) |
| AHC | 10.5 | 42.6 | [2, 5.5] |  |  |  | max A->Y tPLZ (rise) / tPZL (fall) at 5 V, 25 C, 50 pF, no typ printed (sn74ahc7541-q1 p.6; 15 pF: 7.3/17.5); VCC 2-5.5 V, VI 0-5.5 V (p.4) |
| AHCT | 7.9 | 6 | [4.5, 5.5] |  |  |  | typ A->Y tPLZ (rise) / tPZL (fall) at 5 V, 50 pF, -40 to 125 C column (sn74ahct7541-q1 p.5); VCC 4.5-5.5 V, VI 0-5.5 V (p.4) |

## 74_756

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal inverting buffer/line driver, open-collector outputs
- Report: reports/G07.md; parts added: 4
- PDFs: sn74as756, sn74bct756
- Pinouts: `p20` (20 pins) for DW, N
- Source: Pinout sn74as756 p.1 (DW/N/J; pin 19 is 2OE-bar for 'AS756), sn74bct756 p.1 (DW/N). Inverting: sn74bct756 p.1 function table (OE L, A L -> Y H; OE H -> Y H = off), sn74as756 p.1 logic symbol (output negation). Open-collector outputs, 1OE and 2OE both active low (open-collector version of 'AS240).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": true, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AS | 19 | 6 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->Y SN74AS756, no typ printed, CL 50 pF, RL 500 ohm (sn74as756 p.4). VCC 4.5-5.5 V (p.3) |
| BCT | 8.5 | 2 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 25 C, CL 50 pF (sn74bct756 p.3). VCC 4.5-5.5 V (p.2) |

## 74_757

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, open-collector outputs (complementary enables)
- Report: reports/G08.md; parts added: 5
- PDFs: sn64bct757, sn74as757, sn74bct757
- Pinouts: `p20` (20 pins) for DW, N, NS
- Source: Open-collector version of the '241. Pinout sn74bct757 p.1 (DW/N), sn64bct757 p.1 (DW), sn74as757 p.1 (DW/N; pin 19 is 2OE active high for SN74AS757, footnote). Function tables sn74bct757 p.1 / sn64bct757 p.1: 1OE low enables 1Y = 1A, 2OE high enables 2Y = 2A; disabled outputs are off (H through pull-up). Timing sn74bct757 p.3, sn64bct757 p.3, sn74as757 p.4.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "high"}]}], "invert": false, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AS | 18.5 | 6 | [4.5, 5.5] |  |  |  | sn74as757 max tPLH/tPHL A->Y, VCC 4.5-5.5 V, 0-70 C (no typ printed) (p.4). VCC 4.5-5.5 V (p.3) |
| BCT | 8.3 | 4.2 | [4.5, 5.5] |  |  |  | sn74bct757 typ tPLH/tPHL A->Y at 5 V, 25 C, 50 pF (p.3); sn64bct757 same (p.3). VCC 4.5-5.5 V (sn74bct757 p.3, sn64bct757 p.3) |

## 74_760

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/driver, open-collector outputs ('244 pinout)
- Report: reports/G06.md; parts added: 8
- PDFs: sn54as760, sn74als760, sn74as760, sn74bct760, sn74bct760-ep
- Pinouts: `p20` (20 pins) for N, DW, NS, J
- Source: '244 pinout (1OE 1, 1A1 2, 2Y4 3, 1A2 4, 2Y3 5, 1A3 6, 2Y2 7, 1A4 8, 2Y1 9, GND 10, 2A1 11, 1Y4 12, 2A2 13, 1Y3 14, 2A3 15, 1Y2 16, 2A4 17, 1Y1 18, 2OE 19, VCC 20): sn74as760 (= sn74als760 = sn54as760, SDAS141A) p.1 (J, DW, N; FK same numbering), sn74bct760 p.1 (J, W, DW, N), sn74bct760-ep p.1 (DW). Function table OE L: Y = A; OE H: Y off (high via pull-up) (sn74bct760 p.1). Open-collector version of the '244: modeled as tristate with open-drain output.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 15 | 12 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->Y, SN74ALS760, 50 pF (sn74als760 p.3; no typical printed); VCC 4.5-5.5 V (p.2) |
| AS | 18.5 | 6 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->Y, SN74AS760, 50 pF (sn74as760 p.4; no typical printed); VCC 4.5-5.5 V for SN54 and SN74 (p.4) |
| BCT | 8 | 4.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74bct760 p.3); VCC 4.5-5.5 V (p.3); -ep p.3 same |

## 74_827

- Kind / model: `buffer` / `tristate`; function "Buffer"; 10-bit buffer/driver, 3-state outputs
- Report: reports/G15.md; parts added: 17
- PDFs: cy74fct827t, sn54abt827, sn74abt827, sn74lvc827a
- Pinouts: `p24` (24 pins) for DB, DBQ, CY-QC, DW, NT, PW, DGV, JT, W
- Source: Pinout: sn74abt827 p.1 (DB/DW/NT/PW, JT; W in the packaging table, assumed = JT), sn74lvc827a p.3 (DB/DGV/DW/PW), cy74fct827t p.1 (Q/SO; FCT sheet names D0-D9/Y0-Y9, same pins). Function table sn74abt827 p.1: OE1 and OE2 both low = drive.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y10"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ABT | 2.6 | 2.3 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL at 5 V, 25 C, 50 pF (sn74abt827 p.4); VCC 4.5-5.5 V SN54 and SN74 (p.3) |
| LVC | 6.7 | 6.7 | [1.65, 3.6] | 5.5 |  |  | max tpd at 3.3 V, 50 pF, -40 to 85 C, no typ printed (sn74lvc827a p.6); VCC 1.65-3.6 V, VI to 5.5 V (p.5) |
| FCT | 8 | 8 | [4.75, 5.25] |  |  |  | max tPLH/tPHL D->Y CY74FCT827AT, 50 pF, no typ printed (cy74fct827t p.5); the CT speed grade is max 4.4 ns; VCC 4.75-5.25 V CY74 (p.3) |

## 74_8T240

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal inverting buffer/driver, 3-state outputs, 5.5 V tolerant inputs
- Report: reports/G10.md; parts added: 5
- PDFs: sn74lv8t240, sn74lv8t240-q1
- Pinouts: `p20` (20 pins) for PW, DGS, RKS
- Source: sn74lv8t240 p.3 Figure 4-1/4-2 and Table 4-1 (PW/DGS and RKS, same numbers); sn74lv8t240-q1 p.3 same. Function table p.15 Table 7-1: OE low, Y = not A; OE high = Z (both OEs active low).

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 8.8 | 10.4 | [1.65, 5.5] |  |  |  | max tPLH/tPHL A->Y at 3.3 V, 25 C, CL 50 pF, no typical printed (sn74lv8t240 p.6); VCC 1.65-5.5 V, VI 0-5.5 V at any VCC (p.4) |

## 74_8T244

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer with 5.5-V tolerant inputs, 3-state outputs
- Report: reports/G11.md; parts added: 5
- PDFs: sn74lv8t244, sn74lv8t244-q1
- Pinouts: `p20` (20 pins) for DGS, PW, RKS
- Source: Standard '244 pinout: sn74lv8t244 p.3 Figure 4-1/4-2 + Table 4-1 (PW/DGS and RKS VQFN with the same numbers 1-20, thermal pad not a pin); sn74lv8t244-q1 p.3 identical. 1OE/2OE active low (Table 4-1). Single supply; inputs tolerate 5.5 V at any VCC.

```json
{"units": [{"a": ["1A1", "1A2", "1A3", "1A4"], "y": ["1Y1", "1Y2", "1Y3", "1Y4"], "oe": [{"pin": "1OE", "active": "low"}]}, {"a": ["2A1", "2A2", "2A3", "2A4"], "y": ["2Y1", "2Y2", "2Y3", "2Y4"], "oe": [{"pin": "2OE", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 8.8 | 10.4 | [1.65, 5.5] |  |  |  | Max tPLH/tPHL A->Y at 3.3 V, 25 C, 50 pF; no typical printed (sn74lv8t244 p.6). VCC 1.65-5.5 V, VI 0-5.5 V (sn74lv8t244 p.4; -Q1 same) |

## 74_8T540

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal inverting buffer with level-shifting (LVxT) inputs, 3-state outputs
- Report: reports/G12.md; parts added: 6
- PDFs: sn74lv8t540, sn74lv8t540-q1
- Pinouts: `p20` (20 pins) for DGS, PW, RKS
- Source: Single-supply part (one VCC; translation from low-threshold, 5.5 V-tolerant inputs), so tristate rather than translator. Pinout sn74lv8t540 p.3 Pin Functions (PW, DGS, RKS; RKS thermal pad not a pin); sn74lv8t540-q1 p.3 same. Function table sn74lv8t540 p.16: OE1 and OE2 low -> Y = not A.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": true, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 6.5 | 7.8 | [1.65, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 3.3 V, 50 pF (sn74lv8t540 p.7); VCC 1.65-5.5 V, VI to 5.5 V (p.5) |

## 74_8T541

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/driver, 3-state outputs, single-supply level shifter (LVxT reduced-threshold, 5.5 V tolerant inputs)
- Report: reports/G03.md; parts added: 6
- PDFs: sn74lv8t541, sn74lv8t541-q1
- Pinouts: `p20` (20 pins) for DGS, PW, RKS
- Source: Pinout from sn74lv8t541 p.3 (PW/DGS and RKS; RKS exposed pad not a pin) and sn74lv8t541-q1 p.3, identical. OE1 and OE2 active low, both low -> Y = A (sn74lv8t541 p.1, p.10). Single supply: up/down translation through input thresholds and 5.5 V tolerant inputs, so no second domain.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 8.8 | 10.4 | [1.65, 5.5] | 5.5 |  |  | max tPLH/tPHL A->Y at 3.3 V, 50 pF, 25 C (sn74lv8t541 p.6; table prints min and max only); VCC 1.65-5.5 V, VI 0-5.5 V (sn74lv8t541 p.4) |

## 74_8T7541

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer, open-drain outputs, 5.5 V tolerant LVxT inputs
- Report: reports/G04.md; parts added: 5
- PDFs: sn74lv8t7541, sn74lv8t7541-q1
- Pinouts: `p20` (20 pins) for DGS, PW, RKS
- Source: Pinout sn74lv8t7541 p.3 Pin Functions (PW, DGS, RKS same numbers; RKS thermal pad not a pin): 1 OE1, 2-9 A1-A8, 10 GND, 11-18 Y8-Y1, 19 OE2, 20 VCC; sn74lv8t7541-q1 p.3 same. Function table sn74lv8t7541 p.14: OE1 L and OE2 L -> Y = A (H = Z, open-drain off), either OE high -> Z. Single supply (outputs referenced to VCC); the level shifting is the reduced-threshold 5.5 V tolerant inputs plus open-drain outputs.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 13 | 38.8 | [1.65, 5.5] |  |  |  | Max A->Y at 3.3 V, 25 C, 50 pF (no typical printed): rise = tPLZ (output release) 13, fall = tPZL 38.8; at 15 pF 9.3 / 15.8 (sn74lv8t7541 p.6). VCC 1.65-5.5 V, VI 0-5.5 V (sn74lv8t7541 p.4) |

## 74_8T9541

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer/line driver, Schmitt-trigger inputs, 3-state outputs, single-supply level translation
- Report: reports/G05.md; parts added: 6
- PDFs: sn74lv8t9541, sn74lv8t9541-q1
- Pinouts: `p20` (20 pins) for DGS, PW, RKS
- Source: Pinout sn74lv8t9541 p.3 (PW, DGS, RKS same numbering: OE1=1, A1-A8=2-9, GND=10, Y8-Y1=11-18, OE2=19, VCC=20); sn74lv8t9541-q1 p.3 same. Function table p.15 Table 7-1: OE1 L and OE2 L -> Y = A; either H -> Z. Single supply (inputs 5.5 V tolerant at any VCC), so tristate, not translator.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 8.3 | 11.2 | [1.65, 5.5] |  |  |  | max tPLH/tPHL A->Y at 3.3 V, 50 pF, 25 C (sn74lv8t9541 p.6; no typ printed); VCC 1.65-5.5 V, VI to 5.5 V at any VCC (p.4; -Q1 p.4 same) |

## 74_9541

- Kind / model: `buffer` / `tristate`; function "Buffer"; Octal buffer with Schmitt-trigger inputs, 3-state outputs
- Report: reports/G12.md; parts added: 7
- PDFs: sn74ahc9541-q1, sn74ahct9541-q1, sn74lvc9541a-q1
- Pinouts: `p20` (20 pins) for DGS, PW, RKS
- Source: Pinout sn74ahc9541-q1 p.3 Pin Functions (PW, DGS, RKS - RKS thermal pad not a pin); same in sn74ahct9541-q1 p.3 and sn74lvc9541a-q1 p.3. OE1, OE2 active low (p.3). Schmitt-trigger inputs not modeled.

```json
{"units": [{"a": ["A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"], "y": ["Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8"], "oe": [{"pin": "OE1", "active": "low"}, {"pin": "OE2", "active": "low"}]}], "invert": false, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AHC | 6 | 4.7 | [2, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF (sn74ahc9541-q1 p.6); VCC 2-5.5 V, VI to 5.5 V (p.4) |
| AHCT | 5.8 | 7 | [4.5, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 5 V, 50 pF from the -40 to 125 C column, the 25 C column prints only min/max 4.7-6.8 / 5.9-8.7 (sn74ahct9541-q1 p.5); VCC 4.5-5.5 V (p.4) |
| LVC | 5.7 | 5.7 | [1.1, 3.6] | 5.5 |  |  | max tpd A->Y at 3.3 V, 50 pF, 25 C (sn74lvc9541a-q1 p.8, no typ printed); VCC 1.1-3.6 V, VI to 5.5 V (p.5) |

## 1G07

- Kind / model: `gate` / `buf`; function "Buffer"; Single buffer/driver, open-drain output
- Report: reports/G07.md; parts added: 28
- PDFs: sn74auc1g07, sn74aup1g07, sn74lvc1g07, sn74lvc1g07-ep, sn74lvc1g07-q1
- Pinouts: `p5` (5 pins) for DBV, DCK, DRL, DPW; `p6` (6 pins) for DRY, DSF; `dsbga` (5 pins) for YZP; `dsbga4` (4 pins) for YZV
- Source: Pinouts sn74lvc1g07 p.3 Pin Functions (DBV/DCK/DRL, DRY/DSF with NC at 1 and 5, DPW, YZP 5-ball with A1 DNU, YZV 4-ball); sn74aup1g07 p.3 (same, plus YFP not used); sn74auc1g07 p.3 (DBV/DCK, YZP A1 DNU); sn74lvc1g07-q1 p.3 (DBV/DCK, DRY); sn74lvc1g07-ep p.1 (DCK). Open-drain output.

```json
{"gates": [{"op": "buf", "inputs": ["A"], "output": "Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 1.1 | 1.1 | [0.8, 2.7] | 3.6 |  |  | sn74auc1g07 typ tpd at 1.8 V, 15 pF (p.5). VCC 0.8-2.7 V, VI 0-3.6 V (p.4) |
| AUP | 3.7 | 3.7 | [0.8, 3.6] |  |  |  | sn74aup1g07 typ tpd at 3.3 V, 15 pF (p.6). VCC 0.8-3.6 V, VI to 3.6 V (p.4) |
| LVC | 4.2 | 4.2 | [1.65, 5.5] |  |  |  | sn74lvc1g07 max tpd at 3.3 V, -40 to 85 C, no typ printed (p.6). VCC 1.65-5.5 V, VI 0-5.5 V (p.5); -q1 p.4 and -ep p.3 same range |

## 1T34

- Kind / model: `gate` / `buf`; function "Buffer"; Single buffer, reduced-threshold 5.5-V-tolerant input (single-supply level shifter)
- Report: reports/G07.md; parts added: 4
- PDFs: sn74lv1t34, sn74lv1t34-q1
- Pinouts: `p5` (5 pins) for DBV, DCK in sn74lv1t34, sn74lv1t34-q1
- Source: Pinout sn74lv1t34 p.4 (DBV/DCK: 1 NC, 2 A, 3 GND, 4 Y, 5 VCC); sn74lv1t34-q1 p.3 same.

```json
{"gates": [{"op": "buf", "inputs": ["A"], "output": "Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 4.9 | 4.9 | [1.6, 5.5] |  |  |  | sn74lv1t34 typ tpd at 3.3 V, 30 pF, 25 C (p.7). VCC 1.6-5.5 V, VI 0-5.5 V (= VCC max, no inputMax) (p.5); -q1 p.4 same range |

## 2G07

- Kind / model: `gate` / `buf`; function "Buffer"; Dual buffer, open-drain outputs
- Report: reports/G12.md; parts added: 14
- PDFs: sn74auc2g07, sn74aup2g07, sn74lvc2g07, sn74lvc2g07-ep, sn74lvc2g07b-q1
- Pinouts: `p6` (6 pins) for DBV, DCK, DRY, DSF; `dsbga` (6 pins) for YZP, YFP
- Source: Pinout sn74lvc2g07 p.3 Pin Functions (DBV, DCK, DRY, DSF, YZP); sn74aup2g07 p.3 Table 5-1 (DCK, DSF, DRY numbers; YFP balls A1 1A, A2 1Y, B1 GND, B2 VCC, C1 2A, C2 2Y); sn74auc2g07 p.1 (DBV, DCK, YZP); sn74lvc2g07b-q1 p.3 (DCK); sn74lvc2g07-ep p.1 (DCK). Open-drain outputs (sn74lvc2g07 p.3).

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 1.8 | 1 | [0.8, 2.7] | 3.6 |  |  | typ tPLH/tPHL A->Y at 1.8 V, 15 pF (sn74auc2g07 p.4); VCC 0.8-2.7 V, VI to 3.6 V (p.3) |
| AUP | 3.7 | 3.7 | [0.8, 3.6] |  |  |  | typ tpd at 3.3 V, 15 pF (sn74aup2g07 p.7); VCC 0.8-3.6 V (p.5) |
| LVC | 3.7 | 3.7 | [1.65, 5.5] |  |  |  | max tpd at 3.3 V, 50 pF, -40 to 85 C (sn74lvc2g07 p.5); VCC 1.65-5.5 V (p.4); SN74LVC2G07B-Q1 VCC 1.1-5.5 V (sn74lvc2g07b-q1 p.4) |

- Part range: names starting `SN74LVC2G07B` (LVC): vcc [1.1, 5.5]

## 2G34

- Kind / model: `gate` / `buf`; function "Buffer"; Dual buffer
- Report: reports/G12.md; parts added: 11
- PDFs: sn74auc2g34, sn74aup2g34, sn74lvc2g34, sn74lvc2g34-ep
- Pinouts: `p6` (6 pins) for DBV, DCK, DRL, DRY, DSF; `dsbga` (6 pins) for YZP, YFP
- Source: Pinout sn74lvc2g34 p.3 Pin Functions (DBV, DCK, DRL, YZP); sn74auc2g34 p.1 (DBV, DCK, DRL, YZP); sn74aup2g34 p.1 (DCK, DRY, DSF; YFP balls A1 1A, A2 1Y, B1 GND, B2 VCC, C1 2A, C2 2Y); sn74lvc2g34-ep p.1 (DCK).

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 1 | 1 | [0.8, 2.7] | 3.6 |  |  | typ tpd A->Y at 1.8 V, 15 pF (sn74auc2g34 p.4); VCC 0.8-2.7 V, VI to 3.6 V (p.3) |
| AUP | 3.4 | 3.4 | [0.8, 3.6] |  |  |  | typ tpd at 3.3 V, 15 pF (sn74aup2g34 p.5); VCC 0.8-3.6 V (p.3) |
| LVC | 4.1 | 4.1 | [1.65, 5.5] |  |  |  | max tpd at 3.3 V, -40 to 85 C (sn74lvc2g34 p.5); VCC 1.65-5.5 V (p.4) |

## 74_6T17

- Kind / model: `gate` / `buf`; function "Buffer"; Hex Schmitt-trigger buffer with 5.5-V tolerant inputs
- Report: reports/G11.md; parts added: 5
- PDFs: sn74lv6t17, sn74lv6t17-ep, sn74lv6t17-q1
- Pinouts: `p14` (14 pins) for BQA, PW
- Source: p14 from sn74lv6t17 p.3 Figure 5-1/5-2 + Table 5-1 (BQA WQFN and PW TSSOP use the same numbers 1-14; BQA thermal pad is not a pin); sn74lv6t17-q1 p.3 and sn74lv6t17-ep p.3 identical. Y = A per channel.

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}, {"op": "buf", "inputs": ["4A"], "output": "4Y"}, {"op": "buf", "inputs": ["5A"], "output": "5Y"}, {"op": "buf", "inputs": ["6A"], "output": "6Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 8.5 | 8.5 | [1.6, 5.5] |  |  |  | Typical tPD A->Y at 3.3 V, 25 C, 50 pF (sn74lv6t17 p.7). VCC 1.6-5.5 V, VI 0-5.5 V (sn74lv6t17 p.5; -Q1 and -EP p.5 same) |

## 1G17

- Kind / model: `gate` / `buf`; function "Buffer"; Single Schmitt-trigger buffer
- Report: reports/G13.md; parts added: 27
- PDFs: sn74auc1g17, sn74aup1g17, sn74aup1g17-ep, sn74lvc1g17, sn74lvc1g17-ep, sn74lvc1g17-q1
- Pinouts: `p5` (5 pins) for DBV, DCK, DRL, DPW; `p6` (6 pins) for DRY, DSF; `dsbga` (5 pins) for YZP; `dsbga4` (4 pins) for YFP, YZV
- Source: Pinouts: sn74lvc1g17 p.3 Pin Functions (DBV/DCK/DRL/DPW 5-pin; DRY/DSF 6-pin with NC 1,5; YZP 5-ball A1 DNU, B2 unpopulated; YZV 4-ball); sn74lvc1g17-q1 p.3 (DBV/DCK/DRY, same); sn74lvc1g17-ep p.1 (DBV/DCK, same); sn74aup1g17 p.3 (DBV/DCK/DRL/DPW/DRY/DSF/YFP/YZP; YFP 4-ball A1 A, A2 VCC, B1 GND, B2 Y); sn74aup1g17-ep p.1 (DCK, same); sn74auc1g17 p.3 (DBV/DCK/DRL/YZP). Function Y = A, Schmitt-trigger input (sn74lvc1g17 p.1).

```json
{"gates": [{"op": "buf", "inputs": ["A"], "output": "Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 1.1 | 1.1 | [0.8, 2.7] | 3.6 |  |  | typ tpd A->Y at 1.8 V, 15 pF (sn74auc1g17 p.6). VCC 0.8-2.7 V, inputs to 3.6 V (sn74auc1g17 p.4) |
| AUP | 4.1 | 4.1 | [0.8, 3.6] |  |  |  | typ tpd A->Y at 3.3 V, 15 pF, 25 C (sn74aup1g17 p.8). VCC 0.8-3.6 V, VI 0-3.6 V (sn74aup1g17 p.4; sn74aup1g17-ep same) |
| LVC | 5.5 | 5.5 | [1.65, 5.5] |  |  |  | max tpd A->Y at 3.3 V, 30/50 pF, -40 to 85 C; no typical printed (sn74lvc1g17 p.7). VCC 1.65-5.5 V, VI to 5.5 V (sn74lvc1g17 p.5; -q1 and -ep same) |

## 1G34

- Kind / model: `gate` / `buf`; function "Buffer"; Single buffer
- Report: reports/G08.md; parts added: 22
- PDFs: sn74aup1g34, sn74lvc1g34, sn74lvc1g34-q1
- Pinouts: `p5` (5 pins) for DBV, DCK, DRL, DPW; `p6` (6 pins) for DRY, DSF; `dsbga` (5 pins) for YZP; `dsbga4` (4 pins) for YFP, YZV
- Source: Pinout from sn74lvc1g34 p.3 Table 4-1 (DBV/DCK/DRL/DPW 5-pin; DRY/DSF 6-pin with NC 1,5; YZP 5-ball with A1 DNU; YZV/YFP 4-ball); sn74aup1g34 p.3 Pin Functions (same; YZP A1 printed N.C.); sn74lvc1g34-q1 p.3 Table 4-1 (DBV, DCK). Function Y = A.

```json
{"gates": [{"op": "buf", "inputs": ["A"], "output": "Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUP | 2.9 | 2.9 | [0.8, 3.6] |  |  |  | sn74aup1g34 typ tpd A->Y at 3.3 V, 15 pF, 25 C (p.6). VCC 0.8-3.6 V, VI to 3.6 V (p.4) |
| LVC | 4.1 | 4.1 | [1.65, 5.5] |  |  |  | sn74lvc1g34 max tpd A->Y at 3.3 V, 50 pF, -40 to 85 C (no typ printed) (p.7). VCC 1.65-5.5 V, VI to 5.5 V (p.5); sn74lvc1g34-q1 same VCC/VI (p.4) |

## 1G37

- Kind / model: `gate` / `buf`; function "Buffer"; Single buffer, Schmitt-trigger input, open-drain output
- Report: reports/G05.md; parts added: 4
- PDFs: sn74lvc1g37, sn74lvc1g37-q1
- Pinouts: `p5` (5 pins) for DBV, DCK
- Source: Pinout sn74lvc1g37 p.3 Table 4-1 (DBV, DCK: NC=1, A=2, GND=3, Y=4, VCC=5); sn74lvc1g37-q1 p.3 same. Schmitt-trigger input, open-drain output (p.1).

```json
{"gates": [{"op": "buf", "inputs": ["A"], "output": "Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVC | 4 | 4 | [1.65, 5.5] |  |  |  | typ tpd A->Y at 3.3 V, 50 pF (sn74lvc1g37 p.6); VCC 1.65-5.5 V, VI to 5.5 V (sn74lvc1g37 p.4; -Q1 p.4 same range, faster typ 1.8 ns) |

## 1T17

- Kind / model: `gate` / `buf`; function "Buffer"; Single Schmitt-trigger buffer, 1.8/2.5/3.3 V inputs, single-supply up-translator
- Report: reports/G15.md; parts added: 1
- PDFs: sn74aup1t17
- Pinouts: `p5` (5 pins) for DCK
- Source: Pinout sn74aup1t17 p.3 (DCK). Y = A, output referenced to VCC; inputs accept 1.8/2.5/3.3 V levels (p.1).

```json
{"gates": [{"op": "buf", "inputs": ["A"], "output": "Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUP | 2.7 | 2.7 | [2.3, 3.6] |  |  |  | typ tpd at VCC 3.3 V, VI 3.3 V, 15 pF (sn74aup1t17 p.6); VCC 2.3-3.6 V, VI 0-3.6 V (p.4) |

## 1T50

- Kind / model: `gate` / `buf`; function "Buffer"; Single Schmitt-trigger buffer with level shifting
- Report: reports/G16.md; parts added: 1
- PDFs: sn74aup1t50
- Pinouts: `p5` (5 pins) for DCK
- Source: Pinout from sn74aup1t50 p.1 (DCK): 1 NC, 2 A, 3 GND, 4 Y, 5 VCC. Y = A (p.2). Single-supply translator: low-threshold Schmitt inputs, output at VCC.

```json
{"gates": [{"op": "buf", "inputs": ["A"], "output": "Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUP | 2.7 | 2.7 | [2.3, 3.6] |  |  |  | Typical tpd A->Y at VCC 3.3 V, VI 3.3 V, 15 pF, 25 C (sn74aup1t50 p.7). VCC 2.3-3.6 V, VI 0-3.6 V (sn74aup1t50 p.4) |

## 2G17

- Kind / model: `gate` / `buf`; function "Buffer"; Dual Schmitt-trigger buffer
- Report: reports/G14.md; parts added: 11
- PDFs: sn74aup2g17, sn74lvc2g17, sn74lvc2g17-ep
- Pinouts: `dsbga` (6 pins) for YFP, YZP; `p6` (6 pins) for DBV, DCK, DRY, DSF, DRL
- Source: Pinout from sn74lvc2g17 p.3 (DBV/DCK/DRY/DSF/YZP); sn74aup2g17 p.1 (DCK/DRY/DSF/YFP, same order); sn74lvc2g17-ep p.1 (DCK). DSBGA balls: sn74aup2g17 p.1 (YFP: A1 1A, B1 GND, C1 2A, C2 2Y, B2 VCC, A2 1Y), listed in p6 order.

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUP | 4.1 | 4.1 | [0.8, 3.6] |  |  |  | sn74aup2g17 typ tpd at 3.3 V, 15 pF (p.6). VCC 0.8-3.6 V, VI to 3.6 V (p.4) |
| LVC | 5.4 | 5.4 | [1.65, 5.5] |  |  |  | sn74lvc2g17 max tpd at 3.3 V, -40 to 85 C (p.6; no typical given). VCC 1.65-5.5 V, VI to 5.5 V (p.4; sn74lvc2g17-ep p.2 same) |

## 3G07

- Kind / model: `gate` / `buf`; function "Buffer"; Triple buffer/driver, open-drain outputs
- Report: reports/G13.md; parts added: 8
- PDFs: sn74aup3g07, sn74lvc3g07, sn74lvc3g07-ep, sn74lvc3g07-q1
- Pinouts: `p8` (8 pins) for DCT, DCU, DQE; `rse8` (8 pins) for RSE; `dsbga` (8 pins) for YFP
- Source: Pinouts: sn74lvc3g07 p.1 (DCT/DCU); sn74lvc3g07-ep p.1 and sn74lvc3g07-q1 p.1 (DCU, same); sn74aup3g07 p.1 (DCU/DQE same order; RSE top view mirrored: 1Y,3A,2Y,GND,2A,3Y,1A,VCC, same as the existing 3G14_RSE; YFP balls A1..D2). Function table Y = A, open-drain (sn74lvc3g07 p.2).

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUP | 3.7 | 3.7 | [0.8, 3.6] |  |  |  | typ tpd A->Y at 3.3 V, 15 pF, 25 C (sn74aup3g07 p.6). VCC 0.8-3.6 V, VI and VO to 3.6 V (sn74aup3g07 p.4) |
| LVC | 3.7 | 3.7 | [1.65, 5.5] |  |  |  | max tpd A->Y at 3.3 V, -40 to 85 C, no typical printed (sn74lvc3g07 p.3). VCC 1.65-5.5 V, VI and VO to 5.5 V (sn74lvc3g07 p.3; -ep and -q1 same) |

## 3G17

- Kind / model: `gate` / `buf`; function "Buffer"; Triple Schmitt-trigger buffer
- Report: reports/G09.md; parts added: 6
- PDFs: sn74aup3g17, sn74lvc3g17
- Pinouts: `p8` (8 pins) for DCU, DQE, DCT; `rse8` (8 pins) for RSE; `dsbga` (8 pins) for YFP, YZP
- Source: Pinout sn74lvc3g17 p.3 (DCT/DCU, YZP balls 1=A1..4=D1, 5=D2..8=A2) and sn74aup3g17 p.1 (DCU/DQE same order; YFP balls A1..D2 same; RSE UQFN top view: 1=1Y 2=3A 3=2Y 4=GND 5=2A 6=3Y 7=1A 8=VCC, same as existing 3G14_RSE). Schmitt-trigger inputs (hysteresis not modeled).

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUP | 4.1 | 4.1 | [0.8, 3.6] |  |  |  | typ tpd A->Y at 3.3 V, 15 pF, 25 C (sn74aup3g17 p.6); VCC 0.8-3.6 V, VI to 3.6 V (p.4) |
| LVC | 5.4 | 5.4 | [1.65, 5.5] |  |  |  | max tpd A->Y at 3.3 V, 30/50 pF (no typical printed) (sn74lvc3g17 p.6); VCC 1.65-5.5 V, VI 0-5.5 V (p.4) |

## 3G34

- Kind / model: `gate` / `buf`; function "Buffer"; Triple buffer gate
- Report: reports/G10.md; parts added: 7
- PDFs: sn74aup3g34, sn74lvc3g34
- Pinouts: `p8` (8 pins) for DCU, DCT, DQE; `rse8` (8 pins) for RSE; `dsbga8` (8 pins) for YFP, YZP, YEP
- Source: sn74aup3g34 p.1 (DCU/DQE same order; RSE UQFN rotated: 1 1Y, 2 3A, 3 2Y, 4 GND, 5 2A, 6 3Y, 7 1A, 8 VCC; YFP balls lettered A1-D2 = pins 1-8); sn74lvc3g34 p.3 (DCT/DCU, YZP balls given as pins 1-8, same positions as YFP). YEP (older NanoStar bump) is not in the lvc3g34 packaging table; it takes the YZP ball map.

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUP | 3.4 | 3.4 | [0.8, 3.6] |  |  |  | typ tpd at 3.3 V, 25 C, CL 15 pF (sn74aup3g34 p.6); VCC 0.8-3.6 V, VI to 3.6 V (p.4) |
| LVC | 4.1 | 4.1 | [1.65, 5.5] |  |  |  | max tpd at 3.3 V +-0.3 V, -40 to 85 C, 50 pF, no typical printed (sn74lvc3g34 p.5); VCC 1.65-5.5 V, VI to 5.5 V (p.4) |

## 4041UB

- Kind / model: `gate` / `buf`; function "Buffer"; Quad true/complement buffer
- Report: reports/G14.md; parts added: 3
- PDFs: cd4041ub
- Pinouts: `p14` (14 pins) for N, D, PW, J, NS
- Source: Pinout and logic from cd4041ub p.1 (scanned; terminal assignment E=A 1, F=/A 2, A 3, G=B 4, H=/B 5, B 6, VSS 7, K=C 8, L=/C 9, C 10, M=D 11, N=/D 12, D 13, VDD 14). Each input drives a buf (true) and a not (complement) output; 4 units.

```json
{"gates": [{"op": "buf", "inputs": ["A"], "output": "E"}, {"op": "not", "inputs": ["A"], "output": "F"}, {"op": "buf", "inputs": ["B"], "output": "G"}, {"op": "not", "inputs": ["B"], "output": "H"}, {"op": "buf", "inputs": ["C"], "output": "K"}, {"op": "not", "inputs": ["C"], "output": "L"}, {"op": "buf", "inputs": ["D"], "output": "M"}, {"op": "not", "inputs": ["D"], "output": "N"}], "units": 4, "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| C | 60 | 60 | [3, 18] |  |  |  | Typical tPHL/tPLH at VDD 5 V, 25 C, 50 pF (cd4041ub p.2, dynamic characteristics; true and complement outputs share one row). VDD 3-18 V (cd4041ub p.1) |

## 4050B

- Kind / model: `gate` / `buf`; function "Buffer"; Hex non-inverting buffer/converter (high-to-low level shifter, inputs may exceed VCC)
- Report: reports/G03.md; parts added: 5
- PDFs: cd4050b
- Pinouts: `p16` (16 pins) for D, DW, N, NS, PW
- Source: Pinout from cd4050b p.3 Figure 4-2 and p.4 Pin Functions (D, DW, N, NS, PW): inputs A-F, outputs G-L (G = A ... L = F; the table's 'inverting output' wording is copied from CD4049UB), NC on 13 and 16, VCC 1, VSS 8.

```json
{"gates": [{"op": "buf", "inputs": ["A"], "output": "G"}, {"op": "buf", "inputs": ["B"], "output": "H"}, {"op": "buf", "inputs": ["C"], "output": "I"}, {"op": "buf", "inputs": ["D"], "output": "J"}, {"op": "buf", "inputs": ["E"], "output": "K"}, {"op": "buf", "inputs": ["F"], "output": "L"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| C | 70 | 55 | [3, 18] | 18 |  |  | typ tPLH/tPHL CD4050B at VIN = VCC = 5 V, 50 pF, 25 C (cd4050b p.11); VCC 3-18 V (cd4050b p.6); input high level may exceed VCC (p.1), inputs rated 1 uA at 18 V (p.1) -> inputMax 18 |

## 74_07

- Kind / model: `gate` / `buf`; function "Buffer"; Hex buffer/driver, open-collector/open-drain outputs
- Report: reports/G16.md; parts added: 35
- PDFs: sn7407, sn74ac07, sn74ac07-q1, sn74act07, sn74act07-q1, sn74auc07, sn74ls07, sn74lv07a, sn74lvc07a, sn74lvc07a-ep, sn74lvc07a-q1
- Pinouts: `p14` (14 pins) for D, N, NS, DB, DGV, PW, RGY, BQA
- Source: Standard hex pinout, same in every PDF: sn7407 p.3 (D/N/NS), sn74ls07 p.3 (D/DB/N/NS), sn74lv07a p.3, sn74lvc07a p.4 (D/DB/DGV/NS/PW and RGY, one pin table), sn74lvc07a-q1 p.1, sn74lvc07a-ep p.1, sn74ac07 p.3 and sn74ac07-q1 p.3 (PW, BQA/WBQA), sn74act07 p.3 and sn74act07-q1 p.3 (PW, BQA), sn74auc07 p.1 (RGY). Function Y = A. TTL/LS open-collector (30 V outputs), CMOS parts open-drain. BQA/RGY thermal pad is not a pin.

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}, {"op": "buf", "inputs": ["4A"], "output": "4Y"}, {"op": "buf", "inputs": ["5A"], "output": "5Y"}, {"op": "buf", "inputs": ["6A"], "output": "6Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TTL | 6 | 20 | [4.75, 5.25] |  |  |  | Typical tPLH/tPHL A->Y at 5 V and 25 C (15 pF, RL 110 ohm), SN7407 (sn7407 p.5). VCC 4.75-5.25 V (sn7407 p.4). Open-collector, 30 V output |
| LS | 6 | 19 | [4.75, 5.25] |  |  |  | Typical tPLH/tPHL A->Y at 5 V and 25 C (15 pF, RL 110 ohm), SN74LS07 (sn74ls07 p.5). VCC 4.75-5.25 V (sn74ls07 p.4). Open-collector, 30 V output |
| LV | 8.2 | 6.6 | [2, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 3.3 V and 25 C (50 pF load), SN74LV07A (sn74lv07a p.5). VCC 2-5.5 V, VI 0-5.5 V (sn74lv07a p.4) |
| LVC | 3.6 | 3.6 | [1.65, 5.5] |  |  |  | Max tpd A->Y at 3.3 V, -40 to 85 C; SN74LVC07A gives no typical (sn74lvc07a p.6). VCC 1.65-5.5 V, VI 0-5.5 V (sn74lvc07a p.5) |
| AC | 2.6 | 4.7 | [1.5, 6] |  |  |  | Typical at 5 V and 25 C (50 pF); rise = tPLZ, fall = tPZL A->Y (sn74ac07 p.6). VCC 1.5-6 V, VI 0-VCC (sn74ac07 p.4) |
| ACT | 1.9 | 6.1 | [4.5, 5.5] |  |  |  | Typical at 5 V and 25 C (50 pF); rise = tPLZ, fall = tPZL A->Y (sn74act07 p.5). VCC 4.5-5.5 V (sn74act07 p.4) |
| AUC | 1.4 | 1.4 | [0.8, 2.7] | 3.6 |  |  | Typical tpd A->Y at 1.8 V (15 pF), SN74AUC07 (sn74auc07 p.3). VCC 0.8-2.7 V, inputs to 3.6 V (sn74auc07 p.2) |

## 74_1035

- Kind / model: `gate` / `buf`; function "Buffer"; Hex noninverting buffer, open-collector outputs
- Report: reports/G05.md; parts added: 1
- PDFs: sn54als1035
- Pinouts: `p14` (14 pins) for J
- Source: Pinout sn54als1035 p.1 (J/W/D/N); 5962-8874201CA = SNJ54ALS1035J (sn54als1035 packaging table, marking). Function Y = A, open-collector (p.1).

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}, {"op": "buf", "inputs": ["4A"], "output": "4Y"}, {"op": "buf", "inputs": ["5A"], "output": "5Y"}, {"op": "buf", "inputs": ["6A"], "output": "6Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 35 | 14 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->Y, SN54ALS1035, 4.5-5.5 V, 50 pF, RL 680 ohm (sn54als1035 p.3; no typ printed; SN74ALS1035 max 30/12); VCC 4.5-5.5 V SN54/SN74 (p.2). Open-collector, VOH to 5.5 V |

## 74_17

- Kind / model: `gate` / `buf`; function "Buffer"; Hex Schmitt-trigger buffer
- Report: reports/G14.md; parts added: 7
- PDFs: sn74ac17, sn74ac17-q1, sn74act17, sn74act17-q1, sn74auc17
- Pinouts: `p14` (14 pins) for PW, BQA, RGY
- Source: Standard hex pinout: sn74ac17 p.3 (PW TSSOP, BQA WQFN with thermal pad that is not a pin), sn74ac17-q1 p.3, sn74act17 p.3, sn74act17-q1 p.3, sn74auc17 p.1 (RGY VQFN, same numbering). Function Y = A (sn74auc17 p.1). Push-pull; the TTL SN7417 is open-collector and is device 74_17_OD.

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}, {"op": "buf", "inputs": ["4A"], "output": "4Y"}, {"op": "buf", "inputs": ["5A"], "output": "5Y"}, {"op": "buf", "inputs": ["6A"], "output": "6Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AC | 4.6 | 6 | [1.5, 6] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 50 pF, SN74AC17 (sn74ac17 p.6). VCC 1.5-6 V (sn74ac17 p.4; sn74ac17-q1 same) |
| ACT | 4.7 | 6.9 | [4.5, 5.5] |  |  |  | Typical tPLH/tPHL A->Y at 5 V, 50 pF, SN74ACT17 (sn74act17 p.5). VCC 4.5-5.5 V (sn74act17 p.4; sn74act17-q1 same) |
| AUC | 1.2 | 1.2 | [0.8, 2.7] | 3.6 |  |  | Typical tpd A->Y at 1.8 V, 15 pF, SN74AUC17 (sn74auc17 p.3). VCC 0.8-2.7 V, VI to 3.6 V (sn74auc17 p.2) |

## 74_17_OD

- Kind / model: `gate` / `buf`; function "Buffer"; Hex buffer/driver, open-collector high-voltage (15 V) outputs
- Report: reports/G14.md; parts added: 3
- PDFs: sn5417, sn7417
- Pinouts: `p14` (14 pins) for D, N, NS, J, W
- Source: Pinout from sn7417 p.3 (D, N, NS, J, W; same file as sn5417). Open-collector outputs, VO max 15 V for SN5417/SN7417 (sn7417 p.4). Y = A (p.1).

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}, {"op": "buf", "inputs": ["4A"], "output": "4Y"}, {"op": "buf", "inputs": ["5A"], "output": "5Y"}, {"op": "buf", "inputs": ["6A"], "output": "6Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TTL | 6 | 20 | [4.75, 5.25] |  |  |  | Typical at 5 V, 25 C, RL 110 ohm, CL 15 pF (sn7417 p.5). VCC 4.75-5.25 V for SN7417, 4.5-5.5 V for SN5417 (sn7417 p.4) |

- Part range: names starting `SNJ54` (TTL): vcc [4.5, 5.5]

## 74_34

- Kind / model: `gate` / `buf`; function "Buffer"; Hex buffer gate
- Report: reports/G13.md; parts added: 1
- PDFs: sn74auc34
- Pinouts: `p14` (14 pins) for RGY
- Source: Pinout: sn74auc34 p.1 RGY package (top view), standard hex pinout; exposed thermal pad not numbered. Function table Y = A (sn74auc34 p.1). Timing sn74auc34 p.3.

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}, {"op": "buf", "inputs": ["4A"], "output": "4Y"}, {"op": "buf", "inputs": ["5A"], "output": "5Y"}, {"op": "buf", "inputs": ["6A"], "output": "6Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| AUC | 0.9 | 0.9 | [0.8, 2.7] | 3.6 |  |  | typ tpd A->Y at 1.8 V, 15 pF (sn74auc34 p.3). VCC 0.8-2.7 V, inputs to 3.6 V (sn74auc34 p.2) |

## 74_35

- Kind / model: `gate` / `buf`; function "Buffer"; Hex noninverting buffer, open-collector outputs
- Report: reports/G09.md; parts added: 0
- PDFs: sn74als35a
- Pinouts: `p14` (14 pins) for D, N
- Source: Pinout sn74als35a p.1 (D/N top view); function table Y = A p.1; timing p.2. Open-collector outputs (counted as open-drain).

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}, {"op": "buf", "inputs": ["4A"], "output": "4Y"}, {"op": "buf", "inputs": ["5A"], "output": "5Y"}, {"op": "buf", "inputs": ["6A"], "output": "6Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| ALS | 50 | 14 | [4.5, 5.5] |  |  |  | max tPLH/tPHL A->Y (no typical printed), VCC 4.5-5.5 V, CL 50 pF, RL 2 kohm (sn74als35a p.2); VCC 4.5-5.5 V (p.2) |

## 74_4050

- Kind / model: `gate` / `buf`; function "Buffer"; Hex non-inverting buffer, high-to-low level converter (inputs to 15 V)
- Report: reports/G03.md; parts added: 4
- PDFs: cd74hc4050
- Pinouts: `p16` (16 pins) for N, D, NS, PW
- Source: Pinout from cd74hc4050 p.1 (4050 column; PDIP, SOIC, SOP, TSSOP): VCC 1, GND 8, NC 13 and 16. nA to nY non-inverting (p.2 functional diagram).

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}, {"op": "buf", "inputs": ["4A"], "output": "4Y"}, {"op": "buf", "inputs": ["5A"], "output": "5Y"}, {"op": "buf", "inputs": ["6A"], "output": "6Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 6 | 6 | [2, 6] | 15 |  |  | typ tPLH/tPHL at 5 V, 15 pF (cd74hc4050 p.4; max 17 ns at 4.5 V, 50 pF); VCC 2-6 V, DC input voltage 0-15 V (cd74hc4050 p.3) |

## 74_6T07

- Kind / model: `gate` / `buf`; function "Buffer"; Hex buffer, open-drain outputs, LVxT translating inputs
- Report: reports/G09.md; parts added: 4
- PDFs: sn74lv6t07, sn74lv6t07-ep, sn74lv6t07-q1
- Pinouts: `p14` (14 pins) for PW, BQA
- Source: Pinout sn74lv6t07 p.3 (PW and BQA same numbering; BQA thermal pad not a pin), sn74lv6t07-q1 p.3, sn74lv6t07-ep p.3; Y = A open-drain (p.9). Timing p.6 (only tPZL/tPLZ given for the open-drain output).

```json
{"gates": [{"op": "buf", "inputs": ["1A"], "output": "1Y"}, {"op": "buf", "inputs": ["2A"], "output": "2Y"}, {"op": "buf", "inputs": ["3A"], "output": "3Y"}, {"op": "buf", "inputs": ["4A"], "output": "4Y"}, {"op": "buf", "inputs": ["5A"], "output": "5Y"}, {"op": "buf", "inputs": ["6A"], "output": "6Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 7.7 | 7.7 | [1.65, 5.5] |  |  |  | typ tPLZ (rise) / tPZL (fall) A->Y at 3.3 V, 50 pF, 25 C (sn74lv6t07 p.6; -q1 p.6 same); VCC 1.65-5.5 V, VI 0-5.5 V (p.4) |

- Part range: names starting `SN74LV6T07PWREP` (LV): vcc [1.6, 5.5]

## 1G16

- Kind / model: `gate` / `not`; function "NOT"; Single Schmitt-trigger inverter buffer, open-drain output
- Report: reports/G04.md; parts added: 4
- PDFs: sn74lvc1g16, sn74lvc1g16-q1
- Pinouts: `p5` (5 pins) for DBV, DCK
- Source: Pinout sn74lvc1g16 p.3 Table 4-1 (DBV, DCK: 1 NC, 2 A, 3 GND, 4 Y, 5 VCC); sn74lvc1g16-q1 p.3 same. Schmitt-trigger input, open-drain output (sn74lvc1g16 p.1).

```json
{"gates": [{"op": "not", "inputs": ["A"], "output": "Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LVC | 6.5 | 6.5 | [1.65, 5.5] |  |  |  | Max tpd at 3.3 V, 50 pF, -40 to 125 C (no typical printed; 15 pF max 4.6), SN74LVC1G16 (sn74lvc1g16 p.6). VCC 1.65-5.5 V, VI 0-5.5 V (sn74lvc1g16 p.4) |

## 74_16

- Kind / model: `gate` / `not`; function "NOT"; Hex inverter buffer/driver, open-collector/open-drain outputs
- Report: reports/G04.md; parts added: 1
- PDFs: sn5416
- Pinouts: `p14` (14 pins) for J
- Source: DEVICE ALREADY EXISTS in gate.json (74_16, same pinout and TTL timing); only the part SNJ5416J is new - add it there with family TTL and its own vcc 4.5-5.5. Pinout sn5416 p.1 (SN5416 J or W package, same as SN7416 D/N). 15 V open-collector outputs.

```json
{"gates": [{"op": "not", "inputs": ["1A"], "output": "1Y"}, {"op": "not", "inputs": ["2A"], "output": "2Y"}, {"op": "not", "inputs": ["3A"], "output": "3Y"}, {"op": "not", "inputs": ["4A"], "output": "4Y"}, {"op": "not", "inputs": ["5A"], "output": "5Y"}, {"op": "not", "inputs": ["6A"], "output": "6Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| TTL | 10 | 15 | [4.75, 5.25] |  |  |  | Typical at 5 V, 25 C (RL 110 ohm, 15 pF) (sn5416 p.3). VCC 4.75-5.25 V SN74, 4.5-5.5 V SN54 (sn5416 p.3). Open-collector, 15 V output |

- Part range: names starting `SNJ54` (TTL): vcc [4.5, 5.5]

## 74_4049

- Kind / model: `gate` / `not`; function "NOT"; Hex inverting buffer/converter (inputs to 15 V)
- Report: reports/G15.md; parts added: 4
- PDFs: cd74hc4049
- Pinouts: `p16` (16 pins) for N, D, NS, PW, J
- Source: Pinout cd74hc4049 p.1 and p.2 (PDIP/SOIC/SOP/TSSOP, CERDIP): VCC pin 1, GND pin 8, pins 13 and 16 NC. Inputs take up to 15 V for high-to-low level conversion (p.3).

```json
{"gates": [{"op": "not", "inputs": ["1A"], "output": "1Y"}, {"op": "not", "inputs": ["2A"], "output": "2Y"}, {"op": "not", "inputs": ["3A"], "output": "3Y"}, {"op": "not", "inputs": ["4A"], "output": "4Y"}, {"op": "not", "inputs": ["5A"], "output": "5Y"}, {"op": "not", "inputs": ["6A"], "output": "6Y"}], "output": "push-pull"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| HC | 6 | 6 | [2, 6] | 15 |  |  | typ tPLH/tPHL at 5 V, 15 pF (cd74hc4049 p.4; max 17 ns at 4.5 V, 50 pF); VCC 2-6 V, DC input 0-15 V (p.3) |

## 74_6T06

- Kind / model: `gate` / `not`; function "NOT"; Hex inverter buffer/driver, open-drain outputs, level-shifting inputs
- Report: reports/G13.md; parts added: 0
- PDFs: sn74lv6t06-ep
- Pinouts: `p14` (14 pins) for PW
- Source: Pinout: sn74lv6t06-ep p.3 Figure 4-1 / Table 4-1 (PW 14-pin TSSOP), standard hex pinout. Six independent inverters, open-drain outputs, Y = not A (sn74lv6t06-ep p.1). Lower-threshold LVxT inputs, 5.5 V tolerant.

```json
{"gates": [{"op": "not", "inputs": ["1A"], "output": "1Y"}, {"op": "not", "inputs": ["2A"], "output": "2Y"}, {"op": "not", "inputs": ["3A"], "output": "3Y"}, {"op": "not", "inputs": ["4A"], "output": "4Y"}, {"op": "not", "inputs": ["5A"], "output": "5Y"}, {"op": "not", "inputs": ["6A"], "output": "6Y"}], "output": "open-drain"}
```

| Family | rise | fall | vcc | inputMax | vccB | inputMaxB | note |
|---|---|---|---|---|---|---|---|
| LV | 7.8 | 10.2 | [1.6, 5.5] |  |  |  | typ tPLH/tPHL A->Y at 3.3 V, 50 pF, 25 C (sn74lv6t06-ep p.6). VCC 1.6-5.5 V, VI 0-5.5 V (sn74lv6t06-ep p.4) |

