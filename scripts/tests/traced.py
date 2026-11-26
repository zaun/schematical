"""Run a script under `trace` counting, keeping its exit code.

usage: python3 traced.py <counts file> <script> [args...]

Counts the script's lines into the counts file (merged with what it holds), then
exits as the script did: SystemExit and any other exception propagate after the
counts are saved. See support.py.
"""

import os
import sys
import trace

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import support  # noqa: E402


def main():
    counts, path = sys.argv[1], sys.argv[2]
    sys.argv = [path, *sys.argv[3:]]
    sys.path[0] = os.path.dirname(path)
    with open(path) as fh:
        code = compile(fh.read(), path, 'exec')
    tracer = trace.Trace(count=1, trace=0)
    try:
        globs = {'__file__': path, '__name__': '__main__', '__package__': None, '__cached__': None}
        tracer.runctx(code, globs, globs)
    finally:
        support.save_counts(tracer.counts, counts)


main()
