#!/usr/bin/env node
// Search DigiKey's catalog (Product Information API v4, KeywordSearch).
// Run with no arguments, or --help, for instructions (HELP below).

'use strict';

const fs = require('fs');
const os = require('os');
const path = require('path');
const { execFileSync } = require('child_process');

const API = 'https://api.digikey.com';
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
  --opt2                    use the second API key (DIGIKEY_CLIENT_ID2 and
                            DIGIKEY_CLIENT_SECRET2), which has its own quota
  --safari                  send every request through Safari (macOS); see below
  -h, --help                this text

Parts print as they arrive, so an error part way keeps what came before. A
summary line goes to stderr: parts found, pages, API calls, calls left today.

Things to know
  * Credentials: DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET in the
    environment (set in ~/.zshrc); --opt2 uses DIGIKEY_CLIENT_ID2 and
    DIGIKEY_CLIENT_SECRET2 instead. The login token lasts 10 minutes and is
    cached in the temp directory (one file per key); it is renewed when it
    expires.
  * Cloudflare: DigiKey's API answers scripts with a "Just a moment..." 403
    challenge (since 2026-11-23); only a real browser passes. When Node gets
    it, the script retries through Safari automatically (--safari starts
    there): the requests run in a minimized Safari window on api.digikey.com.
    One-time setup: Safari > Develop > Developer Settings > turn on "Allow
    JavaScript from Apple Events". Safari has no headless mode, so a window
    exists (minimized) while it runs.
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
      case '--safari': safari.forced = true; break;
      case '--opt2': opts.opt2 = true; break;
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

// --opt2 uses the second API key (DIGIKEY_CLIENT_ID2 / DIGIKEY_CLIENT_SECRET2),
// which has its own daily quota.
function credentials(opt2) {
  const suffix = opt2 ? '2' : '';
  const id = process.env[`DIGIKEY_CLIENT_ID${suffix}`];
  const secret = process.env[`DIGIKEY_CLIENT_SECRET${suffix}`];
  if (!id || !secret) {
    console.error(`digikey: set DIGIKEY_CLIENT_ID${suffix} and DIGIKEY_CLIENT_SECRET${suffix} (see ~/.zshrc)`);
    process.exit(1);
  }
  return { id, secret };
}

// One cache file per key, so runs on different keys do not overwrite each
// other's login token.
const tokenCache = (creds) => path.join(os.tmpdir(), `digikey-token-${creds.id.slice(0, 8)}.json`);

async function token(creds, fresh = false) {
  if (!fresh) try {
    const cached = JSON.parse(fs.readFileSync(tokenCache(creds), 'utf8'));
    if (cached.id === creds.id && cached.expires > Date.now() + 30_000) return cached.token;
  } catch { /* no usable cache */ }

  const { response, text: tokenText } = await fetchRetry(`${API}/v1/oauth2/token`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams({
      client_id: creds.id, client_secret: creds.secret, grant_type: 'client_credentials',
    }),
  });
  let body = {};
  try { body = JSON.parse(tokenText); } catch { /* not JSON */ }
  if (!response.ok || !body.access_token) {
    throw new Error(`login failed (${response.status}): ${JSON.stringify(body)}`);
  }
  const expires = Date.now() + (body.expires_in || 0) * 1000;
  fs.writeFileSync(tokenCache(creds), JSON.stringify({ id: creds.id, token: body.access_token, expires }), { mode: 0o600 });
  return body.access_token;
}

// A hung connection (seen 2026-11-08: requests stalled for minutes, then
// "terminated" / "fetch failed") is cut after 180 s and retried, up to 3 times.
async function fetchNode(url, options) {
  for (let attempt = 1; ; attempt += 1) {
    try {
      const response = await fetch(url, { ...options, signal: AbortSignal.timeout(180_000) });
      return { response, text: await response.text() };
    } catch (error) {
      if (attempt >= 4) throw error;
      process.stderr.write(`digikey: ${error.message || error}${error.cause ? ` (${error.cause.code || error.cause.message})` : ""}; retry ${attempt} of 3\n`);
      await new Promise((resolve) => setTimeout(resolve, 2000 * attempt));
    }
  }
}

// Cloudflare sits in front of api.digikey.com and, since 2026-11-23, answers
// scripts (Node, curl, Python, any TLS profile, any IP) with an interactive
// challenge: 403 + "cf-mitigated: challenge" + a "Just a moment..." page.
// A real browser gets through, so then every request goes through Safari: a
// synchronous XMLHttpRequest run inside a minimized Safari window that is on
// api.digikey.com (same origin). macOS only; needs Safari's Develop menu >
// "Allow JavaScript from Apple Events". Safari has no headless mode.
const safari = { forced: false, active: false };
const isChallenge = ({ response }) =>
  response.status === 403 && response.headers.get('cf-mitigated') === 'challenge';

// Runs in the Safari page; `req` is spliced in as a JSON literal.
const SAFARI_PAGE_JS = (req) => `(function (req) {
  var x = new XMLHttpRequest();
  x.open(req.method, req.url, false);
  for (var k in req.headers) x.setRequestHeader(k, req.headers[k]);
  try { x.send(req.body); } catch (e) { return JSON.stringify({ error: String(e) }); }
  var h = {};
  x.getAllResponseHeaders().trim().split(/\\r?\\n/).forEach(function (l) {
    var i = l.indexOf(':');
    if (i > 0) h[l.slice(0, i).trim().toLowerCase()] = l.slice(i + 1).trim();
  });
  return JSON.stringify({ status: x.status, headers: h, text: x.responseText });
})(${JSON.stringify(req)})`;

// Finds (or opens, minimized) a Safari document on api.digikey.com and runs
// the JavaScript in the file named by the first argument.
const SAFARI_APPLESCRIPT = [
  'on run argv',
  '  set js to read POSIX file (item 1 of argv) as «class utf8»',
  '  tell application "Safari"',
  '    set found to missing value',
  '    repeat with d in documents',
  '      if (URL of d) starts with "https://api.digikey.com" then',
  '        set found to d',
  '        exit repeat',
  '      end if',
  '    end repeat',
  '    if found is missing value then',
  '      set found to make new document with properties {URL:"https://api.digikey.com/"}',
  '      try',
  '        set miniaturized of window 1 to true',
  '      end try',
  '      repeat 60 times',
  '        delay 0.5',
  '        try',
  '          if (do JavaScript "document.readyState" in found) is "complete" then exit repeat',
  '        end try',
  '      end repeat',
  '    end if',
  '    return do JavaScript js in found',
  '  end tell',
  'end run',
].flatMap((line) => ['-e', line]);

function fetchSafari(url, options) {
  if (process.platform !== 'darwin') throw new Error('--safari / the Safari fallback only works on macOS');
  if (new URL(url).origin !== API) throw new Error(`Safari can only reach ${API}, not ${url}`);
  const req = {
    url, method: options.method || 'GET', headers: options.headers || {},
    body: options.body === undefined ? null : String(options.body),
  };
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'digikey-safari-'));
  const file = path.join(dir, 'request.js');
  fs.writeFileSync(file, SAFARI_PAGE_JS(req), { mode: 0o600 }); // holds the client secret
  let out;
  try {
    out = execFileSync('osascript', [...SAFARI_APPLESCRIPT, file], {
      encoding: 'utf8', timeout: 240_000, maxBuffer: 512 * 1024 * 1024, stdio: ['ignore', 'pipe', 'pipe'],
    });
  } catch (error) {
    const detail = String(error.stderr || error.message);
    if (/Allow JavaScript from Apple Events/i.test(detail)) {
      throw new Error('Safari: turn on Develop > Developer Settings > "Allow JavaScript from Apple Events", then run again');
    }
    throw new Error(`Safari request failed: ${detail.trim().split('\n')[0]}`);
  } finally {
    fs.rmSync(dir, { recursive: true, force: true });
  }
  let result;
  try { result = JSON.parse(out); } catch { throw new Error(`Safari gave an unreadable answer: ${out.slice(0, 200)}`); }
  if (result.error) throw new Error(`Safari request error: ${result.error}`);
  const headers = result.headers || {};
  const response = {
    status: result.status, ok: result.status >= 200 && result.status < 300,
    headers: { get: (name) => headers[String(name).toLowerCase()] ?? null },
  };
  if (isChallenge({ response })) {
    throw new Error('Cloudflare challenged Safari too: open https://api.digikey.com/ in Safari, pass the check, then run again');
  }
  return { response, text: result.text };
}

async function fetchRetry(url, options) {
  if (!safari.forced && !safari.active) {
    const result = await fetchNode(url, options);
    if (!isChallenge(result)) return result;
    safari.active = true;
    process.stderr.write('digikey: Cloudflare challenged Node; sending requests through Safari instead\n');
  }
  return fetchSafari(url, options);
}

async function call(ctx, method, route, body, retried = false) {
  const { response, text } = await fetchRetry(`${API}/products/v4${route}`, {
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
  if (response.status === 401 && !retried) {
    // Tokens last 10 minutes; a long run outlives one.
    ctx.token = await token(ctx.creds, true);
    return call(ctx, method, route, body, true);
  }
  if (response.status === 429) {
    // DigiKey names the reset in headers: x-ratelimit-resettime (ISO time), retry-after (s).
    const resetAt = response.headers.get('x-ratelimit-resettime');
    const retryAfter = response.headers.get('retry-after');
    const when = resetAt ? `resets at ${resetAt}` : 'reset time not given';
    const wait = retryAfter ? ` (in ${Math.ceil(Number(retryAfter) / 60)} min)` : '';
    throw new Error(`rate limited by DigiKey; ${when}${wait}: ${text}`);
  }
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
  const creds = credentials(opts.opt2);
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
