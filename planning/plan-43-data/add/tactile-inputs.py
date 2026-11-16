"""plan-43-C section 4.1: the plan-27 generator inputs of the two reference-board tactile switches.
usage: python3 -I planning/plan-43-data/add/tactile-inputs.py   (from the repository root; a second run refuses)
Appends to planning/plan-27-data/{candidates,specs,maker-urls}.tsv and
datasheets/switch/tactile/datasheets.tsv, and adds the two makers to gen-parts.py's MAKERS."""
import hashlib, sys
D = 'planning/plan-27-data/'
GCT_DK = '//mm.digikey.com/Volume0/opasdata/d220001/medias/docus/8937/swt0105-spec.pdf'
CIT_URL = 'https://www.citrelay.com/Catalog%20Pages/SwitchCatalog/CS1213.pdf'

def append(path, lines, key):
    s = open(path).read()
    if key in s: sys.exit('already applied: ' + path)
    assert s.endswith('\n')
    open(path, 'w').write(s + ''.join(l + '\n' for l in lines))

def tsv_line(path, url):
    b = open(path, 'rb').read()
    return '\t'.join([path, url, str(len(b)), hashlib.sha256(b).hexdigest()])

append(D + 'candidates.tsv', [
    '\t'.join(['CIT Relay and Switch', 'CS1213AGF260', 'Active', '61169', 'CS1213', 'SPST-NO', 'Off-Mom', 'Surface Mount',
               'Standard', 'Top Actuated', 'SMD (SMT) Tab', '3.90mm x 2.90mm', '0.05A @ 50VDC', 'Non-Illuminated', '-',
               '2.00mm', CIT_URL, 'add',
               'plan-43: key Switch-CS1213.pdf p.1 Ordering Information: CS1213 + A (without boss) + G (gull wing) + F260 (260 gF)',
               '1', '1', '0']),
    '\t'.join(['GCT', 'SWT0105-003516JSA', 'Active', '850', 'SWT0105', 'SPST-NO', 'Off-Mom', 'Surface Mount',
               'Flush', 'Top Actuated', 'J Lead', '3.70mm x 3.70mm', '0.05A @ 12VDC', 'Non-Illuminated', '-',
               '0.35mm', GCT_DK, 'add',
               'plan-43: key swt0105-spec.pdf p.1 Ordering Grid: SWT0105 - 0035 (0.35 mm) 16 (160 g) J (J lead) S (silver) A (tape and reel)',
               '1', '1', '0'])], 'CS1213AGF260')
append(D + 'specs.tsv', [
    '\t'.join(['CIT Relay and Switch', 'CS1213', 'Surface Mount', r'CS1213AG.*', '2', 'A=1;B=2', 'SW-TACT-CIT-CS1213-G',
               'Switch-CS1213.pdf p.2 CS1213AG Schematic: 1 - switch - 2']),
    '\t'.join(['GCT', 'SWT0105', 'Surface Mount', r'SWT0105-.*', '4', 'A=1,2;B=3,4', 'SW-TACT-GCT-SWT0105',
               'swt0105.pdf (GCT drawing) sheet 1 Circuit Diagram: 1-2 joined, 3-4 joined, switch between'])], 'SW-TACT-CIT-CS1213-G')
append(D + 'maker-urls.tsv', [
    '\t'.join([GCT_DK, 'https://gct.co/files/specs/swt0105-spec.pdf',
               'gct.co SWT0105 product page, Specification; the same bytes as the DigiKey copy (plan-43-A)'])], 'swt0105-spec.pdf')
append('datasheets/switch/tactile/datasheets.tsv', [
    tsv_line('datasheets/switch/tactile/cit/Switch-CS1213.pdf', CIT_URL),
    tsv_line('datasheets/switch/tactile/gct/swt0105-spec.pdf', GCT_DK),
    tsv_line('datasheets/switch/tactile/gct/swt0105.pdf', 'https://gct.co/files/drawings/swt0105.pdf')], 'Switch-CS1213.pdf')
p = D + 'gen-parts.py'
s = open(p).read()
old = '("Würth", "Würth Elektronik"), ("Test Maker", "Test Maker")]'
assert s.count(old) == 1
open(p, 'w').write(s.replace(old, '("Würth", "Würth Elektronik"), ("CIT Relay", "CIT Relay & Switch"), ("GCT", "GCT"),\n          ("Test Maker", "Test Maker")]'))
print('ok')
