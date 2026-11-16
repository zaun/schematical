#!/usr/bin/env node
// plan-47 census: how many in-stock Active / Last Time Buy parts a DigiKey
// category holds, optionally for some makers and parametric values, with the
// maker ranking and the facet splits. One API call per invocation.
//
// usage: node count.js <category id> [--maker <id>[,<id>…]]
//                      [--param <parameter id>=<value id>[,<value id>…]]…
//                      [--facets <parameter id>[,…]] [--makers <n>]
// Prints `<label>\t<count>` lines: `all`; `maker: <id> <name>` for the <n>
// biggest makers (default 25); `series: <id> <name>` when --maker is given;
// `<parameter id> <parameter name>: <value id> <value name>` for the facets
// named by --facets (default: every facet with at most 40 values).
'use strict';
const { search, parseArgs, state } = require('./dk.js');

const { q, rest } = parseArgs(process.argv.slice(2));
let facets = null;
let makers = 25;
const pos = [];
for (let i = 0; i < rest.length; i++) {
  if (rest[i] === '--facets') facets = rest[++i].split(',');
  else if (rest[i] === '--makers') makers = Number(rest[++i]);
  else pos.push(rest[i]);
}
if (pos.length !== 1) { console.error('usage: node count.js <category id> [--maker ids] [--param id=values]… [--facets ids] [--makers n]'); process.exit(2); }
q.category = pos[0];

(async () => {
  const d = await search(q);
  console.log(`all\t${d.ProductsCount}`);
  const fo = d.FilterOptions || {};
  for (const m of [...(fo.Manufacturers || [])].sort((a, b) => b.ProductCount - a.ProductCount).slice(0, makers)) {
    console.log(`maker: ${m.Id} ${m.Value}\t${m.ProductCount}`);
  }
  if (q.makers.length) {
    for (const s of [...(fo.Series || [])].sort((a, b) => b.ProductCount - a.ProductCount)) console.log(`series: ${s.Id} ${s.Value}\t${s.ProductCount}`);
  }
  for (const f of fo.ParametricFilters || []) {
    if (facets ? !facets.includes(String(f.ParameterId)) : f.FilterValues.length > 40) continue;
    for (const v of f.FilterValues) console.log(`${f.ParameterId} ${f.ParameterName}: ${v.ValueId} ${v.ValueName}\t${v.ProductCount}`);
  }
  console.error(`count.js: calls left today ${state.left}`);
})().catch((e) => { console.error(`count.js: ${e.message}`); process.exit(1); });
