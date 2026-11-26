"""plan-68-C: every line the full-suite baseline left uncovered in the old
SchematicModel.swift, mapped through plan-68-B's split to its new file and
line, must be covered by the run of C's new test classes.

usage: c_union.py <baseline.xcresult> <new.xcresult>
"""
import json, subprocess, sys
W = '/Users/justinzaun/Development/logic/.claude/worktrees/P-68/'
OLD = 'schematical/schematical/Canvas/SchematicModel.swift'
old = subprocess.run(['git', '-C', W, 'show', '2fedca39^:' + OLD], capture_output=True, text=True, check=True).stdout.split('\n')


def find(pred):
    return next(i for i, l in enumerate(old) if pred(l))  # 0-based


grid = find(lambda l: l == '// MARK: - Grid')
block = find(lambda l: l == '// MARK: - Block geometry')
placed = find(lambda l: l == '// MARK: - Placed items')
pinref = find(lambda l: l.startswith('nonisolated struct PinRef'))
while old[pinref - 1].startswith('///'):
    pinref -= 1
route = find(lambda l: l == '// MARK: - Orthogonal routing')
doc = find(lambda l: l == '// MARK: - Document')
pieces = [(grid, block, 'SchematicTool.swift'), (block, placed, 'ChipLayout.swift'),
          (placed, pinref, 'PlacedChip.swift'), (pinref, route, 'Wire.swift'), (route, doc, 'OrthogonalRoute.swift')]


def new_place(line):  # 1-based old line -> (file, 1-based new line)
    k = line - 1
    for a, b, name in pieces:
        if a <= k < b:
            return name, k - a + 1 + 3  # three import/blank lines on top
    assert k >= doc
    return 'SchematicModel.swift', k - doc + 1 + grid


def archive(bundle):
    out = subprocess.run(['xcrun', 'xccov', 'view', '--archive', '--json', bundle], capture_output=True, text=True, check=True).stdout
    return json.loads(out)


base = archive(sys.argv[1])
new = archive(sys.argv[2])
key_old = next(k for k in base if k.endswith(OLD))
missed = [e['line'] for e in base[key_old] if e['isExecutable'] and e.get('executionCount', 0) == 0]
by_file = {}
for k, entries in new.items():
    if '/schematical/schematical/Canvas/' in k:
        by_file[k.split('/')[-1]] = {e['line']: e.get('executionCount', 0) for e in entries if e['isExecutable']}
still = []
src_new = {}
for line in missed:
    name, n = new_place(line)
    if name not in src_new:
        src_new[name] = open(W + 'schematical/schematical/Canvas/' + name).read().split('\n')
    text_old = old[line - 1].strip()
    # plan-68-C edited ChipLayout's supply(): later lines moved up a little.
    for d in sorted(range(-12, 13), key=abs):
        if 0 < n + d <= len(src_new[name]) and src_new[name][n + d - 1].strip() == text_old:
            n += d
            break
    text_new = src_new[name][n - 1].strip()
    count = by_file.get(name, {}).get(n)
    if count is None and text_old != text_new:
        still.append((line, name, n, 'MAPPING', text_old[:60], text_new[:60]))
    elif not count:
        still.append((line, name, n, count, text_old[:80]))
print(f'baseline uncovered: {len(missed)}; still uncovered under the new classes: {len(still)}')
for s in still:
    print(*s, sep='\t')
