// Tests for schematical/schematical/Datasheet/Renderer/datasheet-edit.js, the Verify Datasheet
// window's inline editor (the DOM half; datasheet-edit.test.js tests the model), run in node against
// the fake DOM (scripts/test-support/fake-dom.js) with the real renderer and edit model loaded.
// Run: node --test scripts/datasheet-editor.test.js

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const path = require('path');
const { page, payload, DATASHEETS } = require('./test-support/datasheet-page.js');
const { setup, at, FENCE, BODY, MARKDOWN } = require('./test-support/datasheet-editor-fixture.js');

test('rendering in edit mode posts the state and marks the editable blocks', () => {
  const t = setup();
  assert.deepStrictEqual(t.state(), { type: 'state', path: 'x/test.md', markdown: MARKDOWN, modified: false, edited: 0, undo: 0, editing: true });
  const para = t.block(at('Para one'));
  assert.ok(para.classList.contains('ds-editable'));
  assert.strictEqual(para.title, 'Click to edit');
  assert.ok(!t.block(at('# Test')).classList.contains('ds-editable'), 'a heading is not edited');
  assert.ok(!t.block(at(FENCE + 'pinout')).classList.contains('ds-editable'), 'a pinout comes from the catalog');
  assert.ok(t.block(at(FENCE + 'table id=ec')).classList.contains('ds-editable'));
  assert.ok(t.dom.document.body.classList.contains('editing'));
});

test('clicks: not in edit mode, on a control, off a block, on a read-only block', () => {
  const t = setup({ editing: false });
  assert.strictEqual(t.state().editing, false);
  t.dom.page.querySelector('article p').click();
  assert.strictEqual(t.frame(), null, 'not editing: nothing opens');
  t.dom.window.dsEdit.setEditing(true);
  assert.strictEqual(t.state().editing, true);
  assert.deepStrictEqual(t.dom.window.scrolledTo.pop(), [0, 120], 'a re-render keeps the scroll position');
  t.dom.page.querySelector('nav.outline a').click();
  t.dom.page.querySelector('article').click();
  t.block(at('# Test')).click();
  t.block(at(FENCE + 'pinout')).querySelector('div').click();
  assert.strictEqual(t.frame(), null, 'a link, the page, a heading, a pinout: nothing opens');
  const frame = t.open(at('Para one'));
  assert.ok(frame);
  frame.querySelector('textarea').click();
  assert.strictEqual(t.dom.page.querySelectorAll('.ds-editor').length, 1, 'a click inside the editor opens nothing');
});

test('prose: type, preview, apply, undo, revert', () => {
  const t = setup();
  const l0 = at('Para one');
  const frame = t.open(l0);
  assert.strictEqual(frame.querySelector('.ds-ed-title').textContent, 'Edit paragraph');
  assert.strictEqual(t.block(l0).style.display, 'none', 'the block hides under its editor');
  assert.strictEqual(frame.scrolledIntoView, 1);
  const area = frame.querySelector('textarea');
  assert.strictEqual(t.dom.document.activeElement, area);
  assert.strictEqual(area.value, 'Para one, with ~sub~.');
  assert.strictEqual(area.style.height, '42px', 'sized to its text');
  const apply = frame.querySelector('.ds-ed-apply');
  assert.strictEqual(apply.disabled, true);
  assert.strictEqual(apply.title, 'Nothing changed');
  assert.strictEqual(frame.querySelector('.ds-ed-preview-box').innerHTML, '<p>Para one, with <sub>sub</sub>.</p>\n');

  t.type(area, 'Para two.\n\nAnd more.  ');
  assert.deepStrictEqual(t.messages(), ['warning: A blank line starts a new paragraph, which gets no citation.']);
  assert.strictEqual(apply.disabled, false);
  assert.strictEqual(apply.title, 'Put the change in the datasheet (Cmd-Return)');
  t.type(area, '   ');
  assert.deepStrictEqual(t.messages(), ['error: The text is empty.']);
  assert.strictEqual(apply.title, 'Fix the errors first');
  assert.strictEqual(frame.querySelector('.ds-ed-preview-box'), null);
  t.key(area, 'Enter', { metaKey: true });
  assert.ok(t.frame(), 'Cmd-Return with errors keeps the editor open');

  t.type(area, 'Para two.');
  t.apply();
  assert.strictEqual(t.frame(), null);
  assert.strictEqual(t.body()[l0], 'Para two. [[1::2]]');
  assert.deepStrictEqual([t.state().modified, t.state().edited, t.state().undo], [true, 1, 1]);
  assert.ok(t.block(l0).classList.contains('ds-edited'));

  t.dom.window.dsEdit.undo();
  assert.strictEqual(t.body()[l0], 'Para one, with ~sub~. [[1::2]]');
  assert.deepStrictEqual([t.state().modified, t.state().undo], [false, 0]);
  t.dom.window.dsEdit.undo();
  assert.strictEqual(t.state().undo, 0, 'nothing more to undo');
  const posts = t.dom.posted.edit.length;
  t.dom.window.dsEdit.revertAll();
  assert.strictEqual(t.dom.posted.edit.length, posts, 'nothing to revert');

  t.type(t.open(l0).querySelector('textarea'), 'Third.');
  t.key(t.frame(), 'Enter', { ctrlKey: true });
  assert.strictEqual(t.body()[l0], 'Third. [[1::2]]');
  t.dom.window.dsEdit.revertAll();
  assert.deepStrictEqual([t.body()[l0], t.state().modified, t.state().undo], ['Para one, with ~sub~. [[1::2]]', false, 2]);
});

test('the editor frame: Esc, an unchanged commit, a second block, a dirty one flashes', () => {
  const t = setup();
  const l0 = at('Para one');
  let frame = t.open(l0);
  t.key(frame, 'a');
  assert.ok(t.frame(), 'another key does nothing');
  assert.strictEqual(t.key(frame, 'Escape').defaultPrevented, true);
  assert.strictEqual(t.frame(), null);
  assert.strictEqual(t.block(l0).style.display, '', 'the block shows again');

  frame = t.open(l0);
  t.key(frame, 'Enter', { metaKey: true });
  assert.strictEqual(t.frame(), null, 'an unchanged commit just closes');
  assert.strictEqual(t.state().undo, 0);

  t.open(l0);
  t.open(at('> A quote'));
  assert.strictEqual(t.frame().querySelector('.ds-ed-title').textContent, 'Edit block (raw Markdown)', 'an unchanged editor gives way');
  assert.strictEqual(t.frame().querySelector('.ds-ed-note').textContent, 'This block is edited as raw Markdown.');
  t.type(t.frame().querySelector('textarea'), '> A quote.\n>\n> Two.');
  assert.deepStrictEqual(t.messages(), [], 'raw Markdown may have blank lines');
  t.block(l0).click();
  assert.ok(t.frame().classList.contains('flash'), 'a changed editor stays and flashes');
  assert.strictEqual(t.frame().querySelector('.ds-ed-title').textContent, 'Edit block (raw Markdown)');
  t.frame().querySelector('.ds-ed-cancel').click();
  assert.strictEqual(t.frame(), null);
});

test('citations: add, type, take the PDF page, show in the PDF, remove', () => {
  const t = setup();
  const l0 = at('Para one');
  const frame = t.open(l0);
  const rows = () => frame.querySelectorAll('.ds-cite-row');
  assert.strictEqual(rows().length, 1);
  t.button(frame, 'Show in PDF')[0].click();
  assert.deepStrictEqual(t.dom.posted.cite, [{ page: 1, ref: '2', text: 'Para one, with ~sub~.' }]);

  t.button(frame, 'Add citation')[0].click();
  t.dom.flush();
  assert.strictEqual(rows().length, 2);
  assert.strictEqual(t.dom.document.activeElement, rows()[1].querySelector('input'), 'the new page field has focus');
  assert.deepStrictEqual(t.messages(), ['error: Citation ?::?: the page must be a number from 1.', 'error: Citation ?::?: say which paragraph, table or figure.']);
  t.button(frame, 'Show in PDF')[1].click();
  assert.strictEqual(t.dom.posted.cite.length, 1, 'no page yet: nothing to show');

  t.button(frame, 'Use PDF page')[1].click();
  assert.deepStrictEqual(t.dom.posted.edit.pop(), { type: 'pdfPage' });
  t.dom.window.dsEdit.setPdfPage(12);
  t.dom.flush();
  assert.strictEqual(rows()[1].querySelector('.ds-cite-page').value, '12');
  t.dom.window.dsEdit.setPdfPage(3);
  assert.strictEqual(rows()[1].querySelector('.ds-cite-page').value, '12', 'only the asked-for page is taken');
  t.type(rows()[1].querySelector('.ds-cite-ref'), ' Table 2 ');
  assert.deepStrictEqual(t.messages(), ['warning: Citation 12::Table 2: the PDF has only 9 pages.']);
  t.type(rows()[1].querySelector('.ds-cite-page'), '4');
  t.apply();
  assert.strictEqual(t.body()[l0], 'Para one, with ~sub~. [[1::2]] [[4::Table 2]]');

  const again = t.open(l0);
  again.querySelector('.ds-cite-row .danger').click();
  again.querySelector('.ds-cite-row .danger').click();
  t.dom.flush();
  assert.strictEqual(again.querySelectorAll('.ds-cite-row').length, 0);
  t.apply();
  assert.strictEqual(t.body()[l0], 'Para one, with ~sub~.');
});

test('markup buttons insert around the selection of the last field', () => {
  const t = setup();
  const frame = t.open(at('Para one'));
  const area = frame.querySelector('textarea');
  const tool = (title) => frame.querySelectorAll('.ds-ed-tool').find((b) => b.title === title);
  const down = new t.dom.Event('mousedown', { bubbles: true });
  tool('Subscript: ~text~').dispatchEvent(down);
  assert.strictEqual(down.defaultPrevented, true, 'the field keeps its focus');
  area.setSelectionRange(0, 4);
  tool('Subscript: ~text~').click();
  assert.strictEqual(area.value, '~Para~ one, with ~sub~.');
  assert.deepStrictEqual([area.selectionStart, area.selectionEnd], [1, 5]);
  area.setSelectionRange(0, 0);
  tool('Superscript: ^text^').click();
  assert.strictEqual(area.value.slice(0, 3), '^x^');
  tool('Overline: !{text}').click();
  tool('Insert µ').click();
  t.dom.flush();
  assert.strictEqual(area.value.slice(0, 8), '^!{µx}^~', 'each wraps or replaces the selection');
  const page = frame.querySelector('.ds-cite-page');
  page.focus();
  page.setSelectionRange(1, 1);
  tool('Insert Ω').click();
  assert.strictEqual(page.value, '1Ω', 'the last focused field gets it');
  frame.querySelector('.ds-ed-cancel').click();
  tool('Insert Ω').click();
  assert.strictEqual(page.value, '1Ω', 'a field no longer on the page is left alone');
});

test('the edited copy: blocks that differ from the file as shipped are marked', () => {
  const changed = MARKDOWN.replace('- Item b', '- Item c');
  const t = setup({ markdown: changed, original: MARKDOWN });
  assert.deepStrictEqual([t.state().modified, t.state().edited], [true, 1]);
  assert.ok(t.block(at('- Item a')).classList.contains('ds-edited'));
  t.dom.window.dsEdit.revertAll();
  assert.deepStrictEqual([t.state().modified, t.state().edited, t.state().undo], [false, 0, 1]);
  const bare = setup({ markdown: BODY.join('\n'), devices: [] });
  assert.strictEqual(bare.state().markdown, BODY.join('\n'), 'a file without front matter');
  const frame = bare.open(at('Para one'));
  bare.type(frame.querySelector('.ds-cite-page'), '99');
  assert.deepStrictEqual(bare.messages(), [], 'no page count, no page check');
  delete bare.dom.window.webkit;
  bare.dom.window.dsEdit.setEditing(false);
  assert.strictEqual(bare.dom.page.querySelector('.ds-editable'), null, 'no handler: nothing posted, nothing marked');
});

test('a real datasheet: edit a table cell of sn74hc00, only that line changes', () => {
  const file = path.join(DATASHEETS, 'gate-nand', 'sn74hc00.md');
  const p = payload(file, { editing: true, showCitations: true });
  const dom = page(true);
  dom.window.renderDatasheet(p);
  const lines = p.markdown.split('\n');
  const head = lines.indexOf('---', 1) + 1;
  const l0 = lines.indexOf(lines.find((l) => l.startsWith(FENCE + 'table id=function'))) - head;
  dom.page.querySelector('[data-l0="' + l0 + '"]').click();
  dom.flush();
  const frame = dom.page.querySelector('.ds-editor');
  assert.strictEqual(frame.querySelector('.ds-ed-title').textContent, 'Edit table');
  const cell = frame.querySelectorAll('input.ds-ed-cell').find((i) => i.value === 'X');
  cell.value = 'Z';
  cell.dispatchEvent(new dom.Event('input', { bubbles: true }));
  dom.flush();
  frame.querySelector('.ds-ed-apply').click();
  const after = dom.posted.edit.pop().markdown.split('\n');
  const diff = after.map((l, i) => (l === lines[i] ? null : [lines[i], l])).filter(Boolean);
  assert.deepStrictEqual(diff, [['| L | X | H |', '| L | Z | H |']]);
});
