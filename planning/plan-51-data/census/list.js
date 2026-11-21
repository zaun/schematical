#!/usr/bin/env node
// plan-51 census: every in-stock Active / Last Time Buy part of DigiKey category 409
// (IC Sockets) of one maker, most-stocked first.
//
// usage: node list.js <maker id> <out.jsonl>
//
// The first call gives the count and the facets. A piece of 300 parts or fewer is read
// 50 parts a page, sorted by QuantityAvailable descending. A bigger one is split by the
// facet (Type, Mounting Type, Pitch, Positions) whose largest value is smallest, and each
// half is read the same way (DigiKey returns at most 300 results of a search; a piece no
// facet splits below 300 keeps its 300 most-stocked parts and the summary line says
// `(cut at 300: <stock of the 300th>)`).
// Every product is written once as one JSON line (mpn, maker, makerId, category, series,
// stock, status, datasheet, description, params).
// Credentials: DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET in the environment; the login
// token is the one digikey.js caches.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');

const API = 'https://api.digikey.com';
const TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const PAGE = 50;
const CAP = 300;
const [maker, outFile] = process.argv.slice(2);
if (!maker || !outFile) { console.error('usage: node list.js <maker id> <out.jsonl>'); process.exit(2); }
const category = '409';
const creds = { id: process.env.DIGIKEY_CLIENT_ID, secret: process.env.DIGIKEY_CLIENT_SECRET };
if (!creds.id || !creds.secret) { console.error('list.js: set DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET (see ~/.zshrc)'); process.exit(1); }

const PITCH = 89, POSITIONS = 1075, TYPE = 183, MOUNTING = 69, CURRENT = 2155, TERMINATION = 258;
const ids = (list) => list.map((x) => ({ Id: String(x) }));
let baseParams = [];
let seriesIds = [];

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

async function search(params, series, offset, limit, retried) {
  for (let attempt = 1; ; attempt++) {
    try {
      const options = { CategoryFilter: [{ Id: category }], ManufacturerFilter: [{ Id: maker }],
        MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }] };
      if (params.length) options.ParameterFilterRequest = { CategoryFilter: { Id: category }, ParameterFilters: params };
      if (series.length) options.SeriesFilter = ids(series);
      const r = await fetch(`${API}/products/v4/search/keyword`, {
        method: 'POST', signal: AbortSignal.timeout(180000),
        headers: { Authorization: `Bearer ${tok}`, 'X-DIGIKEY-Client-Id': creds.id, 'X-DIGIKEY-Locale-Site': 'US',
          'X-DIGIKEY-Locale-Language': 'en', 'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
        body: JSON.stringify({ Keywords: '', Limit: limit, Offset: offset, FilterOptionsRequest: options,
          SortOptions: { Field: 'QuantityAvailable', SortOrder: 'Descending' } }),
      });
      calls++;
      left = r.headers.get('x-ratelimit-remaining') ?? left;
      if (r.status === 401 && !retried) { tok = await token(true); return search(params, series, offset, limit, true); }
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
function emit(p) {
  const key = p.ManufacturerProductNumber;
  if (seen.has(key)) return false;
  seen.add(key);
  const params = {};
  for (const x of p.Parameters || []) params[x.ParameterText] = x.ValueText;
  out.write(JSON.stringify({ mpn: key, maker: p.Manufacturer && p.Manufacturer.Name, makerId: maker, category,
    series: (p.Series && p.Series.Name) || '', stock: p.QuantityAvailable,
    status: p.ProductStatus && p.ProductStatus.Status, datasheet: p.DatasheetUrl || '',
    description: (p.Description && p.Description.ProductDescription) || '', params }) + '\n');
  return true;
}

// The facet that splits `first` best: every value of it must hold fewer parts than the piece.
function splitFacet(first, count) {
  let best = null;
  for (const pid of [PITCH, POSITIONS, MOUNTING, TERMINATION, TYPE, CURRENT]) {
    const f = ((first.FilterOptions || {}).ParametricFilters || []).find((x) => x.ParameterId === pid);
    if (!f) continue;
    const vals = f.FilterValues.filter((v) => v.ProductCount > 0);
    if (vals.length < 2) continue;
    const biggest = Math.max(...vals.map((v) => v.ProductCount));
    if (biggest >= count) continue;
    if (!best || biggest < best.biggest) best = { pid, vals, biggest };
  }
  return best;
}

let pieces = 0;
async function read(params, series, label) {
  const first = await search(params, series, 0, PAGE);
  const count = first.ProductsCount || 0;
  if (count === 0) return 0;
  if (count > CAP) {
    const s = splitFacet(first, count);
    if (s) {
      let n = 0;
      for (const v of s.vals) {
        const sub = params.filter((p) => p.ParameterId !== s.pid).concat([{ ParameterId: s.pid, FilterValues: [{ Id: String(v.ValueId) }] }]);
        n += await read(sub, series, `${label}/${s.pid}=${v.ValueName}`);
      }
      return n;
    }
  }
  pieces++;
  let listed = 0;
  let lastStock = null;
  let page = first;
  for (let off = 0; off < Math.min(count, CAP); off += PAGE) {
    if (off > 0) page = await search(params, series, off, PAGE);
    const prods = page.Products || [];
    for (const p of prods) { emit(p); listed++; lastStock = p.QuantityAvailable; }
    if (prods.length < PAGE) break;
  }
  console.error(`piece ${label || 'all'}: ${listed} of ${count}${count > CAP ? ` (cut at ${CAP}: ${lastStock})` : ''}`);
  return listed;
}

(async () => {
  tok = await token(false);
  await read(baseParams, seriesIds, '');
  console.error(`list.js: ${seen.size} parts, ${pieces} pieces, ${calls} calls, ${left} calls left today`);
  out.end();
})().catch((e) => { console.error(`list.js: ${e.message}`); process.exit(1); });
