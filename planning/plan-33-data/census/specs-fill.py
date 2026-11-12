"""plan-33-B Phase 1: write the part-number schemes into specs.tsv.

usage: python3 -I specs-fill.py [specs.tsv]

SPECS maps a datasheet's file name (datasheets/capacitor/<maker>/<name>) to
its scheme, read from the PDF's part-number guide (pdftotext -layout, or the
page image where the guide is a picture): mpn_regex (named groups `cap`,
`volt`, optional `case` and `base`), cap_group, volt_map, case_group and the
note (where the guide is).  Every alternative in a regex is one the guide
prints; a field whose guide says only "see table" (KEMET size codes and
packaging, ESR digits) is matched by its printed shape, and the `base`
group then requires the part number less its packaging code to be printed
in the ratings table (scheme-check.py).  A PDF with no guide gets `NONE`.
specs-fill.py rewrites only rows whose datasheet is in SPECS.
"""
import csv
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)

# ---- shared pieces ----------------------------------------------------------

EIA_CASES = "0201|0402|0603|0805|1206|1210|1805|1808|1812|1825|2220|2225"
EIA_CASE_MAP = ";".join(f"{c}={c}" for c in EIA_CASES.split("|"))
KEMET_PACK = "TU|7411|7210|TM|7040|7215"
KEMET_C0G_V = "8=10;4=16;3=25;5=50;1=100;2=200;A=250"
KEMET_X7R_V = "9=6.3;8=10;4=16;3=25;6=35;5=50;1=100;2=200;A=250"
KEMET_X5R_V = "7=4;9=6.3;8=10;4=16;3=25;6=35;5=50;1=100"
AVX_X7R_V = "4=4;6=6.3;Z=10;Y=16;3=25;5=50;1=100;2=200;7=500"
AVX_C0G_V = "4=4;6=6.3;Z=10;Y=16;3=25;5=50;1=100;2=200;V=250;7=500"
AVX_X5R_V = "4=4;6=6.3;Z=10;Y=16;3=25;D=35;5=50;1=100"
CALCHIP_CASE = "01=01005;02=0201;04=0402;10=0603;21=0805;31=1206;32=1210;40=1808;43=1812;45=1825;55=2220;57=2225"
CALCHIP_V = "4R0=4;6R3=6.3;10=10;16=16;25=25;35=35;50=50;63=63;100=100;200=200"


def kemet_mlcc(diel, volts, tols, page):
    keys = "".join(k.split("=")[0] for k in volts.split(";"))
    return (rf"C(?P<case>{EIA_CASES})C(?P<cap>\d{{3}})(?P<tol>[{tols}])(?P<volt>[{keys}]){diel}AC(?P<pack>{KEMET_PACK})?",
            "eia", volts, EIA_CASE_MAP, f"Ordering Information p.{page}")


def avx_mlcc(diel, volts, tols, terms, packs, page):
    keys = "".join(k.split("=")[0] for k in volts.split(";"))
    return (rf"(?P<case>{EIA_CASES})(?P<volt>[{keys}]){diel}(?P<cap>\d{{3}})(?P<tol>[{tols}])A(?P<term>[{terms}])(?P<pack>[{packs}])A",
            "eia", volts, EIA_CASE_MAP, f"PART NUMBER p.{page}")


KEMET_AL_V = ("004=4;6R3=6.3;010=10;016=16;025=25;035=35;050=50;063=63;080=80;100=100;160=160;200=200;"
              "250=250;350=350;400=400;450=450;500=500")


def kemet_al_radial(series, page):
    """Radial cans: Table 1 prints `ESK107M016AE3(1)`, (1) the packaging code."""
    return (rf"(?P<base>{series}(?P<cap>\d{{3}})M(?P<volt>\dR\d|\d{{3}})A[A-Z0-9]\d)(?P<pack>[A-Z]{{2}})",
            "eia", KEMET_AL_V, "", f"Part Number System p.1; Table 1 Ratings & Part Number Reference p.{page}")


def kemet_al_smd(series, page):
    """SMD cans: Table 1 prints `EDK336M004(1)9BAA`, (1) the electrical parameter A or S."""
    return (rf"(?P<base>{series}(?P<cap>\d{{3}})M(?P<volt>\dR\d|\d{{3}}))[AS][0-9][A-Z](?P<pack>AA|AP)",
            "eia", KEMET_AL_V, "", f"Part Number System p.1; Table 1 Ratings & Part Number Reference p.{page}")


AVX_AL_V = ("002=2.5;004=4;006=6.3;010=10;016=16;020=20;025=25;035=35;040=40;050=50;063=63;080=80;"
            "100=100;125=125;160=160;200=200;250=250;350=350;400=400;450=450")


def avx_al_radial(series, page):
    """Radial cans: the ratings table prints `REA0511101M006` (no packaging code, K or B)."""
    return (rf"(?P<base>{series}(?P<case>\d{{4}})(?P<cap>\d{{3}})M(?P<volt>\d{{3}}))(?P<pack>[KB])?",
            "eiauf", AVX_AL_V, "", f"HOW TO ORDER p.1; RATINGS & PART NUMBER REFERENCE p.{page}")


def avx_al_smd(series, page):
    """SMD cans: the ratings table prints `APA0604221M002R` (R, the reel, included)."""
    return (rf"(?P<base>{series}(?P<case>\d{{4}})(?P<cap>\d{{3}})M(?P<volt>\d{{3}})R)",
            "eiauf", AVX_AL_V, "", f"HOW TO ORDER p.1; RATINGS & PART NUMBER REFERENCE p.{page}")


AVX_TA_V = ("001=1.8;002=2.5;003=3;004=4;006=6.3;010=10;016=16;020=20;025=25;035=35;050=50;063=63;"
            "075=75;100=100;125=125")


def avx_ta(types, page, tols="KM"):
    """KYOCERA AVX molded chips (tantalum, polymer, niobium oxide): the ratings
    table prints `TAJA475*006#NJ` (* the tolerance, # the packaging letter)
    or `TCJA686M002#0250E` (the tolerance printed): b1 b2 b3 (scheme-check.py)."""
    return (rf"(?P<b1>(?:{types})(?P<case>[A-Z0-9])(?P<cap>\d{{3}}))(?P<tol>[{tols}])(?P<b2>(?P<volt>\d{{3}}))"
            rf"(?P<pack>[A-Z])(?P<b3>[0-9A-Z]*)",
            "eia", AVX_TA_V, "", f"HOW TO ORDER p.1; RATINGS & PART NUMBER REFERENCE p.{page}")


def kemet_leaded(styles, diel, volts, tols, fins, packs, page=1, spec="C"):
    """KEMET leaded MLCC (axial, radial, molded): C<style><spec><cap><tol><volt><diel>5<finish>A<C-spec>."""
    keys = "".join(k.split("=")[0] for k in volts.split(";"))
    return (rf"C(?P<case>{styles}){spec}(?P<cap>\d{{3}})(?P<tol>[{tols}])(?P<volt>[{keys}]){diel}5[{fins}]A"
            rf"(?P<pack>{packs})?",
            "eia", volts, "", f"Ordering Information p.{page}")


GOLDMAX_STYLES = "315|316|317|318|320|321|322|323|324|325|326|327|328|330|331|333|335|336|340|346|350|356"
GOLDMAX_PACK = "7301|7303|TR|7305|7317"


KEMET_TA_V = ("2R5=2.5;003=3;004=4;006=6.3;010=10;016=16;020=20;025=25;035=35;040=40;050=50;063=63;"
              "075=75;100=100")


def kemet_ta(page):
    """KEMET tantalum and polymer chips / dipped: T<series><case><cap><tol><volt>A<finish>[E<esr>][C-spec];
    the ratings table prints the part number less its C-spec, whole (`T495C476M016ATE350`) or with
    the tolerance and finish as placeholders (`T491A105(1)010A(2)`, `T520A106M010A(3)E080`)."""
    return (r"(?P<base>(?P<b1>T(?P<series>\d{3})(?P<case>[A-Z])(?P<cap>\d{3}))(?P<tol>[JKM])"
            r"(?P<b2>(?P<volt>2R5|\d{3})A)(?P<fin>[A-Z])(?P<b3>E\d{3})?)(?P<pack>\d{4})?",
            "eia", KEMET_TA_V, "", f"Ordering Information p.2; Table 1 Ratings & Part Number Reference p.{page}")


EIA_V = "0E=2.5;0G=4;0J=6.3;1A=10;1C=16;1D=20;1E=25;1V=35;1H=50;1J=63;1K=80;2A=100"


def kemet_poly(series, page, smd=False):
    """KEMET aluminum polymer A7xx: A<series><size><cap>M<volt><packaging><E|S|V><ESR>. Radial
    tables print `A759EK227M0J(1)E018` (packaging a placeholder); SMD ones `A768KS477M1CLA(1)017`
    (packaging printed, the electrical parameter a placeholder)."""
    if smd:
        return (rf"(?P<base>(?P<b1>A{series}(?P<size>[A-Z]{{2}})(?P<cap>\d{{3}}))(?P<tol>M)"
                rf"(?P<b2>(?P<volt>0E|0G|0J|1A|1C|1D|1E|1V|1H|1J|1K|2A)[A-Z]{{2}})[EVS](?P<b3>\d{{3}}))",
                "eia", EIA_V, "", f"Part Number System p.1; Table 1 Ratings & Part Number Reference p.{page}")
    return (rf"(?P<b1>A{series}(?P<size>[A-Z]{{2}})(?P<cap>\d{{3}}))(?P<tol>M)"
            rf"(?P<b2>(?P<volt>0E|0G|0J|1A|1C|1D|1E|1V|1H|1J|1K|2A))(?P<pack>[A-Z]{{2}})(?P<b3>[EVS]\d{{3}})",
            "eia", EIA_V, "", f"Part Number System p.1; Table 1 Ratings & Part Number Reference p.{page}")


CALCHIP_EL_V = ("OG=4;0G=4;OJ=6.3;0J=6.3;1A=10;1C=16;1E=25;1V=35;1H=50;1J=63;1K=80;2A=100")


def calchip_can(series, volts, cases, page_note):
    """Cal-Chip SMD cans: <series>[-]<volt><cap µF code>M<case>-R. The guide draws the fields as boxes;
    the stocked part numbers put a hyphen after the series (`CV2-1A101ME55-R`). The guide prints
    6.3 V as `OJ` (letter O); a part may use the digit."""
    return (rf"{series}-?(?P<volt>{volts})(?P<cap>[0-9R]{{3}})M(?P<case>{cases})-R",
            "eiauf", CALCHIP_EL_V, "", page_note)


def calchip_dims(dims):
    """Case sizes `DxL` as printed (`8x12.0`); a part may drop a trailing `.0` (`8X12`)."""
    def one(x):
        if x.endswith(".0"):
            return x[:-2].replace(".", r"\.") + r"(?:\.0)?"
        return x.replace(".", r"\.")
    return "|".join(one(a) + "X" + one(b) for a, b in (d.split("x") for d in dims.split()))


def kemet_safety(series, dias, spacings, tols, volts, page=2):
    """KEMET safety discs: <C7|C9><diameter><spacing>U<cap><tol><volt><dielectric>D<lead>A<C-spec>.
    `volts` maps the voltage letter to its X1 rating in VAC (DigiKey's Voltage - Rated)."""
    keys = "".join(k.split("=")[0] for k in volts.split(";"))
    return (rf"{series}(?P<dia>[{dias}])(?P<sp>[{spacings}])U(?P<cap>\d{{3}})(?P<tol>[{tols}])(?P<volt>[{keys}])"
            rf"[SYWV]D[ABCD]A(?P<pack>7301|7317|WL10|WL20|WL25|WL30|WL35|WL40|WL45|WL50)",
            "eia", volts, "", f"Ordering Information p.{page} (voltage letter = X1 VAC rating)")


SPECS = {
    # ---- Cal-Chip MLCC ----
    "GMC-Series-3.pdf": (
        r"GMC(?P<case>01|02|04|10|21|31|32|40|43|45|55|57)(?P<diel>CG|X7R|X5R|X7S|X7T|X6S|Y5V|Z5U)"
        r"(?P<cap>[0-9R]{3})(?P<tol>[ABCDFGJKMZ])(?P<volt>4R0|6R3|10|16|25|35|50|63|100|200)(?P<term>NT|PT)(?P<pack>D|G|Q)?",
        "eia", CALCHIP_V, CALCHIP_CASE, "PART NUMBER GUIDE p.2; sizes from the DIMENSION tables p.3-"),
    "GMX-Series-03122025.pdf": (
        r"GMX(?P<case>04|10|21|31|32|40|43|45|55|57)(?P<diel>CG|X7R|X5R)(?P<cap>[0-9R]{3})(?P<tol>[ABCDFGHJKLMNPQZX])"
        r"(?P<volt>6R3|10|16|25|35|50|100|200|250|400|500|630|1K0|1K5|2K0|3K0|4K0|5K0)(?P<pack>CT|CTD|CTG)?",
        "eia", "6R3=6.3;10=10;16=16;25=25;35=35;50=50;100=100;200=200;250=250;400=400;500=500;630=630;"
        "1K0=1000;1K5=1500;2K0=2000;3K0=3000;4K0=4000;5K0=5000",
        "04=0402;10=0603;21=0805;31=1206;32=1210;40=1808;43=1812;45=1825;55=2220;57=2225",
        "ORDERING INFORMATION p.1 (picture)"),
    # ---- KEMET MLCC ----
    "KEM_C1003_C0G_SMD.pdf": kemet_mlcc("G", KEMET_C0G_V, "BCDFGJKM", 1),
    "KEM_C1002_X7R_SMD.pdf": kemet_mlcc("R", KEMET_X7R_V, "JKM", 1),
    "KEM_C1006_X5R_SMD.pdf": kemet_mlcc("P", KEMET_X5R_V, "KM", 1),
    # ---- KEMET leaded MLCC ----
    "KEM_C1040_AXIMAX_C0G.pdf": kemet_leaded("410|412|420|430|440", "G", "3=25;5=50;1=100;2=200;A=250",
                                             "DFGJK", "TH", "7200|7293"),
    "KEM_C1041_AXIMAX_X7R.pdf": kemet_leaded("410|412|420|430|440", "R", "3=25;5=50;1=100;2=200",
                                             "JKM", "TH", "7200|7293"),
    "KEM_C1071_AXIMAX_X8R.pdf": kemet_leaded("410|430", "H", "5=50;1=100;2=200", "DFGJKM", "TH",
                                             "9170|7200|7293|91707200|91707293"),
    "KEM_C1049_GOLDMAX_C0G.pdf": kemet_leaded(GOLDMAX_STYLES, "G", "3=25;5=50;1=100;2=200;A=250",
                                              "DFGJK", "TH", GOLDMAX_PACK),
    "KEM_C1050_GOLDMAX_X7R.pdf": kemet_leaded(GOLDMAX_STYLES, "R", "3=25;5=50;1=100;2=200;A=250",
                                              "JKM", "TH", GOLDMAX_PACK),
    "KEM_C1055_HIVOLT_GOLDMAX_300_X7R.pdf": kemet_leaded(
        GOLDMAX_STYLES, "R", "C=500;B=630;D=1000;F=1500;G=2000;Z=2500", "JKM", "TH", GOLDMAX_PACK),
    "KEM_C1059_RADIAL_MOLDED_X7R.pdf": kemet_leaded("052|062|512|522", "R", "5=50;1=100;2=200",
                                                    "KM", "TC", "7301|7303|7293"),
    "KEM_C1073_HT_RADIAL_MOLDED_C0G.pdf": kemet_leaded("052|062", "G", "5=50;1=100;2=200", "BCDFGJK", "G",
                                                       "T250|T500|T1K0", spec="H"),
    # ---- KYOCERA AVX leaded MLCC ----
    "SR-Series.pdf": (r"SR(?P<style>15|20|21|22|27|30|40|50)(?P<volt>[512987])(?P<diel>[ACE])(?P<cap>[0-9R]{3})"
                      r"(?P<tol>[CDFGJKMZ])AR(?P<pack>T|TR1|AP1)?",
                      "eia", "5=50;1=100;2=200;9=300;8=400;7=500", "", "HOW TO ORDER p.1"),
    "SE-Style.pdf": (r"SE(?P<size>\d{2})(?P<volt>[351])C(?P<cap>\d{3})(?P<tol>[KMZ])A(?P<lead>[AR])",
                     "eia", "3=25;5=50;1=100", "", "HOW TO ORDER p.1"),
    "MR-Series.pdf": (r"MR(?P<style>04|05|06|07|08)(?P<volt>[512])(?P<diel>[ACE])(?P<cap>\d{3})(?P<tol>[DFJKMZ])"
                      r"(?P<rate>[AT])(?P<rest>[A-Z0-9]*)",
                      "eia", "5=50;1=100;2=200", "", "HOW TO ORDER p.1"),
    "KGM_X7R.pdf": (r"KGM(?P<case>02|03|05|15|21|31|32|43|44|55|56)(?P<th>[A-Z])R7"
                    r"(?P<volt>0G|0J|1A|1C|1E|1H|2A|2D|2E|2H)(?P<cap>\d{3})(?P<tol>[JKM])(?P<pack>[A-Z])",
                    "eia", "0G=4;0J=6.3;1A=10;1C=16;1E=25;1H=50;2A=100;2D=200;2E=250;2H=500",
                    "02=01005;03=0201;05=0402;15=0603;21=0805;31=1206;32=1210;43=1812;44=1825;55=2220;56=2225",
                    "HOW TO ORDER p.1"),
    "KGM_Series_e.pdf": (r"KGM(?P<case>02|03|05|15|21|31|32|43|44|55|56)(?P<th>[A-Z])(?P<diel>[A-Z][0-9A-Z])"
                         r"(?P<volt>0E|0G|0J|1A|1C|1E|1V|1H|2A|2D|2E|2H|2J)(?P<cap>\d{3})(?P<tol>[BCDFGJKMZ])"
                         r"(?P<pack>[A-Z])(?P<opt>[0-9A-Z]{0,4})",
                         "eia", "0E=2.5;0G=4;0J=6.3;1A=10;1C=16;1E=25;1V=35;1H=50;2A=100;2D=200;2E=250;2H=500;"
                         "2J=630",
                         "02=01005;03=0201;05=0402;15=0603;21=0805;31=1206;32=1210;43=1812;44=1825;55=2220;56=2225",
                         "How to Order p.1 (example and field list)"),
    "TAZ.pdf": (r"TAZ(?P<case>[A-Z])(?P<cap>\d{3})(?P<tol>[JKM])(?P<volt>\d{3})(?P<esr>[CL])(?P<pack>[BRSW])"
                r"(?P<insp>[SLM])(?P<rel>[BCDZ])(?P<q>[09T])(?P<fin>[H0897])(?P<rest>\d*)",
                "eia", "004=4;006=6;010=10;015=15;020=20;025=25;035=35;050=50", "",
                "HOW TO ORDER p.2 (COTS-PLUS & MIL QPL; 006 = 6 V as printed)"),
    # ---- KEMET A / S series (leaded, high temperature / standard) ----
    "KEM_C1118_A-SERIES.pdf": (r"A(?P<diel>[CR])(?P<lead>[AR])(?P<style>16|25|39|50|69|05|06|07|08)(?P<volt>[BDS])"
                               r"(?P<cap>\d{3})(?P<tol>[JKM])(?P<bar>[GW])(?P<test>[SAX])",
                               "eia", "B=50;D=100", "", "Ordering Information p.2"),
    "KEM_C1119_S-SERIES.pdf": (r"S(?P<diel>[CR])(?P<lead>[AR])(?P<style>16|25|39|50|69|05|06|07|08)(?P<volt>[BDF])"
                               r"(?P<cap>\d{3})(?P<tol>[JKM])(?P<bar>[GW])(?P<test>[SAX])",
                               "eia", "B=50;D=100;F=200", "", "Ordering Information p.2"),
    # ---- KEMET general-purpose disc ----
    "KEM_C1127_CGP.pdf": (r"CGP(?P<sp>[5712])C(?P<cap>\d{3})(?P<tol>[JKM])(?P<volt>[DGHUKLN])[SYWV]D[ABCD]A"
                          r"(?P<pack>7301|7317|WL10|WL20|WL25|WL30|WL35|WL40|WL45|WL50)?",
                          "eia", "D=1000;G=2000;H=3000;U=4000;K=6000;L=8000;N=10000", "",
                          "Ordering Information p.2"),
    "C971U472MVWDCAWL50.pdf": kemet_safety("C9", "0-7", "1", "JKM", "V=400"),
    # ---- KEMET part spec sheets (one part, no guide) ----
    "C114K101K1X5CA.pdf": ("NONE", "none", "none", "", "a one-page spec sheet of C114K101K1X5CA: no guide"),
    # ---- KEMET tantalum / aluminum polymer, more ----
    "KEM_T2073_T59X.pdf": kemet_ta(9),
    "KEM_T2091_A700-A720.pdf": (
        r"(?P<base>A7[02]0(?P<case>[DQVWX])(?P<cap>\d{3})(?P<tol>M)(?P<volt>002|2R5|004|006|008|010|016|020|025|035)"
        r"A[TP])(?P<esr>E\d{3})(?P<pack>7280)?",
        "eia", "002=2;2R5=2.5;004=4;006=6.3;008=8;010=10;016=16;020=20;025=25;035=35", "",
        "Ordering Information p.3; ratings table prints `A700W227M002AT`"),
    "KEM_A4124_A782.pdf": kemet_poly("782", 8, smd=True),
    "KEM_A4125_A785.pdf": kemet_poly("785", 7, smd=True),
    "KEM_A4147_A740.pdf": kemet_poly("740", 7, smd=True),
    "KEM_A4115_PHH227.pdf": (r"(?P<base>PHH227(?P<volt>[HJM])(?P<size>[A-Z]{2})(?P<cap>\d{3})0M)E4",
                             "phx", "H=25;J=35;M=63", "", "Part Number System p.1; table p.5"),
    "KEM_A4136_PHV227.pdf": (r"(?P<base>PHV227(?P<volt>[HJ])(?P<size>[A-Z]{2})(?P<cap>\d{3})0M)E4",
                             "phx", "H=25;J=35", "", "Part Number System p.2; table p.6"),
    # ---- Cal-Chip polymer tantalum ----
    "PT-SERIES.pdf": (r"PTM(?P<volt>OB|OE|OF|OG|OJ|OK|IA|IC|ID|IE)(?P<cap>\d{3})(?P<case>[A-EGHKLMNPRSTUVWXY5])"
                      r"(?P<fin>[THPNM])(?P<esr>\d{3})",
                      "eia", "OB=2.5;OE=2.5;OF=3;OG=4;OJ=6.3;OK=8;IA=10;IC=16;ID=20;IE=25", "",
                      "PART NUMBERING p.1 (picture; the guide prints 2.5 V as OE, the ratings table p.3 as OB)"),
    # ---- KEMET safety discs ----
    "KEM_C1103_KJY.pdf": kemet_safety("C7", "0-9", "5712", "KM", "Y=400;Z=440"),
    "KEM_C1104_KJN.pdf": kemet_safety("C7", "2-8", "12", "JKM", "S=440;T=440;W=760"),
    "C901U102MYVDAAWL30.pdf": kemet_safety("C9", "0-468", "571", "JKM", "Y=400"),
    "C901U100JZSDAAWL20.pdf": kemet_safety("C9", "0-46", "71", "JKM", "Z=440"),
    "C971U472MUWDAAWL50.pdf": kemet_safety("C9", "0-5", "1", "JKM", "U=400"),
    # ---- KEMET SMD MLCC, other series ----
    "KEM_C1010_X7R_HV_SMD.pdf": (
        rf"C(?P<case>0402|0603|0805|1206|1210|1808|1812|1825|2220|2225)C(?P<cap>\d{{3}})(?P<tol>[JKM])"
        rf"(?P<volt>[CBDFGZ])RA[CL](?P<pack>{KEMET_PACK})?",
        "eia", "C=500;B=630;D=1000;F=1500;G=2000;Z=2500", EIA_CASE_MAP, "Ordering Information p.1"),
    "KEM_C1091_C0G_ESD.pdf": (
        rf"C(?P<case>0402|0603|0805|1206)[CX](?P<cap>\d{{3}})(?P<tol>[FGJKM])(?P<volt>[35M12A])GEC"
        rf"(?P<pack>{KEMET_PACK}|AUTO)?",
        "eia", "3=25;5=50;M=63;1=100;2=200;A=250", EIA_CASE_MAP, "Ordering Information p.2"),
    "KEM_C1007_X8R_ULTRA_150C_SMD.pdf": (
        rf"C(?P<case>0402|0603|0805|1206|1210|1812)C(?P<cap>\d{{3}})(?P<tol>[BCDFGJKM])(?P<volt>[84351])HA[CL]"
        rf"(?P<pack>{KEMET_PACK}|AUTO)?",
        "eia", "8=10;4=16;3=25;5=50;1=100", EIA_CASE_MAP, "Ordering Information p.1"),
    # ---- KYOCERA AVX MLCC ----
    "cx5r.pdf": avx_mlcc("D", AVX_X5R_V, "KM", "T", "24", 1),
    "X7RDielectric.pdf": avx_mlcc("C", AVX_X7R_V, "JKM", "TZ", "24", 1),
    "C0GNP0-Dielectric.pdf": avx_mlcc("A", AVX_C0G_V, "BCDFGJK", "T", "24U", 1),
    # ---- KYOCERA AVX tantalum / polymer / niobium oxide ----
    "TAJ.pdf": avx_ta("TAJ", 3),
    "TCJ.pdf": avx_ta("TCJ", 3),
    "TCM.pdf": avx_ta("TCM", 2),
    "TCN_Undertab.pdf": avx_ta("TCN", 3),
    "J-CAP.pdf": avx_ta("TCJ|TCN", 3),
    "TPS.pdf": avx_ta("TPS", 3),
    "TPM.pdf": avx_ta("TPM", 3),
    "TLJ.pdf": avx_ta("TLJ", 3),
    "TAC.pdf": avx_ta("TAC", 3),
    "NOJ.pdf": avx_ta("NOJ", 3),
    "NOS.pdf": avx_ta("NOS", 3),
    "tap.pdf": (r"TAP(?P<cap>\d{3})(?P<tol>[KM])(?P<volt>\d{3})(?P<sfx>[A-Z]{3})",
                "eia", AVX_TA_V, "", "HOW TO ORDER p.1 (wire-form suffix: three letters, catalog p.222)"),
    "F92.pdf": (r"F92(?P<volt>0E|0G|0J|1A|1C|1D|1E|1V|1H)(?P<cap>\d{3})(?P<tol>[KM])(?P<case>[A-Z])(?P<pack>[A-Z]{1,2})",
                "eia", EIA_V, "", "HOW TO ORDER p.1"),
    "F93.pdf": (r"F93(?P<volt>0E|0G|0J|1A|1C|1D|1E|1V|1H)(?P<cap>\d{3})(?P<tol>[KM])(?P<case>[A-Z])(?P<pack>[A-Z]{1,2})",
                "eia", EIA_V, "", "HOW TO ORDER p.1"),
    # ---- KEMET tantalum ----
    "KEM_T2005_T491.pdf": kemet_ta(4),
    "KEM_T2076_T52X-530.pdf": kemet_ta(4),
    "KEM_T2009_T495.pdf": kemet_ta(4),
    "KEM_T2014_T510.pdf": kemet_ta(4),
    "KEM_T2061_T543.pdf": kemet_ta(4),
    "T354K476K020AT.pdf": (r"T35[0-6](?P<case>[A-M])(?P<cap>\d{3})(?P<tol>[JKM])(?P<volt>\d{3})A(?P<fin>[ST])"
                           r"(?P<pack>7301|7303|7305|7317)?",
                           "eia", "003=3;006=6;010=10;016=16;020=20;025=25;035=35;050=50", "",
                           "T35X Ordering Information p.2 (006 = 6 V, as printed); no part-number table"),
    # ---- Cal-Chip tantalum ----
    "TC-Series.pdf": (r"TC(?P<tol>[KM])(?P<volt>OB|OG|OJ|IA|IC|ID|IE|IV|IX)(?P<cap>\d{3})(?P<case>[A-HKMP-W])T",
                      "eia", "OB=2.5;OG=4;OJ=6.3;IA=10;IC=16;ID=20;IE=25;IV=35;IX=50",
                      "A=3216;B=3528;C=6032;D=7343;E=7343;G=7361;H=7361;F=6032;M=3528;K=3216;P=2012;R=2012;"
                      "S=3216;T=3528;U=6032;W=7343;V=7343",
                      "PART NUMBERING CODE and case tables p.1 (picture; case codes as EIA metric, DigiKey "
                      "prints 3528 as 1411)"),
    # ---- KEMET aluminum ----
    "KEM_A4004_ESK.pdf": kemet_al_radial("ESK", 5),
    "KEM_A4005_ESH.pdf": kemet_al_radial("ESH", 5),
    "KEM_A4006_ESC.pdf": kemet_al_radial("ESC", 5),
    "KEM_A4007_ESY.pdf": kemet_al_radial("ESY", 5),
    "KEM_A4009_ESW.pdf": kemet_al_radial("ESW", 5),
    "KEM_A4010_EST.pdf": kemet_al_radial("EST", 6),
    "KEM_A4055_ESE.pdf": kemet_al_radial("ESE", 6),
    "KEM_A4057_ESS.pdf": kemet_al_radial("ESS", 5),
    "KEM_A4074_ESL.pdf": kemet_al_radial("ESL", 5),
    "KEM_A4073_ESX.pdf": kemet_al_radial("ESX", 5),
    "KEM_A4001_EDK.pdf": kemet_al_smd("EDK", 5),
    "KEM_A4002_EEV.pdf": kemet_al_smd("EEV", 5),
    "KEM_A4003_EXV.pdf": kemet_al_smd("EXV", 5),
    "KEM_A4049_EDH.pdf": kemet_al_smd("EDH", 5),
    "KEM_A4054_EDT.pdf": kemet_al_smd("EDT", 5),
    # ---- Cal-Chip aluminum (pictures, p.1) ----
    "CV2-SERIES.pdf": calchip_can("CV2", "OG|0G|OJ|0J|1A|1C|1E|1V|1H|1J|2A",
                                  "B55|D55|E55|F55|F80|G68|G10|H10|H13|K14", "PART NUMBER p.1 (picture)"),
    "CVH-SERIES-1.pdf": calchip_can("CVH", "OJ|0J|1A|1C|1E|1V|1H|1J|1K|2A",
                                    "D60|E60|F60|F80|G68|G10|H10|H13|K14|K16|L17", "PART NUMBER p.1 (picture)"),
    "CVJ-SERIES-1.pdf": calchip_can("CVJ", "OG|0G|OJ|0J|1A|1C|1E|1V|1H|1J|2A",
                                    "D55|E55|F55|F80|G68|G10|H13|K14|K16|L17|L22", "PART NUMBER p.1 (picture)"),
    "CVL-SERIES-1.pdf": calchip_can("CVL", "OJ|0J|1A|1C|1E|1V|1H|1J|1K|2A",
                                    "D60|E60|F60|F80|G68|G10|H8|H10|H13|K14", "PART NUMBER p.1 (picture)"),
    "CCJ-SERIES-2.pdf": (
        r"CCJ(?P<volt>OE|0E|OG|0G|OJ|0J|1A|1C|1E|1V|1H|1J|1K|2A)(?P<cap>[0-9R]{3})M(?P<case>"
        + calchip_dims("3x5.3 4x5.3 4x5.7 5x5.3 5x5.7 6.3x5.3 6.3x5.7 6.3x5.9 6.3x6.0 6.3x7.0 6.3x7.7 8x6.5 "
                       "8x6.7 8x7.0 8x10.0 8x12.0 10x7.7 10x8.0 10x9.9 10x10.0 10x12.7 10x13.0 12.5x13.5 "
                       "12.5x16.0 16x16.5") + ")-R",
        "eiauf", CALCHIP_EL_V + ";OE=2.5;0E=2.5", "", "PART NUMBERING p.1 (picture)"),
    "CHR-SERIES.pdf": (
        r"CHR(?P<volt>1E|1V|1H|1J|1K)(?P<cap>[0-9R]{3})M(?P<case>"
        + calchip_dims("6.3x6.0 6.3x7.7 8x6.5 8x10.0 10x10.5") + ")-R",
        "eiauf", CALCHIP_EL_V, "", "PART NUMBERING p.1 (picture; 10x10.5 printed `10x105`)"),
    # ---- KEMET aluminum polymer ----
    "KEM_A4067_A750.pdf": kemet_poly("750", 7),
    "KEM_A4068_A755.pdf": kemet_poly("755", 7),
    "KEM_A4069_A758.pdf": kemet_poly("758", 7),
    "KEM_A4072_A759.pdf": kemet_poly("759", 8),
    "KEM_A4070_A765.pdf": kemet_poly("765", 7, smd=True),
    "KEM_A4071_A767.pdf": kemet_poly("767", 7, smd=True),
    "KEM_A4088_A768.pdf": kemet_poly("768", 7, smd=True),
    "KEM_A4089_A766.pdf": kemet_poly("766", 5, smd=True),
    "KEM_A4144_A7C0.pdf": kemet_poly("7C0", 7),
    # ---- KYOCERA AVX aluminum ----
    "rea-series.pdf": avx_al_radial("REA", 3),
    "rha-series.pdf": avx_al_radial("RHA", 2),
    "rpa-series.pdf": avx_al_radial("RPA", 2),
    "kyocera-avx-apa-series.pdf": avx_al_smd("APA", 2),
    "kyocera-avx-aha-series.pdf": avx_al_smd("AHA", 2),
    "kyocera-avx-aea-series.pdf": avx_al_smd("AEA", 2),
}


def main(path):
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t")
        cols = reader.fieldnames
        rows = list(reader)
    done = 0
    for r in rows:
        spec = SPECS.get(os.path.basename(r["datasheet"]))
        if spec is None:
            continue
        rx, cap, volts, case, note = spec
        r.update({"mpn_regex": rx, "cap_group": cap, "volt_group": "map", "volt_map": volts,
                  "case_group": case, "note": note})
        done += 1
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, cols, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)
    empty = sum(1 for r in rows if not r["mpn_regex"])
    print(f"{done} rows filled, {empty} still empty, {len(rows)} rows")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else os.path.join(DATA, "specs.tsv")))
