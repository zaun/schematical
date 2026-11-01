# Agent Guidelines

## Git

- **NEVER create a branch. NEVER create a worktree.** No exceptions, no
  "temporary" branches, no feature branches, no worktree isolation for
  subagents. The only branch in this repo is `main`.
- All work and all commits go directly on `main`.
- If you find the repo on any branch other than `main`, stop and tell the
  user before doing any work. Do not commit to it.
- Do not run `git checkout -b`, `git switch -c`, `git branch <name>`,
  `git worktree add`, or anything else that creates a branch or worktree.
