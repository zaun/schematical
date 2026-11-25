"""Exact-text patching for the spike worktrees: every replacement must match exactly once."""
import sys, re

class Patch:
    def __init__(self, root):
        self.root = root.rstrip("/")
        self.count = 0

    def read(self, rel):
        return open(f"{self.root}/{rel}").read()

    def sub(self, rel, old, new, count=1):
        path = f"{self.root}/{rel}"
        text = open(path).read()
        found = text.count(old)
        if found != count:
            sys.exit(f"{rel}: expected {count} match(es), found {found}:\n{old[:200]}")
        open(path, "w").write(text.replace(old, new))
        self.count += 1

    def delete_block(self, rel, signature):
        """Deletes the declaration whose first line contains `signature`: its leading
        doc comments and attributes, through the closing brace at its own indentation
        (or just the line, for a one-line declaration), and one blank line after it."""
        path = f"{self.root}/{rel}"
        lines = open(path).read().split("\n")
        hits = [i for i, line in enumerate(lines) if signature in line]
        if len(hits) != 1:
            sys.exit(f"{rel}: expected 1 line with {signature!r}, found {len(hits)}")
        start = end = hits[0]
        indent = lines[start][: len(lines[start]) - len(lines[start].lstrip())]
        if lines[start].rstrip().endswith("{") or lines[start].rstrip().endswith(" in"):
            while lines[end] != indent + "}":
                end += 1
        while start > 0 and (lines[start - 1].startswith(indent + "///") or lines[start - 1].startswith(indent + "@")
                             or lines[start - 1].startswith(indent + "// ")):
            start -= 1
        if end + 1 < len(lines) and lines[end + 1].strip() == "":
            end += 1
        del lines[start : end + 1]
        open(path, "w").write("\n".join(lines))
        self.count += 1

    def rename(self, rels, old, new):
        total = 0
        for rel in rels:
            path = f"{self.root}/{rel}"
            text = open(path).read()
            n = len(re.findall(rf"\b{old}\b", text))
            if n:
                open(path, "w").write(re.sub(rf"\b{old}\b", new, text))
                total += n
        self.count += 1
        return total

    def write(self, rel, text):
        open(f"{self.root}/{rel}", "w").write(text)
        self.count += 1

    def done(self):
        print(f"{self.count} edits applied in {self.root}")
