#!/usr/bin/env node
// plan-37 census: one DigiKey call printing the "Package / Case" (param 16)
// facet of a (category, manufacturer) piece, in stock, Active or Last Time Buy.
// usage: node facets.js <category id> <manufacturer id> > facets.tsv
// Output: <value id> TAB <count> TAB <package name>; total on stderr.
'use strict';
const fs = require('fs'), os = require('os'), path = require('path');
const API = 'https://api.digikey.com', TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const [category, maker] = process.argv.slice(2);
const creds = { id: process.env.DIGIKEY_CLIENT_ID, secret: process.env.DIGIKEY_CLIENT_SECRET };
async function token() {
  try { const c = JSON.parse(fs.readFileSync(TOKEN_CACHE, 'utf8'));
    if (c.id === creds.id && c.expires > Date.now() + 60000) return c.token; } catch {}
  const r = await fetch(`${API}/v1/oauth2/token`, { method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams({ client_id: creds.id, client_secret: creds.secret, grant_type: 'client_credentials' }) });
  const b = await r.json(); if (!b.access_token) throw new Error(`login failed (${r.status})`);
  fs.writeFileSync(TOKEN_CACHE, JSON.stringify({ id: creds.id, token: b.access_token,
    expires: Date.now() + (b.expires_in || 0) * 1000 }), { mode: 0o600 });
  return b.access_token;
}
(async () => {
  const tok = await token();
  const r = await fetch(`${API}/products/v4/search/keyword`, { method: 'POST',
    headers: { Authorization: `Bearer ${tok}`, 'X-DIGIKEY-Client-Id': creds.id, 'X-DIGIKEY-Locale-Site': 'US',
      'X-DIGIKEY-Locale-Language': 'en', 'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
    body: JSON.stringify({ Keywords: '', Limit: 1, Offset: 0, FilterOptionsRequest: {
      CategoryFilter: [{ Id: category }], ManufacturerFilter: [{ Id: maker }],
      MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }] } }) });
  const b = await r.json();
  const f = (b.FilterOptions.ParametricFilters || []).find(x => x.ParameterId === 16);
  for (const v of f.FilterValues) console.log(`${v.ValueId}\t${v.ProductCount}\t${v.ValueName}`);
  console.error(`facets.js: ${b.ProductsCount} parts, 1 call, ${r.headers.get('x-ratelimit-remaining')} left today`);
})().catch(e => { console.error(`facets.js: ${e.message}`); process.exit(1); });
