"""plan-36-C/D: the "Drawing Numbers" tables of the newer Microchip datasheets.

usage: python3 -I drawing-numbers.py <candidates.tsv> > drawing-numbers.tsv

The AVR Dx/Ex/Lx/Sx and tinyAVR 2 datasheets list each package as `Pin Count
Package Type Drawing Number Style` (`28  SOIC  C04-00052  SO`). A code whose
drawing page is an image in one datasheet can take the same Microchip drawing
(same C04 number) from another datasheet where it is text. Columns: `pdf pins
type drawing style`.
"""
import csv
import os
import re
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
ROW = re.compile(r"^\s*(\d{1,3})\s+([A-Z][A-Za-z]+(?: \d+x\d+)?)\s+(C04-[0-9A-Z]+)\s+([0-9A-Z]{1,4})\s*$")


def main(tsv):
    pdfs = sorted({r["pdf"] for r in csv.DictReader(open(tsv), delimiter="\t") if r["verdict"] == "add" and "microchip" in r["pdf"]})
    print("pdf\tpins\ttype\tdrawing\tstyle")
    for pdf in pdfs:
        txt = subprocess.run(["pdftotext", "-q", "-layout", os.path.join(ROOT, pdf), "-"], capture_output=True, text=True).stdout
        for line in txt.split("\n"):
            m = ROW.match(line)
            if m:
                print(f"{pdf}\t{m.group(1)}\t{m.group(2)}\t{m.group(3)}\t{m.group(4)}")


if __name__ == "__main__":
    main(sys.argv[1])
