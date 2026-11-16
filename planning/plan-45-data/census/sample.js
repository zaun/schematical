#!/usr/bin/env node
// plan-45 census: the DigiKey datasheet-link hosts of a maker's most-stocked parts in a category.
// usage: node sample.js <category id> <maker id> <n>   (n up to 50: one call)
// Prints `<host or (none)><TAB><count>` lines. Credentials as count.js.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');
const API = 'https://api.digikey.com';
const [category, maker, n] = process.argv.slice(2);
const creds = { id: process.env.DIGIKEY_CLIENT_ID, secret: process.env.DIGIKEY_CLIENT_SECRET };
const tok = JSON.parse(fs.readFileSync(path.join(os.tmpdir(), 'digikey-token.json'), 'utf8')).token;
(async () => {
  const r = await fetch(`${API}/products/v4/search/keyword`, {
    method: 'POST',
    headers: { Authorization: `Bearer ${tok}`, 'X-DIGIKEY-Client-Id': creds.id, 'X-DIGIKEY-Locale-Site': 'US',
      'X-DIGIKEY-Locale-Language': 'en', 'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
    body: JSON.stringify({ Keywords: '', Limit: Number(n || 50), Offset: 0,
      FilterOptionsRequest: { CategoryFilter: [{ Id: category }], ManufacturerFilter: [{ Id: maker }],
        MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }] },
      SortOptions: { Field: 'QuantityAvailable', SortOrder: 'Descending' } }),
  });
  const d = JSON.parse(await r.text());
  const hosts = {};
  for (const p of d.Products || []) {
    const u = p.DatasheetUrl || '';
    const h = u ? new URL(u.startsWith('//') ? `https:${u}` : u).host : '(none)';
    hosts[h] = (hosts[h] || 0) + 1;
  }
  for (const [h, c] of Object.entries(hosts)) console.log(`${h}\t${c}`);
  console.error(`sample.js: ${(d.Products || []).length} parts, calls left today ${r.headers.get('x-ratelimit-remaining')}`);
})();
