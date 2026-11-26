import json,glob,collections,sys
root="schematical/schematical/Parts/"
# which files have drawer Logic/Gates
for f in sorted(glob.glob(root+"*.json")):
    try: d=json.load(open(f))
    except Exception as e: continue
    devs=d.get("devices") if isinstance(d,dict) else None
    if not devs: continue
    n=sum(1 for x in devs if x.get("drawer")==["Logic","Gates"])
    if n: print(f.split("/")[-1], "kind",d.get("kind"),"devices in Logic/Gates:",n,"of",len(devs))
d=json.load(open(root+"gate.json"))
print("top keys",[k for k in d if k!="devices"])
devs=d["devices"]
print("devices",len(devs))
ops=collections.Counter(); shape=collections.Counter(); drawers=collections.Counter()
keys=collections.Counter(); extra=collections.Counter(); roles=collections.Counter()
odd=[]
for x in devs:
    keys.update(x.keys()); drawers[tuple(x.get("drawer",[]))]+=1
    g=x["gates"]
    sig=(len(g),tuple(sorted(set((y["op"],len(y["inputs"])) for y in g))))
    shape[sig]+=1
    for y in g: ops[y["op"]]+=1
    used=set()
    for y in g: used.update(y["inputs"]); used.add(y["output"])
    outs=[y["output"] for y in g]; ins=[tuple(y["inputs"]) for y in g]
    if len(set(ins))!=len(ins): odd.append((x["id"],"shared inputs"))
    allin=[i for y in g for i in y["inputs"]]
    if len(set(allin))!=len(allin) and len(set(ins))==len(ins): odd.append((x["id"],"partially shared inputs"))
    for pk,pins in x.get("pinouts",{}).items():
        for p in pins:
            if isinstance(p,dict):
                roles[p.get("role")]+=1
                if p.get("name") not in used: extra[p.get("name")]+=1
print("device keys",dict(keys))
print("drawers",dict(drawers))
print("ops (gate count)",dict(ops))
for k,v in sorted(shape.items(),key=lambda kv:-kv[1]): print(v,k)
print("non-gate pin names",dict(extra))
print("roles",dict(roles))
print("odd",odd)
fl=collections.Counter()
for x in devs:
    for k in ("schmitt","openDrain","output","outputs","inputType","units","function"):
        if k in x: fl[(k,json.dumps(x[k]) if k!="function" else "")]+=1
print(dict(fl))
print(collections.Counter(x.get("function") for x in devs))
