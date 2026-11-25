// Tests for the model half of schematical/schematical/Datasheet/Renderer/datasheet-edit.js (the Verify
// Datasheet window's inline editor). Run: node --test scripts/datasheet-edit.test.js

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const path = require('path');

const model = require('../schematical/schematical/Datasheet/Renderer/datasheet-edit.js');
const DATASHEETS = path.join(__dirname, '..', 'schematical', 'schematical', 'Datasheets');

function datasheets() {
  const out = [];
  (function walk(dir) {
    for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
      const file = path.join(dir, entry.name);
      if (entry.isDirectory()) walk(file);
      else if (file.endsWith('.md')) out.push(file);
    }
  })(DATASHEETS);
  return out;
}

// The fenced blocks of a file: {kind, info, inner}.
function fences(file) {
  const lines = fs.readFileSync(file, 'utf8').split('\n');
  const out = [];
  for (let i = 0; i < lines.length; i++) {
    const open = /^(`{3,})(.*)$/.exec(lines[i]);
    if (!open) continue;
    let j = i + 1;
    while (j < lines.length && !lines[j].startsWith(open[1])) j++;
    out.push({ kind: open[2].trim().split(/\s+/)[0], info: open[2].trim(), inner: lines.slice(i + 1, j), at: i + 1 });
    i = j;
  }
  return out;
}

test('citations: split off a line, written back, checked', () => {
  const split = model.splitTrailingCites('Supports fanout up to 10 LSTTL loads. [[1::2]] [[3::Table 5-1]]');
  assert.strictEqual(split.text, 'Supports fanout up to 10 LSTTL loads.');
  assert.deepStrictEqual(split.cites, [{ page: '1', ref: '2' }, { page: '3', ref: 'Table 5-1' }]);
  assert.strictEqual(model.citeText(split.cites), '[[1::2]] [[3::Table 5-1]]');
  assert.deepStrictEqual(model.splitTrailingCites('No citation here.'), { text: 'No citation here.', cites: [] });
  assert.ok(model.isCiteLine('[[3::Table 5-1]]'));
  assert.ok(!model.isCiteLine('Text [[3::2]]'));

  assert.deepStrictEqual(model.checkCites([{ page: '4', ref: '2' }, { page: '5', ref: 'Table 6-3' }, { page: '5', ref: 'table #2' }], 9),
    { errors: [], warnings: [] });
  assert.strictEqual(model.checkCites([{ page: '0', ref: '2' }], 9).errors.length, 1);
  assert.strictEqual(model.checkCites([{ page: 'x', ref: '' }], 9).errors.length, 2);
  assert.strictEqual(model.checkCites([{ page: '12', ref: '2' }], 9).warnings.length, 1);
  assert.strictEqual(model.checkCites([{ page: '2', ref: 'second one' }], 9).warnings.length, 1);
});

test('fence info: attributes keep their order and quoting', () => {
  const info = 'table id=roc caption="Recommended operating conditions" parts=SN74* conditions="Over temperature"';
  const parsed = model.parseInfo(info);
  assert.strictEqual(parsed.kind, 'table');
  assert.deepStrictEqual(parsed.attrs.map((a) => a.key), ['id', 'caption', 'parts', 'conditions']);
  assert.strictEqual(model.formatInfo(parsed), info);
  parsed.attrs[1].value = 'New caption';
  assert.strictEqual(model.formatInfo(parsed), 'table id=roc caption="New caption" parts=SN74* conditions="Over temperature"');
  assert.strictEqual(model.parseInfo('table id=a stray words'), null, 'text it cannot write back is not parsed');
});

test('table: parse, edit a cell, only that row changes', () => {
  const lines = [
    '| Symbol | Parameter | Min | Max | Unit |',
    '|---|---|--:|--:|---|',
    '| V~CC~ | Supply voltage | 2 | 6 | V |',
    '| ^ | ^ | 3 | | V |',
    '',
    '[1] A note.',
  ];
  const t = model.parseTable(lines);
  assert.ok(t);
  assert.strictEqual(t.headerCount, 1);
  assert.strictEqual(t.rows.length, 3);
  assert.deepStrictEqual(model.serializeTable(t), lines);

  t.rows[1].cells[3] = '7';
  const out = model.serializeTable(t);
  assert.strictEqual(out[2], '| V~CC~ | Supply voltage | 2 | 7 | V |');
  assert.deepStrictEqual(out.filter((l, i) => i !== 2), lines.filter((l, i) => i !== 2));
});

test('table: rows and columns', () => {
  const t = model.parseTable(['| A | B |', '|---|--:|', '| 1 | 2 |']);
  model.insertRow(t, 2, false);
  t.rows[2].cells = ['3', ''];
  model.insertColumn(t, 1);
  t.rows[0].cells[1] = 'M';
  assert.deepStrictEqual(model.serializeTable(t), ['| A | M | B |', '|---|---|--:|', '| 1 | | 2 |', '| 3 | | |']);
  assert.ok(model.deleteColumn(t, 1));
  assert.ok(model.deleteRow(t, 2));
  assert.deepStrictEqual(model.serializeTable(t), ['| A | B |', '|---|--:|', '| 1 | 2 |']);
  assert.ok(!model.deleteRow(t, 0), 'the last header row stays');
  t.delim.cells[0] = model.delimFor('center');
  assert.strictEqual(model.serializeTable(t)[1], '|:-:|--:|');
});

test('table: notes and a trailing blank line round trip', () => {
  const withNotes = ['| A |', '|---|', '| 1 [1] |', '', '[1] One.', '[2] Two.'];
  assert.deepStrictEqual(model.serializeTable(model.parseTable(withNotes)), withNotes);
  const trailing = ['| A |', '|---|', '| 1 |', ''];
  assert.deepStrictEqual(model.serializeTable(model.parseTable(trailing)), trailing);
  assert.strictEqual(model.parseTable(['| A |', '|---|', 'stray text']), null);
  assert.strictEqual(model.parseTable(['| A |', '| 1 |']), null, 'no delimiter row');
});

test('table checks follow check.py', () => {
  const t = model.parseTable([
    '| Symbol | Parameter | Min | Max | Unit |',
    '|---|---|--:|--:|---|',
    '| V~CC~ | Supply voltage [1] | 2 | abc | V |',
    '| V~I~ | Input | ^ | | V |',
    '',
    '[2] Never used.',
  ]);
  const result = model.checkTable(t, { caption: 'Ratings' }, 'Recommended operating conditions');
  assert.deepStrictEqual(result.errors, []);
  assert.ok(result.warnings.some((w) => w.includes('Marker [1] has no note')));
  assert.ok(result.warnings.some((w) => w.includes('Note [2] has no marker')));
  assert.ok(result.warnings.some((w) => w.includes('"abc"')));
  // outside a spec section values are not checked
  assert.ok(!model.checkTable(t, {}, 'Function tables').warnings.some((w) => w.includes('"abc"')));
  t.rows[1].cells[1] = 'a|b';
  assert.ok(model.checkTable(t, {}, '').errors.some((e) => e.includes('|')));
});

test('lists: items, citations, indents', () => {
  const lines = ['- First item that wraps', '  over two lines. [[1::2]]', '- Second item. [[1::3]] [[2::1]]'];
  const list = model.parseList(lines);
  assert.ok(list);
  assert.strictEqual(list.items.length, 2);
  assert.deepStrictEqual(list.items[1].cites, [{ page: '1', ref: '3' }, { page: '2', ref: '1' }]);
  assert.deepStrictEqual(model.serializeList(list), lines);
  list.items[0].lines[1] = 'over two lines, edited.';
  assert.strictEqual(model.serializeList(list)[1], '  over two lines, edited. [[1::2]]');
  assert.strictEqual(model.parseList(['- A', '  - nested']), null);
  const numbered = model.parseList(['1. One', '2. Two']);
  numbered.items.splice(0, 1);
  assert.deepStrictEqual(model.serializeList(numbered, true), ['1. Two']);
});

test('grids: pins and ordering lines', () => {
  const pins = ['1A: Channel 1, Input A', 'GND, VSS: Ground'];
  const rows = model.parseGrid('pins', pins);
  assert.deepStrictEqual(rows.map((r) => r.cells), [['1A', 'Channel 1, Input A'], ['GND, VSS', 'Ground']]);
  assert.deepStrictEqual(model.serializeGrid('pins', rows), pins);
  rows[0].cells[1] = 'Input A';
  assert.strictEqual(model.serializeGrid('pins', rows)[0], '1A: Input A');
  assert.strictEqual(model.parseGrid('pins', ['no colon']), null);

  const ordering = ['SN74HC00N | Tube of 25 | -40 to 85 | SN74HC00N', 'CD4081BMT | | -55 to 125 | CD4081BM | Obsolete', 'JM38510 | 1 | |'];
  const grid = model.parseGrid('ordering', ordering);
  assert.deepStrictEqual(model.serializeGrid('ordering', grid), ordering);
  grid.push({ cells: ['NEW', 'Reel', '', '', ''], origCells: null, orig: undefined });
  assert.strictEqual(model.serializeGrid('ordering', grid)[3], 'NEW | Reel', 'a new row does not end in empty cells');
});

test('units: a fence takes the line of citations after it', () => {
  const lines = ['## Pin functions', '', '```pins', '1A: Input', '```', '[[3::Table 5-1]]', '', 'Prose. [[4::1]]'];
  const blocks = [
    { l0: 0, l1: 1, type: 'heading_open' },
    { l0: 2, l1: 5, type: 'fence' },
    { l0: 5, l1: 6, type: 'paragraph_open' },
    { l0: 7, l1: 8, type: 'paragraph_open' },
  ];
  const fence = model.describeUnit(lines, blocks, 1);
  assert.strictEqual(fence.kind, 'fence');
  assert.strictEqual(fence.l1, 6);
  assert.deepStrictEqual(fence.cites, [{ page: '3', ref: 'Table 5-1' }]);
  assert.deepStrictEqual(fence.body, ['1A: Input']);
  assert.strictEqual(model.ownerIndex(lines, blocks, 2), 1);
  assert.strictEqual(model.ownerIndex(lines, blocks, 3), 3);
  const prose = model.describeUnit(lines, blocks, 3);
  assert.strictEqual(prose.kind, 'prose');
  assert.deepStrictEqual(prose.text, ['Prose.']);
  assert.strictEqual(model.describeUnit(lines, blocks, 0).kind, 'readonly');
  assert.strictEqual(model.sectionAt(lines, 3), 'Pin functions');
});

test('every table, pins, ordering block and list of every shipped datasheet round trips', () => {
  let tables = 0;
  const failures = [];
  for (const file of datasheets()) {
    const name = path.relative(DATASHEETS, file);
    for (const block of fences(file)) {
      const where = name + ':' + block.at;
      if (block.kind === 'table') {
        tables++;
        const info = model.parseInfo(block.info);
        const table = model.parseTable(block.inner);
        if (!info) failures.push('table attributes ' + where);
        if (!table) failures.push('table ' + where);
        else if (model.serializeTable(table).join('\n') !== block.inner.join('\n')) failures.push('table text ' + where);
      } else if (block.kind === 'pins' || block.kind === 'ordering') {
        const rows = model.parseGrid(block.kind, block.inner);
        if (!rows) failures.push(block.kind + ' ' + where);
        else if (model.serializeGrid(block.kind, rows).join('\n') !== block.inner.join('\n')) failures.push(block.kind + ' text ' + where);
      }
    }
  }
  assert.ok(tables > 1000, 'found the tables: ' + tables);
  assert.deepStrictEqual(failures.slice(0, 10), []);
});
