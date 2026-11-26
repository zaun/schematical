const fs = require('fs'); const path = require('path');
const { countCStyle } = require('./loc.js');
const HDR = /^(\/\* ?(-{2,}|={2,})|\/\/ ?(-{3,}|={3,})|\s*\/\/ MARK:)/;
for (const f of process.argv.slice(2)) {
  const ext = path.extname(f);
  const opts = ext === '.swift' ? { nested: true, swift: true } : ext === '.js' ? { js: true } : {};
  const lines = fs.readFileSync(f, 'utf8').split('\n');
  const starts = [0];
  lines.forEach((l, i) => { if (i > 0 && HDR.test(l)) starts.push(i); });
  console.log(`== ${f}`);
  for (let k = 0; k < starts.length; k++) {
    const a = starts[k], b = k + 1 < starts.length ? starts[k + 1] : lines.length;
    const chunk = lines.slice(a, b).join('\n');
    const title = k === 0 ? '(top)' : lines[a].replace(/[-=*\/]{2,}/g, '').replace(/^\s*(\/\/|\/\*)\s*/, '').trim().slice(0, 70);
    console.log(`${String(countCStyle(chunk, opts)).padStart(5)}  L${a + 1}-${b}  ${title}`);
  }
}
