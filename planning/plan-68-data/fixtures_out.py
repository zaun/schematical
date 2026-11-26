"""plan-68-H Phase 1: the FIXTURE / FIXTURE_FOOTPRINTS C literals → tests/fixtures/<area>/*.json."""
import os, re, subprocess, sys, tempfile
sys.path.insert(0, '/private/tmp/claude-501/p68')
from csplit import items

T = 'backend/tests/'
for area in ['counter', 'shift', 'latch']:
    path = T + area + '_test.c'
    text = open(path).read()
    found = {}
    for kind, t in items(text):
        m = re.match(r'static const char (FIXTURE|FIXTURE_FOOTPRINTS)\[\] =', t)
        if kind == 'code' and m:
            found[m.group(1)] = t
    assert set(found) == {'FIXTURE', 'FIXTURE_FOOTPRINTS'}, (area, found.keys())
    os.makedirs(T + 'fixtures/' + area, exist_ok=True)
    for name, item in found.items():
        prog = ('#include <stdio.h>\n' + item +
                '\nint main(void) { for (const char *p = ' + name + '; *p; p++) putchar(*p == \'\\\'\' ? \'"\' : *p); return 0; }\n')
        d = tempfile.mkdtemp()
        open(d + '/p.c', 'w').write(prog)
        subprocess.run(['cc', '-o', d + '/p', d + '/p.c'], check=True)
        out = subprocess.run([d + '/p'], capture_output=True, check=True).stdout
        target = T + 'fixtures/' + area + '/' + ('parts.json' if name == 'FIXTURE' else 'footprints.json')
        open(target, 'wb').write(out)
        assert b"'" not in out
        print(area, name, len(out), 'bytes')
    # replace the two literals with file-read pointers
    first = text.index(found['FIXTURE'])
    text = text.replace(found['FIXTURE'],
        f'/* The fixture parts: tests/fixtures/{area}/parts.json (moved out of C by plan-68-H). */\n'
        'static char *FIXTURE;\n', 1)
    text = text.replace(found['FIXTURE_FOOTPRINTS'],
        f'/* Their footprints: tests/fixtures/{area}/footprints.json. */\n'
        'static char *FIXTURE_FOOTPRINTS;\n', 1)
    m = re.search(r'^int main\([^)]*\) \{\n', text, re.M)
    read = (f'    FIXTURE = read_file(GOLDEN_DIR "/../fixtures/{area}/parts.json");\n'
            f'    FIXTURE_FOOTPRINTS = read_file(GOLDEN_DIR "/../fixtures/{area}/footprints.json");\n'
            '    if (FIXTURE == NULL || FIXTURE_FOOTPRINTS == NULL) {\n'
            '        printf("FAIL: cannot read the fixture files\\n");\n'
            '        return 1;\n'
            '    }\n')
    text = text[:m.end()] + read + text[m.end():]
    if '#include "read_file.h"' not in text:
        text = text.replace('#include "parts.h"\n', '#include "parts.h"\n#include "read_file.h"\n', 1)
    open(path, 'w').write(text)
