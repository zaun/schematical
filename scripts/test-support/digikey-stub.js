// Preloaded (`node -r`) into scripts/digikey.js by scripts/digikey.test.js:
// replaces globalThis.fetch with a queue of recorded responses, so no test
// touches the network. Environment:
//   DK_QUEUE     JSON file {"fetch": [entry, ...]}; each fetch takes the next
//                entry: {status, headers, body} (body: object → JSON, string
//                as is) or {error, cause} (the fetch throws).
//   DK_LOG       file that gets one JSON line per request (url, method,
//                headers, body).
//   DK_PLATFORM  pretend process.platform is this.
// Retry waits of a second or more run at once, so the 2/4/6 s backoff of
// fetchNode does not slow the tests.

'use strict';

const fs = require('fs');

const queue = JSON.parse(fs.readFileSync(process.env.DK_QUEUE, 'utf8')).fetch || [];
let next = 0;

if (process.env.DK_PLATFORM) Object.defineProperty(process, 'platform', { value: process.env.DK_PLATFORM });

const realSetTimeout = globalThis.setTimeout;
globalThis.setTimeout = (fn, ms, ...rest) => realSetTimeout(fn, ms >= 1000 ? 0 : ms, ...rest);

globalThis.fetch = async (url, options = {}) => {
  const body = options.body === undefined ? null : String(options.body);
  fs.appendFileSync(process.env.DK_LOG, JSON.stringify({
    kind: 'fetch', url: String(url), method: options.method || 'GET', headers: options.headers || {}, body,
  }) + '\n');
  if (next >= queue.length) throw new Error(`digikey-stub: no response queued for request ${next + 1} (${url})`);
  const entry = queue[next++];
  if (entry.error) {
    const error = new Error(entry.error);
    if (entry.cause) error.cause = entry.cause;
    throw error;
  }
  const text = typeof entry.body === 'string' ? entry.body : JSON.stringify(entry.body ?? {});
  return new Response(text, { status: entry.status || 200, headers: entry.headers || {} });
};
