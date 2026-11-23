#!/usr/bin/env node
// plan-55 census: the three power-switch families on DigiKey (in stock, status Active), their makers, and for
// the picked makers the number of parts and of distinct packages.
//
// usage: node families.js            (DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET in the environment; the login
//                                      token is the one scripts/digikey.js caches)
//
// A family is a DigiKey category plus parametric filters (names, not ids: the ids are looked up in the first
// call of each category). One call per category for the facets, one per family for its makers, one per
// (family, picked maker) for the count and the packages. A company's DigiKey labels count together
// (plan-54-A § 2): Analog Devices = 505 + 175, onsemi = 488 + 600039, Microchip = 150 + 576, Texas
// Instruments = 296 + 14.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');

const API = 'https://api.digikey.com/products/v4/search/keyword';
const tok = JSON.parse(fs.readFileSync(path.join(os.tmpdir(), 'digikey-token.json'), 'utf8')).token;
const id = process.env.DIGIKEY_CLIENT_ID;
let left = null;

// op: '=' the named values, '~' every value whose name contains the text, '!~' every value whose name does not.
const FAMILIES = [
  { key: 'ideal', title: 'Ideal diodes (internal FET)', category: '758',
    filters: [['Internal Switch(s)', '=', ['Yes']]],
    picks: { 'Texas Instruments': ['296', '14'], 'Analog Devices': ['505', '175'], 'Monolithic Power Systems': ['1589'] } },
  { key: 'load', title: 'Load switches', category: '726',
    filters: [['Switch Type', '=', ['Load Switch', 'Power Switch']], ['Output Configuration', '=', ['High Side']],
              ['Number of Outputs', '=', ['1', '2']], ['Fault Protection', '!~', ['Current Limit']]],
    picks: { 'Texas Instruments': ['296', '14'], onsemi: ['488', '600039'], Microchip: ['150', '576'] } },
  { key: 'limited', title: 'Current-limited and USB switches', category: '726',
    filters: [['Switch Type', '=', ['Load Switch', 'Power Switch', 'USB Switch']], ['Output Configuration', '=', ['High Side']],
              ['Number of Outputs', '=', ['1', '2', '3', '4']], ['Fault Protection', '~', ['Current Limit']]],
    picks: { 'Texas Instruments': ['296', '14'], 'Diodes Incorporated': ['31'], Microchip: ['150', '576'] } },
];

async function call(category, params, makers) {
  const o = { CategoryFilter: [{ Id: category }], MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }] };
  if (params.length) o.ParameterFilterRequest = { CategoryFilter: { Id: category }, ParameterFilters: params };
  if (makers) o.ManufacturerFilter = makers.map((m) => ({ Id: m }));
  const r = await fetch(API, { method: 'POST', headers: { Authorization: `Bearer ${tok}`, 'X-DIGIKEY-Client-Id': id,
    'X-DIGIKEY-Locale-Site': 'US', 'X-DIGIKEY-Locale-Language': 'en', 'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
    body: JSON.stringify({ Keywords: '', Limit: 1, Offset: 0, FilterOptionsRequest: o }) });
  left = r.headers.get('x-ratelimit-remaining') ?? left;
  if (!r.ok) throw new Error(`HTTP ${r.status}: ${(await r.text()).slice(0, 200)}`);
  return JSON.parse(await r.text());
}

function resolve(facets, filters) {
  return filters.map(([name, op, vals]) => {
    const f = facets.FilterOptions.ParametricFilters.find((x) => x.ParameterName === name);
    if (!f) throw new Error(`no facet ${name}`);
    const live = f.FilterValues.filter((v) => v.ProductCount > 0);
    const chosen = op === '=' ? vals.map((n) => { const v = live.find((x) => x.ValueName === n); if (!v) throw new Error(`no value ${name}=${n}`); return v; })
      : live.filter((v) => (op === '~') === v.ValueName.includes(vals[0]));
    return { ParameterId: f.ParameterId, FilterValues: chosen.map((v) => ({ Id: String(v.ValueId) })) };
  });
}

(async () => {
  const base = {};
  for (const fam of FAMILIES) {
    base[fam.category] = base[fam.category] || await call(fam.category, [], null);
    const params = resolve(base[fam.category], fam.filters);
    const all = await call(fam.category, params, null);
    console.log(`== ${fam.key}\t${fam.title}\tcategory ${fam.category}\tparts ${all.ProductsCount}`);
    console.log('makers\t' + [...(all.FilterOptions.Manufacturers || [])].sort((a, b) => b.ProductCount - a.ProductCount)
      .map((m) => `${m.Value} ${m.ProductCount}`).join('; '));
    for (const [company, ids] of Object.entries(fam.picks)) {
      const d = await call(fam.category, params, ids);
      const pk = (name) => (d.FilterOptions.ParametricFilters.find((x) => x.ParameterName === name)?.FilterValues || []).filter((v) => v.ProductCount > 0).length;
      console.log(`pick\t${fam.key}\t${company}\tparts ${d.ProductsCount}\tpackages(case) ${pk('Package / Case')}\tpackages(supplier) ${pk('Supplier Device Package')}`);
    }
  }
  console.log(`left ${left}`);
})().catch((e) => { console.error(`families.js: ${e.message}`); process.exit(1); });
