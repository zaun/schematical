#!/usr/bin/env node
// plan-44 census: every in-stock Active / Last Time Buy part of one DigiKey
// crystal / resonator category and one maker id, by series, most-stocked
// first.
//
// usage: node list.js <category id> <manufacturer id> <out.jsonl>
//   category 171 = Crystals, 174 = Resonators.
//
// One call lists the makers' series (the `Series` facet, with counts). Each
// series is then listed 50 parts a page, sorted by QuantityAvailable
// descending, up to 6 pages (DigiKey's 300-result cap): a series of 300 parts
// or fewer is complete; a larger one keeps its 300 most-stocked parts. Every
// product is written as one JSON line (mpn, maker, category, series, stock,
// status, datasheet, description, params). The summary per series goes to
// stderr:
//   series <name>: <listed> of <count> [(cut at 300: <stock of the 300th>)]
// A part listed under several packings (TR, bulk) is written once.
// Credentials: DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET in the environment;
// the login token is the one digikey.js caches.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');

const API = 'https://api.digikey.com';
const TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const PAGE = 50;
const CAP = 300;
const [category, maker, outFile] = process.argv.slice(2);
if (!category || !maker || !outFile) { console.error('usage: node list.js <category id> <manufacturer id> <out.jsonl>'); process.exit(2); }
const creds = { id: process.env.DIGIKEY_CLIENT_ID, secret: process.env.DIGIKEY_CLIENT_SECRET };
if (!creds.id || !creds.secret) { console.error('list.js: set DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET (see ~/.zshrc)'); process.exit(1); }

let tok = null;
let calls = 0;
let left = null;

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

async function search(series, offset, limit, retried) {
  for (let attempt = 1; ; attempt++) {
    try {
      const r = await fetch(`${API}/products/v4/search/keyword`, {
        method: 'POST', signal: AbortSignal.timeout(180000),
        headers: { Authorization: `Bearer ${tok}`, 'X-DIGIKEY-Client-Id': creds.id, 'X-DIGIKEY-Locale-Site': 'US',
          'X-DIGIKEY-Locale-Language': 'en', 'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
        body: JSON.stringify({ Keywords: '', Limit: limit, Offset: offset,
          FilterOptionsRequest: { CategoryFilter: [{ Id: category }], ManufacturerFilter: [{ Id: maker }],
            MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }],
            ...(series ? { SeriesFilter: [{ Id: String(series) }] } : {}) },
          SortOptions: { Field: 'QuantityAvailable', SortOrder: 'Descending' } }),
      });
      calls++;
      left = r.headers.get('x-ratelimit-remaining') ?? left;
      if (r.status === 401 && !retried) { tok = await token(true); return search(series, offset, limit, true); }
      const text = await r.text();
      if (r.status === 429) throw Object.assign(new Error(`rate limited: ${text}`), { fatal: true });
      if (!r.ok) throw new Error(`HTTP ${r.status}: ${text.slice(0, 200)}`);
      return JSON.parse(text);
    } catch (e) {
      if (e.fatal || attempt >= 4) throw e;
      console.error(`list.js: ${e.message}; retry ${attempt} of 3`);
      await new Promise((res) => setTimeout(res, 2000 * attempt));
    }
  }
}

const seen = new Set();
const out = fs.createWriteStream(outFile, { flags: 'w' });
function emit(p, series) {
  const key = p.ManufacturerProductNumber;
  if (seen.has(key)) return;
  seen.add(key);
  const params = {};
  for (const x of p.Parameters || []) params[x.ParameterText] = x.ValueText;
  out.write(JSON.stringify({ mpn: key, maker: p.Manufacturer && p.Manufacturer.Name, makerId: maker, category,
    series: (p.Series && p.Series.Name) || series, stock: p.QuantityAvailable,
    status: p.ProductStatus && p.ProductStatus.Status, datasheet: p.DatasheetUrl || '',
    description: (p.Description && p.Description.ProductDescription) || '', params }) + '\n');
}

(async () => {
  tok = await token(false);
  const first = await search(null, 0, 1);
  const list = ((first.FilterOptions || {}).Series || []).map((s) => ({ id: s.Id, name: s.Value, count: s.ProductCount }));
  console.error(`maker ${maker} category ${category}: ${first.ProductsCount} parts, ${list.length} series`);
  for (const s of list) {
    let listed = 0;
    let lastStock = null;
    for (let off = 0; off < Math.min(s.count, CAP); off += PAGE) {
      const page = await search(s.id, off, PAGE);
      const prods = page.Products || [];
      for (const p of prods) { emit(p, s.name); listed++; lastStock = p.QuantityAvailable; }
      if (prods.length < PAGE) break;
    }
    console.error(`series ${s.name}: ${listed} of ${s.count}${s.count > CAP ? ` (cut at ${CAP}: ${lastStock})` : ''}`);
  }
  console.error(`list.js: ${seen.size} parts, ${calls} calls, ${left} calls left today`);
  out.end();
})().catch((e) => { console.error(`list.js: ${e.message}`); process.exit(1); });
