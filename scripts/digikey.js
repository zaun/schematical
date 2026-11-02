#!/usr/bin/env node
// Search DigiKey's catalog (Product Information API v4, KeywordSearch).
//
//   node scripts/digikey.js "ECS-100AX"
//   node scripts/digikey.js --category Oscillators --show Frequency "ECS-100AX"
//
// Filters take a DigiKey id or a name (case-insensitive):
//   --category <id|name>      e.g. 172 or "Oscillators"
//   --manufacturer <id|name>  e.g. 50 or "ECS Inc."
//   --series <id|name>        e.g. "ECS-100X"
//   --status <id|name>        e.g. Active, Obsolete; repeatable
// Output:
//   --show <param,...>        add parameter columns, e.g. "Frequency,Package / Case"
//   --limit <n>               most parts to list (default 500; pages of 50)
//   --json                    print the raw API responses instead of a table
//
// Credentials: DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET, or else the
// "Client ID:" and "Client Secret:" lines of digikey.md at the repo root
// (untracked; never commit it). Uses the 2-legged (client credentials) flow.

'use strict';

const fs = require('fs');
const os = require('os');
const path = require('path');

const API = 'https://api.digikey.com';
const TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const PAGE = 50; // the API's largest page

function usage(message) {
  if (message) console.error(`digikey: ${message}`);
  console.error('usage: node scripts/digikey.js [--category X] [--manufacturer X] [--series X]\n' +
    '                               [--status X] [--show p1,p2] [--limit N] [--json] "keywords"');
  process.exit(2);
}

function parseArgs(argv) {
  const opts = { status: [], show: [], limit: 500, json: false, keywords: [] };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    const value = () => {
      if (i + 1 >= argv.length) usage(`${arg} needs a value`);
      return argv[++i];
    };
    switch (arg) {
      case '--category': opts.category = value(); break;
      case '--manufacturer': opts.manufacturer = value(); break;
      case '--series': opts.series = value(); break;
      case '--status': opts.status.push(value()); break;
      case '--show': opts.show.push(...value().split(',').map((s) => s.trim()).filter(Boolean)); break;
      case '--limit': opts.limit = Number(value()); break;
      case '--json': opts.json = true; break;
      case '-h': case '--help': usage();
      default:
        if (arg.startsWith('--')) usage(`unknown option ${arg}`);
        opts.keywords.push(arg);
    }
  }
  if (!(opts.limit > 0)) usage('--limit must be a positive number');
  opts.keywords = opts.keywords.join(' ');
  if (!opts.keywords && !opts.category && !opts.manufacturer && !opts.series) usage('nothing to search for');
  return opts;
}

function credentials() {
  let id = process.env.DIGIKEY_CLIENT_ID;
  let secret = process.env.DIGIKEY_CLIENT_SECRET;
  if (!id || !secret) {
    const file = path.join(__dirname, '..', 'digikey.md');
    if (fs.existsSync(file)) {
      const text = fs.readFileSync(file, 'utf8');
      id = id || (text.match(/^Client ID:\s*(\S+)/mi) || [])[1];
      // Also accepts the "Clietn" spelling the file was written with.
      secret = secret || (text.match(/^Cli\w*\s+Secret:\s*(\S+)/mi) || [])[1];
    }
  }
  if (!id || !secret) {
    console.error('digikey: set DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET (or add them to digikey.md)');
    process.exit(1);
  }
  return { id, secret };
}

async function token(creds) {
  try {
    const cached = JSON.parse(fs.readFileSync(TOKEN_CACHE, 'utf8'));
    if (cached.id === creds.id && cached.expires > Date.now() + 30_000) return cached.token;
  } catch { /* no usable cache */ }

  const response = await fetch(`${API}/v1/oauth2/token`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams({
      client_id: creds.id, client_secret: creds.secret, grant_type: 'client_credentials',
    }),
  });
  const body = await response.json().catch(() => ({}));
  if (!response.ok || !body.access_token) {
    throw new Error(`login failed (${response.status}): ${JSON.stringify(body)}`);
  }
  const expires = Date.now() + (body.expires_in || 0) * 1000;
  fs.writeFileSync(TOKEN_CACHE, JSON.stringify({ id: creds.id, token: body.access_token, expires }), { mode: 0o600 });
  return body.access_token;
}

async function call(ctx, method, route, body) {
  const response = await fetch(`${API}/products/v4${route}`, {
    method,
    headers: {
      Authorization: `Bearer ${ctx.token}`,
      'X-DIGIKEY-Client-Id': ctx.creds.id,
      'X-DIGIKEY-Locale-Site': 'US',
      'X-DIGIKEY-Locale-Language': 'en',
      'X-DIGIKEY-Locale-Currency': 'USD',
      ...(body ? { 'Content-Type': 'application/json' } : {}),
    },
    body: body ? JSON.stringify(body) : undefined,
  });
  const text = await response.text();
  if (!response.ok) throw new Error(`${method} ${route} failed (${response.status}): ${text}`);
  return JSON.parse(text);
}

const isId = (value) => /^\d+$/.test(String(value));
const same = (a, b) => String(a).trim().toLowerCase() === String(b).trim().toLowerCase();

function flattenCategories(nodes, out = []) {
  for (const node of nodes || []) {
    out.push(node);
    flattenCategories(node.Children || node.ChildCategories, out);
  }
  return out;
}

// Turns names into DigiKey ids: categories and manufacturers from their own
// endpoints, series and status from a search's filter options.
async function resolveFilters(ctx, opts) {
  const filters = {};

  if (opts.category) {
    let id = opts.category;
    if (!isId(id)) {
      const all = flattenCategories((await call(ctx, 'GET', '/search/categories')).Categories);
      const hit = all.find((c) => same(c.Name, id));
      if (!hit) throw new Error(`no category named "${id}"`);
      id = hit.CategoryId;
    }
    filters.CategoryFilter = [{ Id: String(id) }];
  }

  if (opts.manufacturer) {
    let id = opts.manufacturer;
    if (!isId(id)) {
      const all = (await call(ctx, 'GET', '/search/manufacturers')).Manufacturers || [];
      const hit = all.find((m) => same(m.Name, id));
      if (!hit) throw new Error(`no manufacturer named "${id}"`);
      id = hit.Id;
    }
    filters.ManufacturerFilter = [{ Id: String(id) }];
  }

  const named = [];
  if (opts.series && !isId(opts.series)) named.push(['series', 'Series', 'SeriesFilter', [opts.series]]);
  else if (opts.series) filters.SeriesFilter = [{ Id: String(opts.series) }];
  const statusNames = opts.status.filter((s) => !isId(s));
  const statusIds = opts.status.filter(isId);
  if (statusIds.length) filters.StatusFilter = statusIds.map((s) => ({ Id: String(s) }));
  if (statusNames.length) named.push(['status', 'Status', 'StatusFilter', statusNames]);

  if (named.length) {
    const probe = await call(ctx, 'POST', '/search/keyword',
      { Keywords: opts.keywords, Limit: 1, Offset: 0, FilterOptionsRequest: filters });
    for (const [label, key, filterKey, names] of named) {
      const options = (probe.FilterOptions || {})[key] || [];
      for (const name of names) {
        const hit = options.find((o) => same(o.Value, name));
        if (!hit) {
          const known = options.map((o) => o.Value).join(', ') || 'none';
          throw new Error(`no ${label} "${name}" in these results (have: ${known})`);
        }
        (filters[filterKey] = filters[filterKey] || []).push({ Id: String(hit.Id) });
      }
    }
  }
  return filters;
}

function parameter(product, name) {
  const hit = (product.Parameters || []).find((p) => same(p.ParameterText, name));
  return hit ? hit.ValueText : '';
}

function printTable(products, total, opts) {
  const headers = ['Part number', 'Status', 'Stock', 'Series', ...opts.show, 'Description'];
  const rows = products.map((p) => [
    p.ManufacturerProductNumber || '',
    (p.ProductStatus && p.ProductStatus.Status) || '',
    String(p.QuantityAvailable ?? ''),
    (p.Series && p.Series.Name) || '',
    ...opts.show.map((name) => parameter(p, name)),
    (p.Description && p.Description.ProductDescription) || '',
  ]);
  const widths = headers.map((h, i) => Math.max(h.length, ...rows.map((r) => r[i].length)));
  const line = (cells) => cells.map((c, i) => (i === cells.length - 1 ? c : c.padEnd(widths[i]))).join('  ');
  console.log(line(headers));
  console.log(line(widths.map((w) => '-'.repeat(w))));
  for (const row of rows) console.log(line(row));
  const more = total > products.length ? ` (showing ${products.length}; raise --limit for more)` : '';
  console.log(`\n${total} part${total === 1 ? '' : 's'}${more}`);
}

async function main() {
  const opts = parseArgs(process.argv.slice(2));
  const creds = credentials();
  const ctx = { creds, token: await token(creds) };
  const filters = await resolveFilters(ctx, opts);

  const pages = [];
  const products = [];
  let total = 0;
  for (let offset = 0; offset < opts.limit; offset += PAGE) {
    const page = await call(ctx, 'POST', '/search/keyword', {
      Keywords: opts.keywords,
      Limit: Math.min(PAGE, opts.limit - offset),
      Offset: offset,
      FilterOptionsRequest: filters,
      SortOptions: { Field: 'ManufacturerProductNumber', SortOrder: 'Ascending' },
    });
    pages.push(page);
    total = page.ProductsCount || 0;
    products.push(...(page.Products || []));
    if (!page.Products || page.Products.length === 0 || products.length >= total) break;
  }

  if (opts.json) {
    console.log(JSON.stringify(pages.length === 1 ? pages[0] : pages, null, 2));
  } else {
    printTable(products, total, opts);
  }
}

main().catch((error) => {
  console.error(`digikey: ${error.message}`);
  process.exit(1);
});
