#!/usr/bin/env node
// plan-40 census: every in-stock Active / Last Time Buy part of one DigiKey
// transistor category and one maker id, listed by stock (highest first).
//
// usage: node top.js <category id> <manufacturer id> <out.jsonl>
//
// A search above DigiKey's 300-result cap is split once, by the type facet
// (Transistor Type / FET Type / Configuration); a piece still above 300 keeps
// its first 300 by QuantityAvailable, descending, which are its 300
// most-stocked parts (`cut at 300` on its summary line); a piece of 301 to 600
// parts is also listed from the other end (ascending), so it is complete. Every product is written
// as one JSON line (mpn, maker, category, piece, stock, status, datasheet,
// series, base, description, params). The summary per piece goes to stderr:
//   piece <label>: <listed> of <count> (cut at 300: <stock of the 300th>)
// Credentials: DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET in the environment.
// A piece DigiKey refuses with HTTP 400 is written as `piece <label>: failed`
// and the run goes on. The login token is the one digikey.js caches.
'use strict';

const fs = require('fs');
const os = require('os');
const path = require('path');

const API = 'https://api.digikey.com';
const TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const PAGE = 50;
const CAP = 300;
// The type facet per category: Transistor Type (BJT 276 277 292 293),
// FET Type (278), Configuration (MOSFET arrays 289).
const TYPE_FACET = { 276: 74, 277: 74, 292: 74, 293: 74, 278: 606, 289: 207 };

const [category, maker, outFile, splitArg] = process.argv.slice(2);
if (!category || !maker || !outFile || !TYPE_FACET[category]) {
  console.error('usage: node top.js <category id> <manufacturer id> <out.jsonl> [<split facet ids, comma separated>]');
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
            // Without a parameter filter the request must not carry one: DigiKey then
            // returns the facets the split reads.
            ...(filt.params.length
              ? { ParameterFilterRequest: { CategoryFilter: { Id: category }, ParameterFilters: filt.params } }
              : {}),
            ...(filt.series ? { SeriesFilter: [{ Id: String(filt.series) }] } : {}),
          },
          SortOptions: { Field: 'QuantityAvailable', SortOrder: filt.ascending ? 'Ascending' : 'Descending' },
        }),
      });
      calls++;
      left = r.headers.get('x-ratelimit-remaining') ?? left;
      if (r.status === 401 && !retried) { tok = await token(true); return search(filt, offset, true); }
      const text = await r.text();
      if (r.status === 429) throw Object.assign(new Error(`rate limited: ${text}`), { fatal: true });
      if (r.status === 400) {
        throw Object.assign(new Error(`HTTP 400: ${text.slice(0, 200)}`), { fatal: true, refused: true });
      }
      if (!r.ok) throw new Error(`HTTP ${r.status}: ${text.slice(0, 200)}`);
      return JSON.parse(text);
    } catch (e) {
      if (e.fatal || attempt >= 4) throw e;
      console.error(`top.js: ${e.message}; retry ${attempt} of 3`);
      await new Promise((res) => setTimeout(res, 2000 * attempt));
    }
  }
}

const seen = new Set();
const out = fs.createWriteStream(outFile, { flags: 'w' });

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

// The facets a piece may be split by, in this order (each once per path).
// By default one split, the type facet; recheck.sh passes more (74,16 = type, then Package / Case).
const SPLITS = (splitArg || String(TYPE_FACET[category])).split(',').map((id) => ['param', Number(id)]);

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
  let first;
  try {
    first = await search(filt, 0);
  } catch (e) {
    if (!e.refused) throw e;
    console.error(`piece ${label}: failed: ${e.message.replace(/\s+/g, ' ')}`);
    return;
  }
  const count = first.ProductsCount || 0;
  if (count > CAP) {
    for (let i = 0; i < SPLITS.length; i++) {
      if (used.includes(i)) continue;
      const values = facetValues(first, SPLITS[i]);
      const largest = Math.max(0, ...values.map((v) => v.count));
      if (values.length > 1 && largest < count) {
        // Some parts carry no value of the facet (the facet counts sum to less
        // than the piece), so the piece's own 300 most-stocked parts are listed
        // as well, below; the union is deduplicated by part number.
        for (const v of values) {
          if (v.count) await piece(`${label}/${v.name}`, v.apply(filt), [...used, i]);
        }
        break;
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
  if (count > CAP && count <= 2 * CAP) {
    // 301 to 600 parts: the other end, ascending by stock, lists the rest, so the
    // piece is complete (the top 300 above and the lowest count - 300 here).
    for (let off = 0; off < count - CAP; off += PAGE) {
      ((await search({ ...filt, ascending: true }, off)).Products || []).slice(0, count - CAP - off)
        .forEach((p) => emit(p, label));
    }
    console.error(`piece ${label}: ${count} of ${count} (both ends)`);
    return;
  }
  console.error(`piece ${label}: ${Math.min(count, CAP)} of ${count}` +
    (count > CAP ? ` (cut at 300: ${last})` : ''));
}

(async () => {
  tok = await token(false);
  await piece('all', { params: [] }, []);
  out.end();
  console.error(`top.js: ${category}/${maker}: ${seen.size} parts, ${calls} calls, ${left} left today`);
})().catch((e) => { console.error(`top.js: ${e.message}; ${calls} calls, ${left} left`); process.exit(1); });
