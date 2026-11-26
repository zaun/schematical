// Tests for the block editors of schematical/schematical/Datasheet/Renderer/datasheet-edit.js: lists,
// fenced blocks as text, the pins and ordering grids, and tables. The frame, prose and citations are
// in datasheet-editor.test.js. Run: node --test scripts/datasheet-editor-blocks.test.js

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const { setup, at, FENCE, BODY } = require('./test-support/datasheet-editor-fixture.js');

/** The body lines from the line starting with `prefix` (n lines). */
function linesAt(t, prefix, n) {
  const body = t.body();
  const i = body.findIndex((l) => l.startsWith(prefix));
  return body.slice(i, i + n);
}

test('lists: type an item, add, remove, renumber, citations per item', () => {
  const t = setup();
  let frame = t.open(at('- Item a'));
  assert.strictEqual(frame.querySelector('.ds-ed-title').textContent, 'Edit list');
  const items = () => frame.querySelectorAll('.ds-ed-item');
  assert.deepStrictEqual(items().map((i) => i.querySelector('.ds-ed-item-n').textContent), ['•', '•']);
  assert.strictEqual(t.dom.document.activeElement, items()[0].querySelector('textarea'));
  assert.strictEqual(items()[0].querySelector('textarea').value, 'Item a');
  assert.deepStrictEqual(frame.querySelectorAll('.ds-ed-preview-box li').map((li) => li.textContent), ['Item a  p1 ¶3', 'Item b']);

  t.button(items()[0], 'Add item below')[0].click();
  t.dom.flush();
  assert.strictEqual(items().length, 3);
  assert.strictEqual(t.dom.document.activeElement, items()[1].querySelector('textarea'));
  assert.deepStrictEqual(t.messages(), ['error: Item 2 is empty.']);
  t.type(items()[1].querySelector('textarea'), 'New  \n');
  t.button(items()[2], 'Add citation')[0].click();
  t.type(items()[2].querySelector('.ds-cite-page'), '12');
  t.type(items()[2].querySelector('.ds-cite-ref'), '3');
  assert.deepStrictEqual(t.messages(), ['warning: Item 3: Citation 12::3: the PDF has only 9 pages.']);
  t.type(items()[0].querySelector('.ds-cite-page'), 'x');
  assert.deepStrictEqual(t.messages().slice(0, 1), ['error: Item 1: Citation x::3: the page must be a number from 1.']);
  t.type(items()[0].querySelector('.ds-cite-page'), '1');
  t.apply();
  assert.deepStrictEqual(linesAt(t, '- Item a', 3), ['- Item a [[1::3]]', '- New', '- Item b [[12::3]]']);

  frame = t.open(at('- Item a'));
  t.button(items()[1], 'Remove')[0].click();
  t.button(items()[1], 'Remove')[0].click();
  t.dom.flush();
  assert.strictEqual(items().length, 1);
  t.button(items()[0], 'Remove')[0].click();
  assert.strictEqual(items().length, 1, 'the last item stays');
  t.apply();
  assert.deepStrictEqual(linesAt(t, '- Item a', 2), ['- Item a [[1::3]]', '']);

  frame = t.open(at('1. First') - 1); // the list above is a line shorter now
  assert.deepStrictEqual(items().map((i) => i.querySelector('.ds-ed-item-n').textContent), ['1.', '2.']);
  t.button(items()[1], 'Add item below')[0].click();
  t.type(items()[2].querySelector('textarea'), 'Third');
  t.apply();
  assert.deepStrictEqual(linesAt(t, '1. First', 3), ['1. First', '2. Second', '3. Third']);
});

test('a fence edited as text: attributes, JSON checks, a citation line after it', () => {
  const t = setup();
  const frame = t.open(at(FENCE + 'wavedrom'));
  assert.strictEqual(frame.querySelector('.ds-ed-title').textContent, 'Edit wavedrom block');
  assert.strictEqual(frame.querySelector('.ds-ed-field span').textContent, 'wavedrom attributes');
  const attrs = frame.querySelector('.ds-ed-attrs');
  const area = frame.querySelector('textarea.code');
  assert.strictEqual(attrs.value, 'id=w1');
  assert.strictEqual(t.dom.document.activeElement, area);
  assert.ok(frame.querySelector('.ds-ed-preview-box figure.ds-wave svg'), 'a WaveDrom preview');
  t.type(area, '{"signal": [');
  assert.match(t.messages()[0], /^error: Not valid JSON: /);
  assert.strictEqual(frame.querySelector('.ds-ed-preview-box'), null);
  t.type(area, '');
  assert.strictEqual(t.messages()[0], 'error: The block is empty.');
  t.type(area, '{"signal": []}');
  t.type(attrs, ' id=w2 caption="Two" ');
  t.button(frame, 'Add citation')[0].click();
  t.type(frame.querySelector('.ds-cite-page'), '5');
  t.type(frame.querySelector('.ds-cite-ref'), 'Figure 2');
  t.button(frame, 'Show in PDF')[0].click();
  assert.deepStrictEqual(t.dom.posted.cite.pop(), { page: 5, ref: 'Figure 2', text: '' });
  t.apply();
  assert.deepStrictEqual(linesAt(t, FENCE + 'wavedrom', 4),
    [FENCE + 'wavedrom id=w2 caption="Two"', '{"signal": []}', FENCE, '[[5::Figure 2]]']);
});

test('fences the grids and the table editor cannot read fall back to text', () => {
  const t = setup();
  let frame = t.open(at(FENCE + 'table id=bad'));
  assert.strictEqual(frame.querySelector('.ds-ed-title').textContent, 'Edit table block');
  assert.strictEqual(frame.querySelector('.ds-ed-preview-box pre.error').textContent, 'Table without a delimiter row');
  t.type(frame.querySelector('.ds-ed-attrs'), 'id="open');
  assert.strictEqual(frame.querySelector('.ds-ed-preview-box pre.error').textContent, 'Table without a delimiter row');
  frame.querySelector('.ds-ed-cancel').click();
  frame = t.open(at(FENCE + 'pins', 1));
  assert.strictEqual(frame.querySelector('.ds-ed-title').textContent, 'Edit pins block');
  t.type(frame.querySelector('textarea'), 'A: In');
  t.apply();
  assert.deepStrictEqual(t.body().slice(-3), [FENCE + 'pins', 'A: In', FENCE]);
});

test('the pins grid: cells, pin names from the catalog, rows moved, added, deleted, keys', () => {
  const t = setup();
  const frame = t.open(at(FENCE + 'pins'));
  assert.strictEqual(frame.querySelector('.ds-ed-title').textContent, 'Edit pin descriptions');
  assert.deepStrictEqual(frame.querySelectorAll('table.ds-ed-grid th').map((th) => th.textContent), ['', 'Pin name(s)', 'Description', '']);
  const cells = () => frame.querySelectorAll('table.ds-ed-grid tr').slice(1).map((tr) => tr.querySelectorAll('input'));
  const values = () => cells().map((r) => r.map((i) => i.value).join(': '));
  assert.ok(cells()[0][1].classList.contains('wide'));
  assert.strictEqual(t.dom.document.activeElement, cells()[0][0]);
  assert.ok(frame.querySelector('.ds-ed-preview-box table.pins'));

  t.type(cells()[0][0], 'A, Q');
  t.type(cells()[1][1], '');
  assert.deepStrictEqual(t.messages(), ['warning: "Q" is not a pin name of the catalog parts.', 'warning: Row 2 has no description.']);
  t.type(cells()[1][0], ' ');
  assert.deepStrictEqual(t.messages()[0], 'error: Row 2 has no pin name.');
  t.type(cells()[1][0], 'Y');
  t.type(cells()[1][1], 'Out');

  const rowButton = (r, title) => frame.querySelectorAll('table.ds-ed-grid tr')[r + 1].querySelectorAll('button').find((b) => b.title === title);
  rowButton(0, 'Move up').click();
  assert.deepStrictEqual(values(), ['A, Q: Input', 'Y: Out'], 'the first row cannot move up');
  rowButton(0, 'Move down').click();
  assert.deepStrictEqual(values(), ['Y: Out', 'A, Q: Input']);
  rowButton(1, 'Move down').click();
  rowButton(1, 'Move up').click();
  assert.deepStrictEqual(values(), ['A, Q: Input', 'Y: Out']);
  rowButton(0, 'Add a row below').click();
  assert.deepStrictEqual(values(), ['A, Q: Input', ': ', 'Y: Out']);
  assert.strictEqual(t.dom.document.activeElement, cells()[1][0]);
  rowButton(1, 'Delete this row').click();
  t.button(frame, 'Add row')[0].click();
  assert.deepStrictEqual(values(), ['A, Q: Input', 'Y: Out', ': ']);
  rowButton(2, 'Delete this row').click();

  const focused = () => t.dom.document.activeElement;
  assert.strictEqual(t.key(cells()[0][1], 'ArrowDown').defaultPrevented, true);
  assert.strictEqual(focused(), cells()[1][1]);
  t.key(cells()[1][1], 'Enter', { shiftKey: true });
  assert.strictEqual(focused(), cells()[0][1]);
  t.key(cells()[0][1], 'Enter');
  assert.strictEqual(focused(), cells()[1][1]);
  t.key(cells()[1][1], 'ArrowUp');
  assert.strictEqual(focused(), cells()[0][1]);
  t.key(cells()[0][1], 'ArrowUp');
  assert.strictEqual(focused(), cells()[0][1], 'nothing above the first row');
  assert.strictEqual(t.key(cells()[0][1], 'ArrowDown', { metaKey: true }).defaultPrevented, false);
  assert.strictEqual(t.key(cells()[0][1], 'a').defaultPrevented, false);
  t.apply();
  assert.deepStrictEqual(linesAt(t, FENCE + 'pins', 4), [FENCE + 'pins', 'A, Q: Input', 'Y: Out', FENCE]);

  const one = t.open(at(FENCE + 'pins'));
  one.querySelectorAll('table.ds-ed-grid tr')[2].querySelectorAll('button').find((b) => b.title === 'Delete this row').click();
  one.querySelectorAll('table.ds-ed-grid tr')[1].querySelectorAll('button').find((b) => b.title === 'Delete this row').click();
  assert.strictEqual(one.querySelectorAll('table.ds-ed-grid tr').length, 2, 'the last row stays');
});

test('the ordering grid: claimed parts, the | rule, trailing empty cells', () => {
  const t = setup();
  const frame = t.open(at(FENCE + 'ordering'));
  assert.strictEqual(frame.querySelector('.ds-ed-title').textContent, 'Edit ordering information');
  assert.deepStrictEqual(frame.querySelectorAll('table.ds-ed-grid th').map((th) => th.textContent),
    ['', 'Part', 'Packing', 'Temperature, °C', 'Marking', 'Status', '']);
  const cells = () => frame.querySelectorAll('table.ds-ed-grid tr').slice(1).map((tr) => tr.querySelectorAll('input'));
  t.type(cells()[0][0], 'P9');
  t.type(cells()[0][3], 'a|b');
  assert.deepStrictEqual(t.messages(), ['error: Row 1: a cell cannot contain |.', 'warning: "P9" is not one of the parts this datasheet claims.']);
  t.type(cells()[0][3], 'M1');
  t.button(frame, 'Add row')[0].click();
  t.type(cells()[1][0], 'P2');
  t.apply();
  assert.deepStrictEqual(linesAt(t, FENCE + 'ordering', 4), [FENCE + 'ordering', 'P9 | Tape | -40 to 85 | M1', 'P2', FENCE]);
});

test('grids with no catalog behind them check no names', () => {
  const t = setup({ markdown: BODY.join('\n'), devices: [] });
  let frame = t.open(at(FENCE + 'pins'));
  t.type(frame.querySelector('table.ds-ed-grid input'), 'Q');
  assert.deepStrictEqual(t.messages(), []);
  frame.querySelector('.ds-ed-cancel').click();
  frame = t.open(at(FENCE + 'ordering'));
  t.type(frame.querySelector('table.ds-ed-grid input'), 'P9');
  assert.deepStrictEqual(t.messages(), []);
});

test('tables: attributes, cells, merges, columns, rows, alignment, keys, notes', () => {
  const t = setup();
  const l0 = at(FENCE + 'table id=ec');
  const frame = t.open(l0);
  assert.strictEqual(frame.querySelector('.ds-ed-title').textContent, 'Edit table');
  assert.strictEqual(frame.querySelector('.ds-ed-id').textContent, 'id: ec');
  const field = (label) => frame.querySelectorAll('.ds-ed-attrs-box label').find((l) => l.querySelector('span').textContent === label).querySelector('input');
  assert.deepStrictEqual(frame.querySelectorAll('.ds-ed-attrs-box label span').map((s) => s.textContent), ['Caption', 'Conditions', 'Applies to parts', 'note']);
  assert.strictEqual(field('Caption').value, 'EC');
  const rows = () => frame.querySelectorAll('table.ds-ed-grid tr').filter((tr) => tr.querySelector('input'));
  const cells = () => rows().map((tr) => tr.querySelectorAll('input'));
  const grid = () => cells().map((r) => r.map((i) => i.value).join('|'));
  assert.deepStrictEqual(grid(), ['Parameter|MIN|UNIT', 'V~IH~ [1]|2|V', 'V~IL~|^|V']);
  assert.ok(cells()[2][1].classList.contains('merge'));
  assert.strictEqual(frame.querySelector('tr.ds-ed-delim td').getAttribute('colspan'), '5');
  assert.strictEqual(t.dom.document.activeElement, cells()[0][0]);
  const preview = frame.querySelector('.ds-ed-preview-box figure.ds-table');
  assert.strictEqual(preview.getAttribute('id'), null, 'the preview has no id to clash with the page');

  t.type(cells()[1][1], 'abc');
  assert.deepStrictEqual(t.messages(), ['warning: Row 2: "abc" in min is not a number, ratio or symbol expression.']);
  assert.strictEqual(cells()[1][1].size, 4);
  t.type(cells()[1][1], '<');
  assert.ok(cells()[1][1].classList.contains('merge'));
  t.type(cells()[1][1], '3');
  assert.ok(!cells()[1][1].classList.contains('merge'));

  t.type(field('Caption'), ' New "cap" ');
  assert.deepStrictEqual(t.messages(), ['error: An attribute cannot contain ".']);
  assert.strictEqual(frame.querySelector('.ds-ed-preview-box'), null);
  t.type(field('Caption'), 'New cap');
  t.type(field('Conditions'), '');
  t.type(field('note'), 'y');

  const colTools = () => frame.querySelector('tr.ds-ed-coltools').querySelectorAll('td').slice(1, -1);
  const tool = (c, title) => colTools()[c].querySelectorAll('button').find((b) => b.title.startsWith(title));
  const align = tool(1, 'Column alignment');
  assert.deepStrictEqual([align.textContent, align.title], ['R', 'Column alignment: right (click to change)']);
  align.click();
  assert.deepStrictEqual([align.textContent, align.title], ['C', 'Column alignment: center (click to change)']);
  tool(2, 'Column alignment').click();
  tool(0, 'Column alignment').click();
  tool(0, 'Add a column after').click();
  assert.deepStrictEqual(grid(), ['Parameter||MIN|UNIT', 'V~IH~ [1]||3|V', 'V~IL~||^|V']);
  tool(0, 'Add a column before').click();
  tool(0, 'Delete this column').click();
  tool(1, 'Delete this column').click();
  assert.deepStrictEqual(grid(), ['Parameter|MIN|UNIT', 'V~IH~ [1]|3|V', 'V~IL~|^|V']);

  const rowTool = (r, title) => rows()[r].querySelectorAll('button').find((b) => b.title === title);
  assert.strictEqual(rowTool(0, 'A table needs a header row').textContent, '×');
  rowTool(0, 'A table needs a header row').click();
  assert.strictEqual(rows().length, 3, 'the only header row stays');
  rowTool(0, 'Add a row above').click();
  assert.strictEqual(t.dom.document.activeElement, cells()[0][0]);
  assert.strictEqual(frame.querySelectorAll('tr.header').length, 2);
  t.type(cells()[0][0], 'Group');
  rowTool(0, 'Delete this row').click();
  assert.strictEqual(frame.querySelectorAll('tr.header').length, 1);
  rowTool(2, 'Add a row below').click();
  assert.strictEqual(t.dom.document.activeElement, cells()[3][0]);
  rowTool(3, 'Delete this row').click();

  const focused = () => t.dom.document.activeElement;
  t.key(cells()[0][2], 'ArrowDown');
  assert.strictEqual(focused(), cells()[1][2]);
  t.key(cells()[1][2], 'Enter', { shiftKey: true });
  assert.strictEqual(focused(), cells()[0][2]);
  t.key(cells()[0][2], 'Enter');
  t.key(cells()[1][2], 'ArrowUp');
  assert.strictEqual(focused(), cells()[0][2]);
  t.key(cells()[2][2], 'ArrowDown');
  assert.strictEqual(focused(), cells()[0][2], 'nothing below the last row');
  assert.strictEqual(t.key(cells()[0][2], 'Enter', { altKey: true }).defaultPrevented, false);
  assert.strictEqual(t.key(cells()[0][2], 'Tab').defaultPrevented, false);

  const notes = () => frame.querySelectorAll('.ds-ed-noterow');
  assert.deepStrictEqual(notes().map((n) => n.querySelector('.ds-ed-noten').value + ' ' + n.querySelector('.ds-ed-notetext').value), ['1 Note one.']);
  t.type(notes()[0].querySelector('.ds-ed-noten'), '2');
  assert.deepStrictEqual(t.messages(), ['warning: Marker [1] has no note.', 'warning: Note [2] has no marker.']);
  t.type(notes()[0].querySelector('.ds-ed-noten'), '1');
  t.button(frame, 'Add note')[0].click();
  assert.strictEqual(t.dom.document.activeElement, notes()[1].querySelector('.ds-ed-notetext'));
  assert.strictEqual(notes()[1].querySelector('.ds-ed-noten').value, '2');
  t.dom.flush();
  assert.ok(t.messages().includes('error: Note [2] is empty.'));
  t.type(notes()[1].querySelector('.ds-ed-notetext'), 'Note two.');
  t.type(cells()[2][0], 'V~IL~ [2]');
  t.apply();
  assert.deepStrictEqual(linesAt(t, FENCE + 'table id=ec', 10), [
    FENCE + 'table id=ec caption="New cap" parts=P* note=y',
    '| Parameter | MIN | UNIT |', '|--:|:-:|---|', '| V~IH~ [1] | 3 | V |', '| V~IL~ [2] | ^ | V |', '',
    '[1] Note one.', '[2] Note two.', FENCE, '[[2::Table 6-1]]',
  ]);
});

test('a one-column table: no last column to delete, notes added and removed, new attributes', () => {
  const t = setup();
  const frame = t.open(at(FENCE + 'table id=one'));
  const field = (label) => frame.querySelectorAll('.ds-ed-attrs-box label').find((l) => l.querySelector('span').textContent === label).querySelector('input');
  t.type(field('Caption'), 'One');
  t.type(field('Conditions'), '');
  frame.querySelector('tr.ds-ed-coltools').querySelectorAll('button').find((b) => b.title === 'Delete this column').click();
  assert.strictEqual(frame.querySelectorAll('input.ds-ed-cell').length, 2, 'the last column stays');
  const notes = () => frame.querySelectorAll('.ds-ed-noterow');
  t.button(frame, 'Add note')[0].click();
  t.dom.flush();
  assert.strictEqual(notes()[0].querySelector('.ds-ed-noten').value, '1');
  t.type(notes()[0].querySelector('.ds-ed-noten'), 'x');
  assert.ok(t.messages().includes('error: A note number must be a number.'));
  notes()[0].querySelector('.danger').click();
  assert.strictEqual(notes().length, 0);
  t.apply();
  assert.deepStrictEqual(linesAt(t, FENCE + 'table id=one', 5), [FENCE + 'table id=one caption=One', '| A |', '|---|', '| 1 |', FENCE]);
  const none = setup({ markdown: BODY.join('\n').replace(FENCE + 'table id=one', FENCE + 'table') });
  assert.strictEqual(none.open(at(FENCE + 'table id=one')).querySelector('.ds-ed-id').textContent, 'no id');
});

test('a table under no section: merges with nothing to merge into are warned about', () => {
  const body = [FENCE + 'table id=t', '| A | B |', '|---|---|', '| 1 | 2 |', FENCE];
  const t = setup({ markdown: body.join('\n'), devices: [] });
  const frame = t.open(0);
  const cells = frame.querySelectorAll('input.ds-ed-cell');
  t.type(cells[1], '^');
  t.type(cells[2], '<');
  t.type(cells[3], '^');
  assert.deepStrictEqual(t.messages(), [
    'warning: Row 1: ^ in the first row of the header or body has nothing above it.',
    'warning: Row 2: ^ in the first row of the header or body has nothing above it.',
    'warning: Row 2: < in the first column has nothing to its left.',
  ]);
});
