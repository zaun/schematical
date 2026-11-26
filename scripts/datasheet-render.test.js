// Tests for schematical/schematical/Datasheet/Renderer/datasheet.js, the datasheet window's renderer,
// run in node against the fake DOM (scripts/test-support/fake-dom.js).
// Run: node --test scripts/datasheet-render.test.js

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const { page, datasheets, payload } = require('./test-support/datasheet-page.js');

// Values made in the page's realm, as plain node values (deepStrictEqual compares prototypes).
const plain = (v) => JSON.parse(JSON.stringify(v));
const CITE =/\[\[(\d+)::([^\[\]]+)\]\]/g;

// The body's top-level lines outside fences, and the fences' kinds.
function outline(markdown) {
  const lines = markdown.split('\n');
  const start = lines[0] === '---' ? lines.indexOf('---', 1) + 1 : 0;
  const prose = [];
  const fences = [];
  for (let i = start; i < lines.length; i++) {
    const open = /^(`{3,})\s*(\S*)/.exec(lines[i]);
    if (!open) { prose.push(lines[i]); continue; }
    let j = i + 1;
    while (j < lines.length && !lines[j].startsWith(open[1])) j++;
    fences.push(open[2]);
    i = j;
  }
  return { prose, fences };
}

test('every shipped datasheet renders its headings, tables, diagrams and citations', () => {
  const dom = page(false);
  const files = datasheets();
  assert.ok(files.length > 400, files.length + ' datasheets');
  for (const file of files) {
    const p = payload(file, { showCitations: true });
    dom.window.renderDatasheet(p);
    const where = p.path;
    const { prose, fences } = outline(p.markdown);
    assert.strictEqual(dom.page.querySelector('pre.error'), null, where + ': rendered without an error');

    const headings = prose.filter((l) => /^##\s/.test(l)).map((l) => l.replace(/^##\s+/, '').trim());
    const h2 = dom.page.querySelectorAll('article h2');
    assert.deepStrictEqual(h2.map((h) => h.textContent.trim()).length, headings.length, where + ': h2 count');
    const nav = dom.page.querySelectorAll('nav.outline a');
    assert.deepStrictEqual(nav.map((a) => a.getAttribute('href')), h2.map((h) => '#' + h.id), where + ': outline');

    const count = (kind) => fences.filter((k) => k === kind).length;
    assert.strictEqual(dom.page.querySelectorAll('figure.ds-table').length, count('table'), where + ': tables');
    assert.strictEqual(dom.page.querySelectorAll('figure.ds-wave svg').length, count('wavedrom'), where + ': wavedrom');
    assert.strictEqual(dom.page.querySelectorAll('figure.ds-curve svg').length, count('curve'), where + ': curves');
    assert.strictEqual(dom.page.querySelectorAll('table.ordering').length, count('ordering'), where + ': ordering');
    assert.strictEqual(dom.page.querySelectorAll('table.pins').length >= count('pins'), true, where + ': pins');

    const chips = new Set(dom.page.querySelectorAll('a.cite').map((a) => a.dataset.page + '::' + a.dataset.ref));
    for (const line of prose) {
      for (const m of line.matchAll(CITE)) assert.ok(chips.has(m[1] + '::' + m[2]), where + ': chip for ' + m[0]);
    }
    assert.strictEqual(dom.document.title, JSON.parse(p.markdown.split('\n---\n')[0].slice(4)).title || '');
  }
});

// ---- Targeted cases over a small made-up datasheet ----------------------------------

function sheet(front, body) {
  return '---\n' + JSON.stringify(Object.assign({ format: 'schematical-datasheet', title: 'T' }, front)) + '\n---\n' + body.join('\n');
}

const FENCE = '```';

test('citations are hidden, with the space before them, outside the Verify window', () => {
  const dom = page(false);
  dom.window.renderDatasheet({ markdown: sheet({}, ['Text one. [[1::2]]', '', '- An item [[3::Table 5-1]]']) });
  assert.strictEqual(dom.page.querySelector('article p').textContent, 'Text one.');
  assert.strictEqual(dom.page.querySelector('article li').textContent, 'An item');
  assert.strictEqual(dom.page.querySelectorAll('a.cite').length, 0);
});

test('a citation chip posts its page, reference and its paragraph text', () => {
  const dom = page(false);
  dom.window.renderDatasheet({ showCitations: true, markdown: sheet({}, [
    'Fanout up to **ten** loads. [[1::2]]', '',
    FENCE + 'table id=t1', '| A | B |', '|---|---|', '| x [[4::Table 2]] | y |', FENCE,
  ]) });
  const chip = dom.page.querySelector('p a.cite');
  assert.strictEqual(chip.textContent, 'p1 ¶2');
  chip.click();
  assert.deepStrictEqual(dom.posted.cite, [{ page: 1, ref: '2', text: 'Fanout up to ten loads.' }]);
  dom.page.querySelector('td a.cite').click();
  assert.deepStrictEqual(dom.posted.cite[1], { page: 4, ref: 'Table 2', text: '' });
  dom.page.querySelector('p').click();
  assert.strictEqual(dom.posted.cite.length, 2, 'a click off a chip posts nothing');
  delete dom.window.webkit;
  chip.click();
  assert.strictEqual(dom.posted.cite.length, 2, 'no handler, no message');
});

test('edit mode: blocks carry their source lines, fences are wrapped, the outline stays', () => {
  const dom = page(false);
  const seen = [];
  dom.window.dsEdit = { rendered: (p, info) => seen.push(info.blocks) };
  const markdown = sheet({}, ['## Description', '', 'Para.', '', FENCE + 'equation', 'Y = A', FENCE, '', '## Pinout']);
  dom.window.renderDatasheet({ markdown, editing: true });
  const lines = dom.page.querySelectorAll('[data-l0]').map((el) => el.localName + ':' + el.dataset.l0 + '-' + el.dataset.l1);
  assert.deepStrictEqual(lines, ['h2:0-1', 'p:2-3', 'div:4-7', 'h2:8-9']);
  assert.ok(dom.page.querySelector('div.ds-blk div.equations'));
  assert.deepStrictEqual(plain(seen[0].map((b) => b.type)), ['heading_open', 'paragraph_open', 'fence', 'heading_open']);
  assert.deepStrictEqual(dom.page.querySelectorAll('nav.outline a').map((a) => a.textContent), ['Description', 'Pinout']);
  assert.ok(dom.document.body.classList.contains('editing'));
});

test('the editor interface: previews keep the page counters, blocks of a body', () => {
  const dom = page(false);
  dom.window.renderDatasheet({ markdown: sheet({}, [FENCE + 'table', '| A |', '|---|', '| 1 |', FENCE]) });
  assert.strictEqual(dom.page.querySelector('figure.ds-table').id, 'table-1');
  const R = dom.window.dsRender;
  const a = R.renderBlock('table', '| A |\n|---|\n| 2 |', {});
  assert.strictEqual(a, R.renderBlock('table', '| A |\n|---|\n| 2 |', {}), 'the counter is put back');
  assert.match(a, /id="table-2"/);
  assert.deepStrictEqual(plain(R.parseBlocks('# H\n\npara\n\n- a\n- b')),
    [{ l0: 0, l1: 1, type: 'heading_open' }, { l0: 2, l1: 3, type: 'paragraph_open' }, { l0: 4, l1: 6, type: 'bullet_list_open' }]);
  assert.strictEqual(R.ctx().front.title, 'T');
  assert.strictEqual(R.md.render('x', { noBlocks: true }), '<p>x</p>\n');
});

test('who verified the conversion', () => {
  const dom = page(false);
  const verified = (front) => {
    dom.window.renderDatasheet({ markdown: sheet(front, ['x']) });
    return dom.page.querySelector('.verified').textContent;
  };
  assert.strictEqual(verified({}), 'Unverified');
  assert.strictEqual(verified({ verifiedDate: '2026-11-01', verifiedBy: 'A B' }), 'Verified on 2026-11-01 by A B');
  assert.strictEqual(verified({ verifiedBy: 'A B' }), 'Verified by A B');
  assert.strictEqual(verified({ verifiedDate: '2026-11-01' }), 'Verified on 2026-11-01');
});

test('a table that does not apply to the part the window was opened for', () => {
  const dom = page(false);
  dom.window.renderDatasheet({ part: 'SN74X00', markdown: sheet({}, [
    FENCE + 'table id=mil parts="SN54*, SNJ54*" caption="Military"', '| A |', '|---|', '| 1 |', FENCE,
    FENCE + 'table id=com parts=SN74*', '| A |', '|---|', '| 1 |', FENCE,
  ]) });
  const [mil, com] = dom.page.querySelectorAll('figure.ds-table');
  assert.ok(mil.classList.contains('not-applicable'));
  assert.strictEqual(mil.querySelector('.applies').textContent, "Doesn't apply to SN74X00 (applies to SN54*, SNJ54*)");
  assert.ok(!com.classList.contains('not-applicable'));
  assert.strictEqual(dom.page.querySelector('.opened').textContent, 'Opened for SN74X00');
});

test('footprint drawings: round bodies, balls, slots, bare holes, pads on all four sides', () => {
  const dom = page(false);
  const pad = (d, x, y, more) => Object.assign({ d, x, y, w: 1, h: 1, drill: 0, slot: 0, plated: true, shape: 'rect' }, more);
  const footprints = {
    BGA: { description: 'balls', body: { w: 4, h: 4 }, round: false, pads: [pad('A1', -1, -1, { shape: 'round' }), pad('B2', 1, 1, { shape: 'round' })] },
    TO: { description: 'round can', body: { w: 6, h: 6 }, round: true, pads: [
      pad('1', -2, 0, { drill: 0.8, shape: 'round' }), pad('2', 0, -2, { drill: 0.8, slot: 2, w: 3, h: 1.2 }),
      pad('3', 2, 0, { drill: 0.8, slot: 2, w: 1.2, h: 3 }), pad('4', 0, 2, { drill: 0.8 }), pad(null, 0, 0, { drill: 1, plated: false })] },
  };
  const device = { id: 'd', kind: 'x', description: 'Dev', pinouts: {
    bga: [{ ball: 'A1', name: 'OE#', role: 'input' }, { ball: 'B2', name: 'GND', role: 'ground' }, { ball: 'C3', name: 'X', role: 'absent' }],
    can: [{ number: 1, name: 'A', role: 'input' }, { number: 2, name: 'B#', role: 'output' }, { number: 3, name: 'C', role: 'power' }, { number: 4, name: 'D', role: 'nc' }],
  }, parts: [{ name: 'P1', pinout: 'bga', footprint: 'BGA' }, { name: 'P2', pinout: 'can', footprint: 'TO', package: 'TO-5', flags: ['obsolete'] }] };
  dom.window.renderDatasheet({ part: 'P2', devices: [device], footprints, markdown: sheet({ parts: ['P*'], packages: { can: 'LMC' } }, [
    FENCE + 'pinout', FENCE, FENCE + 'package footprint=TO', FENCE, FENCE + 'package footprint=BGA', FENCE,
    FENCE + 'pins', 'A, B#: Inputs', 'OE#: Enable', FENCE, FENCE + 'ordering', FENCE,
  ]) });
  const svgs = dom.page.querySelectorAll('figure.ds-pinout svg');
  assert.strictEqual(svgs.length, 2);
  const names = (svg) => svg.querySelectorAll('text').map((t) => t.getAttribute('class') + ':' + t.textContent);
  assert.deepStrictEqual(names(svgs[0]), ['num:A1', 'name:OE', 'num:B2', 'name:GND']);
  assert.strictEqual(svgs[0].querySelector('tspan').getAttribute('text-decoration'), 'overline');
  assert.deepStrictEqual(names(svgs[1]), ['name:A', 'num:1', 'name:B', 'num:2', 'name:C', 'num:3', 'name:D', 'num:4']);
  assert.ok(svgs[1].querySelector('ellipse.body'), 'a round body');
  assert.strictEqual(svgs[1].querySelectorAll('rect.drill').length, 2, 'two slots');
  assert.strictEqual(svgs[1].querySelectorAll('circle.drill').length, 3, 'two round drills and the bare hole');
  assert.strictEqual(svgs[1].querySelectorAll('.pad').length, 4, 'the bare hole has no copper');
  const can = dom.page.querySelector('#package-to');
  assert.strictEqual(can.querySelector('.facts').innerHTML, 'Pads: 4, 1 × 1 mm, drill 0.8 mm<br>Pitch: 4 mm<br>Pad centres, side to side: 4 mm<br>Body: 6 × 6 mm');
  assert.strictEqual(can.querySelector('.used-by').textContent, 'Used by P2');
  assert.strictEqual(dom.page.querySelector('#package-bga .facts').innerHTML,
    'Pads: 2, 1 × 1 mm<br>Pad centres, side to side: 2 mm<br>Body: 4 × 4 mm');
  const rows = dom.page.querySelectorAll('table.pins tbody tr').map((tr) => tr.children.map((td) => td.textContent).join('|'));
  assert.deepStrictEqual(rows, ['A|—|1|I|Inputs', 'B|—|2|O', 'OE|A1|—|I|Enable']);
  const order = dom.page.querySelectorAll('table.ordering tbody tr');
  assert.deepStrictEqual(order.map((tr) => tr.textContent), ['P1bgaBGA', 'P2TO-5LMCTOobsolete']);
  assert.ok(order[1].classList.contains('current'));
});

test('broken blocks show an error in their place; a broken file shows one for the page', () => {
  const dom = page(false);
  dom.window.renderDatasheet({ markdown: sheet({}, [
    FENCE + 'wavedrom id=w caption="Timing"', '{not json', FENCE, FENCE + 'curve', '{', FENCE,
    FENCE + 'figure ref=nowhere id=f', 'R: 10k', 'C: 1n', FENCE,
    FENCE + 'figure ref=there', 'R: 10k', FENCE,
  ]), figures: { there: '<svg><text>R~1~ = {R}, {C}</text></svg>' } });
  assert.match(dom.page.querySelector('#w pre.error').textContent, /SyntaxError/);
  assert.strictEqual(dom.page.querySelector('#w .caption').textContent, 'Timing');
  const errors = dom.page.querySelectorAll('pre.error');
  assert.strictEqual(errors.length, 2, 'the wave and the curve');
  assert.match(errors[1].textContent, /SyntaxError/);
  assert.strictEqual(dom.page.querySelector('figure.ds-curve'), null);
  assert.strictEqual(dom.page.querySelector('#f .missing').textContent, 'Figure "nowhere" is not in the app');
  assert.strictEqual(dom.page.querySelector('#f .facts').textContent, 'R = 10k, C = 1n');
  const filled = dom.page.querySelectorAll('figure.ds-figure')[1];
  assert.strictEqual(filled.querySelector('text').innerHTML, 'R<tspan baseline-shift="sub" font-size="75%">1</tspan> = 10k, —');
  assert.strictEqual(filled.querySelector('.facts'), null);

  dom.window.renderDatasheet({ markdown: '---\n{oops\n---\nx' });
  assert.match(dom.page.querySelector('pre.error').textContent, /^This datasheet could not be shown: SyntaxError/);
});
