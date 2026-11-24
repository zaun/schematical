#!/usr/bin/env node
// plan-55-B Phase 3 (§ 3.3): download every URL of <urls.tsv> (url, company; urls.sh) into
// <pdf-dir>/<md5 of the URL>.pdf.
//
// usage: node fetch.js <urls.tsv> <pdf-dir>
//
// Node fetch with a Safari user agent, 4 at a time, one retry; a 403 is tried again with a bare `curl -sL`
// (onsemi's host answers that and not the browser header, plan-29). A body that does not start with `%PDF-`
// is not kept, and `pdfinfo` must read it. A cached file that is a PDF is kept, so the script can be run
// again. Writes <pdf-dir>/index.tsv: md5, the attempt that worked or the last HTTP status, bytes, `%PDF` when
// it is one, the URL. The files are only read afterwards, by pdftotext; nothing runs them.
'use strict';
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { execFileSync } = require('child_process');

const SAFARI = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 ' +
  '(KHTML, like Gecko) Version/18.6 Safari/605.1.15';
const [listFile, dir] = process.argv.slice(2);
if (!listFile || !dir) {
  console.error('usage: node fetch.js <urls.tsv> <pdf-dir>');
  process.exit(2);
}
fs.mkdirSync(dir, { recursive: true });
const rows = fs.readFileSync(listFile, 'utf8').split('\n').filter(Boolean).map((l) => l.split('\t'));

const md5 = (s) => crypto.createHash('md5').update(s).digest('hex');
const isPdf = (buf) => buf && buf.length > 5 && buf.subarray(0, 5).toString('latin1') === '%PDF-';

function pdfinfoOk(file) {
  try {
    execFileSync('pdfinfo', [file], { stdio: 'ignore', timeout: 60000 });
    return true;
  } catch (e) {
    return false;
  }
}

async function get(url) {
  const ctl = new AbortController();
  const timer = setTimeout(() => ctl.abort(), 120000);
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

function bare(url) {
  try {
    const buf = execFileSync('curl', ['-sL', '--max-time', '120', url], { maxBuffer: 256 * 1024 * 1024, timeout: 130000 });
    return { code: 'curl', buf };
  } catch (e) {
    return { code: 'curl-failed', buf: null };
  }
}

async function one([url]) {
  const id = md5(url);
  const file = path.join(dir, id + '.pdf');
  if (fs.existsSync(file) && isPdf(fs.readFileSync(file)) && pdfinfoOk(file)) {
    return [id, 'cached', String(fs.statSync(file).size), '%PDF', url];
  }
  let code = 'none';
  for (let attempt = 1; attempt <= 2; attempt++) {
    let r = await get(url);
    if (!isPdf(r.buf) && (r.code === '403' || r.code === '406')) r = bare(url);
    code = r.code;
    if (isPdf(r.buf)) {
      fs.writeFileSync(file, r.buf);
      if (pdfinfoOk(file)) return [id, 'ok' + attempt, String(r.buf.length), '%PDF', url];
      fs.unlinkSync(file);
      code = 'pdfinfo-failed';
      break;
    }
  }
  return [id, code, '0', '', url];
}

(async () => {
  const results = [];
  let next = 0;
  let done = 0;
  async function worker() {
    while (next < rows.length) {
      const r = rows[next++];
      results.push(await one(r));
      if (++done % 50 === 0) process.stderr.write(`fetch: ${done} of ${rows.length}\n`);
    }
  }
  await Promise.all([worker(), worker(), worker(), worker()]);
  results.sort((a, b) => (a[4] < b[4] ? -1 : a[4] > b[4] ? 1 : 0));
  fs.writeFileSync(path.join(dir, 'index.tsv'), results.map((r) => r.join('\t')).join('\n') + '\n');
  const pdfs = results.filter((r) => r[3] === '%PDF').length;
  console.log(`${rows.length} urls; ${pdfs} are PDFs; ${rows.length - pdfs} failed`);
})();
