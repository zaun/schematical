"""Split a C test file into shared benches (with generated headers) and test
programs. Top-level items are found by brace matching.

Used by plan-68-G/H. A bench region's functions and data lose `static` and get
declarations in the bench's header; its types, macros and includes move to the
header whole.
"""
import re


def items(text):
    """Top-level items of C source: (kind, text), kinds 'pp', 'blank',
    'comment', 'code'. A code item runs to its ';' at depth 0, or to the end
    of the line holding the '}' that closes its outermost brace."""
    out = []
    i, n = 0, len(text)
    while i < n:
        if text[i] == '\n':
            out.append(('blank', '\n'))
            i += 1
            continue
        line_end = text.find('\n', i)
        line_end = n if line_end < 0 else line_end
        line = text[i:line_end]
        stripped = line.lstrip()
        if stripped.startswith('#'):
            e = line_end
            while text[i:e].rstrip().endswith('\\') or (e < n and text[e - 1:e] == '\\'):
                e2 = text.find('\n', e + 1)
                e = n if e2 < 0 else e2
            # continuation lines
            j = i
            while True:
                e = text.find('\n', j)
                e = n if e < 0 else e
                if text[j:e].rstrip().endswith('\\'):
                    j = e + 1
                    continue
                break
            out.append(('pp', text[i:e + 1]))
            i = e + 1
            continue
        if stripped.startswith('/*'):
            e = text.index('*/', i) + 2
            rest = text.find('\n', e)
            rest = n if rest < 0 else rest
            if text[e:rest].strip() == '':
                out.append(('comment', text[i:rest + 1]))
                i = rest + 1
                continue
        if stripped.startswith('//'):
            out.append(('comment', text[i:line_end + 1]))
            i = line_end + 1
            continue
        j, depth = i, 0
        while j < n:
            if text.startswith('/*', j):
                j = text.index('*/', j) + 2
                continue
            if text.startswith('//', j):
                j = text.find('\n', j)
                continue
            c = text[j]
            if c in '"\'':
                j += 1
                while text[j] != c:
                    j += 2 if text[j] == '\\' else 1
                j += 1
                continue
            if c == '{':
                depth += 1
            elif c == '}':
                depth -= 1
                if depth == 0:
                    e = text.find('\n', j)
                    e = n if e < 0 else e
                    out.append(('code', text[i:e + 1]))
                    i = e + 1
                    break
            elif c == ';' and depth == 0:
                e = text.find('\n', j)
                e = n if e < 0 else e
                out.append(('code', text[i:e + 1]))
                i = e + 1
                break
            j += 1
        else:
            out.append(('code', text[i:]))
            break
    return out


def is_function(code):
    if '{' not in code:
        return False
    head = code.split('{', 1)[0]
    return '(' in head and '=' not in head and not head.lstrip().startswith(('typedef', 'struct', 'enum', 'union'))


def is_type(code):
    return code.lstrip().startswith(('typedef', 'struct ', 'enum ', 'union ')) and not is_function(code)


def bench(text, guard, includes):
    """(bench .c text, bench .h text) from a region of a test file."""
    c_parts, h_parts = [], []
    pending = []
    for kind, t in items(text):
        if kind in ('blank', 'comment'):
            pending.append(t)
            continue
        lead = ''.join(pending)
        pending = []
        if kind == 'pp' or is_type(t):
            h_parts.append(lead + t)
            continue
        body = re.sub(r'^static\s+', '', t)
        if is_function(t):
            h_parts.append(body.split('{', 1)[0].rstrip() + ';\n')
        else:
            decl = body.split('=', 1)[0].rstrip() if '=' in body else body.rstrip()
            decl = decl.rstrip(';').rstrip()
            h_parts.append('extern ' + decl + ';\n')
        c_parts.append(lead + body)
    h = f'#ifndef {guard}\n#define {guard}\n\n' + ''.join(h_parts) + '\n#endif\n'
    c = ''.join(f'#include "{x}"\n' for x in includes) + '\n' + ''.join(c_parts)
    return c, h
