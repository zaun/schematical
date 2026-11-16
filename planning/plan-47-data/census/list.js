#!/usr/bin/env node
// plan-47 census: the most-stocked in-stock Active / Last Time Buy parts of
// one piece (a DigiKey category, some maker ids of one company, parametric
// values), most-stocked first, up to DigiKey's 300-result cap.
//
// usage: node list.js <out.jsonl> <type> <piece name> <category id>
//                     --maker <id>[,<id>…] [--param <parameter id>=<value id>[,…]]…
//   type: inductor, fuse, ptc, relay or buzzer.
//
// Pages of 50, sorted by QuantityAvailable descending, at most 6 pages: a
// piece of 300 parts or fewer is complete; a larger one keeps its 300
// most-stocked parts. Every product is one JSON line (type, piece, mpn,
// maker, makerId, category, series, stock, status, datasheet, description,
// params). One summary line goes to stderr:
//   piece <name>: <listed> of <count> [(cut at 300: <stock of the 300th>)]
// A part listed under several packings is written once.
'use strict';
const fs = require('fs');
const { search, parseArgs, state } = require('./dk.js');

const PAGE = 50;
const CAP = 300;
const { q, rest } = parseArgs(process.argv.slice(2));
const [outFile, type, piece, category] = rest;
if (!category || !q.makers.length) { console.error('usage: node list.js <out.jsonl> <type> <piece name> <category id> --maker ids [--param id=values]…'); process.exit(2); }
q.category = category;
q.sort = true;
q.limit = PAGE;

(async () => {
  const seen = new Set();
  const lines = [];
  let count = null;
  let lastStock = null;
  let listed = 0;
  for (let off = 0; off < CAP; off += PAGE) {
    const page = await search({ ...q, offset: off });
    if (count === null) count = page.ProductsCount || 0;
    const prods = page.Products || [];
    for (const p of prods) {
      listed++;
      lastStock = p.QuantityAvailable;
      const mpn = p.ManufacturerProductNumber;
      if (seen.has(mpn)) continue;
      seen.add(mpn);
      const params = {};
      for (const x of p.Parameters || []) params[x.ParameterText] = x.ValueText;
      lines.push(JSON.stringify({ type, piece, mpn, maker: p.Manufacturer && p.Manufacturer.Name,
        makerId: p.Manufacturer && p.Manufacturer.Id, category,
        series: (p.Series && p.Series.Name) || '', stock: p.QuantityAvailable,
        status: p.ProductStatus && p.ProductStatus.Status, datasheet: p.DatasheetUrl || '',
        description: (p.Description && p.Description.ProductDescription) || '', params }));
    }
    if (prods.length < PAGE || off + PAGE >= count) break;
  }
  fs.writeFileSync(outFile, lines.length ? lines.join('\n') + '\n' : '');
  console.error(`piece ${piece}: ${listed} of ${count}${count > CAP ? ` (cut at ${CAP}: ${lastStock})` : ''}`);
  console.error(`list.js: ${seen.size} parts, ${state.calls} calls, ${state.left} calls left today`);
})().catch((e) => { console.error(`list.js: ${e.message}`); process.exit(1); });
