# Builds a SingleStepTests subset for cpu_core_test: the first test (or the
# first <count>) of every opcode file, one minified JSON object per line,
# sorted by file path.
#
# Usage:
#   python3 -I make_sst_subset.py <repo> <sha> <listfile> <prefix-regex> <out.jsonl> [count]
#
# <listfile> holds "<path> <size>" lines, from the repository's git tree:
#   gh api "repos/SingleStepTests/<repo>/git/trees/<sha>?recursive=1" \
#     -q '.tree[] | select(.path|test("<regex>")) | "\(.path) \(.size)"' > list.txt
#
# The committed subsets (see plan-09-A §4.4):
#   65x02 2f6980a2d95757486c7bee24355c360e40e2a224 '^6502/v1/'     sst-6502.jsonl
#   65x02 2f6980a2d95757486c7bee24355c360e40e2a224 '^wdc65c02/v1/' sst-65c02.jsonl
#   z80   ebe1875d48f374bcfd4b505d8eb8ee751568b5f7 '^v1/'          sst-z80.jsonl
#   65816 307c63f5673c8c6b861c52f71ddb89212a227480 '^v1/'          sst-65816.jsonl 10
#         (plan-14-G: the first 10 tests of each of the 512 v1/XX.{e,n}.json)
#
# Each file's head is fetched with an HTTP Range request (the whole file on
# 416, or when the head holds fewer than <count> whole tests), and its first
# <count> test objects are kept. Empty files (65C02 WAI, STP) are
# skipped. The test data is MIT; see LICENSE-SingleStepTests-*.
import json, re, sys, urllib.request, urllib.error, urllib.parse, concurrent.futures as cf

repo, sha, listfile, prefix, out = sys.argv[1:6]
count = int(sys.argv[6]) if len(sys.argv) > 6 else 1
paths = [l.rsplit(None, 1)[0] for l in open(listfile) if re.match(prefix, l)]


def objects(text, n):
    """Up to n whole JSON objects from the start of a JSON array's text."""
    found = []
    decoder = json.JSONDecoder()
    at = text.find("{")
    while at >= 0 and len(found) < n:
        try:
            obj, end = decoder.raw_decode(text[at:])
        except json.JSONDecodeError:
            break
        found.append(obj)
        at = text.find("{", at + end)
    return found


def head(path):
    url = f"https://raw.githubusercontent.com/SingleStepTests/{repo}/{sha}/" + urllib.parse.quote(path)
    req = urllib.request.Request(url, headers={"Range": f"bytes=0-{16384 * count - 1}"})
    try:
        text = urllib.request.urlopen(req, timeout=60).read().decode()
    except urllib.error.HTTPError as e:
        if e.code != 416:
            raise
        text = urllib.request.urlopen(url, timeout=60).read().decode()
    found = objects(text, count)
    if len(found) < count and "{" in text:
        found = objects(urllib.request.urlopen(url, timeout=300).read().decode(), count)
    return path, found


with cf.ThreadPoolExecutor(16) as ex:
    rows = sorted(ex.map(head, paths))
with open(out, "w") as f:
    for path, objs in rows:
        if not objs:
            print("empty:", path)
            continue
        for obj in objs:
            f.write(json.dumps(obj, separators=(",", ":")) + "\n")
print(out, len(rows))
