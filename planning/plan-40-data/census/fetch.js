#!/usr/bin/env node
// plan-40-A Phase 3 (section 4): download every URL of <urls.txt> (urls.py)
// into <pdf-dir>/<md5 of the URL>.pdf, 4 at a time.
//
// usage: node fetch.js <urls.txt> <pdf-dir>
//
// Each URL: Node fetch with a Safari user agent (www.infineon.com answers a
// request without one with 202 and an empty body; assets.nexperia.com 403s
// curl but not Node), one retry; when that gives no PDF, a bare `curl -sL`
// (onsemi's CDN answers a bare curl). A body that does not start with
// `%PDF-` is not kept. A cached file that is a PDF is kept, so the script can
// be run again. Writes <pdf-dir>/index.tsv: md5, the attempt that worked or
// the last HTTP status, bytes, `%PDF` when it is one, the URL. The files are
// only read afterwards, by pdftotext; nothing runs them.
'use strict';
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { execFile } = require('child_process');

const SAFARI = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 ' +
  '(KHTML, like Gecko) Version/18.6 Safari/605.1.15';
const [listFile, dir] = process.argv.slice(2);
if (!listFile || !dir) {
  console.error('usage: node fetch.js <urls.txt> <pdf-dir>');
  process.exit(2);
}
fs.mkdirSync(dir, { recursive: true });
const urls = fs.readFileSync(listFile, 'utf8').split('\n').map((s) => s.trim()).filter(Boolean);

const md5 = (s) => crypto.createHash('md5').update(s).digest('hex');
const isPdf = (buf) => buf && buf.length > 5 && buf.subarray(0, 5).toString('latin1') === '%PDF-';

async function viaNode(url) {
  const ctl = new AbortController();
  const timer = setTimeout(() => ctl.abort(), 90000);
  try {
    const res = await fetch(url, {
      redirect: 'follow', signal: ctl.signal,
      headers: { 'User-Agent': SAFARI, Accept: 'application/pdf,text/html;q=0.9,*/*;q=0.8',
        'Accept-Language': 'en-US,en;q=0.9' },
    });
    const buf = Buffer.from(await res.arrayBuffer());
    return { code: String(res.status), buf };
  } catch (e) {
    return { code: 'error:' + ((e.cause && e.cause.code) || e.name), buf: null };
  } finally {
    clearTimeout(timer);
  }
}

function viaCurl(url, out) {
  return new Promise((resolve) => {
    execFile('curl', ['-sL', '--max-time', '90', '-o', out, '-w', '%{http_code}', url],
      { timeout: 120000 }, (err, stdout) => {
        let buf = null;
        try { buf = fs.readFileSync(out); } catch (e) { /* none */ }
        resolve({ code: (stdout || '').trim() || 'curl-error', buf });
      });
  });
}

async function one(url) {
  const id = md5(url);
  const file = path.join(dir, id + '.pdf');
  if (fs.existsSync(file) && isPdf(fs.readFileSync(file))) {
    return [id, 'cached', String(fs.statSync(file).size), '%PDF', url];
  }
  let code = 'none';
  for (const attempt of ['node', 'node-retry', 'curl']) {
    const r = attempt === 'curl' ? await viaCurl(url, file + '.part') : await viaNode(url);
    code = r.code;
    if (isPdf(r.buf)) {
      fs.writeFileSync(file, r.buf);
      try { fs.unlinkSync(file + '.part'); } catch (e) { /* none */ }
      return [id, attempt, String(r.buf.length), '%PDF', url];
    }
  }
  try { fs.unlinkSync(file + '.part'); } catch (e) { /* none */ }
  return [id, code, '0', '', url];
}

(async () => {
  const results = [];
  let next = 0;
  let done = 0;
  async function worker() {
    while (next < urls.length) {
      const u = urls[next++];
      results.push(await one(u));
      if (++done % 100 === 0) process.stderr.write(`fetch: ${done} of ${urls.length}\n`);
    }
  }
  await Promise.all([worker(), worker(), worker(), worker()]);
  results.sort((a, b) => (a[4] < b[4] ? -1 : a[4] > b[4] ? 1 : 0));
  fs.writeFileSync(path.join(dir, 'index.tsv'), results.map((r) => r.join('\t')).join('\n') + '\n');
  const pdfs = results.filter((r) => r[3] === '%PDF').length;
  console.log(`${urls.length} urls; ${pdfs} are PDFs; ${urls.length - pdfs} failed`);
})();
