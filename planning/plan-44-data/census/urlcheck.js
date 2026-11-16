#!/usr/bin/env node
// plan-44 census: tests every URL of urls-needed.tsv and writes url-check.tsv.
//
// usage: node urlcheck.js <urls-needed.tsv> > url-check.tsv
//
// Columns: url, via, status (HTTP status of the final answer; 0 = no answer),
// pdf, bytes, from (the URL asked).
// via `direct`: the URL is fetched with a Safari user agent; pdf 1 when the
// body starts with %PDF-.
// via `archive` (suntsu.com answers 403 to every automated fetch, Cloudflare):
// the Internet Archive's availability API
// (http://archive.org/wayback/available?url=<url>) is asked for the nearest
// snapshot; pdf 1 when it has one (the snapshot's timestamp is in `bytes`),
// else 0. The archive answers a burst of page fetches with HTTP 429, so the
// PDFs themselves are fetched by plan-44-A's downloader, one at a time.
// Direct URLs: 4 requests at a time, one retry. Archive URLs: one at a time,
// 2 s apart, a 429 waited out (30 s) and an empty answer asked again (10 s), up to 4 tries.
'use strict';
const fs = require('fs');

const UA = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15';
const lines = fs.readFileSync(process.argv[2], 'utf8').trim().split('\n').slice(1).map((l) => l.split('\t'));

const sleep = (ms) => new Promise((res) => setTimeout(res, ms));

async function get(url, archive) {
  for (let attempt = 1; attempt <= (archive ? 4 : 2); attempt++) {
    try {
      const r = await fetch(url, { redirect: 'follow', signal: AbortSignal.timeout(90000), headers: { 'User-Agent': UA } });
      if (r.status === 429 && attempt < 4 && archive) { await sleep(20000); continue; }
      const buf = Buffer.from(await r.arrayBuffer());
      return { status: r.status, pdf: buf.slice(0, 5).toString() === '%PDF-' ? 1 : 0, bytes: buf.length };
    } catch (e) {
      if (attempt === (archive ? 4 : 2)) return { status: 0, pdf: 0, bytes: 0 };
    }
  }
}

(async () => {
  console.log('url\tvia\tstatus\tpdf\tbytes\tfrom');
  let next = 0;
  const out = [];
  async function worker() {
    while (next < lines.length) {
      const i = next++;
      const [url, via] = lines[i];
      const from = url;
      if (via === 'archive') continue;
      const r = await get(from, false);
      out[i] = [url, via, r.status, r.pdf, r.bytes, from].join('\t');
    }
  }
  await Promise.all([1, 2, 3, 4].map(worker));
  for (let i = 0; i < lines.length; i++) {
    const [url, via] = lines[i];
    if (via !== 'archive') continue;
    const api = `http://archive.org/wayback/available?url=${url.replace(/^https?:\/\//, '')}`;
    let row = [url, via, 0, 0, 0, api];
    for (let attempt = 1; attempt <= 4; attempt++) {
      try {
        const r = await fetch(api, { signal: AbortSignal.timeout(60000) });
        if (r.status === 429) { await sleep(30000); continue; }
        const snap = ((await r.json()).archived_snapshots || {}).closest;
        row = [url, via, r.status, snap && snap.available ? 1 : 0, snap ? snap.timestamp : 0, api];
        // An empty answer is also what a throttled archive gives: ask again before believing it.
        if (snap && snap.available) break;
        await sleep(10000);
      } catch (e) { await sleep(5000); }
    }
    out[i] = row.join('\t');
    await sleep(2000);
  }
  console.log(out.join('\n'));
})();
