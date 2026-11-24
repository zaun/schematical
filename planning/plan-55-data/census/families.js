#!/usr/bin/env node
// plan-55 census: the three power-switch families on DigiKey (in stock, status Active), their makers, and for
// the picked makers the number of parts and of distinct packages.
//
// usage: node families.js [--opt2]    (DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET in the environment, or the
//                                      DIGIKEY_CLIENT_ID2 / DIGIKEY_CLIENT_SECRET2 pair with --opt2; the
//                                      requests and the login are scripts/digikey.js's, through common.js)
//
// A family is a DigiKey category plus parametric filters (names, not ids: the ids are looked up in the first
// call of each category; common.js `FAMILIES`). One call per category for the facets, one per family for its
// makers, one per (family, picked maker) for the count and the packages. A company's DigiKey labels count
// together (plan-54-A § 2): Analog Devices = 505 + 175, onsemi = 488 + 600039, Microchip = 150 + 576, Texas
// Instruments = 296 + 14.
'use strict';
const { FAMILIES, open, search, resolve } = require('./common.js');

(async () => {
  const ctx = await open(process.argv.includes('--opt2'));
  const call = (category, params, makers) => search(ctx, { category, params, makers });
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
  console.log(`left ${ctx.left}`);
})().catch((e) => { console.error(`families.js: ${e.message}`); process.exit(1); });
