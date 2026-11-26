// Tests for scripts/digikey.js (plan-68-T). Run: node --test scripts/digikey.test.js
//
// No test touches the network: digikey.js runs with scripts/test-support/digikey-stub.js
// preloaded, which replaces globalThis.fetch with a queue of responses (shaped as DigiKey's
// Product Information API v4 answers; bodies in scripts/fixtures/digikey/), and with a fake
// `osascript` first on PATH for the Safari path. TMPDIR points at a fresh directory, so the
// login-token cache is the test's own.

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');

const TOOL = path.join(__dirname, 'digikey.js');
const STUB = path.join(__dirname, 'test-support', 'digikey-stub.js');
const OSASCRIPT = path.join(__dirname, 'test-support', 'digikey-osascript.js');
const fixture = (name) => JSON.parse(fs.readFileSync(path.join(__dirname, 'fixtures', 'digikey', name), 'utf8'));

const ID = 'testclient-0001';
const LOGIN = { status: 200, body: { access_token: 'tok-1', expires_in: 599, token_type: 'Bearer' } };
const ECS = fixture('keyword-ecs.json');
const ok = (body, headers) => ({ status: 200, body, headers });

// Runs digikey.js with the queued answers. `cache` writes a login token the run will reuse.
function run(args, { fetch = [], safari = [], env = {}, cache = false } = {}) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'digikey-test-'));
  const queue = path.join(dir, 'queue.json');
  const log = path.join(dir, 'requests.log');
  const bin = path.join(dir, 'bin');
  const tmp = path.join(dir, 'tmp');
  fs.mkdirSync(bin);
  fs.mkdirSync(tmp);
  fs.writeFileSync(queue, JSON.stringify({ fetch, safari }));
  fs.writeFileSync(path.join(bin, 'osascript'), `#!/bin/sh\nexec "${process.execPath}" "${OSASCRIPT}" "$@"\n`, { mode: 0o755 });
  const base = Object.fromEntries(Object.entries(process.env).filter(([k]) => !k.startsWith('DIGIKEY_')));
  const fullEnv = {
    ...base, TMPDIR: tmp, PATH: `${bin}:${process.env.PATH}`, DK_QUEUE: queue, DK_LOG: log,
    DIGIKEY_CLIENT_ID: ID, DIGIKEY_CLIENT_SECRET: 'secret-1', ...env,
  };
  for (const [k, v] of Object.entries(fullEnv)) if (v === undefined) delete fullEnv[k];
  if (cache) {
    fs.writeFileSync(path.join(tmp, `digikey-token-${ID.slice(0, 8)}.json`),
      JSON.stringify({ id: ID, token: 'cached-tok', expires: Date.now() + 3_600_000 }));
  }
  const r = childProcess.spawnSync(process.execPath, ['-r', STUB, TOOL, ...args], { encoding: 'utf8', env: fullEnv });
  const requests = fs.existsSync(log)
    ? fs.readFileSync(log, 'utf8').trim().split('\n').filter(Boolean).map((l) => JSON.parse(l)) : [];
  return { status: r.status, stdout: r.stdout, stderr: r.stderr, requests, tmp };
}

const bodyOf = (req) => JSON.parse(req.body);
const lines = (text) => text.split('\n').filter(Boolean);

test('no arguments, --help and -h print the help and exit 0 without a request', () => {
  for (const args of [[], ['--help'], ['x', '-h']]) {
    const r = run(args);
    assert.strictEqual(r.status, 0);
    assert.match(r.stdout, /^Search DigiKey's catalog\.\n\nusage: node scripts\/digikey\.js/);
    assert.deepStrictEqual(r.requests, []);
  }
});

test('bad command lines exit 2 with the reason and make no request', () => {
  const cases = [
    [['--category'], '--category needs a value'],
    [['--bogus', 'x'], 'unknown option --bogus'],
    [['--limit', '0', 'x'], '--limit must be a positive number'],
    [['--pages', '-1', 'x'], '--pages must be a positive number'],
    [['--active-only', '--status', 'Active', 'x'], '--active-only and --status do not mix'],
    [['--list', '--json', '--safari'], 'nothing to search for'],
  ];
  for (const [args, message] of cases) {
    const r = run(args);
    assert.strictEqual(r.status, 2, args.join(' '));
    assert.strictEqual(r.stderr, `digikey: ${message}\nRun with no arguments for help.\n`);
    assert.deepStrictEqual(r.requests, []);
  }
});

test('missing credentials exit 1 naming the variables; --opt2 names the second key', () => {
  const r = run(['x'], { env: { DIGIKEY_CLIENT_ID: undefined } });
  assert.strictEqual(r.status, 1);
  assert.strictEqual(r.stderr, 'digikey: set DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET (see ~/.zshrc)\n');
  const r2 = run(['--opt2', 'x']);
  assert.strictEqual(r2.status, 1);
  assert.strictEqual(r2.stderr, 'digikey: set DIGIKEY_CLIENT_ID2 and DIGIKEY_CLIENT_SECRET2 (see ~/.zshrc)\n');
});

test('a keyword search logs in, prints a table with --show columns once per part, and caches the token', () => {
  const r = run(['--show', 'Frequency,Package / Case, ', 'ECS-100AX'],
    { fetch: [LOGIN, ok(ECS, { 'x-ratelimit-remaining': '997' })] });
  assert.strictEqual(r.status, 0, r.stderr);
  const out = r.stdout.split('\n');
  assert.strictEqual(out[0], 'Part number           Status                Stock    Series                Frequency         ' +
    'Package / Case    Description');
  assert.match(out[1], /^-{11} {11}-{6}/);
  assert.strictEqual(out[2], 'ECS-100AX-100         Active                1234     ECS-100AX             10 MHz            ' +
    '4-DIP, Half Size  XTAL OSC XO 10.0000MHZ HCMOS TTL');
  assert.strictEqual(out[3], 'ECS-100AX-200         Not For New Designs   0        ECS-100AX             20 MHz            ' +
    '                  XTAL OSC XO 20.0000MHZ HCMOS TTL');
  assert.strictEqual(out[4].trim(), '', 'the empty product prints an empty row');
  assert.strictEqual(out.length, 6);
  assert.strictEqual(r.stderr, 'digikey: 3 of 3 parts, 1 pages, 1 API calls, 997 left today\n');

  const [login, search] = r.requests;
  assert.strictEqual(login.url, 'https://api.digikey.com/v1/oauth2/token');
  assert.strictEqual(login.body, `client_id=${ID}&client_secret=secret-1&grant_type=client_credentials`);
  assert.strictEqual(search.url, 'https://api.digikey.com/products/v4/search/keyword');
  assert.strictEqual(search.headers.Authorization, 'Bearer tok-1');
  assert.strictEqual(search.headers['X-DIGIKEY-Client-Id'], ID);
  assert.deepStrictEqual(bodyOf(search), {
    Keywords: 'ECS-100AX', Limit: 50, Offset: 0, FilterOptionsRequest: {},
    SortOptions: { Field: 'ManufacturerProductNumber', SortOrder: 'Ascending' },
  });
  const cacheFile = path.join(r.tmp, `digikey-token-${ID.slice(0, 8)}.json`);
  const cached = JSON.parse(fs.readFileSync(cacheFile, 'utf8'));
  assert.strictEqual(cached.token, 'tok-1');
  assert.strictEqual(fs.statSync(cacheFile).mode & 0o777, 0o600);
});

test('a cached token skips the login; --list asks for three fields and prints tab-separated rows', () => {
  const r = run(['--list', 'ECS-100AX'], { cache: true, fetch: [ok(ECS)] });
  assert.strictEqual(r.status, 0, r.stderr);
  assert.strictEqual(r.requests.length, 1);
  assert.strictEqual(r.requests[0].headers.Authorization, 'Bearer cached-tok');
  assert.strictEqual(r.requests[0].url, 'https://api.digikey.com/products/v4/search/keyword?includes=' +
    encodeURIComponent('ProductsCount,FilterOptions,Products(ManufacturerProductNumber,Manufacturer,ProductStatus,DatasheetUrl)'));
  assert.deepStrictEqual(r.stdout.split('\n'), [
    'ECS-100AX-100\tActive\thttps://ecsxtal.com/store/pdf/ECS-100AX.pdf',
    'ECS-100AX-200\tNot For New Designs\thttps://ecsxtal.com/store/pdf/ECS-100AX.pdf',
    '\t\t', '',
  ]);
  assert.strictEqual(r.stderr, 'digikey: 3 of 3 parts, 1 pages, 1 API calls\n');
});

test('--json prints each raw page as one line and no table; --limit stops early', () => {
  const r = run(['--json', '--limit', '1', 'ECS-100AX'], { cache: true, fetch: [ok(ECS)] });
  assert.strictEqual(r.status, 0, r.stderr);
  assert.strictEqual(r.requests[0].url, 'https://api.digikey.com/products/v4/search/keyword');
  assert.deepStrictEqual(lines(r.stdout).map((l) => JSON.parse(l)), [ECS]);
  assert.strictEqual(r.stderr, 'digikey: 1 of 3 parts, 1 pages, 1 API calls\n');
});

test('filter names are looked up: category, manufacturer prefix, series and status from a probe', () => {
  const r = run(['--category', 'Oscillators', '--manufacturer', 'ECS', '--series', 'ECS-2100X',
    '--status', 'obsolete', '--status', '7', '--in-stock', '--list'],
  { cache: true, fetch: [ok(fixture('categories.json')), ok(fixture('manufacturers.json')), ok(ECS), ok(ECS)] });
  assert.strictEqual(r.status, 0, r.stderr);
  assert.deepStrictEqual(r.requests.map((q) => `${q.method} ${q.url.replace(/\?.*/, '')}`), [
    'GET https://api.digikey.com/products/v4/search/categories',
    'GET https://api.digikey.com/products/v4/search/manufacturers',
    'POST https://api.digikey.com/products/v4/search/keyword',
    'POST https://api.digikey.com/products/v4/search/keyword',
  ]);
  assert.strictEqual(r.requests[0].body, null);
  assert.strictEqual(r.requests[0].headers['Content-Type'], undefined);
  assert.deepStrictEqual(bodyOf(r.requests[2]), {
    Keywords: '', Limit: 1, Offset: 0,
    FilterOptionsRequest: { CategoryFilter: [{ Id: '172' }], ManufacturerFilter: [{ Id: '50' }],
      MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '7' }] },
  });
  assert.deepStrictEqual(bodyOf(r.requests[3]).FilterOptionsRequest, {
    CategoryFilter: [{ Id: '172' }], ManufacturerFilter: [{ Id: '50' }], MinimumQuantityAvailable: 1,
    StatusFilter: [{ Id: '7' }, { Id: '1' }], SeriesFilter: [{ Id: '1401' }],
  });
  assert.strictEqual(r.stderr, 'digikey: 3 of 3 parts, 1 pages, 4 API calls\n');
});

test('ids skip the lookups; a category inside ChildCategories is found by part of its name', () => {
  const ids = run(['--category', '172', '--manufacturer', '50', '--series', '1401', '--active-only', '--list'],
    { cache: true, fetch: [ok(ECS)] });
  assert.strictEqual(ids.status, 0, ids.stderr);
  assert.deepStrictEqual(bodyOf(ids.requests[0]).FilterOptionsRequest, {
    CategoryFilter: [{ Id: '172' }], ManufacturerFilter: [{ Id: '50' }], SeriesFilter: [{ Id: '1401' }],
    StatusFilter: [{ Id: '0' }],
  });
  const child = run(['--category', 'ceramic', '--list'], { cache: true, fetch: [ok(fixture('categories.json')), ok(ECS)] });
  assert.strictEqual(child.status, 0, child.stderr);
  assert.deepStrictEqual(bodyOf(child.requests[1]).FilterOptionsRequest, { CategoryFilter: [{ Id: '1740' }] });
});

test('ambiguous and unknown names fail with the candidates, then the summary', () => {
  const mfr = ok(fixture('manufacturers.json'));
  const cases = [
    [['--manufacturer', 'Texas'], [mfr],
      'digikey: manufacturer "Texas" is ambiguous: Texas Instruments; Texas Instruments Japan\n'],
    [['--manufacturer', 'acme'], [mfr], 'digikey: manufacturer "acme" is ambiguous: ' +
      Array.from({ length: 10 }, (_, i) => `Acme Parts ${i + 1}`).join('; ') + '; …\n'],
    [['--manufacturer', 'Nobody'], [mfr], 'digikey: no manufacturer named "Nobody"\n'],
    [['--manufacturer', 'Nobody'], [ok({})], 'digikey: no manufacturer named "Nobody"\n'],
    [['--status', 'Gone'], [ok({ ProductsCount: 0 })], 'digikey: no status named "Gone"\n'],
  ];
  for (const [args, fetch, message] of cases) {
    const r = run(args, { cache: true, fetch });
    assert.strictEqual(r.status, 1, args.join(' '));
    assert.strictEqual(r.stderr, `${message}digikey: 0 of ? parts, 0 pages, 1 API calls\n`);
  }
});

test('a 401 renews the token once and repeats the request', () => {
  const r = run(['--list', 'ECS-100AX'], {
    cache: true,
    fetch: [{ status: 401, body: 'expired' }, { status: 200, body: { access_token: 'tok-2' } }, ok(ECS)],
  });
  assert.strictEqual(r.status, 0, r.stderr);
  assert.deepStrictEqual(r.requests.map((q) => q.headers.Authorization), ['Bearer cached-tok', undefined, 'Bearer tok-2']);
  assert.strictEqual(r.stderr, 'digikey: 3 of 3 parts, 1 pages, 2 API calls\n');
});

test('a 429 names the reset time when given; other errors name the route and status', () => {
  const cases = [
    [{ status: 429, body: 'quota', headers: { 'x-ratelimit-resettime': '2026-11-27T00:00:00Z', 'retry-after': '3570' } },
      'digikey: rate limited by DigiKey; resets at 2026-11-27T00:00:00Z (in 60 min): quota'],
    [{ status: 429, body: 'quota' }, 'digikey: rate limited by DigiKey; reset time not given: quota'],
    [{ status: 500, body: 'oops', headers: { 'x-ratelimit-remaining': '5' } },
      'digikey: POST /search/keyword failed (500): oops'],
  ];
  for (const [answer, message] of cases) {
    const r = run(['x'], { cache: true, fetch: [answer] });
    assert.strictEqual(r.status, 1);
    const left = answer.status === 500 ? ', 5 left today' : '';
    assert.strictEqual(r.stderr, `${message}\ndigikey: 0 of ? parts, 0 pages, 1 API calls${left}\n`);
  }
});

test('a failed login exits 1 with the answer and no summary', () => {
  const bad = run(['x'], { fetch: [{ status: 400, body: 'not json' }] });
  assert.strictEqual(bad.status, 1);
  assert.strictEqual(bad.stderr, 'digikey: login failed (400): {}\n');
  const noToken = run(['x'], { fetch: [{ status: 200, body: { error: 'invalid_client' } }] });
  assert.strictEqual(noToken.stderr, 'digikey: login failed (200): {"error":"invalid_client"}\n');
});

test('a failed connection is retried three times, then the error ends the run', () => {
  const r = run(['--list', 'x'], {
    fetch: [{ error: 'fetch failed', cause: { code: 'ECONNRESET' } }, { error: 'terminated', cause: { message: 'other side closed' } },
      { error: 'stalled' }, LOGIN, ok(ECS)],
  });
  assert.strictEqual(r.status, 0, r.stderr);
  assert.strictEqual(r.stderr, 'digikey: fetch failed (ECONNRESET); retry 1 of 3\n' +
    'digikey: terminated (other side closed); retry 2 of 3\ndigikey: stalled; retry 3 of 3\n' +
    'digikey: 3 of 3 parts, 1 pages, 1 API calls\n');
  assert.strictEqual(r.requests.length, 5);

  const dead = run(['x'], { fetch: [{ error: 'a' }, { error: 'b' }, { error: 'c' }, { error: 'gone for good' }] });
  assert.strictEqual(dead.status, 1);
  assert.match(dead.stderr, /retry 3 of 3\ndigikey: gone for good\n$/);
});

test('a Cloudflare challenge moves every request to Safari', () => {
  const challenge = { status: 403, headers: { 'cf-mitigated': 'challenge' }, body: '<title>Just a moment...</title>' };
  const r = run(['--list', 'ECS-100AX'], {
    fetch: [challenge],
    safari: [{ result: { status: 200, headers: {}, text: JSON.stringify(LOGIN.body) } },
      { result: { status: 200, headers: { 'x-ratelimit-remaining': '42' }, text: JSON.stringify(ECS) } }],
  });
  assert.strictEqual(r.status, 0, r.stderr);
  assert.strictEqual(r.stderr, 'digikey: Cloudflare challenged Node; sending requests through Safari instead\n' +
    'digikey: 3 of 3 parts, 1 pages, 1 API calls, 42 left today\n');
  assert.strictEqual(lines(r.stdout).length, 3);
  const [, login, search] = r.requests;
  assert.strictEqual(login.kind, 'safari');
  assert.match(login.script, /new XMLHttpRequest\(\)/);
  assert.match(login.script, /"url":"https:\/\/api\.digikey\.com\/v1\/oauth2\/token","method":"POST"/);
  assert.match(login.script, /client_secret=secret-1/);
  assert.ok(login.applescript.includes('  tell application "Safari"'));
  assert.match(search.script, /"Authorization":"Bearer tok-1"/);
  assert.match(search.script, /\\"Keywords\\":\\"ECS-100AX\\"/);
  assert.deepStrictEqual(fs.readdirSync(r.tmp).filter((f) => f.startsWith('digikey-safari-')), [],
    'the request file (it holds the secret) is removed');
});

test('--safari failures each say what to do', () => {
  const cases = [
    [{ stderr: 'execution error: You must enable "Allow JavaScript from Apple Events" (-1)\n' },
      'Safari: turn on Develop > Developer Settings > "Allow JavaScript from Apple Events", then run again'],
    [{ stderr: 'boom\nsecond line\n', code: 3 }, 'Safari request failed: boom'],
    [{ stdout: 'garbage' }, 'Safari gave an unreadable answer: garbage'],
    [{ result: { error: 'NetworkError: Load failed' } }, 'Safari request error: NetworkError: Load failed'],
    [{ result: { status: 403, headers: { 'cf-mitigated': 'challenge' } } },
      'Cloudflare challenged Safari too: open https://api.digikey.com/ in Safari, pass the check, then run again'],
    [{ result: { status: 404, text: 'missing' } }, 'POST /search/keyword failed (404): missing'],
  ];
  for (const [answer, message] of cases) {
    const r = run(['--safari', 'x'], { cache: true, safari: [answer] });
    assert.strictEqual(r.status, 1, message);
    assert.strictEqual(lines(r.stderr)[0], `digikey: ${message}`);
    assert.strictEqual(r.requests.filter((q) => q.kind === 'fetch').length, 0);
  }
  const linux = run(['--safari', '--list', 'x'], { cache: true, env: { DK_PLATFORM: 'linux' } });
  assert.strictEqual(lines(linux.stderr)[0], 'digikey: --safari / the Safari fallback only works on macOS');
});

// Product pages for the split tests: n distinct parts named <prefix>1..n.
const parts = (prefix, from, n) => Array.from({ length: n }, (_, i) => ({
  ManufacturerProductNumber: `${prefix}${from + i}`, Manufacturer: { Id: 50 },
  ProductStatus: { Status: 'Active' }, DatasheetUrl: `https://example.invalid/${prefix}.pdf`,
}));
const param = (id, name, category, values) => ({
  ParameterId: id, ParameterName: name, Category: { Id: category },
  FilterValues: values.map(([ValueId, ValueName, ProductCount]) => ({ ValueId, ValueName, ProductCount })),
});

test('a search over the 300 cap is split by the parameter with the smallest largest piece, recursively', () => {
  const root = {
    ProductsCount: 700, Products: parts('A', 1, 50),
    FilterOptions: {
      TopCategories: [{ Category: { Id: 172, Name: 'Oscillators', ProductCount: 400 } },
        { Category: { Id: 171, Name: 'Crystals', ProductCount: 300 } }],
      Manufacturers: [{ Id: 50, Value: 'ECS Inc.', ProductCount: 350 }, { Id: 2000, Value: 'Abracon', ProductCount: 340 }],
      Series: [{ Id: 1, Value: '*', ProductCount: 700 }, { Id: 2, Value: 'X' }],
      Status: [{ Id: 0, Value: 'Active', ProductCount: 700 }],
      ParametricFilters: [
        param(9, 'Type', 172, [['t', 'XO', 700]]),
        { ParameterId: 10, ParameterName: 'Empty', Category: { Id: 172 } },
        param(2085, 'Frequency', 172, [['z', '1 Hz', 0], ['a', '10 MHz', 350], ['b', '20 MHz', 350]]),
      ],
    },
  };
  const freq10 = {
    ProductsCount: 350, Products: parts('B', 1, 50),
    FilterOptions: {
      ParametricFilters: [
        param(2085, 'Frequency', 172, [['a', '10 MHz', 350], ['b', '20 MHz', 1]]),
        param(16, 'Package / Case', 171, [['p', 'DIP', 200], ['q', 'SMD', 150]]),
        param(69, 'Voltage - Supply', 172, [['v3', '3.3V', 120], ['v5', '5V', 230]]),
      ],
    },
  };
  const fetch = [
    ok(root), ok(freq10),
    ok({ ProductsCount: 120, Products: parts('C', 1, 50) }),
    ok({ ProductsCount: 120, Products: parts('C', 51, 50) }),
    ok({ ProductsCount: 120, Products: parts('C', 101, 20) }),
    ok({ ProductsCount: 50, Products: parts('D', 1, 50) }),
    ok({ ProductsCount: 350, Products: parts('E', 1, 50) }),
    ok({ ProductsCount: 350, Products: parts('E', 51, 50) }),
  ];
  const r = run(['--list', '--limit', '370', 'osc'], { cache: true, fetch });
  assert.strictEqual(r.status, 0, r.stderr);
  const out = lines(r.stdout).map((l) => l.split('\t')[0]);
  assert.strictEqual(out.length, 370);
  assert.deepStrictEqual([out[0], out[50], out[100], out[220], out[270], out[369]], ['A1', 'B1', 'C1', 'D1', 'E1', 'E100']);
  assert.strictEqual(r.stderr, 'digikey: Frequency=20 MHz: 350 parts and no way to split it; only the first 300 listed\n' +
    'digikey: 370 of 700 parts, 8 pages, 8 API calls\n');

  const bodies = r.requests.map(bodyOf);
  assert.deepStrictEqual(bodies[1].FilterOptionsRequest, {
    ParameterFilterRequest: { CategoryFilter: { Id: '172' }, ParameterFilters: [{ ParameterId: 2085, FilterValues: [{ Id: 'a' }] }] },
  });
  assert.deepStrictEqual(bodies[2].FilterOptionsRequest.ParameterFilterRequest.ParameterFilters,
    [{ ParameterId: 2085, FilterValues: [{ Id: 'a' }] }, { ParameterId: 69, FilterValues: [{ Id: 'v3' }] }]);
  assert.deepStrictEqual(bodies.slice(2, 5).map((b) => [b.Offset, b.Limit]), [[0, 50], [50, 50], [100, 20]]);
  assert.deepStrictEqual(bodies[5].FilterOptionsRequest.ParameterFilterRequest.ParameterFilters[1].FilterValues, [{ Id: 'v5' }]);
  assert.deepStrictEqual(bodies[6].FilterOptionsRequest.ParameterFilterRequest.ParameterFilters,
    [{ ParameterId: 2085, FilterValues: [{ Id: 'b' }] }]);
  assert.deepStrictEqual([bodies[7].Offset, bodies[7].Limit], [50, 50]);
});

test('a category split adds the category filter; --pages stops the run', () => {
  const root = {
    ProductsCount: 400, Products: parts('A', 1, 50),
    FilterOptions: { TopCategories: [{ Category: { Id: 172, Name: 'Oscillators', ProductCount: 250 } },
      { Category: { Id: 171, Name: 'Crystals', ProductCount: 150 } }] },
  };
  const r = run(['--list', '--pages', '2', 'osc'], { cache: true, fetch: [ok(root), ok({ ProductsCount: 250, Products: parts('O', 1, 50) })] });
  assert.strictEqual(r.status, 0, r.stderr);
  assert.deepStrictEqual(bodyOf(r.requests[1]).FilterOptionsRequest, { CategoryFilter: [{ Id: '172' }] });
  assert.strictEqual(lines(r.stdout).length, 100);
  assert.strictEqual(r.stderr, 'digikey: 100 of 400 parts, 2 pages, 2 API calls\n');
});

test('required as a module it exports the login and request path; Safari reaches only the API', async () => {
  const saved = { fetch: globalThis.fetch, id: process.env.DIGIKEY_CLIENT_ID, secret: process.env.DIGIKEY_CLIENT_SECRET };
  const written = [];
  const write = process.stderr.write;
  try {
    const digikey = require(TOOL);
    assert.deepStrictEqual(Object.keys(digikey), ['credentials', 'token', 'fetchRetry']);
    process.env.DIGIKEY_CLIENT_ID = 'module-id';
    process.env.DIGIKEY_CLIENT_SECRET = 'module-secret';
    assert.deepStrictEqual(digikey.credentials(false), { id: 'module-id', secret: 'module-secret' });
    globalThis.fetch = async () => new Response('Just a moment...', { status: 403, headers: { 'cf-mitigated': 'challenge' } });
    process.stderr.write = (s) => { written.push(String(s)); return true; };
    await assert.rejects(digikey.fetchRetry('https://example.invalid/x', {}),
      { message: 'Safari can only reach https://api.digikey.com, not https://example.invalid/x' });
    assert.deepStrictEqual(written, ['digikey: Cloudflare challenged Node; sending requests through Safari instead\n']);
  } finally {
    process.stderr.write = write;
    globalThis.fetch = saved.fetch;
    for (const [k, v] of [['DIGIKEY_CLIENT_ID', saved.id], ['DIGIKEY_CLIENT_SECRET', saved.secret]]) {
      if (v === undefined) delete process.env[k]; else process.env[k] = v;
    }
  }
});
