#!/usr/bin/env node
// plan-45 census: every in-stock Active / Last Time Buy part of one scope piece
// (a DigiKey category, a maker and a scope), most-stocked first.
//
// usage: node list.js <piece> <out.jsonl>
//   piece = <category id>-<maker id>-<scope>, scope `t` (terminal blocks:
//   pitch 3.50 / 3.81 / 5.00 / 5.08 / 7.50 / 7.62 mm, 2..12 positions per
//   level; category 370 adds the header Types) or `jst` (the JST series XH,
//   PH, EH, ZH, SH/SR, GH, VH; category 314).
//
// The first call of a piece gives its count and its facets (the facets count
// the filtered set). A piece of 300 parts or fewer is read 50 parts a page,
// sorted by QuantityAvailable descending. A bigger one is split by the facet
// (Pitch, Positions Per Level, Mounting Type, Current, Termination Style,
// Type) whose largest value is smallest, and each half is read the same way;
// DigiKey returns at most 300 results of a search, so a piece that no facet
// splits below 300 keeps its 300 most-stocked parts and the summary line says
// `(cut at 300: <stock of the 300th>)`.
// Every product is written once as one JSON line (mpn, maker, makerId,
// category, series, stock, status, datasheet, description, params).
// Credentials and transport: dk.js (the second key unless DK_OPT2=0; Safari when Cloudflare challenges Node).
'use strict';
const fs = require('fs');
const dk = require('./dk.js');

const PAGE = 50;
const CAP = 300;
const [piece, outFile] = process.argv.slice(2);
if (!piece || !outFile) { console.error('usage: node list.js <category>-<maker>-<t|jst> <out.jsonl>'); process.exit(2); }
const [category, maker, scopeName] = piece.split('-');

const PITCH = 89, POSITIONS = 1075, TYPE = 183, MOUNTING = 69, CURRENT = 2155, TERMINATION = 258;
const ids = (list) => list.map((x) => ({ Id: String(x) }));
const T_PITCH = [11374, 11909, 13741, 14243, 17717, 18160];          // 3.50 3.81 5.00 5.08 7.50 7.62 mm
const T_POS = [121326, 169295, 203167, 229001, 249730, 267825, 281410, 295538, 63986, 74826, 80383]; // 2..12
const T_HEADER_TYPES = [353507, 353519, 353520, 353521, 353531, 353535, 353537, 353538, 353540, 353541,
  353542, 353543, 353544, 393049];
const JST_SERIES = { XH: 3777, PH: 3742, ZH: 3743, 'SH/SR': 86050, VH: 3741, EH: 3740, GH: 2395 };

let baseParams = [];
let seriesIds = [];
if (scopeName === 't') {
  baseParams = [{ ParameterId: PITCH, FilterValues: ids(T_PITCH) }, { ParameterId: POSITIONS, FilterValues: ids(T_POS) }];
  if (category === '370') baseParams.push({ ParameterId: TYPE, FilterValues: ids(T_HEADER_TYPES) });
} else if (scopeName === 'jst') {
  seriesIds = Object.values(JST_SERIES);
} else { console.error('list.js: scope must be t or jst'); process.exit(2); }

let ctx = null;

async function search(params, series, offset, limit) {
  for (let attempt = 1; ; attempt++) {
    try {
      const options = { CategoryFilter: [{ Id: category }], ManufacturerFilter: [{ Id: maker }],
        MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }] };
      if (params.length) options.ParameterFilterRequest = { CategoryFilter: { Id: category }, ParameterFilters: params };
      if (series.length) options.SeriesFilter = ids(series);
      const { response: r, text } = await dk.post(ctx, { Keywords: '', Limit: limit, Offset: offset, FilterOptionsRequest: options,
        SortOptions: { Field: 'QuantityAvailable', SortOrder: 'Descending' } });
      if (r.status === 429) throw Object.assign(new Error(`rate limited: ${text}`), { fatal: true });
      if (!r.ok) throw new Error(`HTTP ${r.status}: ${text.slice(0, 200)}`);
      return JSON.parse(text);
    } catch (e) {
      if (e.fatal || attempt >= 4) throw e;
      console.error(`list.js: ${e.message}; retry ${attempt} of 3`);
      await new Promise((res) => setTimeout(res, 2000 * attempt));
    }
  }
}

const seen = new Set();
const out = fs.createWriteStream(outFile, { flags: 'w' });
function emit(p) {
  const key = p.ManufacturerProductNumber;
  if (seen.has(key)) return false;
  seen.add(key);
  const params = {};
  for (const x of p.Parameters || []) params[x.ParameterText] = x.ValueText;
  out.write(JSON.stringify({ mpn: key, maker: p.Manufacturer && p.Manufacturer.Name, makerId: maker, category,
    series: (p.Series && p.Series.Name) || '', stock: p.QuantityAvailable,
    status: p.ProductStatus && p.ProductStatus.Status, datasheet: p.DatasheetUrl || '',
    description: (p.Description && p.Description.ProductDescription) || '', params }) + '\n');
  return true;
}

// The facet that splits `first` best: every value of it must hold fewer parts than the piece.
function splitFacet(first, count) {
  let best = null;
  for (const pid of [PITCH, POSITIONS, MOUNTING, TERMINATION, TYPE, CURRENT]) {
    const f = ((first.FilterOptions || {}).ParametricFilters || []).find((x) => x.ParameterId === pid);
    if (!f) continue;
    const vals = f.FilterValues.filter((v) => v.ProductCount > 0);
    if (vals.length < 2) continue;
    const biggest = Math.max(...vals.map((v) => v.ProductCount));
    if (biggest >= count) continue;
    if (!best || biggest < best.biggest) best = { pid, vals, biggest };
  }
  return best;
}

let pieces = 0;
async function read(params, series, label) {
  const first = await search(params, series, 0, PAGE);
  const count = first.ProductsCount || 0;
  if (count === 0) return 0;
  if (count > CAP) {
    const s = splitFacet(first, count);
    if (s) {
      let n = 0;
      for (const v of s.vals) {
        const sub = params.filter((p) => p.ParameterId !== s.pid).concat([{ ParameterId: s.pid, FilterValues: [{ Id: String(v.ValueId) }] }]);
        n += await read(sub, series, `${label}/${s.pid}=${v.ValueName}`);
      }
      return n;
    }
  }
  pieces++;
  let listed = 0;
  let lastStock = null;
  let page = first;
  for (let off = 0; off < Math.min(count, CAP); off += PAGE) {
    if (off > 0) page = await search(params, series, off, PAGE);
    const prods = page.Products || [];
    for (const p of prods) { emit(p); listed++; lastStock = p.QuantityAvailable; }
    if (prods.length < PAGE) break;
  }
  console.error(`piece ${label || 'all'}: ${listed} of ${count}${count > CAP ? ` (cut at ${CAP}: ${lastStock})` : ''}`);
  return listed;
}

(async () => {
  ctx = await dk.open();
  await read(baseParams, seriesIds, '');
  console.error(`list.js: ${seen.size} parts, ${pieces} pieces, ${ctx.calls} calls, ${ctx.left} calls left today`);
  out.end();
})().catch((e) => { console.error(`list.js: ${e.message}`); process.exit(1); });
