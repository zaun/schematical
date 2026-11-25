#!/usr/bin/env node
// plan-45 census: the DigiKey datasheet-link hosts of a maker's most-stocked parts in a category.
// usage: node sample.js <category id> <maker id> <n>   (n up to 50: one call)
// Prints `<host or (none)><TAB><count>` lines. Credentials and transport as count.js (dk.js).
'use strict';
const dk = require('./dk.js');
const [category, maker, n] = process.argv.slice(2);
if (!category || !maker) { console.error('usage: node sample.js <category id> <maker id> <n>'); process.exit(2); }
(async () => {
  const ctx = await dk.open();
  const { response: r, text } = await dk.post(ctx, { Keywords: '', Limit: Number(n || 50), Offset: 0,
    FilterOptionsRequest: { CategoryFilter: [{ Id: category }], ManufacturerFilter: [{ Id: maker }],
      MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }] },
    SortOptions: { Field: 'QuantityAvailable', SortOrder: 'Descending' } });
  if (!r.ok) { console.error(`sample.js: HTTP ${r.status}: ${text.slice(0, 200)}`); process.exit(1); }
  const d = JSON.parse(text);
  const hosts = {};
  for (const p of d.Products || []) {
    const u = p.DatasheetUrl || '';
    const h = u ? new URL(u.startsWith('//') ? `https:${u}` : u).host : '(none)';
    hosts[h] = (hosts[h] || 0) + 1;
  }
  for (const [h, c] of Object.entries(hosts)) console.log(`${h}\t${c}`);
  console.error(`sample.js: ${(d.Products || []).length} parts, calls left today ${ctx.left}`);
})().catch((e) => { console.error(`sample.js: ${e.message}`); process.exit(1); });
