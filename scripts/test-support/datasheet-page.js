// Shared set-up for the datasheet window's page tests (datasheet-render.test.js,
// datasheet-editor.test.js): a fake-DOM page with markdown-it, WaveDrom, datasheet.js and, on request,
// the editor loaded, plus the payload the app builds (DatasheetPayload.build in
// MarkdownDatasheetWindow.swift) from the catalog part files.

'use strict';

const fs = require('fs');
const path = require('path');
const { createDom } = require('./fake-dom.js');

const ROOT = path.join(__dirname, '..', '..');
const RENDERER = path.join(ROOT, 'schematical', 'schematical', 'Datasheet', 'Renderer');
const DATASHEETS = path.join(ROOT, 'schematical', 'schematical', 'Datasheets');
const PARTS = path.join(ROOT, 'schematical', 'schematical', 'Parts');

/** A page with the renderer (and with `editor`, the edit model and editor) loaded, in html order. */
function page(editor) {
  const dom = createDom();
  for (const f of ['markdown-it.min.js', 'wavedrom.min.js', 'datasheet.js']) dom.load(path.join(RENDERER, f));
  if (editor) for (const f of ['datasheet-edit-model.js', 'datasheet-edit.js']) dom.load(path.join(RENDERER, f));
  dom.page = dom.document.getElementById('page');
  return dom;
}

function datasheets() {
  const out = [];
  (function walk(dir) {
    for (const entry of fs.readdirSync(dir, { withFileTypes: true }).sort((a, b) => (a.name < b.name ? -1 : 1))) {
      const file = path.join(dir, entry.name);
      if (entry.isDirectory()) walk(file);
      else if (file.endsWith('.md')) out.push(file);
    }
  })(DATASHEETS);
  return out;
}

let partFiles = null;
let footprintList = null;

function catalog() {
  if (!partFiles) {
    partFiles = {};
    for (const f of fs.readdirSync(PARTS)) {
      if (!f.endsWith('.json') || f === 'datasheets.json' || f === 'footprints.json') continue;
      const object = JSON.parse(fs.readFileSync(path.join(PARTS, f), 'utf8'));
      if (object.kind) partFiles[object.kind] = object;
    }
    footprintList = new Map(JSON.parse(fs.readFileSync(path.join(PARTS, 'footprints.json'), 'utf8')).footprints.map((f) => [f.id, f]));
  }
  return { partFiles, footprintList };
}

// The pads of the two simplest footprint styles, as the backend expands them (dual: pins down the
// left side then up the right; explicit: as listed). Other styles are left out (the page draws a pin
// table for a pinout whose footprint is missing).
function expand(fp) {
  const pad = (d, x, y, p) => ({ d: String(d), x, y, w: p.width, h: p.height, drill: p.drill || 0, slot: 0, plated: true, shape: p.shape });
  let pads;
  if (fp.style === 'dual') {
    const per = fp.pins / 2;
    pads = [];
    for (let i = 0; i < fp.pins; i++) {
      const left = i < per;
      const k = left ? i : fp.pins - 1 - i;
      pads.push(pad(i + 1, (left ? -1 : 1) * fp.span / 2, (k - (per - 1) / 2) * fp.pitch, fp.pad));
    }
  } else if (fp.style === 'explicit') {
    pads = fp.pads.map((p) => pad(p.pin, p.x, p.y, p));
  } else {
    return null;
  }
  return { description: fp.description, body: { w: fp.body.width, h: fp.body.height }, round: !!fp.round, pads };
}

/** The payload for a datasheet file: its devices from the part files, their footprints, the figures. */
function payload(file, extra) {
  const { partFiles: files, footprintList: fps } = catalog();
  const markdown = fs.readFileSync(file, 'utf8');
  const lines = markdown.split('\n');
  const front = lines[0] === '---' ? JSON.parse(lines.slice(1, lines.indexOf('---', 1)).join('\n')) : {};
  const devices = [];
  const footprints = {};
  for (const entry of front.devices || []) {
    const [kind, id] = entry.split(':');
    const found = ((files[kind] || {}).devices || []).find((d) => d.id === id);
    if (!found) continue;
    const device = Object.assign({}, found, { kind });
    delete device.note;
    delete device.timing;
    devices.push(device);
    for (const p of device.parts || []) {
      if (p.footprint && !(p.footprint in footprints) && fps.has(p.footprint)) {
        const fp = expand(fps.get(p.footprint));
        if (fp) footprints[p.footprint] = fp;
      }
    }
  }
  const figures = {};
  for (const m of markdown.matchAll(/(?:ref|src)=("[^"]*"|\S+)/g)) {
    const key = m[1].replace(/"/g, '');
    const svg = path.join(DATASHEETS, 'figures', path.basename(key).replace(/\.svg$/, '') + '.svg');
    if (!(key in figures) && fs.existsSync(svg)) figures[key] = fs.readFileSync(svg, 'utf8');
  }
  return Object.assign({ markdown, part: null, path: path.relative(DATASHEETS, file), devices, footprints, figures }, extra || {});
}

module.exports = { page, datasheets, payload, DATASHEETS, RENDERER };
