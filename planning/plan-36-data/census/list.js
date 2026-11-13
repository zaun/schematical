#!/usr/bin/env node
// plan-36 census: every in-stock (qty > 0), Active or Last Time Buy part of one
// DigiKey (category, manufacturer[, series]) piece, with full parameters.
// usage: node list.js <category id> <manufacturer id> <series id|-> <label> <out.jsonl> [<keywords>]
// A piece above DigiKey's 300-result cap is split by "Package / Case" (param 16)
// values from the first page's facets. One JSON line per product; a summary
// line per piece goes to stderr: "piece <label>[/<pkg>]: <listed> of <count>".
'use strict';
const fs = require('fs'), os = require('os'), path = require('path');
const API = 'https://api.digikey.com', TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const PAGE = 50, CAP = 300, PACKAGE = 16;
const [category, maker, series, label, outFile, keywords = ''] = process.argv.slice(2);
if (!outFile) { console.error('usage: node list.js <cat> <maker> <series|-> <label> <out.jsonl> [keywords]'); process.exit(2); }
const creds = { id: process.env.DIGIKEY_CLIENT_ID, secret: process.env.DIGIKEY_CLIENT_SECRET };
if (!creds.id || !creds.secret) { console.error('list.js: set DIGIKEY_CLIENT_ID/SECRET'); process.exit(1); }
let tok = null, calls = 0, left = null, tokAt = 0;
async function token(fresh) {
  if (!fresh) { try { const c = JSON.parse(fs.readFileSync(TOKEN_CACHE, 'utf8'));
    if (c.id === creds.id && c.expires > Date.now() + 60000) return c.token; } catch {} }
  const r = await fetch(`${API}/v1/oauth2/token`, { method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams({ client_id: creds.id, client_secret: creds.secret, grant_type: 'client_credentials' }) });
  const b = await r.json(); if (!b.access_token) throw new Error(`login failed (${r.status})`);
  fs.writeFileSync(TOKEN_CACHE, JSON.stringify({ id: creds.id, token: b.access_token,
    expires: Date.now() + (b.expires_in || 0) * 1000 }), { mode: 0o600 });
  return b.access_token;
}
async function search(pkg, offset) {
  if (!tok || Date.now() - tokAt > 8 * 60000) { tok = await token(false); tokAt = Date.now(); }
  const fo = { CategoryFilter: [{ Id: category }], ManufacturerFilter: [{ Id: maker }],
    MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }] };
  if (series !== '-') fo.SeriesFilter = [{ Id: String(series) }];
  if (pkg) fo.ParameterFilterRequest = { CategoryFilter: { Id: category },
    ParameterFilters: [{ ParameterId: PACKAGE, FilterValues: [{ Id: pkg }] }] };
  for (let attempt = 1; ; attempt++) {
    const r = await fetch(`${API}/products/v4/search/keyword`, { method: 'POST',
      headers: { Authorization: `Bearer ${tok}`, 'X-DIGIKEY-Client-Id': creds.id,
        'X-DIGIKEY-Locale-Site': 'US', 'X-DIGIKEY-Locale-Language': 'en',
        'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
      body: JSON.stringify({ Keywords: keywords, Limit: PAGE, Offset: offset, FilterOptionsRequest: fo }) });
    calls++; const l = r.headers.get('x-ratelimit-remaining'); if (l !== null) left = l;
    if (r.status === 401 && attempt < 3) { tok = await token(true); tokAt = Date.now(); continue; }
    if (r.status === 429) throw new Error('rate limited');
    if (r.status >= 500 && attempt < 3) { await new Promise(s => setTimeout(s, 5000)); continue; }
    if (!r.ok) throw new Error(`HTTP ${r.status}: ${(await r.text()).slice(0, 200)}`);
    return r.json();
  }
}
const out = fs.createWriteStream(outFile, { flags: 'a' });
const seen = new Set();
function emit(p, piece) {
  if (seen.has(p.ManufacturerProductNumber)) return; seen.add(p.ManufacturerProductNumber);
  const params = {}; for (const x of p.Parameters || []) params[x.ParameterText] = x.ValueText;
  out.write(JSON.stringify({ mpn: p.ManufacturerProductNumber, maker: p.Manufacturer.Name, piece,
    stock: p.QuantityAvailable, status: p.ProductStatus && p.ProductStatus.Status,
    datasheet: p.DatasheetUrl, series: p.Series && p.Series.Name,
    base: p.BaseProductNumber && p.BaseProductNumber.Name, description: p.Description && p.Description.ProductDescription,
    detail: p.Description && p.Description.DetailedDescription, params }) + '\n');
}
async function piece(pkg, name) {
  const first = await search(pkg, 0); const n = first.ProductsCount;
  if (n > CAP && !pkg) {
    const f = (first.FilterOptions.ParametricFilters || []).find(x => x.ParameterId === PACKAGE);
    if (f) { for (const v of f.FilterValues) await piece(v.ValueId, v.ValueName); return; }
  }
  let listed = 0; for (const p of first.Products) { emit(p, name); listed++; }
  for (let off = PAGE; off < Math.min(n, CAP); off += PAGE) {
    const r = await search(pkg, off); for (const p of r.Products) { emit(p, name); listed++; } }
  console.error(`piece ${name}: ${listed} of ${n}${n > CAP ? ' (cut at 300)' : ''}`);
}
piece(null, label).then(() => { out.end(); console.error(`list.js: ${seen.size} parts, ${calls} calls, ${left} left today`); })
  .catch(e => { out.end(); console.error(`list.js: ${e.message} after ${calls} calls, ${left} left`); process.exit(1); });
