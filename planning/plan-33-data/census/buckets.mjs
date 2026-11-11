// plan-33: one DigiKey call per bucket query; prints each response's
// Capacitance facet as JSON lines. Usage (DIGIKEY_* in the environment):
//   node buckets.mjs <out.jsonl> [<label prefix>]   (re-running appends only missing labels)
// Each line: label, ProductsCount, the Capacitance facet (values) and every
// other parametric facet plus Manufacturer (facets), for footprint counts.
// Every query: makers KEMET 399, Cal-Chip 4713, KYOCERA AVX 478; in stock;
// status Active (0) or Last Time Buy (4); Limit 1 (only facets are read).
import fs from 'fs';
const API = 'https://api.digikey.com';
import { queries as q, MAKERS } from './queries.mjs';
const out = process.argv[2];
const prefix = process.argv[3] || '';
if (!out) { console.error('usage: node buckets.mjs <out.jsonl>'); process.exit(2); }

const id = process.env.DIGIKEY_CLIENT_ID, secret = process.env.DIGIKEY_CLIENT_SECRET;
const tok = await (await fetch(`${API}/v1/oauth2/token`, { method: 'POST',
  headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
  body: new URLSearchParams({ client_id: id, client_secret: secret, grant_type: 'client_credentials' }) })).json();
const have = new Set(fs.existsSync(out) ? fs.readFileSync(out, 'utf8').split('\n').filter(Boolean).map((l) => JSON.parse(l).label) : []);
let left = '?';
for (const one of q) {
  if (have.has(one.label) || !one.label.startsWith(prefix)) continue; // resumable: a re-run skips labels already written
  const r = await fetch(`${API}/products/v4/search/keyword`, { method: 'POST',
    headers: { Authorization: `Bearer ${tok.access_token}`, 'X-DIGIKEY-Client-Id': id,
      'X-DIGIKEY-Locale-Site': 'US', 'X-DIGIKEY-Locale-Language': 'en', 'Content-Type': 'application/json' },
    body: JSON.stringify({ Keywords: '', Limit: 1, Offset: 0, FilterOptionsRequest: {
      CategoryFilter: [{ Id: String(one.cat) }], ManufacturerFilter: MAKERS,
      StatusFilter: [{ Id: '0' }, { Id: '4' }], MinimumQuantityAvailable: 1,
      ParameterFilterRequest: { CategoryFilter: { Id: String(one.cat) }, ParameterFilters: one.params } } }) });
  left = r.headers.get('x-ratelimit-remaining') ?? left;
  const body = await r.json();
  if (!r.ok) { console.error(one.label, r.status, JSON.stringify(body).slice(0, 300)); continue; }
  const cap = (((body.FilterOptions || {}).ParametricFilters) || []).find((f) => f.ParameterId === 2049);
  fs.appendFileSync(out, JSON.stringify({ label: one.label, count: body.ProductsCount,
    values: cap ? cap.FilterValues.map((v) => [v.ValueName, v.ProductCount]) : [],
    facets: Object.fromEntries([['Manufacturer', ((body.FilterOptions || {}).Manufacturers || []).map((m) => [m.Value, m.ProductCount])],
      ...(((body.FilterOptions || {}).ParametricFilters) || []).filter((f) => f.ParameterId !== 2049)
        .map((f) => [f.ParameterName, f.FilterValues.map((v) => [v.ValueName, v.ProductCount])])]) }) + '\n');
}
console.error(`${q.length} queries, ${left} calls left today`);
