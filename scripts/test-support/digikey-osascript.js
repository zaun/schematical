// Stands in for `osascript` when scripts/digikey.test.js runs digikey.js's
// Safari path: a shell wrapper named `osascript` on PATH execs this file.
// It takes the next entry of DK_QUEUE's "safari" list (its position kept in
// DK_LOG + ".safari"), logs the request file Safari would have run, and
// answers as osascript would:
//   {result: {status, headers, text}}  stdout is that object as JSON
//   {stdout: "..."}                    stdout as is
//   {stderr: "...", code: n}           fails with that stderr and exit code

'use strict';

const fs = require('fs');

const queue = JSON.parse(fs.readFileSync(process.env.DK_QUEUE, 'utf8')).safari || [];
const counter = `${process.env.DK_LOG}.safari`;
const n = fs.existsSync(counter) ? Number(fs.readFileSync(counter, 'utf8')) : 0;
fs.writeFileSync(counter, String(n + 1));

const args = process.argv.slice(2);
const script = fs.readFileSync(args[args.length - 1], 'utf8');
fs.appendFileSync(process.env.DK_LOG, JSON.stringify({ kind: 'safari', script, applescript: args.slice(0, -1) }) + '\n');

const entry = queue[n];
if (!entry) {
  process.stderr.write(`digikey-osascript: no answer queued for request ${n + 1}\n`);
  process.exit(1);
}
if (entry.stderr !== undefined) {
  process.stderr.write(entry.stderr);
  process.exit(entry.code || 1);
}
process.stdout.write(entry.stdout !== undefined ? entry.stdout : JSON.stringify(entry.result));
