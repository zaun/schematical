"""plan-68-K: every backend allocation through logic_alloc."""
import glob, re, sys
sys.path.insert(0, '/private/tmp/claude-501/p68')
from split_model import code_replace

S = 'backend/src/'
names = {'malloc': 'logic_malloc', 'calloc': 'logic_calloc', 'realloc': 'logic_realloc', 'free': 'logic_free',
         'strdup': 'logic_strdup'}
files = [f for f in glob.glob(S + '**/*.c', recursive=True) if '/third_party/' not in f and not f.endswith('logic_alloc.c')]
changed = []
for f in sorted(files):
    s = open(f).read()
    # only calls: a name directly followed by "("
    t = code_replace(s, names)
    # a name used as a value without a call (a function pointer) would be renamed too; check
    for old, new in names.items():
        for m in re.finditer(r'\b' + new + r'\b(?!\s*\()', t):
            raise SystemExit(f'{f}: {new} used without a call at {t[:m.start()].count(chr(10)) + 1}')
    if t != s:
        incs = list(re.finditer(r'^#include [<"][^>"]+[>"]\n', t, re.M))
        last = incs[-1]
        t = t[:last.end()] + '#include "logic_alloc.h"\n' + t[last.end():]
        open(f, 'w').write(t)
        changed.append(f)

# memory handed to the caller: the error messages
edits = {
    S + 'library/library_load_device.c': [
        ('*error = ctx->error != NULL ? ctx->error : lib_copy_message("out of memory");',
         '*error = logic_alloc_disown(ctx->error != NULL ? ctx->error : lib_copy_message("out of memory"));'),
        ('*error = lib_copy_message("load_footprints: no library or text");',
         '*error = logic_alloc_disown(lib_copy_message("load_footprints: no library or text"));'),
        ('*error = lib_copy_message("load_datasheets: no library or text");',
         '*error = logic_alloc_disown(lib_copy_message("load_datasheets: no library or text"));'),
        ('*error = lib_copy_message("load_parts: no library or text");',
         '*error = logic_alloc_disown(lib_copy_message("load_parts: no library or text"));'),
    ],
    S + 'library/library.c': [
        ('*error = lib_copy_message("load_part_file: no library or text");',
         '*error = logic_alloc_disown(lib_copy_message("load_part_file: no library or text"));'),
    ],
    S + 'memory_image.c': [
        ('*error = logic_malloc((size_t)length + 1);', '*error = logic_alloc_disown(logic_malloc((size_t)length + 1));'),
    ],
}
for f, pairs in edits.items():
    s = open(f).read()
    for a, b in pairs:
        assert s.count(a) == 1, (f, a)
        s = s.replace(a, b)
    open(f, 'w').write(s)
print(len(changed), 'files')
