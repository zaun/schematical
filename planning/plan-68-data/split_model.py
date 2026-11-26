"""plan-68-I/J: split a backend model source into pieces sharing one internal header.

Spec: (file, header, [(marker, piece), ...]). Text before the first marker is
the original file's (the core piece). The header gets, in file order, every
include, macro, type and anonymous enum of the file, then declarations of the
functions and data one piece defines and another uses (those lose `static`).
Everything else stays `static` in its piece. Function bodies are not touched.
"""
import re, sys
sys.path.insert(0, '/private/tmp/claude-501/p68')
from csplit import items, is_function, is_type

S = 'backend/src/'


def doc_start(lines, k):
    j = k
    while j > 0 and (lines[j - 1].startswith((' *', '/*', '//')) or lines[j - 1].strip() == '*/'):
        j -= 1
    return j


def marker_line(lines, m):
    kind, key = m
    for k, l in enumerate(lines):
        if kind == 'sec' and l.startswith('/* ') and key in l:
            return k
        if kind == 'fn' and re.match(r'^(static )?[^(=]*\b' + re.escape(key) + r'\(', l) and not l.rstrip().endswith(';'):
            return doc_start(lines, k)
    raise SystemExit(f'marker {m} not found')


def item_names(kind, t):
    if kind == 'pp':
        m = re.match(r'\s*#\s*define\s+(\w+)', t)
        return {m.group(1)} if m else set()
    if kind != 'code':
        return set()
    if re.match(r'\s*(typedef\s+)?(enum|struct|union)\b', t):
        names = set(re.findall(r'}\s*(\w+)\s*;', t))
        if re.match(r'\s*(typedef\s+)?enum', t):
            body = t[t.find('{') + 1:t.rfind('}')]
            names |= set(re.findall(r'\b([A-Za-z_]\w*)\b\s*(?:=[^,}]*)?(?:,|$)', body, re.M))
        m = re.match(r'\s*(?:typedef\s+)?(?:struct|union|enum)\s+(\w+)', t)
        if m:
            names.add(m.group(1))
        return names
    head = t.split('{', 1)[0] if '{' in t else t
    if '(' in head and '=' not in head.split('(')[0]:
        m = re.search(r'\b(\w+)\s*\(', head)
        return {m.group(1)} if m else set()
    m = re.search(r'\b(\w+)\s*(?:\[[^]]*\])*\s*(?:=|;)', t)
    return {m.group(1)} if m else set()


def code_replace(text, names):
    """Rename whole-word identifiers outside string literals and comments."""
    out, i, n = [], 0, len(text)
    # not a member access (.name, ->name): a field may share a function's name
    pattern = re.compile(r'(?<!\.)(?<!->)\b(' + '|'.join(map(re.escape, names)) + r')\b') if names else None
    start = 0
    while i < n:
        if text.startswith('/*', i):
            j = text.index('*/', i) + 2
        elif text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
        elif text[i] in '"\'':
            q, j = text[i], i + 1
            while text[j] != q:
                j += 2 if text[j] == '\\' else 1
            j += 1
        else:
            i += 1
            continue
        out.append(pattern.sub(lambda m: names[m.group(1)], text[start:i]) if pattern else text[start:i])
        out.append(text[i:j])
        i = start = j
    out.append(pattern.sub(lambda m: names[m.group(1)], text[start:]) if pattern else text[start:])
    return ''.join(out)


def split(path, header, cuts, prefix=''):
    src = open(S + path).read()
    lines = src.split('\n')
    points = sorted((marker_line(lines, m), piece) for m, piece in cuts)
    regions = [(0, path)] + points + [(len(lines), None)]
    piece_text = {}
    for (a, piece), (b, _) in zip(regions, regions[1:]):
        piece_text.setdefault(piece, '')
        piece_text[piece] += '\n'.join(lines[a:b]) + ('\n' if b < len(lines) else '')
    # items per piece, with leading comments
    pieces = {}
    for piece, text in piece_text.items():
        out, pending = [], ''
        for kind, t in items(text):
            if kind in ('blank', 'comment'):
                pending += t
                continue
            out.append((kind, pending, t, item_names(kind, t)))
            pending = ''
        pieces[piece] = (out, pending)
    header_parts = []
    decls = []
    shared_defs = {}  # name -> piece defining it
    bodies = {p: [] for p in pieces}
    for piece, (its, tail) in pieces.items():
        for kind, lead, t, names in its:
            if kind == 'pp' or (kind == 'code' and is_type(t)) or re.match(r'\s*enum\s*\{', t):
                header_parts.append((lead, t))
                bodies[piece].append(('moved', lead, t, names))
            else:
                bodies[piece].append((kind, lead, t, names))
    # which functions/data are used outside their piece
    text_of = {p: ''.join(l + t for k, l, t, n in b if k != 'moved') for p, b in bodies.items()}
    for piece, b in bodies.items():
        for kind, lead, t, names in b:
            if kind != 'code':
                continue
            if not t.lstrip().startswith('static'):
                continue  # already a global: nothing to share or rename
            for n in names:
                if any(re.search(r'(?<![\w.>])' + re.escape(n) + r'\b', text_of[o]) for o in bodies if o != piece):
                    shared_defs[n] = piece
    out = {}
    for piece, b in bodies.items():
        parts = []
        for kind, lead, t, names in b:
            if kind == 'moved':
                continue
            is_decl_only = kind == 'code' and not is_function(t) and re.search(r'\)\s*;\s*$', t.split('\n')[-2] if t.endswith('\n') else t)
            if names & set(shared_defs):
                if is_decl_only and t.lstrip().startswith('static'):
                    continue  # a static prototype of a shared function: the header declares it
                body = re.sub(r'^static\s+', '', t)
                if is_function(t):
                    decls.append(body.split('{', 1)[0].rstrip() + ';\n')
                elif not is_decl_only:
                    d = body.split('=', 1)[0].rstrip() if '=' in body else body.rstrip()
                    d = d.rstrip(';').rstrip()
                    if d.endswith('[]') and '=' in body:
                        # sizeof in another piece needs the length: count the
                        # initializer's elements (the compiler checks the two agree).
                        init = body[body.index('{', body.index('=')) + 1:body.rindex('}')]
                        depth, count, has = 0, 0, False
                        for ch in re.sub(r'"(?:\\.|[^"\\])*"', '""', init):
                            if ch in '{(':
                                depth += 1
                            elif ch in '})':
                                depth -= 1
                            if depth == 0 and ch == ',':
                                count += 1
                                has = False
                            elif not ch.isspace():
                                has = True
                        count += 1 if has else 0
                        d = d[:-2] + f'[{count}]'
                    decls.append('extern ' + d + ';\n')
                parts.append(lead + body)
            else:
                parts.append(lead + t)
        out[piece] = ''.join(parts) + pieces[piece][1]
    # A tentative definition (`const T NAME;`, no initializer) whose real
    # definition went to another piece becomes an extern declaration.
    for piece in list(out):
        for m in list(re.finditer(r'^((?:const\s+)?[A-Za-z_]\w*(?:\s*\*)*\s+([A-Za-z_]\w*))\s*;\s*$', out[piece], re.M)):
            name = m.group(2)
            if any(re.search(r'^[^\n;]*\b' + name + r'\s*=\s*\{', t, re.M) for o, t in out.items() if o != piece):
                out[piece] = out[piece].replace(m.group(0), 'extern ' + m.group(1) + ';', 1)
    # A shared name without the model's prefix gets it, so it cannot clash with
    # another global (a test bench's helper, another model's).
    renames = {nm: prefix + nm for nm in shared_defs if prefix and not nm.startswith(prefix)}
    if renames:
        out = {p: code_replace(t, renames) for p, t in out.items()}
        decls = [code_replace(d, renames) for d in decls]
    guard = re.sub(r'\W', '_', header).upper()
    h = (f'/* {header}: what the pieces of {path} share (plan-68-I/J split it). */\n'
         f'#ifndef {guard}\n#define {guard}\n\n' + ''.join(l + t for l, t in header_parts)
         + '\n' + ''.join(dict.fromkeys(decls)) + '\n#endif\n')
    open(S + header, 'w').write(h)
    for piece, text in out.items():
        intro = '' if piece == path else f'/* {piece}: part of the model in {path} (split by plan-68-I/J). */\n'
        open(S + piece, 'w').write(intro + f'#include "{header.split("/")[-1]}"\n' + text)
    return sorted(out)


if __name__ == '__main__':
    import json
    spec = json.loads(sys.argv[1])
    print(split(*spec))
