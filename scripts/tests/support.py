"""Shared helpers for the scripts/ Python tests (plan-68-U).

The scripts are hyphen-named, so `load` imports one by its path. `run` runs one
as a program. Both use the same absolute path for a script, so `trace` counts
an in-process case and a program-style case under one file name.

Coverage (docs/testing.md § Coverage): `node scripts/coverage.js --scripts` runs
this suite under `python3 -m trace --count --file=<counts> --no-report` and sets
SCRIPTS_COVERAGE_FILE=<counts>. That `trace` command does not save its counts
with --no-report (it writes the counts file only from write_results), so this
module saves them itself when the suite exits, merged with what is already in
the file. A program-style case runs its script through `traced.py`, which counts
the child's lines into the same file and keeps the script's exit code (`python3
-m trace` itself drops it: it catches SystemExit).
"""

import atexit
import importlib.util
import os
import pickle
import subprocess
import sys
import trace

TESTS = os.path.dirname(os.path.abspath(__file__))
SCRIPTS = os.path.dirname(TESTS)
REPO = os.path.dirname(SCRIPTS)
FIXTURES = os.path.join(TESTS, 'fixtures')
FAKES = os.path.join(TESTS, 'fakes')
COUNTS = os.environ.get('SCRIPTS_COVERAGE_FILE')


def script(name):
    return os.path.join(SCRIPTS, name)


def fixture(*parts):
    return os.path.join(FIXTURES, *parts)


def load(name, module_name):
    """Import scripts/<name> as module_name."""
    spec = importlib.util.spec_from_file_location(module_name, script(name))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def run(name, *args, cwd=None, env=None):
    """Run scripts/<name> as a program: CompletedProcess with text stdout/stderr."""
    cmd = [sys.executable, script(name), *args]
    if COUNTS:
        cmd = [sys.executable, os.path.join(TESTS, 'traced.py'), COUNTS, script(name), *args]
    return subprocess.run(cmd, cwd=cwd, env=env, capture_output=True, text=True, timeout=600)


def save_counts(counts, path):
    """Merge {(file, line): n} into trace's counts file at path."""
    results = trace.CoverageResults(dict(counts), infile=path, outfile=path)
    with open(path, 'wb') as fh:
        pickle.dump((results.counts, results.calledfuncs, results.callers), fh, 1)


def _parent_tracer():
    """The Trace object of `python3 -m trace` running this suite, if any. Its
    class is `__main__.Trace` (trace ran as a program), not `trace.Trace`."""
    owner = getattr(sys.gettrace(), '__self__', None)
    is_trace = type(owner).__name__ == 'Trace' and isinstance(getattr(owner, 'counts', None), dict)
    return owner if is_trace else None


_tracer = _parent_tracer() if COUNTS else None
if _tracer is not None:
    atexit.register(lambda: save_counts(_tracer.counts, COUNTS))
