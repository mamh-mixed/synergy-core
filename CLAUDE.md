# AI instructions

## Working style

- **Never configure or build unless explicitly asked.** No `cmake`, `cmake --build`, `make`, `ninja`, etc. After code changes, stop at the edit and ask the user to build manually.

## GitHub Actions

- `matrix` context is NOT available in job-level `if` — the `if` runs before the matrix is expanded. Use step-level `if` conditions instead (e.g. set a `skip` output in a setup step and check it per-step).

## Code style

- Default to **no comments**. Only add one when the WHY is non-obvious — hidden constraints, subtle invariants, workarounds for specific bugs, behavior that would surprise a reader. Never add comments that explain WHAT or HOW (well-named identifiers do that).
- Don't reference the current task, fix, ticket, or CVE in comments. That belongs in the commit message.

## Working with commits

- **Never commit, amend, or rewrite history.** The user reviews staged changes and commits themselves. No `git commit`, no `git commit --amend`, no `git rebase`, no force-pushes.
- When cherry-picking, use `git cherry-pick --no-commit` so changes land in the index/working tree without producing a commit. Halt after conflict resolution and let the user commit.

## Branches

- **`master`** — stable 1.21.x line. Cherry-picks critical fixes from upstream Deskflow. Conservative.
- **`beta`** — re-forked from `deskflow/master` in 2026-05. Catches the fork up to upstream and re-establishes a clean base. Will replace master after months of stabilization.

The `deskflow` git remote points at [deskflow/deskflow](https://github.com/deskflow/deskflow). Reference upstream as `deskflow/master`.

## Backporting from upstream Deskflow

Mechanics depend on which Synergy branch you're on:

- **On `beta`**: paths align with `deskflow/master` since beta was forked from it. `git cherry-pick` should mostly work cleanly. Conflicts are usually genuine (overlapping changes), not path drift.
- **On `master`**: paths often diverge. Files may be at different locations than upstream (e.g. `DaemonApp.cpp` location may differ), and surrounding refactors may not be present. Plan to port hunks manually.

In both cases: backport multi-commit PRs one commit at a time. Apply, surface conflicts, resolve together, wait for the user to review/commit, then move to the next.

## Synergy overlay (`extra/`)

Houses Synergy-only content that doesn't belong upstream: license/activation kernel, branding overrides, Synergy resources. Was a separate `ext/synergy-extra/` submodule until the re-fork; folded into the main repo for sanity (one PR per change, atomic refactors, no submodule init dance).

The directory structure under `extra/` mirrors the root: `extra/src/lib/synergy/` parallels `src/lib/`, `extra/src/unittests/` parallels `src/unittests/`, `extra/deploy/` parallels `deploy/`, `extra/cmake/` parallels `cmake/`. This makes backport hygiene mechanical (anything under `extra/` is Synergy-only) and re-fork mapping positional (`extra/X/Y` ↔ `X/Y`).

- Same license as the rest of the repo (GPLv2). Synergy is open source; commercial value is server-side activation infrastructure, signed builds, support, and brand — not closed binaries. Code linked into Synergy binaries must be GPLv2-compatible.
- Branding is driven by `extra/cmake/Synergy.cmake` overriding the `CMAKE_PROJECT_*` family (`CMAKE_PROJECT_PROPER_NAME`, `CMAKE_PROJECT_VENDOR`, `CMAKE_PROJECT_COPYRIGHT`, `CMAKE_PROJECT_CONTACT`, `CMAKE_PROJECT_REV_FQDN`, `CMAKE_PROJECT_HOMEPAGE_URL`) *after* deskflow's defaults are set. The old `DESKFLOW_APP_ID` / `DESKFLOW_GUI_HOOK_*` mechanism is gone.
- Binary names (`synergy-core`, `synergy-daemon`, `synergy`) come from `OUTPUT_NAME` overrides in `src/apps/*/CMakeLists.txt`. CMake target identifiers stay deskflow-flavored internally to keep file-path references aligned with upstream.

## Planned

- **Hook modernization**: replace the macro-string-based `DESKFLOW_GUI_HOOK_*` hook scheme with a `synergy::extra` namespace. Real C++ function calls from patched src/ files. Single macro guard `SYNERGY_EXTRA_HEADER` for the include. Adds Core-side hook callsites for license enforcement.
