// plan-58-A: writes urls.tsv, one DigiKey filter URL per (maker, circuit) of the slide-switch census.
// DigiKey's `s=` query value is lz-string (compressToEncodedURIComponent) of
//   {"1": {<filter key>: {"c": false, "vn": false, "o": [<value ids>], "pt": <"String" for a parametric key, else "">}}}
// (decoded from DigiKey's own links and tested 2026-11-24: Circuit SPDT alone gives 451 in-stock parts).
// usage: in a temp dir: npm i lz-string; NODE_PATH=$PWD/node_modules node <path>/make-urls.js > urls.tsv
const lz = require('lz-string');
const MAKERS = [['C&K', 108], ['NKK', 360], ['CIT', 2449], ['E-Switch', 141]];
const CIRCUITS = [['SPST', 406576], ['SPDT', 405967], ['DPST', 340489], ['DPDT', 340464]];
for (const [maker, mid] of MAKERS) {
  for (const [circuit, cid] of CIRCUITS) {
    const f = {
      '-2': {c: false, vn: false, o: ['5'], pt: ''},          // Stocking Options: In Stock
      '1989': {c: false, vn: false, o: ['0'], pt: ''},        // Product Status: Active
      '-1': {c: false, vn: false, o: [String(mid)], pt: ''},  // Manufacturer
      '130': {c: false, vn: false, o: [String(cid)], pt: 'String'}, // Circuit
    };
    const s = lz.compressToEncodedURIComponent(JSON.stringify({'1': f}));
    console.log([maker, circuit, 'https://www.digikey.com/en/products/filter/slide-switches/213?s=' + s].join('\t'));
  }
}
