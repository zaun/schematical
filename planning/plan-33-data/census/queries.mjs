// plan-33: the bucket queries, shared by buckets.mjs and bucket-parts.mjs.
// Each query: label "<type>|<package>|<dielectric or voltage>", DigiKey
// category id, and parametric filters (ParameterId, value ids). A bucket is a
// query plus one E6 capacitance (buckets.py).
export const MAKERS = [399, 4713, 478].map((id) => ({ Id: String(id) }));
const PKG = { '0402': '39158', '0603': '39246', '0805': '39329', '1206': '83711',
  Radial: '392278', Disc: '392342', Axial: '317190', Can: '392320', CanSMD: '392327', '2917': '166804' };
const C0G = '324183';
// Every Temperature Coefficient id DigiKey lists for category 60 except
// C0G, NP0 (324183) and '-' (1), as of 2026-11-11: the "other" bucket.
const CLASS2 = ["317283", "760697", "319545", "319892", "319900", "320953", "322366", "322750", "760698", "760699", "323944", "323952", "324184", "324185", "324186", "324187", "328981", "760700", "329106", "808526", "330635", "342322", "345849", "350366", "363024", "370517", "378741", "757556", "378749", "378750", "378753", "378772", "760701", "506234", "760692", "760691", "383166", "383213", "392131", "392133", "392155", "399579", "399606", "403491", "409860", "868212", "417026", "419142", "422308", "422405", "422407", "422408", "854907", "422413", "422414", "422415", "422416", "422417", "422418", "422419", "422420", "422424", "422425", "477815", "758225", "422426", "447888", "423162", "423163", "423164", "423165", "423166", "423167", "423168", "423169", "423401", "423402", "423404", "423405", "447642"];
const TA_SMD_PKG = ['39246', '39329', '83711', '94862', '94863', '150796', '166804'];
const AL_V = ['6.3 V', '10 V', '16 V', '25 V', '35 V', '50 V', '63 V', '100 V'];
const TA_V = ['4 V', '6.3 V', '10 V', '16 V', '20 V', '25 V', '35 V', '50 V'];

export const queries = [];
const q = queries;
const p = (id, values) => ({ ParameterId: id, FilterValues: values.map((v) => ({ Id: v })) });
for (const pkg of ['0402', '0603', '0805', '1206', 'Radial', 'Disc', 'Axial']) {
  q.push({ label: `ceramic|${pkg}|C0G`, cat: 60, params: [p(16, [PKG[pkg]]), p(17, [C0G])] });
  q.push({ label: `ceramic|${pkg}|other`, cat: 60, params: [p(16, [PKG[pkg]]), p(17, CLASS2)] });
}
for (const v of AL_V) {
  q.push({ label: `aluminum|Radial|${v}`, cat: 58, params: [p(16, [PKG.Can]), p(2079, [v])] });
  q.push({ label: `aluminum|SMD|${v}`, cat: 58, params: [p(16, [PKG.CanSMD]), p(2079, [v])] });
  q.push({ label: `al-polymer|Radial|${v}`, cat: 69, params: [p(16, [PKG.Can]), p(2079, [v])] });
  q.push({ label: `al-polymer|SMD|${v}`, cat: 69, params: [p(16, [PKG.CanSMD, PKG['2917']]), p(2079, [v])] });
}
for (const v of TA_V) {
  q.push({ label: `tantalum|SMD|${v}`, cat: 59, params: [p(69, ['409393']), p(183, ['376942']), p(16, TA_SMD_PKG), p(2079, [v])] });
  q.push({ label: `tantalum|Radial|${v}`, cat: 59, params: [p(16, [PKG.Radial]), p(2079, [v])] });
  q.push({ label: `ta-polymer|SMD|${v}`, cat: 70, params: [p(69, ['409393']), p(16, TA_SMD_PKG), p(2079, [v])] });
}
for (const v of ['4 V', '6.3 V', '10 V']) {
  q.push({ label: `niobium|SMD|${v}`, cat: 67, params: [p(2079, [v])] });
}

