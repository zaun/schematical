import json
d = json.load(open('/tmp/p68-full-arch.json'))
k = '/Users/justinzaun/Development/logic/schematical/schematical/Canvas/SchematicModel.swift'
old = open('/Users/justinzaun/Development/logic/schematical/schematical/Canvas/SchematicModel.swift').read().split('\n')
miss = [l['line'] for l in d[k] if l['isExecutable'] and l.get('executionCount', 0) == 0]
# boundaries in the old file (main checkout still has it unsplit)
def idx(pred):
    return next(i + 1 for i, l in enumerate(old) if pred(l))
b = {
    'SchematicTool.swift': idx(lambda l: l == '// MARK: - Grid'),
    'ChipLayout.swift': idx(lambda l: l == '// MARK: - Block geometry'),
    'PlacedChip.swift': idx(lambda l: l == '// MARK: - Placed items'),
    'Wire.swift': idx(lambda l: l.startswith('nonisolated struct PinRef')) - 1,
    'OrthogonalRoute.swift': idx(lambda l: l == '// MARK: - Orthogonal routing'),
    'SchematicModel.swift': idx(lambda l: l == '// MARK: - Document'),
}
order = sorted(b.items(), key=lambda x: x[1])
from collections import Counter
c = Counter()
for n in miss:
    name = [nm for nm, start in order if start <= n][-1]
    c[name] += 1
print(len(miss), dict(c))
for n in miss[:400]:
    pass
