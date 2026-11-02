# Builds a SingleStepTests subset for cpu_core_test: the first test of every
# opcode file, one minified JSON object per line, sorted by file path.
#
# Usage:
#   python3 -I make_sst_subset.py <repo> <sha> <listfile> <prefix-regex> <out.jsonl>
#
# <listfile> holds "<path> <size>" lines, from the repository's git tree:
#   gh api "repos/SingleStepTests/<repo>/git/trees/<sha>?recursive=1" \
#     -q '.tree[] | select(.path|test("<regex>")) | "\(.path) \(.size)"' > list.txt
#
# The committed subsets (see plan-09-A §4.4):
#   65x02 2f6980a2d95757486c7bee24355c360e40e2a224 '^6502/v1/'     sst-6502.jsonl
#   65x02 2f6980a2d95757486c7bee24355c360e40e2a224 '^wdc65c02/v1/' sst-65c02.jsonl
#   z80   ebe1875d48f374bcfd4b505d8eb8ee751568b5f7 '^v1/'          sst-z80.jsonl
#
# Each file's head is fetched with an HTTP Range request (the whole file on
# 416), and its first test object is kept. Empty files (65C02 WAI, STP) are
# skipped. The test data is MIT; see LICENSE-SingleStepTests-*.
import json, re, sys, urllib.request, urllib.error, urllib.parse, concurrent.futures as cf

repo, sha, listfile, prefix, out = sys.argv[1:6]
paths = [l.rsplit(None, 1)[0] for l in open(listfile) if re.match(prefix, l)]


def first(path):
    url = f"https://raw.githubusercontent.com/SingleStepTests/{repo}/{sha}/" + urllib.parse.quote(path)
    req = urllib.request.Request(url, headers={"Range": "bytes=0-16383"})
    try:
        text = urllib.request.urlopen(req, timeout=60).read().decode()
    except urllib.error.HTTPError as e:
        if e.code != 416:
            raise
        text = urllib.request.urlopen(url, timeout=60).read().decode()
    if "{" not in text:
        return path, None
    obj, _ = json.JSONDecoder().raw_decode(text[text.index("{"):])
    return path, obj


with cf.ThreadPoolExecutor(16) as ex:
    rows = sorted(ex.map(first, paths))
with open(out, "w") as f:
    for path, obj in rows:
        if obj is None:
            print("empty:", path)
            continue
        f.write(json.dumps(obj, separators=(",", ":")) + "\n")
print(out, len(rows))
