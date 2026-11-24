#!/usr/bin/env node
// plan-55 census: every in-stock Active part of one (family, maker label) piece, most-stocked first.
//
// usage: node list.js <family>-<maker id> <out.jsonl> [--opt2]      e.g. limited-296 raw/limited-296.jsonl
//
// The family is a DigiKey category plus parametric filters (common.js `FAMILIES`); the filters' ids come from
// the category's facets, read once (one call) and cached beside the output as facets-<category>.json. The
// first call of a piece gives its count and its facets (the facets count the filtered set). A piece of 300
// parts or fewer is read 50 parts a page, sorted by QuantityAvailable descending. A bigger one is split by
// the parametric facet whose largest value is smallest (then by Series), and each half is read the same way;
// DigiKey returns at most 300 results of a search, so a piece that no facet splits below 300 keeps its 300
// most-stocked parts and the summary line says `(cut at 300: <stock of the 300th>)`.
// Every product is written once as one JSON line (mpn, maker, makerId, family, category, series, stock,
// status, datasheet, description, params). Only status Active (0) is asked for.
// Credentials: DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET (--opt2: DIGIKEY_CLIENT_ID2 / DIGIKEY_CLIENT_SECRET2)
// in the environment; the requests are scripts/digikey.js's (login token, Safari fallback).
'use strict';
const fs = require('fs');
const path = require('path');
const { FAMILIES, open, search, resolve } = require('./common.js');

const PAGE = 50;
const CAP = 300;
const args = process.argv.slice(2).filter((a) => !a.startsWith('--'));
const opt2 = process.argv.includes('--opt2');
const [piece, outFile] = args;
if (!piece || !outFile) { console.error('usage: node list.js <family>-<maker id> <out.jsonl> [--opt2]'); process.exit(2); }
const [familyKey, maker] = piece.split('-');
const family = FAMILIES.find((f) => f.key === familyKey);
if (!family) { console.error(`list.js: unknown family ${familyKey}`); process.exit(2); }
const category = family.category;

let ctx = null;
let base = [];      // the family's resolved ParameterFilters

const seen = new Set();
const out = fs.createWriteStream(outFile, { flags: 'w' });
function emit(p) {
  const key = p.ManufacturerProductNumber;
  if (seen.has(key)) return false;
  seen.add(key);
  const params = {};
  for (const x of p.Parameters || []) params[x.ParameterText] = x.ValueText;
  out.write(JSON.stringify({ mpn: key, maker: p.Manufacturer && p.Manufacturer.Name, makerId: maker, family: familyKey, category,
    series: (p.Series && p.Series.Name) || '', stock: p.QuantityAvailable,
    status: p.ProductStatus && p.ProductStatus.Status, datasheet: p.DatasheetUrl || '',
    description: (p.Description && p.Description.ProductDescription) || '', params }) + '\n');
  return true;
}

// The family's filters resolved against the category's facets, cached beside the output.
async function familyFilters() {
  const cache = path.join(path.dirname(path.resolve(outFile)), `facets-${category}.json`);
  let facets;
  try { facets = JSON.parse(fs.readFileSync(cache, 'utf8')); } catch {
    const all = await search(ctx, { category });
    facets = { FilterOptions: { ParametricFilters: all.FilterOptions.ParametricFilters.map((f) => ({
      ParameterId: f.ParameterId, ParameterName: f.ParameterName,
      FilterValues: f.FilterValues.map((v) => ({ ValueId: v.ValueId, ValueName: v.ValueName, ProductCount: v.ProductCount })) })) } };
    fs.writeFileSync(cache, JSON.stringify(facets));
  }
  return resolve(facets, family.filters);
}

// The facet that splits `first` best: every value of it must hold fewer parts than the piece, and the values
// together must hold exactly the piece (a facet some parts have no value for, or one a part has several values
// of, would lose or double parts: plan-55-B Correction 1). Among those, the one that reads the piece in the
// fewest calls (a value of n parts costs ceil(n / 50) calls, at least one: Correction 2). A parametric facet
// is tried first (`pid` its ParameterId), then the Series facet (`pid` -1).
function splitFacet(first, count) {
  let best = null;
  const fo = first.FilterOptions || {};
  const cost = (vals) => vals.reduce((n, v) => n + Math.max(1, Math.ceil(v.ProductCount / PAGE)), 0);
  for (const f of fo.ParametricFilters || []) {
    const vals = f.FilterValues.filter((v) => v.ProductCount > 0);
    if (vals.length < 2) continue;
    const biggest = Math.max(...vals.map((v) => v.ProductCount));
    if (biggest >= count || vals.reduce((n, v) => n + v.ProductCount, 0) !== count) continue;
    if (!best || cost(vals) < best.cost) best = { pid: f.ParameterId, vals, biggest, cost: cost(vals) };
  }
  if (!best) {
    const vals = (fo.Series || []).filter((v) => v.ProductCount > 0).map((v) => ({ ValueId: v.Id, ValueName: v.Value, ProductCount: v.ProductCount }));
    if (vals.length >= 2) {
      const biggest = Math.max(...vals.map((v) => v.ProductCount));
      if (biggest < count && vals.reduce((n, v) => n + v.ProductCount, 0) === count) best = { pid: -1, vals, biggest, cost: cost(vals) };
    }
  }
  return best;
}

const query = (params, series, offset, limit) => search(ctx, { category, params, makers: [maker], series, limit, offset, sort: true });

let pieces = 0;
async function read(params, series, label) {
  const first = await query(params, series, 0, PAGE);
  const count = first.ProductsCount || 0;
  if (count === 0) return 0;
  if (count > CAP) {
    const s = splitFacet(first, count);
    if (s) {
      let n = 0;
      for (const v of s.vals) {
        if (s.pid === -1) { n += await read(params, [v.ValueId], `${label}/series=${v.ValueName}`); continue; }
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
    if (off > 0) page = await query(params, series, off, PAGE);
    const prods = page.Products || [];
    for (const p of prods) { emit(p); listed++; lastStock = p.QuantityAvailable; }
    if (prods.length < PAGE) break;
  }
  console.error(`piece ${label || 'all'}: ${listed} of ${count}${count > CAP ? ` (cut at ${CAP}: ${lastStock})` : ''}`);
  return listed;
}

(async () => {
  ctx = await open(opt2);
  base = await familyFilters();
  await read(base, [], '');
  console.error(`list.js: ${seen.size} parts, ${pieces} pieces, ${ctx.calls} calls, ${ctx.left} calls left today`);
  out.end();
})().catch((e) => { console.error(`list.js: ${e.message}`); process.exit(1); });
