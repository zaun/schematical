"""Exact-text patching for the spike worktrees: every replacement must match exactly once."""
import sys

class Patch:
    def __init__(self, root):
        self.root = root.rstrip("/")
        self.count = 0

    def sub(self, rel, old, new, count=1):
        path = f"{self.root}/{rel}"
        text = open(path).read()
        found = text.count(old)
        if found != count:
            sys.exit(f"{rel}: expected {count} match(es), found {found}:\n{old[:200]}")
        open(path, "w").write(text.replace(old, new))
        self.count += 1

    def write(self, rel, text):
        open(f"{self.root}/{rel}", "w").write(text)
        self.count += 1

    def done(self):
        print(f"{self.count} edits applied in {self.root}")
