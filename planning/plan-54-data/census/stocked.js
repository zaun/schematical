// plan-54 census: how many of a (category, maker)'s first 50 listed parts have QuantityAvailable above 0.
// usage: node stocked.js <category>:<maker id>...  One call each. Credentials as list.js.
const fs=require('fs'),os=require('os'),path=require('path');
const tok=JSON.parse(fs.readFileSync(path.join(os.tmpdir(),'digikey-token.json'),'utf8')).token;
(async()=>{
 for (const pair of process.argv.slice(2)) {
  const [cat,mfr]=pair.split(':');
  const r=await fetch('https://api.digikey.com/products/v4/search/keyword',{method:'POST',headers:{Authorization:'Bearer '+tok,'X-DIGIKEY-Client-Id':process.env.DIGIKEY_CLIENT_ID,'X-DIGIKEY-Locale-Site':'US','X-DIGIKEY-Locale-Language':'en','X-DIGIKEY-Locale-Currency':'USD','Content-Type':'application/json'},body:JSON.stringify({Keywords:'',Limit:50,Offset:0,FilterOptionsRequest:{CategoryFilter:[{Id:cat}],ManufacturerFilter:[{Id:mfr}],MinimumQuantityAvailable:1,StatusFilter:[{Id:'0'}]}})});
  const d=JSON.parse(await r.text()); const P=d.Products||[];
  console.log(pair,'count',d.ProductsCount,'returned',P.length,'stocked',P.filter(p=>(p.QuantityAvailable||0)>0).length,'left',r.headers.get('x-ratelimit-remaining'));
 }
})();
