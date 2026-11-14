"""plan-36-B Phase 3: the by-eye readings, written as TSV.

usage: python3 -I eye-checks.py <full-checks-auto.tsv>
Writes full-checks.tsv (full-check.py's output with the three figures the
word boxes cannot read replaced by the by-eye result) and eye-checks.tsv
(name checks read on the figure where the text check cannot read it:
image figures, or a figure that prints a pad's function instead of its
name). Each line says what was compared.
"""
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..")
M = "datasheets/mcu/microchip/"
EYE_FULL = {
    ("ATmega1284P", "p40"): (M + "ATmega164A_PA-324A_PA-644A_PA-1284_P_Data-Sheet-40002070B.pdf", "11",
                             "full ok (by eye, Figure 1-1 PDIP, every pin)"),
    ("ATmega1284P", "quad44"): (M + "ATmega164A_PA-324A_PA-644A_PA-1284_P_Data-Sheet-40002070B.pdf", "11",
                                "full ok (by eye, Figure 1-1 TQFP/QFN/MLF, every pin; pin 29 printed AREF for the ATDF REF)"),
    ("ATtiny1614", "p14"): (M + "ATtiny1614-16-17-DataSheet-DS40002204A.pdf", "14",
                            "full ok (by eye, 4.1 14-Pin SOIC, every pin)"),
}
SD = M + "AVR32SD20-28-32-Data-Sheet-DS40002629.pdf"
M8 = M + "Atmel-2486-8-bit-AVR-microcontroller-ATmega8_L_summary.pdf"
C3 = M + "Atmel-8492-8-and-16-bit-AVR-microcontroller-ATxmega32C3_64C3_128C3_192C3_256C3_datasheet.pdf"
EYE_NAME = [
    ("AVR32SD20", "p20", SD, "18", "ok (by eye: every pin matches; the figure prints the functions RESET / UPDI for the ATDF pads PF6 / PF7)"),
    ("AVR32SD28", "quad28", SD, "19", "ok (by eye: every pin matches; RESET / UPDI printed for PF6 / PF7)"),
    ("AVR32SD28", "p28", SD, "20", "ok (by eye: every pin matches; RESET / UPDI printed for PF6 / PF7)"),
    ("AVR32SD32", "quad32", SD, "21", "ok (by eye: every pin matches; RESET / UPDI printed for PF6 / PF7)"),
    ("ATmega8", "p28", M8, "2", "ok (by eye: PDIP figure, every pin; the page is an image)"),
    ("ATmega8", "quad32", M8, "2", "ok (by eye: TQFP and MLF figures, every pin; the page is an image)"),
    ("ATxmega64C3", "quad64", C3, "4", "ok (by eye: Figure 2-1 Block Diagram and Pinout, every pin; an image)"),
    ("ATxmega128C3", "quad64", C3, "4", "ok (by eye: Figure 2-1, every pin; an image)"),
    ("ATxmega256C3", "quad64", C3, "4", "ok (by eye: Figure 2-1, every pin; an image)"),
]


def main(auto):
    with open(os.path.join(OUT, "full-checks.tsv"), "w") as f:
        f.write("device\tpinout\tpdf\tpages\tresult\n")
        for r in csv.DictReader(open(auto), delimiter="\t"):
            k = (r["device"], r["pinout"])
            if k in EYE_FULL:
                r["pdf"], r["pages"], r["result"] = EYE_FULL[k]
            f.write("\t".join([r["device"], r["pinout"], r["pdf"], r["pages"], r["result"]]) + "\n")
    with open(os.path.join(OUT, "eye-checks.tsv"), "w") as f:
        f.write("device\tpinout\tpdf\tpages\tresult\n")
        for row in EYE_NAME:
            f.write("\t".join(row) + "\n")
    print("ok")


if __name__ == "__main__":
    main(sys.argv[1])
