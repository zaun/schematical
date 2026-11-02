#!/usr/bin/env node
// Search DigiKey's catalog (Product Information API v4, KeywordSearch).
// Run with no arguments, or --help, for instructions (HELP below).

'use strict';

const fs = require('fs');
const os = require('os');
const path = require('path');

const API = 'https://api.digikey.com';
const TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const PAGE = 50; // the API's largest page

const HELP = `Search DigiKey's catalog.

usage: node scripts/digikey.js [options] ["keywords"]

Examples
  node scripts/digikey.js "ECS-100AX"
  node scripts/digikey.js --show "Frequency,Package / Case" "ECS-2100AX"
  node scripts/digikey.js --category Oscillators --manufacturer ECS --active-only --list
  node scripts/digikey.js --category 172 --manufacturer 50 --series ECS-2100X --list
  node scripts/digikey.js --list --pages 3 --category Oscillators --manufacturer ECS

Filters (sent to DigiKey; each takes a DigiKey id or a name)
  --category <id|name>      e.g. 172 or Oscillators
  --manufacturer <id|name>  e.g. 50 or ECS ("ECS Inc."; a unique prefix works)
  --series <id|name>        e.g. ECS-2100X (the Series column of the table)
  --status <id|name>        e.g. Active, Obsolete, "Not For New Designs"; repeatable
  --active-only             only Active parts (cannot be combined with --status)
  --in-stock                only parts DigiKey has in stock (quantity above 0)

Output
  (default)                 a table: part number, status, stock, series, description
  --list                    <part number> <TAB> <status> <TAB> <datasheet url>
  --show <param,...>        add table columns from DigiKey's parameters,
                            e.g. "Frequency,Package / Case,Voltage - Supply"
  --json                    each raw API response, one JSON line per page
  --limit <n>               most parts (default 500; with --list, all)
  --pages <n>               most result pages (50 parts each)
  -h, --help                this text

Parts print as they arrive, so an error part way keeps what came before. A
summary line goes to stderr: parts found, pages, API calls, calls left today.

Things to know
  * Credentials: DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET in the
    environment (set in ~/.zshrc). The login token lasts 10 minutes and is
    cached in the temp directory; it is renewed when it expires.
  * Calls are rationed: about 1000 a day. Every page is one call, and each
    name given to a filter costs one more to look up; ids skip that
    (Oscillators = 172, ECS Inc. = 50). Try --pages 1 or --limit first.
  * DigiKey returns at most the first 300 results of any one search. A bigger
    search is split into smaller ones by category, manufacturer, series,
    status, then whichever parameter splits it best, and the results merged
    without duplicates. Listing everything is roughly one call per 50 parts
    plus one per piece. A piece that cannot be split below 300 is cut short
    with a warning (DigiKey uses "*" as the series of some parts).
  * Keywords are fuzzy, not a part-number prefix. "ECS-" and "ECS-*" match
    every ECS part (the maker's name), "ECX-" also finds ECX2-..., and
    "ECS-21" finds the same parts as "ECS-2100". A fuller part number
    ("ECS-100AX") or --series narrows the search. With a manufacturer
    filter, leave the keywords out to get all of its parts.
  * There is no filter on part names; to keep only some, filter the --list
    output (e.g. | grep '^ECS-'). That does not save calls.
  * One part can be listed several times, once per packing: a -TR suffix
    is tape and reel, otherwise often bulk, tube or cut tape. They are the
    same part.
  * Status ids: 0 Active, 1 Obsolete, 7 Not For New Designs.
  * The parts library takes only in-stock parts, never made-to-order ones
    (AGENTS.md): check with --in-stock.
`;

function usage(message) {
  if (!message) {
    process.stdout.write(HELP);
    process.exit(0);
  }
  console.error(`digikey: ${message}\nRun with no arguments for help.`);
  process.exit(2);
}

function parseArgs(argv) {
  if (argv.length === 0) usage();
  const opts = { status: [], show: [], limit: null, pages: Infinity, list: false, json: false, keywords: [] };
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
      case '--active-only': opts.activeOnly = true; break;
      case '--in-stock': opts.inStock = true; break;
      case '--show': opts.show.push(...value().split(',').map((s) => s.trim()).filter(Boolean)); break;
      case '--limit': opts.limit = Number(value()); break;
      case '--pages': opts.pages = Number(value()); break;
      case '--json': opts.json = true; break;
      case '--list': opts.list = true; break;
      case '-h': case '--help': usage();
      default:
        if (arg.startsWith('--')) usage(`unknown option ${arg}`);
        opts.keywords.push(arg);
    }
  }
  if (opts.limit === null) opts.limit = opts.list ? Infinity : 500;
  if (!(opts.limit > 0)) usage('--limit must be a positive number');
  if (opts.activeOnly && opts.status.length) usage('--active-only and --status do not mix');
  if (!(opts.pages > 0)) usage('--pages must be a positive number');
  opts.keywords = opts.keywords.join(' ');
  if (!opts.keywords && !opts.category && !opts.manufacturer && !opts.series && !opts.status.length &&
      !opts.activeOnly && !opts.inStock) {
    usage('nothing to search for');
  }
  return opts;
}

function credentials() {
  const id = process.env.DIGIKEY_CLIENT_ID;
  const secret = process.env.DIGIKEY_CLIENT_SECRET;
  if (!id || !secret) {
    console.error('digikey: set DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET (see ~/.zshrc)');
    process.exit(1);
  }
  return { id, secret };
}

async function token(creds, fresh = false) {
  if (!fresh) try {
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

async function call(ctx, method, route, body, retried = false) {
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
  ctx.calls += 1;
  const remaining = response.headers.get('x-ratelimit-remaining');
  if (remaining !== null) ctx.remaining = remaining;
  const text = await response.text();
  if (response.status === 401 && !retried) {
    // Tokens last 10 minutes; a long run outlives one.
    ctx.token = await token(ctx.creds, true);
    return call(ctx, method, route, body, true);
  }
  if (response.status === 429) throw new Error(`rate limited by DigiKey: ${text}`);
  if (!response.ok) throw new Error(`${method} ${route} failed (${response.status}): ${text}`);
  return JSON.parse(text);
}

const ACTIVE_STATUS = '0';
const isId = (value) => /^\d+$/.test(String(value));
const norm = (value) => String(value).trim().toLowerCase();
const same = (a, b) => norm(a) === norm(b);

// The option named `name`: an exact match, else the only one starting with
// it as a whole word ("ECS" → "ECS Inc."), else the only one containing it.
function pick(options, nameOf, name, label) {
  const want = norm(name);
  const escaped = want.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  const tiers = [
    (o) => norm(nameOf(o)) === want,
    (o) => new RegExp(`^${escaped}(?![a-z0-9])`).test(norm(nameOf(o))),
    (o) => norm(nameOf(o)).includes(want),
  ];
  for (const test of tiers) {
    const hits = options.filter(test);
    if (hits.length === 1) return hits[0];
    if (hits.length > 1) {
      const names = hits.slice(0, 10).map(nameOf).join('; ');
      throw new Error(`${label} "${name}" is ambiguous: ${names}${hits.length > 10 ? '; …' : ''}`);
    }
  }
  throw new Error(`no ${label} named "${name}"`);
}

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
      id = pick(all, (c) => c.Name, id, 'category').CategoryId;
    }
    filters.CategoryFilter = [{ Id: String(id) }];
  }

  if (opts.manufacturer) {
    let id = opts.manufacturer;
    if (!isId(id)) {
      const all = (await call(ctx, 'GET', '/search/manufacturers')).Manufacturers || [];
      id = pick(all, (m) => m.Name, id, 'manufacturer').Id;
    }
    filters.ManufacturerFilter = [{ Id: String(id) }];
  }

  const named = [];
  if (opts.series && !isId(opts.series)) named.push(['series', 'Series', 'SeriesFilter', [opts.series]]);
  else if (opts.series) filters.SeriesFilter = [{ Id: String(opts.series) }];
  // DigiKey's status ids: 0 Active, 1 Obsolete, 7 Not For New Designs, ...
  if (opts.activeOnly) filters.StatusFilter = [{ Id: ACTIVE_STATUS }];
  if (opts.inStock) filters.MinimumQuantityAvailable = 1;
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
        const hit = pick(options, (o) => o.Value, name, label);
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

// Table rows print one at a time, so columns have fixed widths; the last
// column (description) is not padded.
function tableLine(cells, opts) {
  const widths = [20, 20, 7, 20, ...opts.show.map(() => 16)];
  return cells.map((c, i) => (i < widths.length ? String(c).padEnd(widths[i]) : c)).join('  ');
}

function tableHeader(opts) {
  const headers = ['Part number', 'Status', 'Stock', 'Series', ...opts.show, 'Description'];
  return tableLine(headers, opts) + '\n' + tableLine(headers.map((h) => '-'.repeat(h.length)), opts);
}

function tableRow(p, opts) {
  return tableLine([
    p.ManufacturerProductNumber || '',
    (p.ProductStatus && p.ProductStatus.Status) || '',
    String(p.QuantityAvailable ?? ''),
    (p.Series && p.Series.Name) || '',
    ...opts.show.map((name) => parameter(p, name)),
    (p.Description && p.Description.ProductDescription) || '',
  ], opts);
}

function listRow(p) {
  const status = (p.ProductStatus && p.ProductStatus.Status) || '';
  return `${p.ManufacturerProductNumber || ''}\t${status}\t${p.DatasheetUrl || ''}`;
}

const CAP = 300; // the API refuses Offset + Limit above this

// Ways to split a search, each one a filter the search doesn't have yet:
// [label, the options DigiKey reported, the filter that selects one option].
function splits(page, filters) {
  const options = page.FilterOptions || {};
  const out = [];
  const base = (label, key, list) => {
    if (!filters[key] && list && list.length > 1) {
      out.push([label, list.map((o) => ({
        name: o.Value, count: o.ProductCount, apply: (f) => ({ ...f, [key]: [{ Id: String(o.Id) }] }),
      }))]);
    }
  };
  if (!filters.CategoryFilter && options.TopCategories && options.TopCategories.length > 1) {
    out.push(['category', options.TopCategories.map((t) => ({
      name: t.Category.Name, count: t.Category.ProductCount,
      apply: (f) => ({ ...f, CategoryFilter: [{ Id: String(t.Category.Id) }] }),
    }))]);
  }
  base('manufacturer', 'ManufacturerFilter', options.Manufacturers);
  base('series', 'SeriesFilter', options.Series);
  base('status', 'StatusFilter', options.Status);
  const used = new Set(((filters.ParameterFilterRequest || {}).ParameterFilters || []).map((p) => p.ParameterId));
  for (const param of options.ParametricFilters || []) {
    if (used.has(param.ParameterId) || !param.FilterValues || param.FilterValues.length < 2) continue;
    const category = String(param.Category.Id);
    const current = filters.ParameterFilterRequest;
    if (current && current.CategoryFilter.Id !== category) continue;
    out.push([param.ParameterName, param.FilterValues.map((v) => ({
      name: v.ValueName, count: v.ProductCount,
      apply: (f) => ({
        ...f,
        ParameterFilterRequest: {
          CategoryFilter: { Id: category },
          ParameterFilters: [...((current || {}).ParameterFilters || []),
            { ParameterId: param.ParameterId, FilterValues: [{ Id: v.ValueId }] }],
        },
      }),
    }))]);
  }
  return out;
}

// The split that covers every result with the smallest largest piece.
function bestSplit(page, filters) {
  const total = page.ProductsCount || 0;
  let best = null;
  for (const [label, parts] of splits(page, filters)) {
    const covered = parts.reduce((sum, p) => sum + (p.count || 0), 0);
    if (covered < total) continue;
    const largest = Math.max(...parts.map((p) => p.count || 0));
    if (largest >= total) continue;
    if (!best || largest < best.largest) best = { label, parts, largest };
  }
  return best;
}

async function search(ctx, opts, filters, offset, limit) {
  const page = await call(ctx, 'POST', `/search/keyword${ctx.includes}`, {
    Keywords: opts.keywords,
    Limit: limit,
    Offset: offset,
    FilterOptionsRequest: filters,
    SortOptions: { Field: 'ManufacturerProductNumber', SortOrder: 'Ascending' },
  });
  ctx.pages += 1;
  if (ctx.opts.json) console.log(JSON.stringify(page));
  return page;
}

const done = (ctx, opts) => ctx.seen.size >= opts.limit || ctx.pages >= opts.pages;

// Prints each product the first time it is seen (keyed by manufacturer +
// part number).
function add(ctx, opts, page) {
  for (const p of page.Products || []) {
    if (ctx.seen.size >= opts.limit) return;
    const key = `${(p.Manufacturer && p.Manufacturer.Id) ?? ''}|${p.ManufacturerProductNumber}`;
    if (ctx.seen.has(key)) continue;
    ctx.seen.add(key);
    if (opts.json) continue;
    console.log(opts.list ? listRow(p) : tableRow(p, opts));
  }
}

// Fetches every product the search finds, splitting searches the cap would
// cut short. A split search's own first page prints too; the duplicates
// its pieces return later are skipped.
async function collect(ctx, opts, filters, path) {
  if (done(ctx, opts)) return;
  const first = await search(ctx, opts, filters, 0, PAGE);
  const total = first.ProductsCount || 0;
  if (ctx.total === null) ctx.total = total;
  add(ctx, opts, first);

  if (total > CAP) {
    const split = bestSplit(first, filters);
    if (split) {
      for (const part of split.parts) {
        if (!part.count) continue;
        await collect(ctx, opts, part.apply(filters), [...path, `${split.label}=${part.name}`]);
        if (done(ctx, opts)) return;
      }
      return;
    }
    ctx.warnings.push(`${path.join(', ') || 'search'}: ${total} parts and no way to split it; only the first ${CAP} listed`);
  }

  const end = Math.min(total, CAP);
  for (let offset = PAGE; offset < end && !done(ctx, opts); offset += PAGE) {
    add(ctx, opts, await search(ctx, opts, filters, offset, Math.min(PAGE, end - offset)));
  }
}

function summary(ctx) {
  for (const warning of ctx.warnings) console.error(`digikey: ${warning}`);
  const remaining = ctx.remaining !== null ? `, ${ctx.remaining} left today` : '';
  console.error(`digikey: ${ctx.seen.size} of ${ctx.total ?? '?'} parts, ${ctx.pages} pages, ` +
    `${ctx.calls} API calls${remaining}`);
}

let current = null; // the running search, for the summary on error

async function main() {
  const opts = parseArgs(process.argv.slice(2));
  const creds = credentials();
  const ctx = current = {
    opts, creds, token: await token(creds), calls: 0, remaining: null,
    pages: 0, seen: new Set(), total: null, warnings: [],
    // --list needs only three fields; asking for just those keeps pages small.
    includes: opts.list && !opts.json
      ? `?includes=${encodeURIComponent('ProductsCount,FilterOptions,Products(ManufacturerProductNumber,Manufacturer,ProductStatus,DatasheetUrl)')}`
      : '',
  };
  const filters = await resolveFilters(ctx, opts);
  if (!opts.list && !opts.json) console.log(tableHeader(opts));
  await collect(ctx, opts, filters, []);
  summary(ctx);
}

main().catch((error) => {
  console.error(`digikey: ${error.message}`);
  if (current) summary(current);
  process.exit(1);
});
