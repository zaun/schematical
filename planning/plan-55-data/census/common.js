'use strict';
// plan-55 census: what families.js and list.js share. The three power-switch families as DigiKey category +
// parametric filters (names, not ids: the ids are looked up from the category's facets), the 14 listing
// pieces, and the request path: scripts/digikey.js's login and transport (Cloudflare answers scripts with a
// challenge since 2026-11-23, so the requests fall back to Safari there).
//
// A company's DigiKey labels count together (plan-54-A § 2): Analog Devices = 505 + 175, onsemi = 488 +
// 600039, Microchip = 150 + 576, Texas Instruments = 296 + 14.
const path = require('path');
const dk = require(path.join(__dirname, '..', '..', '..', 'scripts', 'digikey.js'));

const API = 'https://api.digikey.com';

// op: '=' the named values, '~' every value whose name contains the text, '!~' every value whose name does not.
const FAMILIES = [
  { key: 'ideal', title: 'Ideal diodes (internal FET)', category: '758',
    filters: [['Internal Switch(s)', '=', ['Yes']]],
    picks: { 'Texas Instruments': ['296', '14'], 'Analog Devices': ['505', '175'], 'Monolithic Power Systems': ['1589'] } },
  { key: 'load', title: 'Load switches', category: '726',
    filters: [['Switch Type', '=', ['Load Switch', 'Power Switch']], ['Output Configuration', '=', ['High Side']],
              ['Number of Outputs', '=', ['1', '2']], ['Fault Protection', '!~', ['Current Limit']]],
    picks: { 'Texas Instruments': ['296', '14'], onsemi: ['488', '600039'], Microchip: ['150', '576'] } },
  { key: 'limited', title: 'Current-limited and USB switches', category: '726',
    filters: [['Switch Type', '=', ['Load Switch', 'Power Switch', 'USB Switch']], ['Output Configuration', '=', ['High Side']],
              ['Number of Outputs', '=', ['1', '2', '3', '4']], ['Fault Protection', '~', ['Current Limit']]],
    picks: { 'Texas Instruments': ['296', '14'], 'Diodes Incorporated': ['31'], Microchip: ['150', '576'] } },
];

// The 14 listing pieces (family-maker id) of plan-55-B § 3.1. Texas Instruments' label 14 (National) has no
// ideal-diode or load-switch parts in stock, so only `limited` lists it.
const PIECES = ['ideal-296', 'ideal-505', 'ideal-175', 'ideal-1589', 'load-296', 'load-488', 'load-600039',
  'load-150', 'load-576', 'limited-296', 'limited-14', 'limited-31', 'limited-150', 'limited-576'];

// A request context: the key (`opt2` = the second key, its own quota), the login token, and the counters.
async function open(opt2) {
  const creds = dk.credentials(opt2);
  return { creds, token: await dk.token(creds), calls: 0, left: null };
}

// One KeywordSearch call. `q`: category, params (resolved ParameterFilters), makers (ids), series (ids),
// limit, offset, sort (true: QuantityAvailable descending). In stock (1) and Active (0) only. Retries a
// failed transport three times; a 429 (the daily quota) is fatal.
async function search(ctx, q) {
  const filter = { CategoryFilter: [{ Id: q.category }], MinimumQuantityAvailable: 1, StatusFilter: [{ Id: '0' }] };
  if (q.params && q.params.length) filter.ParameterFilterRequest = { CategoryFilter: { Id: q.category }, ParameterFilters: q.params };
  if (q.makers && q.makers.length) filter.ManufacturerFilter = q.makers.map((m) => ({ Id: String(m) }));
  if (q.series && q.series.length) filter.SeriesFilter = q.series.map((s) => ({ Id: String(s) }));
  const body = { Keywords: '', Limit: q.limit || 1, Offset: q.offset || 0, FilterOptionsRequest: filter };
  if (q.sort) body.SortOptions = { Field: 'QuantityAvailable', SortOrder: 'Descending' };
  for (let attempt = 1, retried = false; ; attempt++) {
    try {
      const { response, text } = await dk.fetchRetry(`${API}/products/v4/search/keyword`, {
        method: 'POST',
        headers: { Authorization: `Bearer ${ctx.token}`, 'X-DIGIKEY-Client-Id': ctx.creds.id, 'X-DIGIKEY-Locale-Site': 'US',
          'X-DIGIKEY-Locale-Language': 'en', 'X-DIGIKEY-Locale-Currency': 'USD', 'Content-Type': 'application/json' },
        body: JSON.stringify(body) });
      ctx.calls++;
      ctx.left = response.headers.get('x-ratelimit-remaining') ?? ctx.left;
      if (response.status === 401 && !retried) { retried = true; ctx.token = await dk.token(ctx.creds, true); attempt--; continue; }
      if (response.status === 429) throw Object.assign(new Error(`rate limited: ${text.slice(0, 200)}`), { fatal: true });
      if (!response.ok) throw new Error(`HTTP ${response.status}: ${text.slice(0, 200)}`);
      return JSON.parse(text);
    } catch (e) {
      if (e.fatal || attempt >= 4) throw e;
      console.error(`common.js: ${e.message}; retry ${attempt} of 3`);
      await new Promise((res) => setTimeout(res, 2000 * attempt));
    }
  }
}

// The family's filters as DigiKey ParameterFilters, from the category's facets (`facets` = a search result).
function resolve(facets, filters) {
  return filters.map(([name, op, vals]) => {
    const f = facets.FilterOptions.ParametricFilters.find((x) => x.ParameterName === name);
    if (!f) throw new Error(`no facet ${name}`);
    const live = f.FilterValues.filter((v) => v.ProductCount > 0);
    const chosen = op === '=' ? vals.map((n) => { const v = live.find((x) => x.ValueName === n); if (!v) throw new Error(`no value ${name}=${n}`); return v; })
      : live.filter((v) => (op === '~') === v.ValueName.includes(vals[0]));
    return { ParameterId: f.ParameterId, FilterValues: chosen.map((v) => ({ Id: String(v.ValueId) })) };
  });
}

module.exports = { FAMILIES, PIECES, open, search, resolve };
