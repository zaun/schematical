#!/usr/bin/env node
// plan-34 census: the most-stocked in-stock resistors of one maker and one
// DigiKey category, for the tolerances ±1%, ±5% and Jumper (0 Ω).
//
// usage: node top.js <category id> <manufacturer id> <out.jsonl> [<tolerance> [<package value id> <package name>]]
//   the optional tail re-lists one piece only (plan-34-A § 5).
//
// Each slice is one tolerance. A slice above DigiKey's 300-result cap is split
// by the first facet of SPLITS that covers it; a piece no facet splits keeps its first 300 by
// QuantityAvailable, descending, which are the most-stocked ones (the census
// only needs those: plan-34-A § 3). Every product is written as one JSON line
// with the fields select.py reads; the summary per piece goes to stderr as
//   piece <tolerance>[/<package>]: <listed> of <count> (cut at 300: <stock of the 300th>)
// Credentials: DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET in the environment.
'use strict';

const fs = require('fs');
const os = require('os');
const path = require('path');

const API = 'https://api.digikey.com';
const TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const PAGE = 50;
const CAP = 300;
const TOLERANCE = 3;        // DigiKey parameter id of "Tolerance"
const PACKAGE = 16;         // DigiKey parameter id of "Package / Case"
const TOLERANCES = [['1131', '±1%'], ['2503', '±5%'], ['362150', 'Jumper']];

const [category, maker, outFile, ...only] = process.argv.slice(2);
if (!category || !maker || !outFile) {
  console.error('usage: node top.js <category id> <manufacturer id> <out.jsonl>');
  process.exit(2);
}
const creds = { id: process.env.DIGIKEY_CLIENT_ID, secret: process.env.DIGIKEY_CLIENT_SECRET };
if (!creds.id || !creds.secret) {
  console.error('top.js: set DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET (see ~/.zshrc)');
  process.exit(1);
}

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
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams({ client_id: creds.id, client_secret: creds.secret, grant_type: 'client_credentials' }),
  });
  const body = await r.json();
  if (!body.access_token) throw new Error(`login failed (${r.status})`);
  fs.writeFileSync(TOKEN_CACHE, JSON.stringify({ id: creds.id, token: body.access_token,
    expires: Date.now() + (body.expires_in || 0) * 1000 }), { mode: 0o600 });
  return body.access_token;
}

async function search(filt, offset, retried) {
  for (let attempt = 1; ; attempt++) {
    try {
      const r = await fetch(`${API}/products/v4/search/keyword`, {
        method: 'POST',
        signal: AbortSignal.timeout(180000),
        headers: {
          Authorization: `Bearer ${tok}`, 'X-DIGIKEY-Client-Id': creds.id,
          'X-DIGIKEY-Locale-Site': 'US', 'X-DIGIKEY-Locale-Language': 'en',
          'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          Keywords: '', Limit: PAGE, Offset: offset,
          FilterOptionsRequest: {
            CategoryFilter: [{ Id: category }], ManufacturerFilter: [{ Id: maker }],
            MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }, { Id: '4' }],
            ParameterFilterRequest: { CategoryFilter: { Id: category }, ParameterFilters: filt.params },
            ...(filt.series ? { SeriesFilter: [{ Id: String(filt.series) }] } : {}),
          },
          SortOptions: { Field: 'QuantityAvailable', SortOrder: 'Descending' },
        }),
      });
      calls++;
      left = r.headers.get('x-ratelimit-remaining') ?? left;
      if (r.status === 401 && !retried) { tok = await token(true); return search(filt, offset, true); }
      const text = await r.text();
      if (r.status === 429) throw Object.assign(new Error(`rate limited: ${text}`), { fatal: true });
      if (!r.ok) throw new Error(`HTTP ${r.status}: ${text.slice(0, 200)}`);
      return JSON.parse(text);
    } catch (e) {
      if (e.fatal || attempt >= 4) throw e;
      console.error(`top.js: ${e.message}; retry ${attempt} of 3`);
      await new Promise((res) => setTimeout(res, 2000 * attempt));
    }
  }
}

const out = fs.createWriteStream(outFile);
const seen = new Set();

function emit(p, piece) {
  const key = p.ManufacturerProductNumber;
  if (seen.has(key)) return;
  seen.add(key);
  const params = {};
  for (const x of p.Parameters || []) params[x.ParameterText] = x.ValueText;
  out.write(JSON.stringify({
    mpn: key, maker: p.Manufacturer && p.Manufacturer.Name, makerId: maker, category,
    piece, stock: p.QuantityAvailable, status: p.ProductStatus && p.ProductStatus.Status,
    datasheet: p.DatasheetUrl || '', series: (p.Series && p.Series.Name) || '',
    base: p.BaseProductNumber && p.BaseProductNumber.Name, description:
      (p.Description && p.Description.ProductDescription) || '', params,
  }) + '\n');
}

// The facets a piece may be split by, in this order (each once per path):
// Package / Case, Series, Features, Composition, Power, Temperature Coefficient.
const SPLITS = [['param', PACKAGE], ['series'], ['param', 5], ['param', 174], ['param', 2], ['param', 17]];

function facetValues(page, split) {
  const fo = page.FilterOptions || {};
  if (split[0] === 'series') {
    return (fo.Series || []).map((v) => ({ name: v.Value, count: v.ProductCount,
      apply: (f) => ({ ...f, series: v.Id }) }));
  }
  const facet = (fo.ParametricFilters || []).find((x) => x.ParameterId === split[1]);
  return ((facet && facet.FilterValues) || []).map((v) => ({ name: v.ValueName, count: v.ProductCount,
    apply: (f) => ({ ...f, params: [...f.params, { ParameterId: split[1], FilterValues: [{ Id: v.ValueId }] }] }) }));
}

async function piece(label, filt, used) {
  const first = await search(filt, 0);
  const count = first.ProductsCount || 0;
  if (count > CAP) {
    for (let i = 0; i < SPLITS.length; i++) {
      if (used.includes(i)) continue;
      // Automotive and current-sense parts are out (plan-34-A § 3 rule 2):
      // their Features pieces are never listed.
      const values = facetValues(first, SPLITS[i]).filter((v) =>
        !(SPLITS[i][1] === 5 && /Automotive|Current Sense/.test(v.name)));
      const all = facetValues(first, SPLITS[i]);
      const covered = all.reduce((s, v) => s + v.count, 0);
      const largest = Math.max(0, ...all.map((v) => v.count));
      if (all.length > 1 && covered >= count && largest < count) {
        for (const v of values) {
          if (v.count) await piece(`${label}/${v.name}`, v.apply(filt), [...used, i]);
        }
        return;
      }
    }
  }
  let products = first.Products || [];
  products.forEach((p) => emit(p, label));
  const end = Math.min(count, CAP);
  let last = products.length ? products[products.length - 1].QuantityAvailable : 0;
  for (let off = PAGE; off < end; off += PAGE) {
    products = (await search(filt, off)).Products || [];
    products.forEach((p) => emit(p, label));
    if (products.length) last = products[products.length - 1].QuantityAvailable;
  }
  console.error(`piece ${label}: ${Math.min(count, CAP)} of ${count}` +
    (count > CAP ? ` (cut at 300: ${last})` : ''));
}

(async () => {
  tok = await token(false);
  for (const [id, name] of TOLERANCES) {
    if (only.length && only[0] !== name) continue;
    const filt = { params: [{ ParameterId: TOLERANCE, FilterValues: [{ Id: id }] }] };
    if (only.length > 1) {
      // Re-list one cut piece: <tolerance> <package value id>.
      filt.params.push({ ParameterId: PACKAGE, FilterValues: [{ Id: only[1] }] });
      await piece(`${name}/${only[2] || only[1]}`, filt, [0]);
    } else {
      await piece(name, filt, []);
    }
  }
  out.end();
  console.error(`top.js: ${category}/${maker}: ${seen.size} parts, ${calls} calls, ${left} left today`);
})().catch((e) => { console.error(`top.js: ${e.message}; ${calls} calls, ${left} left`); process.exit(1); });
