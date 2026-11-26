# plan-68-B: move the value types out of SchematicModel.swift, verbatim.
import re
d = 'schematical/schematical/Canvas/'
src = open(d + 'SchematicModel.swift').read().split('\n')

def find(pred, start=0):
    for i in range(start, len(src)):
        if pred(src[i]):
            return i
    raise SystemExit('marker not found')

grid = find(lambda l: l == '// MARK: - Grid')
block = find(lambda l: l == '// MARK: - Block geometry')
placed = find(lambda l: l == '// MARK: - Placed items')
pinref = find(lambda l: l.startswith('nonisolated struct PinRef'))
while src[pinref - 1].startswith('///'):
    pinref -= 1
route = find(lambda l: l == '// MARK: - Orthogonal routing')
doc = find(lambda l: l == '// MARK: - Document')

head = src[:grid]  # imports and the blank line after them
pieces = {
    'SchematicTool.swift': src[grid:block],
    'ChipLayout.swift': src[block:placed],
    'PlacedChip.swift': src[placed:pinref],
    'Wire.swift': src[pinref:route],
    'OrthogonalRoute.swift': src[route:doc],
}
imports = ['import CoreGraphics', 'import Foundation', '']
for name, body in pieces.items():
    while body and body[-1] == '':
        body = body[:-1]
    open(d + name, 'w').write('\n'.join(imports + body) + '\n')
open(d + 'SchematicModel.swift', 'w').write('\n'.join(head + src[doc:]))
# ChipLayout's memberwise init is used by PlacedChip.layout (now another file).
p = d + 'ChipLayout.swift'
s = open(p).read()
assert s.count('    fileprivate init(size: CGSize, slots: [PinSlot]) {') == 1
s = s.replace('    fileprivate init(size: CGSize, slots: [PinSlot]) {', '    init(size: CGSize, slots: [PinSlot]) {')
open(p, 'w').write(s)
print({k: len(v) for k, v in pieces.items()})
