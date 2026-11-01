/*
 * LED 7-segment display data, transcribed from datasheets/led-7/. Included by
 * led7.c. Generated from the datasheet pin tables; each row is
 *   X(pin number, name, kind, digit, segments)
 * kind SEG: segment pin. digit 0 = shared by every digit (multiplexed), else
 *           that digit only. segments = the LEDs this pin drives.
 * kind COM: common anode/cathode of a digit; digit LED7_EXTRAS = the common
 *           of the colon and dot LEDs (L1-L3).
 * kind NC:  no connection (a lead that connects to nothing).
 * kind NP:  no pin at that position. Listed so the numbering is complete,
 *           in the hidden pin group, so the schematic does not draw it.
 */

/* LTC-4727JR: C4727JR.pdf */
#define LTC_4727JR_PINS(X) \
    X(1, "CC1", COM, 1, 0) \
    X(2, "CC2", COM, 2, 0) \
    X(3, "D", SEG, 0, S(D)) \
    X(4, "CC L", COM, LED7_EXTRAS, 0) \
    X(5, "E", SEG, 0, S(E)) \
    X(6, "CC3", COM, 3, 0) \
    X(7, "DP", SEG, 0, S(DP)) \
    X(8, "CC4", COM, 4, 0) \
    X(9, "NC", NC, 0, 0) \
    X(10, "NP", NP, 0, 0) \
    X(11, "F", SEG, 0, S(F)) \
    X(12, "NP", NP, 0, 0) \
    X(13, "C/L3", SEG, 0, S(C) | S(L3)) \
    X(14, "A/L1", SEG, 0, S(A) | S(L1)) \
    X(15, "G", SEG, 0, S(G)) \
    X(16, "B/L2", SEG, 0, S(B) | S(L2))
LED7_PART(LTC_4727JR, "LTC-4727JR", "4-digit 0.4 in AlInGaP Super Red 7-segment display, common cathode, colon", 4, "0.4 in", "Red", "AlInGaP Super Red", 0, 2, 3, 0)

/* LTC-4627JR: LTC-4627JR.pdf */
#define LTC_4627JR_PINS(X) \
    X(1, "CA1", COM, 1, 0) \
    X(2, "CA2", COM, 2, 0) \
    X(3, "D", SEG, 0, S(D)) \
    X(4, "CA L", COM, LED7_EXTRAS, 0) \
    X(5, "E", SEG, 0, S(E)) \
    X(6, "CA3", COM, 3, 0) \
    X(7, "DP", SEG, 0, S(DP)) \
    X(8, "CA4", COM, 4, 0) \
    X(9, "NC", NC, 0, 0) \
    X(10, "NP", NP, 0, 0) \
    X(11, "F", SEG, 0, S(F)) \
    X(12, "NP", NP, 0, 0) \
    X(13, "C/L3", SEG, 0, S(C) | S(L3)) \
    X(14, "A/L1", SEG, 0, S(A) | S(L1)) \
    X(15, "G", SEG, 0, S(G)) \
    X(16, "B/L2", SEG, 0, S(B) | S(L2))
LED7_PART(LTC_4627JR, "LTC-4627JR", "4-digit 0.4 in AlInGaP Super Red 7-segment display, common anode, colon", 4, "0.4 in", "Red", "AlInGaP Super Red", 1, 2, 3, 0)

/* LTC-5723HR: C5723HR.pdf */
#define LTC_5723HR_PINS(X) \
    X(1, "E", SEG, 0, S(E)) \
    X(2, "D", SEG, 0, S(D)) \
    X(3, "DP", SEG, 0, S(DP)) \
    X(4, "C", SEG, 0, S(C)) \
    X(5, "G", SEG, 0, S(G)) \
    X(6, "CC4", COM, 4, 0) \
    X(7, "B", SEG, 0, S(B)) \
    X(8, "CC3", COM, 3, 0) \
    X(9, "CC2", COM, 2, 0) \
    X(10, "F", SEG, 0, S(F)) \
    X(11, "A", SEG, 0, S(A)) \
    X(12, "CC1", COM, 1, 0)
LED7_PART(LTC_5723HR, "LTC-5723HR", "4-digit 0.56 in Hi-Eff Red 7-segment display, common cathode", 4, "0.56 in", "Red", "Hi-Eff Red", 0, 0, 0, 0)

/* LTC-5623G: LTC-5623G.pdf */
#define LTC_5623G_PINS(X) \
    X(1, "E", SEG, 0, S(E)) \
    X(2, "D", SEG, 0, S(D)) \
    X(3, "DP", SEG, 0, S(DP)) \
    X(4, "C", SEG, 0, S(C)) \
    X(5, "G", SEG, 0, S(G)) \
    X(6, "CA4", COM, 4, 0) \
    X(7, "B", SEG, 0, S(B)) \
    X(8, "CA3", COM, 3, 0) \
    X(9, "CA2", COM, 2, 0) \
    X(10, "F", SEG, 0, S(F)) \
    X(11, "A", SEG, 0, S(A)) \
    X(12, "CA1", COM, 1, 0)
LED7_PART(LTC_5623G, "LTC-5623G", "4-digit 0.56 in Green 7-segment display, common anode", 4, "0.56 in", "Green", "Green", 1, 0, 0, 0)

/* LTC-2623G: LTC-2623G.pdf */
#define LTC_2623G_PINS(X) \
    X(1, "CA1", COM, 1, 0) \
    X(2, "C/L3", SEG, 0, S(C) | S(L3)) \
    X(3, "DP", SEG, 0, S(DP)) \
    X(4, "NC", NC, 0, 0) \
    X(5, "E", SEG, 0, S(E)) \
    X(6, "D", SEG, 0, S(D)) \
    X(7, "G", SEG, 0, S(G)) \
    X(8, "CA4", COM, 4, 0) \
    X(9, "NC", NC, 0, 0) \
    X(10, "NP", NP, 0, 0) \
    X(11, "CA3", COM, 3, 0) \
    X(12, "CA L", COM, LED7_EXTRAS, 0) \
    X(13, "A/L1", SEG, 0, S(A) | S(L1)) \
    X(14, "CA2", COM, 2, 0) \
    X(15, "B/L2", SEG, 0, S(B) | S(L2)) \
    X(16, "F", SEG, 0, S(F))
LED7_PART(LTC_2623G, "LTC-2623G", "4-digit 0.28 in Green 7-segment display, common anode, colon", 4, "0.28 in", "Green", "Green", 1, 2, 3, 0)

/* LTC-4624JR: LTC-4624JR.pdf */
#define LTC_4624JR_PINS(X) \
    X(1, "CA1", COM, 1, 0) \
    X(2, "E", SEG, 0, S(E)) \
    X(3, "C/L3", SEG, 0, S(C) | S(L3)) \
    X(4, "D", SEG, 0, S(D)) \
    X(5, "CA2", COM, 2, 0) \
    X(6, "DP", SEG, 0, S(DP)) \
    X(7, "CA3", COM, 3, 0) \
    X(8, "G", SEG, 0, S(G)) \
    X(9, "NP", NP, 0, 0) \
    X(10, "NP", NP, 0, 0) \
    X(11, "B/L2", SEG, 0, S(B) | S(L2)) \
    X(12, "A/L1", SEG, 0, S(A) | S(L1)) \
    X(13, "NP", NP, 0, 0) \
    X(14, "CA L", COM, LED7_EXTRAS, 0) \
    X(15, "F", SEG, 0, S(F))
LED7_PART(LTC_4624JR, "LTC-4624JR", "3-digit 0.4 in AlInGaP Super Red 7-segment display, common anode, colon", 3, "0.4 in", "Red", "AlInGaP Super Red", 1, 1, 2, 0)

/* LTC-4724JR: LTC-4724JR.pdf */
#define LTC_4724JR_PINS(X) \
    X(1, "CC1", COM, 1, 0) \
    X(2, "E", SEG, 0, S(E)) \
    X(3, "C/L3", SEG, 0, S(C) | S(L3)) \
    X(4, "D", SEG, 0, S(D)) \
    X(5, "CC2", COM, 2, 0) \
    X(6, "DP", SEG, 0, S(DP)) \
    X(7, "CC3", COM, 3, 0) \
    X(8, "G", SEG, 0, S(G)) \
    X(9, "NP", NP, 0, 0) \
    X(10, "NP", NP, 0, 0) \
    X(11, "B/L2", SEG, 0, S(B) | S(L2)) \
    X(12, "A/L1", SEG, 0, S(A) | S(L1)) \
    X(13, "NP", NP, 0, 0) \
    X(14, "CC L", COM, LED7_EXTRAS, 0) \
    X(15, "F", SEG, 0, S(F))
LED7_PART(LTC_4724JR, "LTC-4724JR", "3-digit 0.4 in AlInGaP Super Red 7-segment display, common cathode, colon", 3, "0.4 in", "Red", "AlInGaP Super Red", 0, 1, 2, 0)

/* LTD-4608JF: D4608JF.pdf */
#define LTD_4608JF_PINS(X) \
    X(1, "C", SEG, 0, S(C)) \
    X(2, "DP", SEG, 0, S(DP)) \
    X(3, "E", SEG, 0, S(E)) \
    X(4, "CA2", COM, 2, 0) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "F", SEG, 0, S(F)) \
    X(7, "G", SEG, 0, S(G)) \
    X(8, "B", SEG, 0, S(B)) \
    X(9, "CA1", COM, 1, 0) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LTD_4608JF, "LTD-4608JF", "2-digit 0.4 in AlInGaP Yellow Orange 7-segment display, common anode", 2, "0.4 in", "Amber", "AlInGaP Yellow Orange", 1, 0, 0, 0)

/* LTD-4608JR: LTD-4608JR.pdf */
#define LTD_4608JR_PINS(X) \
    X(1, "C", SEG, 0, S(C)) \
    X(2, "DP", SEG, 0, S(DP)) \
    X(3, "E", SEG, 0, S(E)) \
    X(4, "CA2", COM, 2, 0) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "F", SEG, 0, S(F)) \
    X(7, "G", SEG, 0, S(G)) \
    X(8, "B", SEG, 0, S(B)) \
    X(9, "CA1", COM, 1, 0) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LTD_4608JR, "LTD-4608JR", "2-digit 0.4 in AlInGaP Super Red 7-segment display, common anode", 2, "0.4 in", "Red", "AlInGaP Super Red", 1, 0, 0, 0)

/* LTD-4708JR: LTD-4708JR.pdf */
#define LTD_4708JR_PINS(X) \
    X(1, "C", SEG, 0, S(C)) \
    X(2, "DP", SEG, 0, S(DP)) \
    X(3, "E", SEG, 0, S(E)) \
    X(4, "CC2", COM, 2, 0) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "F", SEG, 0, S(F)) \
    X(7, "G", SEG, 0, S(G)) \
    X(8, "B", SEG, 0, S(B)) \
    X(9, "CC1", COM, 1, 0) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LTD_4708JR, "LTD-4708JR", "2-digit 0.4 in AlInGaP Super Red 7-segment display, common cathode", 2, "0.4 in", "Red", "AlInGaP Super Red", 0, 0, 0, 0)

/* LTD-2601WC: LTD-2601WC.pdf */
#define LTD_2601WC_PINS(X) \
    X(1, "E", SEG, 0, S(E)) \
    X(2, "D", SEG, 0, S(D)) \
    X(3, "C", SEG, 0, S(C)) \
    X(4, "G", SEG, 0, S(G)) \
    X(5, "DP", SEG, 0, S(DP)) \
    X(6, "CA2", COM, 2, 0) \
    X(7, "A", SEG, 0, S(A)) \
    X(8, "B", SEG, 0, S(B)) \
    X(9, "CA1", COM, 1, 0) \
    X(10, "F", SEG, 0, S(F))
LED7_PART(LTD_2601WC, "LTD-2601WC", "2-digit 0.28 in AlGaAs Red 7-segment display, common anode", 2, "0.28 in", "Red", "AlGaAs Red", 1, 0, 0, 0)

/* LTD-2701JD: LTD-2701JD.pdf */
#define LTD_2701JD_PINS(X) \
    X(1, "E", SEG, 0, S(E)) \
    X(2, "D", SEG, 0, S(D)) \
    X(3, "C", SEG, 0, S(C)) \
    X(4, "G", SEG, 0, S(G)) \
    X(5, "DP", SEG, 0, S(DP)) \
    X(6, "CC2", COM, 2, 0) \
    X(7, "A", SEG, 0, S(A)) \
    X(8, "B", SEG, 0, S(B)) \
    X(9, "CC1", COM, 1, 0) \
    X(10, "F", SEG, 0, S(F))
LED7_PART(LTD_2701JD, "LTD-2701JD", "2-digit 0.28 in AlInGaP Hyper Red 7-segment display, common cathode", 2, "0.28 in", "Red", "AlInGaP Hyper Red", 0, 0, 0, 0)

/* LSHD-5503: LSHD-5503.pdf */
#define LSHD_5503_PINS(X) \
    X(1, "E", SEG, 0, S(E)) \
    X(2, "D", SEG, 0, S(D)) \
    X(3, "CC", COM, 1, 0) \
    X(4, "C", SEG, 0, S(C)) \
    X(5, "DP", SEG, 0, S(DP)) \
    X(6, "B", SEG, 0, S(B)) \
    X(7, "A", SEG, 0, S(A)) \
    X(8, "CC", COM, 1, 0) \
    X(9, "F", SEG, 0, S(F)) \
    X(10, "G", SEG, 0, S(G))
LED7_PART(LSHD_5503, "LSHD-5503", "1-digit 0.56 in AlInGaP Red 7-segment display, common cathode", 1, "0.56 in", "Red", "AlInGaP Red", 0, 0, 0, 0)

/* LSHD-5601: LSHD-5601.pdf */
#define LSHD_5601_PINS(X) \
    X(1, "E", SEG, 0, S(E)) \
    X(2, "D", SEG, 0, S(D)) \
    X(3, "CA", COM, 1, 0) \
    X(4, "C", SEG, 0, S(C)) \
    X(5, "DP", SEG, 0, S(DP)) \
    X(6, "B", SEG, 0, S(B)) \
    X(7, "A", SEG, 0, S(A)) \
    X(8, "CA", COM, 1, 0) \
    X(9, "F", SEG, 0, S(F)) \
    X(10, "G", SEG, 0, S(G))
LED7_PART(LSHD_5601, "LSHD-5601", "1-digit 0.56 in Green 7-segment display, common anode", 1, "0.56 in", "Green", "Green", 1, 0, 0, 0)

/* LTS-6960HR: LTS-6960HR.pdf */
#define LTS_6960HR_PINS(X) \
    X(1, "E", SEG, 0, S(E)) \
    X(2, "D", SEG, 0, S(D)) \
    X(3, "CA", COM, 1, 0) \
    X(4, "C", SEG, 0, S(C)) \
    X(5, "DP", SEG, 0, S(DP)) \
    X(6, "B", SEG, 0, S(B)) \
    X(7, "A", SEG, 0, S(A)) \
    X(8, "CA", COM, 1, 0) \
    X(9, "F", SEG, 0, S(F)) \
    X(10, "G", SEG, 0, S(G))
LED7_PART(LTS_6960HR, "LTS-6960HR", "1-digit 0.56 in Hi-Eff Red 7-segment display, common anode", 1, "0.56 in", "Red", "Hi-Eff Red", 1, 0, 0, 0)

/* LSHD-7501: LSHD-7501.pdf */
#define LSHD_7501_PINS(X) \
    X(1, "CA", COM, 1, 0) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "G", SEG, 0, S(G)) \
    X(4, "E", SEG, 0, S(E)) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "CA", COM, 1, 0) \
    X(7, "DP", SEG, 0, S(DP)) \
    X(8, "C", SEG, 0, S(C)) \
    X(9, "B", SEG, 0, S(B)) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LSHD_7501, "LSHD-7501", "1-digit 0.3 in AlInGaP Red 7-segment display, common anode", 1, "0.3 in", "Red", "AlInGaP Red", 1, 0, 0, 0)

/* LSHD-7503: LSHD-7503.PDF */
#define LSHD_7503_PINS(X) \
    X(1, "CC", COM, 1, 0) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "G", SEG, 0, S(G)) \
    X(4, "E", SEG, 0, S(E)) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "CC", COM, 1, 0) \
    X(7, "DP", SEG, 0, S(DP)) \
    X(8, "C", SEG, 0, S(C)) \
    X(9, "B", SEG, 0, S(B)) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LSHD_7503, "LSHD-7503", "1-digit 0.3 in AlInGaP Red 7-segment display, common cathode", 1, "0.3 in", "Red", "AlInGaP Red", 0, 0, 0, 0)

/* LSHD-7801: LSHD-7801.pdf */
#define LSHD_7801_PINS(X) \
    X(1, "CA", COM, 1, 0) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "G", SEG, 0, S(G)) \
    X(4, "E", SEG, 0, S(E)) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "CA", COM, 1, 0) \
    X(7, "DP", SEG, 0, S(DP)) \
    X(8, "C", SEG, 0, S(C)) \
    X(9, "B", SEG, 0, S(B)) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LSHD_7801, "LSHD-7801", "1-digit 0.3 in Green 7-segment display, common anode", 1, "0.3 in", "Green", "Green", 1, 0, 0, 0)

/* LSHD-7803: LSHD-7803.pdf */
#define LSHD_7803_PINS(X) \
    X(1, "CC", COM, 1, 0) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "G", SEG, 0, S(G)) \
    X(4, "E", SEG, 0, S(E)) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "CC", COM, 1, 0) \
    X(7, "DP", SEG, 0, S(DP)) \
    X(8, "C", SEG, 0, S(C)) \
    X(9, "B", SEG, 0, S(B)) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LSHD_7803, "LSHD-7803", "1-digit 0.3 in Green 7-segment display, common cathode", 1, "0.3 in", "Green", "Green", 0, 0, 0, 0)

/* LSHD-A101: LSHD-A101.pdf */
#define LSHD_A101_PINS(X) \
    X(1, "CA", COM, 1, 0) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "G", SEG, 0, S(G)) \
    X(4, "E", SEG, 0, S(E)) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "CA", COM, 1, 0) \
    X(7, "DP", SEG, 0, S(DP)) \
    X(8, "C", SEG, 0, S(C)) \
    X(9, "B", SEG, 0, S(B)) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LSHD_A101, "LSHD-A101", "1-digit 0.3 in AlInGaP Red 7-segment display, common anode", 1, "0.3 in", "Red", "AlInGaP Red", 1, 0, 0, 0)

/* LSHD-A103: LSHD-A103.PDF; its circuit drawing shows common anode; the text and pin table say common cathode */
#define LSHD_A103_PINS(X) \
    X(1, "CC", COM, 1, 0) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "G", SEG, 0, S(G)) \
    X(4, "E", SEG, 0, S(E)) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "CC", COM, 1, 0) \
    X(7, "DP", SEG, 0, S(DP)) \
    X(8, "C", SEG, 0, S(C)) \
    X(9, "B", SEG, 0, S(B)) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LSHD_A103, "LSHD-A103", "1-digit 0.3 in AlInGaP Red 7-segment display, common cathode", 1, "0.3 in", "Red", "AlInGaP Red", 0, 0, 0, 0)

/* LTS-4802BJR-H1: LTS-4802BJR-H1.pdf; 0.39 in */
#define LTS_4802BJR_H1_PINS(X) \
    X(1, "CA", COM, 1, 0) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "G", SEG, 0, S(G)) \
    X(4, "E", SEG, 0, S(E)) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "CA", COM, 1, 0) \
    X(7, "DP", SEG, 0, S(DP)) \
    X(8, "C", SEG, 0, S(C)) \
    X(9, "B", SEG, 0, S(B)) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LTS_4802BJR_H1, "LTS-4802BJR-H1", "1-digit 0.4 in AlInGaP Super Red 7-segment display, common anode", 1, "0.4 in", "Red", "AlInGaP Super Red", 1, 0, 0, 0)

/* LTS-4301JR: LTS-4301JR.pdf */
#define LTS_4301JR_PINS(X) \
    X(1, "G", SEG, 0, S(G)) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "CC", COM, 1, 0) \
    X(4, "E", SEG, 0, S(E)) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "DP", SEG, 0, S(DP)) \
    X(7, "C", SEG, 0, S(C)) \
    X(8, "CC", COM, 1, 0) \
    X(9, "B", SEG, 0, S(B)) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LTS_4301JR, "LTS-4301JR", "1-digit 0.4 in AlInGaP Super Red 7-segment display, common cathode", 1, "0.4 in", "Red", "AlInGaP Super Red", 0, 0, 0, 0)

/* LTS-4801JR: LTS-4801JR.pdf; 0.39 in */
#define LTS_4801JR_PINS(X) \
    X(1, "G", SEG, 0, S(G)) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "CA", COM, 1, 0) \
    X(4, "E", SEG, 0, S(E)) \
    X(5, "D", SEG, 0, S(D)) \
    X(6, "DP", SEG, 0, S(DP)) \
    X(7, "C", SEG, 0, S(C)) \
    X(8, "CA", COM, 1, 0) \
    X(9, "B", SEG, 0, S(B)) \
    X(10, "A", SEG, 0, S(A))
LED7_PART(LTS_4801JR, "LTS-4801JR", "1-digit 0.4 in AlInGaP Super Red 7-segment display, common anode", 1, "0.4 in", "Red", "AlInGaP Super Red", 1, 0, 0, 0)

/* LTS-312AHR: LTS-312AHR.pdf */
#define LTS_312AHR_PINS(X) \
    X(1, "A", SEG, 0, S(A)) \
    X(2, "F", SEG, 0, S(F)) \
    X(3, "CA", COM, 1, 0) \
    X(4, "NP", NP, 0, 0) \
    X(5, "NP", NP, 0, 0) \
    X(6, "LDP", SEG, 0, S(LDP)) \
    X(7, "E", SEG, 0, S(E)) \
    X(8, "D", SEG, 0, S(D)) \
    X(9, "DP", SEG, 0, S(DP)) \
    X(10, "C", SEG, 0, S(C)) \
    X(11, "G", SEG, 0, S(G)) \
    X(12, "NP", NP, 0, 0) \
    X(13, "B", SEG, 0, S(B)) \
    X(14, "CA", COM, 1, 0)
LED7_PART(LTS_312AHR, "LTS-312AHR", "1-digit 0.3 in Hi-Eff Red 7-segment display, common anode", 1, "0.3 in", "Red", "Hi-Eff Red", 1, 0, 0, 0)

/* LTD-6910HR: LTD-6910HR.pdf */
#define LTD_6910HR_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6910HR, "LTD-6910HR", "2-digit 0.56 in Hi-Eff Red 7-segment display, common anode", 2, "0.56 in", "Red", "Hi-Eff Red", 1, 0, 0, 0)

/* LTD-6710R: LTD-6000.pdf */
#define LTD_6710R_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6710R, "LTD-6710R", "2-digit 0.56 in Red 7-segment display, common anode", 2, "0.56 in", "Red", "Red", 1, 0, 0, PART_FLAG_OBSOLETE)

/* LTD-6710P: LTD-6000.pdf */
#define LTD_6710P_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6710P, "LTD-6710P", "2-digit 0.56 in Bright Red 7-segment display, common anode", 2, "0.56 in", "Red", "Bright Red", 1, 0, 0, PART_FLAG_OBSOLETE)

/* LTD-6410G: LTD-6000.pdf */
#define LTD_6410G_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6410G, "LTD-6410G", "2-digit 0.56 in Green 7-segment display, common anode", 2, "0.56 in", "Green", "Green", 1, 0, 0, 0)

/* LTD-6810Y: LTD-6000.pdf */
#define LTD_6810Y_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6810Y, "LTD-6810Y", "2-digit 0.56 in Yellow 7-segment display, common anode", 2, "0.56 in", "Yellow", "Yellow", 1, 0, 0, 0)

/* LTD-6610E: LTD-6000.pdf */
#define LTD_6610E_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6610E, "LTD-6610E", "2-digit 0.56 in Orange 7-segment display, common anode", 2, "0.56 in", "Orange", "Orange", 1, 0, 0, 0)

/* LTD-6730R: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6730R_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6730R, "LTD-6730R", "2-digit 0.56 in Red 7-segment display, common anode, +/-1 overflow", 2, "0.56 in", "Red", "Red", 1, 0, 0, 0)

/* LTD-6730P: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6730P_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6730P, "LTD-6730P", "2-digit 0.56 in Bright Red 7-segment display, common anode, +/-1 overflow", 2, "0.56 in", "Red", "Bright Red", 1, 0, 0, 0)

/* LTD-6430G: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6430G_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6430G, "LTD-6430G", "2-digit 0.56 in Green 7-segment display, common anode, +/-1 overflow", 2, "0.56 in", "Green", "Green", 1, 0, 0, 0)

/* LTD-6830Y: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6830Y_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6830Y, "LTD-6830Y", "2-digit 0.56 in Yellow 7-segment display, common anode, +/-1 overflow", 2, "0.56 in", "Yellow", "Yellow", 1, 0, 0, 0)

/* LTD-6630E: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6630E_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6630E, "LTD-6630E", "2-digit 0.56 in Orange 7-segment display, common anode, +/-1 overflow", 2, "0.56 in", "Orange", "Orange", 1, 0, 0, 0)

/* LTD-6930HR: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6930HR_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CA2", COM, 2, 0) \
    X(14, "CA1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6930HR, "LTD-6930HR", "2-digit 0.56 in Hi-Eff Red 7-segment display, common anode, +/-1 overflow", 2, "0.56 in", "Red", "Hi-Eff Red", 1, 0, 0, 0)

/* LTD-6740R: LTD-6000.pdf */
#define LTD_6740R_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6740R, "LTD-6740R", "2-digit 0.56 in Red 7-segment display, common cathode", 2, "0.56 in", "Red", "Red", 0, 0, 0, 0)

/* LTD-6740P: LTD-6000.pdf */
#define LTD_6740P_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6740P, "LTD-6740P", "2-digit 0.56 in Bright Red 7-segment display, common cathode", 2, "0.56 in", "Red", "Bright Red", 0, 0, 0, 0)

/* LTD-6440G: LTD-6000.pdf */
#define LTD_6440G_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6440G, "LTD-6440G", "2-digit 0.56 in Green 7-segment display, common cathode", 2, "0.56 in", "Green", "Green", 0, 0, 0, 0)

/* LTD-6840Y: LTD-6000.pdf */
#define LTD_6840Y_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6840Y, "LTD-6840Y", "2-digit 0.56 in Yellow 7-segment display, common cathode", 2, "0.56 in", "Yellow", "Yellow", 0, 0, 0, 0)

/* LTD-6640E: LTD-6000.pdf */
#define LTD_6640E_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6640E, "LTD-6640E", "2-digit 0.56 in Orange 7-segment display, common cathode", 2, "0.56 in", "Orange", "Orange", 0, 0, 0, 0)

/* LTD-6940HR: LTD-6000.pdf */
#define LTD_6940HR_PINS(X) \
    X(1, "E1", SEG, 1, S(E)) \
    X(2, "D1", SEG, 1, S(D)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "A1", SEG, 1, S(A)) \
    X(17, "G1", SEG, 1, S(G)) \
    X(18, "F1", SEG, 1, S(F))
LED7_PART(LTD_6940HR, "LTD-6940HR", "2-digit 0.56 in Hi-Eff Red 7-segment display, common cathode", 2, "0.56 in", "Red", "Hi-Eff Red", 0, 0, 0, 0)

/* LTD-6750R: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6750R_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6750R, "LTD-6750R", "2-digit 0.56 in Red 7-segment display, common cathode, +/-1 overflow", 2, "0.56 in", "Red", "Red", 0, 0, 0, 0)

/* LTD-6750P: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6750P_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6750P, "LTD-6750P", "2-digit 0.56 in Bright Red 7-segment display, common cathode, +/-1 overflow", 2, "0.56 in", "Red", "Bright Red", 0, 0, 0, 0)

/* LTD-6450G: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6450G_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6450G, "LTD-6450G", "2-digit 0.56 in Green 7-segment display, common cathode, +/-1 overflow", 2, "0.56 in", "Green", "Green", 0, 0, 0, 0)

/* LTD-6850Y: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6850Y_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6850Y, "LTD-6850Y", "2-digit 0.56 in Yellow 7-segment display, common cathode, +/-1 overflow", 2, "0.56 in", "Yellow", "Yellow", 0, 0, 0, 0)

/* LTD-6650E: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6650E_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6650E, "LTD-6650E", "2-digit 0.56 in Orange 7-segment display, common cathode, +/-1 overflow", 2, "0.56 in", "Orange", "Orange", 0, 0, 0, 0)

/* LTD-6950HR: LTD-6000.pdf; plus/minus 1 overflow */
#define LTD_6950HR_PINS(X) \
    X(1, "G1", SEG, 1, S(G)) \
    X(2, "HJ1", SEG, 1, S(HJ)) \
    X(3, "C1", SEG, 1, S(C)) \
    X(4, "DP1", SEG, 1, S(DP)) \
    X(5, "E2", SEG, 2, S(E)) \
    X(6, "D2", SEG, 2, S(D)) \
    X(7, "G2", SEG, 2, S(G)) \
    X(8, "C2", SEG, 2, S(C)) \
    X(9, "DP2", SEG, 2, S(DP)) \
    X(10, "B2", SEG, 2, S(B)) \
    X(11, "A2", SEG, 2, S(A)) \
    X(12, "F2", SEG, 2, S(F)) \
    X(13, "CC2", COM, 2, 0) \
    X(14, "CC1", COM, 1, 0) \
    X(15, "B1", SEG, 1, S(B)) \
    X(16, "NC", NC, 0, 0) \
    X(17, "NC", NC, 0, 0) \
    X(18, "NC", NC, 0, 0)
LED7_PART(LTD_6950HR, "LTD-6950HR", "2-digit 0.56 in Hi-Eff Red 7-segment display, common cathode, +/-1 overflow", 2, "0.56 in", "Red", "Hi-Eff Red", 0, 0, 0, 0)
