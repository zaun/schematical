// plan-48 census: the DigiKey calls count.js and list.js share (login with
// the token digikey.js caches, one keyword search with filters).
// Credentials: DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET in the environment.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');

const API = 'https://api.digikey.com';
const TOKEN_CACHE = path.join(os.tmpdir(), 'digikey-token.json');
const creds = { id: process.env.DIGIKEY_CLIENT_ID, secret: process.env.DIGIKEY_CLIENT_SECRET };
if (!creds.id || !creds.secret) { console.error('set DIGIKEY_CLIENT_ID and DIGIKEY_CLIENT_SECRET (see ~/.zshrc)'); process.exit(1); }

const state = { calls: 0, left: null };
let tok = null;

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

// q: { category, makers: [id], params: { <parameter id>: [<value id>] },
//      offset, limit, sort (true = most-stocked first) }
// In stock (quantity above 0), status Active (0) or Last Time Buy (4).
async function search(q) {
  const filter = { CategoryFilter: [{ Id: String(q.category) }], MinimumQuantityAvailable: 1,
    StatusFilter: [{ Id: '0' }, { Id: '4' }] };
  if (q.makers && q.makers.length) filter.ManufacturerFilter = q.makers.map((m) => ({ Id: String(m) }));
  const pf = Object.entries(q.params || {});
  if (pf.length) {
    filter.ParameterFilterRequest = { CategoryFilter: { Id: String(q.category) },
      ParameterFilters: pf.map(([id, values]) => ({ ParameterId: Number(id), FilterValues: values.map((v) => ({ Id: String(v) })) })) };
  }
  const body = { Keywords: '', Limit: q.limit || 1, Offset: q.offset || 0, FilterOptionsRequest: filter };
  if (q.sort) body.SortOptions = { Field: 'QuantityAvailable', SortOrder: 'Descending' };
  let relogged = false;
  for (let attempt = 1; ; attempt++) {
    try {
      if (!tok) tok = await token(false);
      const r = await fetch(`${API}/products/v4/search/keyword`, {
        method: 'POST', signal: AbortSignal.timeout(180000),
        headers: { Authorization: `Bearer ${tok}`, 'X-DIGIKEY-Client-Id': creds.id, 'X-DIGIKEY-Locale-Site': 'US',
          'X-DIGIKEY-Locale-Language': 'en', 'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
      });
      state.calls++;
      state.left = r.headers.get('x-ratelimit-remaining') ?? state.left;
      if (r.status === 401 && !relogged) { relogged = true; tok = await token(true); continue; }
      const text = await r.text();
      if (r.status === 429) throw Object.assign(new Error(`rate limited: ${text.slice(0, 200)}`), { fatal: true });
      if (!r.ok) throw new Error(`HTTP ${r.status}: ${text.slice(0, 200)}`);
      return JSON.parse(text);
    } catch (e) {
      if (e.fatal || attempt >= 4) throw e;
      console.error(`dk.js: ${e.message}; retry ${attempt} of 3`);
      await new Promise((res) => setTimeout(res, 2000 * attempt));
    }
  }
}

// "--maker 18,283 --param 69=409393,411897" -> { makers, params }
function parseArgs(argv) {
  const q = { makers: [], params: {} };
  const rest = [];
  for (let i = 0; i < argv.length; i++) {
    if (argv[i] === '--maker') q.makers = argv[++i].split(',');
    else if (argv[i] === '--param') { const [id, v] = argv[++i].split('='); q.params[id] = v.split(','); }
    else rest.push(argv[i]);
  }
  return { q, rest };
}

module.exports = { search, parseArgs, state };
