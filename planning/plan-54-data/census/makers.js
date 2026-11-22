// plan-54 census: every maker of DigiKey categories (in stock, Active), biggest first. One call per category.
// usage: node makers.js <category id>...   (414, 413, 303). Credentials as list.js.
const fs=require('fs'),os=require('os'),path=require('path');
const tok=JSON.parse(fs.readFileSync(path.join(os.tmpdir(),'digikey-token.json'),'utf8')).token;
(async()=>{
 for (const cat of process.argv.slice(2)) {
  const r=await fetch('https://api.digikey.com/products/v4/search/keyword',{method:'POST',headers:{Authorization:'Bearer '+tok,'X-DIGIKEY-Client-Id':process.env.DIGIKEY_CLIENT_ID,'X-DIGIKEY-Locale-Site':'US','X-DIGIKEY-Locale-Language':'en','X-DIGIKEY-Locale-Currency':'USD','Content-Type':'application/json'},body:JSON.stringify({Keywords:'',Limit:1,Offset:0,FilterOptionsRequest:{CategoryFilter:[{Id:cat}],MinimumQuantityAvailable:1,StatusFilter:[{Id:'0'}]}})});
  const d=JSON.parse(await r.text());
  console.log('== cat',cat,'active in stock',d.ProductsCount,'left',r.headers.get('x-ratelimit-remaining'));
  console.log([...(d.FilterOptions.Manufacturers||[])].sort((a,b)=>b.ProductCount-a.ProductCount).map(m=>`${m.Id} ${m.Value} ${m.ProductCount}`).join('\n'));
 }
})();
