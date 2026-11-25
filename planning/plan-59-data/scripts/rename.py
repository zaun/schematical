#!/usr/bin/env python3
"""Rename the app target of a spike worktree: product name, bundle id, ad-hoc signing."""
import sys, re
path, bundle = sys.argv[1], sys.argv[2]
text = open(path).read()
blocks = 0
def fix(match):
    global blocks
    block = match.group(0)
    if "PRODUCT_BUNDLE_IDENTIFIER = jgz.schematical;" not in block:
        return block
    blocks += 1
    block = block.replace("PRODUCT_BUNDLE_IDENTIFIER = jgz.schematical;", f"PRODUCT_BUNDLE_IDENTIFIER = {bundle};")
    block = block.replace('PRODUCT_NAME = "$(TARGET_NAME)";',
                          'PRODUCT_MODULE_NAME = schematical;\n\t\t\t\tPRODUCT_NAME = SchematicalSpike59;')
    block = block.replace("CODE_SIGN_STYLE = Automatic;", 'CODE_SIGN_IDENTITY = "-";\n\t\t\t\tCODE_SIGN_STYLE = Manual;')
    block = block.replace("DEVELOPMENT_TEAM = 8BU2Y8C5J7;", 'DEVELOPMENT_TEAM = "";')
    return block
text = re.sub(r"/\* (Debug|Release) \*/ = \{.*?\n\t\t\};", fix, text, flags=re.S)
assert blocks == 2, blocks
open(path, "w").write(text)
print("edited", blocks, "configurations")
