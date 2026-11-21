#!/usr/bin/env node
// plan-42-A Phase 3 (section 4): download every URL of <urls.txt> (urls.py)
// into <pdf-dir>/<md5 of the line>.pdf, 4 at a time.
//
// usage: node fetch.js <urls.txt> <pdf-dir>
//
// Each URL: Node fetch with a Safari user agent (www.analog.com breaks curl's
// HTTP/2 stream and answers Node; www.microchip.com 403s curl), one retry;
// when that gives no PDF, a bare `curl -sL`. A body that does not start with
// `%PDF-` is not kept. A `page:<url>` line is a Microchip product page
// (section 4 rule 4): it is fetched and the first
// `ww1.microchip.com/downloads/...pdf` link of its HTML is downloaded in its
// place. A cached file that is a PDF is kept, so the script can be run again.
// Writes <pdf-dir>/index.tsv: md5, the attempt that worked or the last HTTP
// status, bytes, `%PDF` when it is one, the line, and (page: lines) the PDF URL
// the page led to. The files are only read afterwards, by pdftotext; nothing
// runs them.
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

async function getPdf(url, file) {
  let code = 'none';
  for (const attempt of ['node', 'node-retry', 'curl']) {
    const r = attempt === 'curl' ? await viaCurl(url, file + '.part') : await viaNode(url);
    code = r.code;
    if (isPdf(r.buf)) {
      fs.writeFileSync(file, r.buf);
      try { fs.unlinkSync(file + '.part'); } catch (e) { /* none */ }
      return { attempt, bytes: r.buf.length };
    }
    if (attempt !== 'node' && code === '404') break;
  }
  try { fs.unlinkSync(file + '.part'); } catch (e) { /* none */ }
  return { code };
}

async function one(line) {
  const id = md5(line);
  const file = path.join(dir, id + '.pdf');
  const prior = (line.startsWith('page:') ? (cachedActual.get(line) || '') : '');
  if (fs.existsSync(file) && isPdf(fs.readFileSync(file))) {
    return [id, 'cached', String(fs.statSync(file).size), '%PDF', line, prior];
  }
  if (line.startsWith('page:')) {
    const page = line.slice(5);
    const r = await viaNode(page);
    const html = r.buf ? r.buf.toString('latin1') : '';
    const m = html.match(/https?:\/\/ww1\.microchip\.com\/downloads\/[^"'\s<>]+?\.pdf/i);
    if (!m) return [id, r.code === '200' ? 'no-pdf-link' : r.code, '0', '', line, ''];
    const got = await getPdf(m[0], file);
    if (got.attempt) return [id, got.attempt, String(got.bytes), '%PDF', line, m[0]];
    return [id, got.code, '0', '', line, m[0]];
  }
  const got = await getPdf(line, file);
  if (got.attempt) return [id, got.attempt, String(got.bytes), '%PDF', line, ''];
  return [id, got.code, '0', '', line, ''];
}

const cachedActual = new Map();
const oldIndex = path.join(dir, 'index.tsv');
if (fs.existsSync(oldIndex)) {
  for (const l of fs.readFileSync(oldIndex, 'utf8').split('\n')) {
    const f = l.split('\t');
    if (f.length >= 6 && f[5]) cachedActual.set(f[4], f[5]);
  }
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
  fs.writeFileSync(oldIndex, results.map((r) => r.join('\t')).join('\n') + '\n');
  const pdfs = results.filter((r) => r[3] === '%PDF').length;
  console.log(`${urls.length} urls; ${pdfs} are PDFs; ${urls.length - pdfs} failed`);
})();
