#!/usr/bin/env node
// plan-37 census (registers): every in-stock (qty > 0), Active or Last Time Buy
// part of one DigiKey (category, manufacturer) piece, with full parameters.
// usage: node list.js <category id|-> <manufacturer id|-> <package id|-> <label> <out.jsonl> [<keywords>]
// <package id> limits the piece to one "Package / Case" (param 16) value id
// (facets.js prints them). A piece above DigiKey's 300-result cap is split by
// the package values of the first page's facets. One JSON line per product; a
// summary line per piece goes to stderr: "piece <label>[/<pkg>]: <listed> of
// <count>" (+ " (cut at 300)"). (A "Number of Bits per Element", param 2207,
// filter returns no products from the v4 API, so it is not offered.)
'use strict';
const fs = require('fs'), os = require('os'), path = require('path');
const API = 'https://api.digikey.com', TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const PAGE = 50, CAP = 300, PACKAGE = 16;
const [category, maker, pkgArg, label, outFile, keywords = ''] = process.argv.slice(2);
if (!outFile) { console.error('usage: node list.js <cat|-> <maker|-> <pkg|-> <label> <out.jsonl> [keywords]'); process.exit(2); }
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
  const fo = { MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }] };
  if (category !== '-') fo.CategoryFilter = [{ Id: category }];
  if (maker !== '-') fo.ManufacturerFilter = [{ Id: maker }];
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
  const k = `${p.Manufacturer.Id}|${p.ManufacturerProductNumber}`; if (seen.has(k)) return; seen.add(k);
  const params = {}; for (const x of p.Parameters || []) params[x.ParameterText] = x.ValueText;
  out.write(JSON.stringify({ mpn: p.ManufacturerProductNumber, maker: p.Manufacturer.Name, piece,
    stock: p.QuantityAvailable, status: p.ProductStatus && p.ProductStatus.Status,
    datasheet: p.DatasheetUrl, series: p.Series && p.Series.Name,
    base: p.BaseProductNumber && p.BaseProductNumber.Name, description: p.Description && p.Description.ProductDescription,
    detail: p.Description && p.Description.DetailedDescription,
    category: (function leaf(c) { return c && c.ChildCategories && c.ChildCategories.length ? leaf(c.ChildCategories[0]) : c && c.Name; })(p.Category), params }) + '\n');
}
async function piece(pkg, name) {
  const first = await search(pkg, 0); const n = first.ProductsCount;
  if (n === undefined && first.Products && first.Products.length === 0) { console.error(`piece ${name}: 0 of 0`); return; }
  if (n === undefined) throw new Error(`no ProductsCount: ${JSON.stringify(first).slice(0, 400)}`);
  if (n > CAP && !pkg) {
    const f = (first.FilterOptions.ParametricFilters || []).find(x => x.ParameterId === PACKAGE);
    if (f) { for (const v of f.FilterValues) await piece(v.ValueId, `${name}/${v.ValueName}`); return; }
  }
  let listed = 0; for (const p of first.Products) { emit(p, name); listed++; }
  for (let off = PAGE; off < Math.min(n, CAP); off += PAGE) {
    const r = await search(pkg, off); for (const p of r.Products) { emit(p, name); listed++; } }
  console.error(`piece ${name}: ${listed} of ${n}${n > CAP ? ' (cut at 300)' : ''}`);
}
piece(pkgArg && pkgArg !== '-' ? pkgArg : null, label).then(() => { out.end(); console.error(`list.js: ${seen.size} parts, ${calls} calls, ${left} left today`); })
  .catch(e => { out.end(); console.error(`list.js: ${e.message} after ${calls} calls, ${left} left`); process.exit(1); });
