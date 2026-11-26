// The made-up datasheet and the helpers the editor tests (datasheet-editor*.test.js) share: a page
// rendered in edit mode, and ways to open a block's editor, type, press keys and read what was posted.

'use strict';

const assert = require('node:assert');
const { page } = require('./datasheet-page.js');

const FENCE = '```';
const FRONT = {
  format: 'schematical-datasheet', title: 'Test part', source: { vendor: 'V', document: 'D1', pages: 9 },
  devices: ['x:d'], parts: ['P*'],
};
const DEVICE = {
  id: 'd', kind: 'x', description: 'Dev',
  pinouts: { p: [{ number: 1, name: 'A', role: 'input' }, { number: 2, name: 'Y', role: 'output' }] },
  parts: [{ name: 'P1', pinout: 'p' }, { name: 'P2' }],
};
const BODY = [
  '# Test part', '',
  '## Description', '',
  'Para one, with ~sub~. [[1::2]]', '',
  '- Item a [[1::3]]', '- Item b', '',
  '1. First', '2. Second', '',
  '> A quote.', '',
  '## Electrical characteristics', '',
  FENCE + 'table id=ec caption="EC" conditions="Free air" parts=P* note=x',
  '| Parameter | MIN | UNIT |', '|---|--:|:-:|', '| V~IH~ [1] | 2 | V |', '| V~IL~ | ^ | V |', '', '[1] Note one.',
  FENCE, '[[2::Table 6-1]]', '',
  FENCE + 'pins', 'A: Input', 'Y: Output', FENCE, '',
  FENCE + 'ordering', 'P1 | Tape | -40 to 85', FENCE, '',
  FENCE + 'wavedrom id=w1', '{"signal": [{"name": "A", "wave": "01"}]}', FENCE, '',
  FENCE + 'pinout', FENCE, '',
  FENCE + 'table id=one', '| A |', '|---|', '| 1 |', FENCE, '',
  FENCE + 'table id=bad', 'not a table', FENCE, '',
  FENCE + 'pins', 'no colon here', FENCE,
];
const MARKDOWN = '---\n' + JSON.stringify(FRONT) + '\n---\n' + BODY.join('\n');

/** The body line that starts with `prefix` (the n-th one). */
function at(prefix, n) {
  let seen = 0;
  for (let i = 0; i < BODY.length; i++) if (BODY[i].startsWith(prefix) && seen++ === (n || 0)) return i;
  throw new Error('no line ' + prefix);
}

function setup(extra) {
  const dom = page(true);
  dom.window.renderDatasheet(Object.assign({ markdown: MARKDOWN, path: 'x/test.md', devices: [DEVICE], editing: true, showCitations: true }, extra));
  dom.flush();
  const block = (l0) => dom.page.querySelector('[data-l0="' + l0 + '"]');
  const frame = () => dom.page.querySelector('.ds-editor');
  const button = (root, text) => {
    const found = root.querySelectorAll('button').filter((b) => b.textContent === text);
    assert.ok(found.length, 'a button "' + text + '"');
    return found;
  };
  return {
    dom, block, frame, button,
    /** Clicks a block (by its first body line) and returns the editor frame. */
    open(l0) { block(l0).click(); dom.flush(); return frame(); },
    type(el, value) { el.value = value; el.dispatchEvent(new dom.Event('input', { bubbles: true })); dom.flush(); },
    key(el, key, mods) { const e = new dom.Event('keydown', Object.assign({ bubbles: true, key }, mods)); el.dispatchEvent(e); dom.flush(); return e; },
    apply() { frame().querySelector('.ds-ed-apply').click(); dom.flush(); },
    state() { return dom.posted.edit.filter((m) => m.type === 'state').pop(); },
    /** The markdown body of the last state message. */
    body() { const md = dom.posted.edit.filter((m) => m.type === 'state').pop().markdown; return md.slice(md.indexOf('\n---\n') + 5).split('\n'); },
    messages() { return frame().querySelectorAll('.ds-ed-error, .ds-ed-warning').map((m) => m.getAttribute('class').slice(6) + ': ' + m.textContent); },
  };
}

module.exports = { setup, at, FENCE, BODY, MARKDOWN };
