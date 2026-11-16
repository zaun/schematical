#!/usr/bin/env node
// plan-44 census: how many in-stock Active / Last Time Buy parts a DigiKey
// crystal / resonator category holds for one maker (or all makers), and how
// they split by the Type, Operating Mode, Features, Ratings and Mounting Type
// facets. One API call per invocation.
//
// usage: node count.js <category id> [<manufacturer id>]
//   category 171 = Crystals, 174 = Resonators; manufacturer 0 / absent = all.
// Prints `<label>\t<count>` lines: `all`, then `<facet>: <value>`; and, with no
// manufacturer, `maker: <id> <name>` for the 15 biggest makers.
// Credentials: DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET in the environment;
// the login token is the one digikey.js caches.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');

const API = 'https://api.digikey.com';
const TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const [category, maker] = process.argv.slice(2);
if (!category) { console.error('usage: node count.js <category id> [<manufacturer id>]'); process.exit(2); }
const creds = { id: process.env.DIGIKEY_CLIENT_ID, secret: process.env.DIGIKEY_CLIENT_SECRET };
if (!creds.id || !creds.secret) { console.error('count.js: set DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET (see ~/.zshrc)'); process.exit(1); }

async function token(fresh) {
  if (!fresh) {
    try {
      const c = JSON.parse(fs.readFileSync(TOKEN_CACHE, 'utf8'));
      if (c.id === creds.id && c.expires > Date.now() + 30000) return c.token;
    } catch { /* none */ }
  }
  const r = await fetch(`${API}/v1/oauth2/token`, {
    method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams({ client_id: creds.id, client_secret: creds.secret, grant_type: 'client_credentials' }),
  });
  const body = await r.json();
  if (!body.access_token) throw new Error(`login failed (${r.status})`);
  fs.writeFileSync(TOKEN_CACHE, JSON.stringify({ id: creds.id, token: body.access_token,
    expires: Date.now() + (body.expires_in || 0) * 1000 }), { mode: 0o600 });
  return body.access_token;
}

(async () => {
  let tok = await token(false);
  for (let attempt = 0; attempt < 2; attempt++) {
    const r = await fetch(`${API}/products/v4/search/keyword`, {
      method: 'POST',
      headers: { Authorization: `Bearer ${tok}`, 'X-DIGIKEY-Client-Id': creds.id, 'X-DIGIKEY-Locale-Site': 'US',
        'X-DIGIKEY-Locale-Language': 'en', 'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
      body: JSON.stringify({ Keywords: '', Limit: 1, Offset: 0, FilterOptionsRequest: {
        CategoryFilter: [{ Id: category }],
        ...(maker && maker !== '0' ? { ManufacturerFilter: [{ Id: maker }] } : {}),
        MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }] } }),
    });
    if (r.status === 401 && attempt === 0) { tok = await token(true); continue; }
    const text = await r.text();
    if (!r.ok) { console.error(`count.js: HTTP ${r.status}: ${text.slice(0, 200)}`); process.exit(1); }
    const d = JSON.parse(text);
    console.log(`all\t${d.ProductsCount}`);
    const fo = d.FilterOptions || {};
    for (const f of fo.ParametricFilters || []) {
      if (![183, 538, 5, 707, 69].includes(f.ParameterId)) continue;
      for (const v of f.FilterValues) console.log(`${f.ParameterName}: ${v.ValueName}\t${v.ProductCount}`);
    }
    if (!maker || maker === '0') {
      for (const m of [...(fo.Manufacturers || [])].sort((a, b) => b.ProductCount - a.ProductCount).slice(0, 15)) {
        console.log(`maker: ${m.Id} ${m.Value}\t${m.ProductCount}`);
      }
    }
    console.error(`count.js: calls left today ${r.headers.get('x-ratelimit-remaining')}`);
    return;
  }
})();
