#!/usr/bin/env node
// plan-45 census: how many in-stock Active / Last Time Buy parts a DigiKey category holds (for one
// maker, with a scope filter), and who the 12 biggest makers are. One API call per invocation.
//
// usage: node count.js <category id> [<maker id> | 0] [t | jst]
//   category 371 Terminal Blocks - Wire to Board, 370 Terminal Blocks - Headers, Plugs and
//   Sockets, 368 Barrier Blocks, 369 DIN Rail / Channel, 314 Rectangular Connectors - Headers,
//   Male Pins. Maker 0 / absent = all makers. Scope `t`: the terminal-block filter of list.js
//   (pitch 3.50 / 3.81 / 5.00 / 5.08 / 7.50 / 7.62 mm, 2..12 positions per level; category 370
//   also the header Types); `jst`: the JST series XH, PH, EH, ZH, SH/SR, GH, VH.
// Environment: SERIES=<n> also prints the n biggest series of the filtered set (`series <id> <name><TAB>count`);
// SERIESF=<series id> restricts the search to that series.
// Prints `all<TAB>n`, then `mfr <id> <name><TAB>n` for the 12 biggest makers and, with a scope,
// `levels <n><TAB>count` (the Number of Levels facet of the filtered set; 2168 in category 371,
// 927 in 370). With a maker, only that maker's `all` and levels.
// Credentials and transport: dk.js (the second key unless DK_OPT2=0; Safari when Cloudflare challenges Node).
'use strict';
const dk = require('./dk.js');

const [category, maker, scope] = process.argv.slice(2);
if (!category) { console.error('usage: node count.js <category id> [<maker id> | 0] [t | jst]'); process.exit(2); }

const ids = (list) => list.map((x) => ({ Id: String(x) }));
const T_PITCH = [11374, 11909, 13741, 14243, 17717, 18160];
const T_POS = [121326, 169295, 203167, 229001, 249730, 267825, 281410, 295538, 63986, 74826, 80383];
const T_HEADER_TYPES = [353507, 353519, 353520, 353521, 353531, 353535, 353537, 353538, 353540, 353541,
  353542, 353543, 353544, 393049];
const JST_SERIES = [3777, 3742, 3743, 86050, 3741, 3740, 2395];

(async () => {
  const ctx = await dk.open();
  const options = { CategoryFilter: [{ Id: category }], MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }] };
  if (maker && maker !== '0') options.ManufacturerFilter = [{ Id: maker }];
  if (scope === 't') {
    const params = [{ ParameterId: 89, FilterValues: ids(T_PITCH) }, { ParameterId: 1075, FilterValues: ids(T_POS) }];
    if (category === '370') params.push({ ParameterId: 183, FilterValues: ids(T_HEADER_TYPES) });
    options.ParameterFilterRequest = { CategoryFilter: { Id: category }, ParameterFilters: params };
  } else if (scope === 'jst') {
    options.SeriesFilter = ids(JST_SERIES);
  }
  if (process.env.SERIESF) options.SeriesFilter = ids([process.env.SERIESF]);
  const { response: r, text } = await dk.post(ctx, { Keywords: '', Limit: 1, Offset: 0, FilterOptionsRequest: options });
  if (!r.ok) { console.error(`count.js: HTTP ${r.status}: ${text.slice(0, 200)}`); process.exit(1); }
  const d = JSON.parse(text);
  console.log(`all\t${d.ProductsCount === undefined ? 0 : d.ProductsCount}`);
  const fo = d.FilterOptions || {};
  if (!maker || maker === '0') {
    for (const m of [...(fo.Manufacturers || [])].sort((a, b) => b.ProductCount - a.ProductCount).slice(0, 12)) {
      console.log(`mfr ${m.Id} ${m.Value}\t${m.ProductCount}`);
    }
  }
  for (const x of [...(fo.Series || [])].sort((a, b) => b.ProductCount - a.ProductCount).slice(0, Number(process.env.SERIES || 0))) {
    console.log(`series ${x.Id} ${x.Value}\t${x.ProductCount}`);
  }
  if (scope === 't') {
    const f = (fo.ParametricFilters || []).find((x) => x.ParameterId === (category === '371' ? 2168 : 927));
    for (const v of (f && f.FilterValues) || []) console.log(`levels ${v.ValueName}\t${v.ProductCount}`);
  }
  console.error(`count.js: calls left today ${ctx.left}`);
})().catch((e) => { console.error(`count.js: ${e.message}`); process.exit(1); });
