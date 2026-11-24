'use strict';
// plan-45 census: the DigiKey request path count.js, sample.js and list.js share: scripts/digikey.js's login
// and transport (Cloudflare answers scripts with a challenge since 2026-11-23, so the requests fall back to
// Safari there; needs Safari > Develop > Developer Settings > Allow JavaScript from Apple Events).
//
// Key: the second key (DIGIKEY_CLIENT_ID2 / DIGIKEY_CLIENT_SECRET2, its own daily quota) unless DK_OPT2=0
// (owner, 2026-11-24: "using opt2", the first key's quota was spent by another session).
const path = require('path');
const dk = require(path.join(__dirname, '..', '..', '..', 'scripts', 'digikey.js'));

const API = 'https://api.digikey.com';
const opt2 = process.env.DK_OPT2 !== '0';

// A request context: the key, the login token, the counters.
async function open() {
  const creds = dk.credentials(opt2);
  return { creds, token: await dk.token(creds), calls: 0, left: null };
}

// One POST to the keyword search. Returns { response, text }; a 401 renews the token once. The caller reads
// ctx.left (calls left today) and decides what a 429 or another status means.
async function post(ctx, body) {
  for (let retried = false; ; retried = true) {
    const result = await dk.fetchRetry(`${API}/products/v4/search/keyword`, {
      method: 'POST',
      headers: { Authorization: `Bearer ${ctx.token}`, 'X-DIGIKEY-Client-Id': ctx.creds.id, 'X-DIGIKEY-Locale-Site': 'US',
        'X-DIGIKEY-Locale-Language': 'en', 'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
      body: JSON.stringify(body) });
    ctx.calls++;
    ctx.left = result.response.headers.get('x-ratelimit-remaining') ?? ctx.left;
    if (result.response.status === 401 && !retried) { ctx.token = await dk.token(ctx.creds, true); continue; }
    return result;
  }
}

module.exports = { API, opt2, open, post };
