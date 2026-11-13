#!/usr/bin/env python3
# plan-37: fetch one Nexperia URL with a browser TLS fingerprint (Nexperia
# answers 403 to plain HTTP clients; plan-37-A § 2).
# usage: <venv>/bin/python -I nxget.py <url> <out>
#   venv: python3 -m venv /tmp/p37venv && /tmp/p37venv/bin/pip install curl_cffi
# Prints status, content type and bytes. Exit 1 unless the body starts with
# %PDF- (out is then not written). With out "-" the body (any type) goes to
# stdout and the summary to stderr (used to read product pages).
import sys
from curl_cffi import requests

url, out = sys.argv[1], sys.argv[2]
r = requests.get(url, impersonate='chrome', timeout=120)
body = r.content
summary = f'{r.status_code} {r.headers.get("content-type", "")} {len(body)}'
if out == '-':
    sys.stdout.buffer.write(body)
    print(summary, file=sys.stderr)
    sys.exit(0 if r.status_code == 200 else 1)
print(summary)
if r.status_code != 200 or not body.startswith(b'%PDF-'):
    sys.exit(1)
with open(out, 'wb') as f:
    f.write(body)
