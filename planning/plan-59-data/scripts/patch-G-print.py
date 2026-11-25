#!/usr/bin/env python3
"""A4: the sandbox printing entitlement (com.apple.security.print), absent at the base commit."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from patchlib import Patch
p = Patch("/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G/schematical")
p.sub("schematical.xcodeproj/project.pbxproj", "\t\t\t\tENABLE_OUTGOING_NETWORK_CONNECTIONS = YES;\n",
      "\t\t\t\tENABLE_OUTGOING_NETWORK_CONNECTIONS = YES;\n\t\t\t\tENABLE_RESOURCE_ACCESS_PRINTING = YES;\n", count=2)
p.done()
