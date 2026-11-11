// plan-33 Phase 1: the 50 most-stocked parts of each bucket, one DigiKey call
// per bucket. Usage (DIGIKEY_* in the environment):
//   node bucket-parts.mjs <buckets.tsv> <raw.jsonl> [--only N]
// <buckets.tsv> comes from buckets.py (capacitance: value ids joined by ';'). Each output line is one bucket:
// {label, capacitance, count, products: [...]} with the fields select.py reads.
// Resumable: buckets already in <raw.jsonl> are skipped, so a run cut by the
// daily quota continues the next day.
import fs from 'fs';
import { queries, MAKERS } from './queries.mjs';
const API = 'https://api.digikey.com';
const [tsv, out] = process.argv.slice(2);
const onlyAt = process.argv.indexOf('--only');
const only = onlyAt > 0 ? Number(process.argv[onlyAt + 1]) : Infinity;
if (!tsv || !out) { console.error('usage: node bucket-parts.mjs <buckets.tsv> <raw.jsonl> [--only N]'); process.exit(2); }
const byLabel = new Map(queries.map((q) => [q.label, q]));
const buckets = fs.readFileSync(tsv, 'utf8').split('\n').slice(1).filter(Boolean).map((l) => l.split('\t'));
const key = (label, cap) => `${label}\t${cap}`;
const have = new Set(fs.existsSync(out) ? fs.readFileSync(out, 'utf8').split('\n').filter(Boolean)
  .map((l) => { const r = JSON.parse(l); return key(r.label, r.capacitance); }) : []);

const id = process.env.DIGIKEY_CLIENT_ID, secret = process.env.DIGIKEY_CLIENT_SECRET;
// Tokens last 10 minutes; a 401 logs in again and retries the bucket once.
const login = async () => (await (await fetch(`${API}/v1/oauth2/token`, { method: 'POST',
  headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
  body: new URLSearchParams({ client_id: id, client_secret: secret, grant_type: 'client_credentials' }) })).json()).access_token;
let token = await login();
const param = (p, name) => ((p.Parameters || []).find((x) => x.ParameterText === name) || {}).ValueText || '';
let calls = 0, left = '?';
for (const [label, cap] of buckets) {
  if (have.has(key(label, cap)) || calls >= only) continue;
  const q = byLabel.get(label);
  const ask = () => fetch(`${API}/products/v4/search/keyword`, { method: 'POST',
    headers: { Authorization: `Bearer ${token}`, 'X-DIGIKEY-Client-Id': id,
      'X-DIGIKEY-Locale-Site': 'US', 'X-DIGIKEY-Locale-Language': 'en', 'Content-Type': 'application/json' },
    body: JSON.stringify({ Keywords: '', Limit: 50, Offset: 0,
      SortOptions: { Field: 'QuantityAvailable', SortOrder: 'Descending' },
      FilterOptionsRequest: { CategoryFilter: [{ Id: String(q.cat) }], ManufacturerFilter: MAKERS,
        StatusFilter: [{ Id: '0' }, { Id: '4' }], MinimumQuantityAvailable: 1,
        ParameterFilterRequest: { CategoryFilter: { Id: String(q.cat) },
          ParameterFilters: [...q.params, { ParameterId: 2049,
            FilterValues: cap.split(';').map((v) => ({ Id: v })) }] } } }) });
  let r = await ask();
  calls += 1;
  if (r.status === 401) { token = await login(); r = await ask(); calls += 1; }
  left = r.headers.get('x-ratelimit-remaining') ?? left;
  if (r.status === 429) { console.error('rate limited; re-run tomorrow'); break; }
  const body = await r.json();
  if (!r.ok) { console.error(label, cap, r.status, JSON.stringify(body).slice(0, 300)); continue; }
  const products = (body.Products || []).map((p) => ({
    mpn: p.ManufacturerProductNumber, maker: p.Manufacturer && p.Manufacturer.Name,
    status: p.ProductStatus && p.ProductStatus.Status, stock: p.QuantityAvailable,
    datasheet: p.DatasheetUrl || '', series: (p.Series && p.Series.Name) || '',
    description: (p.Description && p.Description.ProductDescription) || '',
    voltage: param(p, 'Voltage - Rated'), dielectric: param(p, 'Temperature Coefficient'),
    tolerance: param(p, 'Tolerance'), package: param(p, 'Package / Case'),
    mounting: param(p, 'Mounting Type'), size: param(p, 'Size / Dimension'),
    height: param(p, 'Height - Seated (Max)') || param(p, 'Thickness (Max)'),
    spacing: param(p, 'Lead Spacing'), ratings: param(p, 'Ratings') || param(p, 'Grade'),
    sizeCode: param(p, 'Manufacturer Size Code'), esr: param(p, 'ESR (Equivalent Series Resistance)'),
  }));
  fs.appendFileSync(out, JSON.stringify({ label, capacitance: cap, count: body.ProductsCount, products }) + '\n');
}
console.error(`${calls} calls, ${left} calls left today`);
