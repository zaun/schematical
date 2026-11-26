// usage: node url.js <slug/id> '<json of extra filters {key:[ids]}>' ; adds In Stock + Active
const lz=require('lz-string');
const [,, cat, extra] = process.argv;
const f={'-2':{c:false,vn:false,o:['5'],pt:''},'1989':{c:false,vn:false,o:['0'],pt:''}};
const x=JSON.parse(extra||'{}');
for (const k in x) f[k]={c:false,vn:false,o:x[k].map(String),pt:(k.startsWith('-')?'':'String')};
console.log('https://www.digikey.com/en/products/filter/'+cat+'?s='+lz.compressToEncodedURIComponent(JSON.stringify({'1':f})));
