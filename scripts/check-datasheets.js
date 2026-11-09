#!/usr/bin/env node
// Check that every URL in Parts/datasheets.json still serves a PDF.
// Run with --help for instructions (HELP below).

'use strict';

const fs = require('fs');
const path = require('path');

const FILE = path.join(__dirname, '..', 'schematical', 'schematical', 'Parts', 'datasheets.json');

const HELP = `Check the datasheet URLs in schematical/schematical/Parts/datasheets.json.

usage: node scripts/check-datasheets.js [options] [id ...]

Fetches each URL (following redirects) and reports the ones that fail: an
HTTP error, a timeout, or a reply that is not a PDF. With ids, checks only
those entries. Exits 1 if any URL failed.

Options
  --jobs <n>       requests at a time (default 4; be gentle with vendor sites)
  --timeout <s>    seconds per request (default 30)
  --host <name>    only URLs on this host, e.g. www.ti.com
  --verbose        print every URL, not just the failures
  --help           this text`;

function parseArgs(argv) {
    const options = { jobs: 4, timeout: 30, host: null, verbose: false, ids: [] };
    for (let i = 0; i < argv.length; i++) {
        const arg = argv[i];
        const value = () => {
            if (i + 1 >= argv.length) throw new Error(`${arg} needs a value`);
            return argv[++i];
        };
        if (arg === '--help' || arg === '-h') options.help = true;
        else if (arg === '--jobs') options.jobs = Math.max(1, parseInt(value(), 10) || 1);
        else if (arg === '--timeout') options.timeout = Math.max(1, parseInt(value(), 10) || 30);
        else if (arg === '--host') options.host = value();
        else if (arg === '--verbose') options.verbose = true;
        else if (arg.startsWith('--')) throw new Error(`unknown option ${arg}`);
        else options.ids.push(arg);
    }
    return options;
}

// Fetches the URL and reads just enough of the body to see the PDF signature.
async function check(url, timeoutSeconds) {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), timeoutSeconds * 1000);
    try {
        const response = await fetch(url, {
            redirect: 'follow',
            signal: controller.signal,
            headers: { 'User-Agent': 'Mozilla/5.0 (Macintosh) Schematical datasheet check' },
        });
        if (!response.ok) {
            return { ok: false, why: `HTTP ${response.status}` };
        }
        const reader = response.body.getReader();
        let head = Buffer.alloc(0);
        while (head.length < 1024) {
            const { done, value } = await reader.read();
            if (done) break;
            head = Buffer.concat([head, Buffer.from(value)]);
        }
        await reader.cancel().catch(() => {});
        // Some sites put a few bytes before the header; look in the first KB.
        if (head.indexOf('%PDF-') < 0) {
            const type = response.headers.get('content-type') || 'no content-type';
            return { ok: false, why: `not a PDF (${type})` };
        }
        const moved = response.url !== url ? ` -> ${response.url}` : '';
        return { ok: true, why: `PDF${moved}` };
    } catch (error) {
        return { ok: false, why: error.name === 'AbortError' ? 'timed out' : error.message };
    } finally {
        clearTimeout(timer);
    }
}

async function main() {
    let options;
    try {
        options = parseArgs(process.argv.slice(2));
    } catch (error) {
        console.error(`${error.message}\n\n${HELP}`);
        process.exit(2);
    }
    if (options.help) {
        console.log(HELP);
        return;
    }

    let entries = JSON.parse(fs.readFileSync(FILE, 'utf8')).datasheets;
    if (options.ids.length > 0) {
        const wanted = new Set(options.ids);
        entries = entries.filter((entry) => wanted.has(entry.id));
    }
    if (options.host) {
        entries = entries.filter((entry) => new URL(entry.url).host === options.host);
    }

    const failures = [];
    let next = 0;
    let done = 0;
    async function worker() {
        while (next < entries.length) {
            const entry = entries[next++];
            const result = await check(entry.url, options.timeout);
            done++;
            if (!result.ok) failures.push({ ...entry, why: result.why });
            if (!result.ok || options.verbose) {
                console.log(`${result.ok ? 'ok  ' : 'FAIL'} ${entry.id}  ${entry.url}  ${result.why}`);
            } else if (done % 100 === 0) {
                console.error(`  ${done}/${entries.length}`);
            }
        }
    }
    await Promise.all(Array.from({ length: Math.min(options.jobs, entries.length) }, worker));

    console.log(`\n${entries.length - failures.length} of ${entries.length} URLs serve a PDF; `
        + `${failures.length} failed.`);
    process.exit(failures.length > 0 ? 1 : 0);
}

main();
