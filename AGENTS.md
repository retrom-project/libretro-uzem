# Retrom Uzem fork

`master` mirrors upstream `libretro/libretro-uzem` without Retrom patches.
`retrom/gd991ee94547c` is the maintenance/default branch. Work on `feat/*`,
`fix/*`, `build/*` or `sync/upstream-*` branches from that maintenance baseline.
Never put integration changes into the upstream mirror.

Read `docs/MAINTENANCE.md` and `retrom-fork.json` before changing build inputs.
Core implementation, native regression checks, web builds and source archives
belong here. retrom-runtime only consumes verified artifacts and public ABI facts.
Do not add games or generated binaries to Git.

Before changing behavior, add a regression which fails on the previous behavior.
Run `.github/rpg-runtime/test-native.sh`; use its optional local ROM argument for
an explicitly supplied game. Build candidate bytes through Retrom `pfb-core-build`.
A fresh Chrome instance must restore instant checkpoints and accept standard
controller input through the actual Retrom product before release acceptance.

Future immutable tags use `retrom-core-gd991ee94547c-rN` (or `-rc.N` for prerelease).
Do not merge, publish or move tags without authorization. Production aggregation
must never point to a dirty worktree or an unpublished candidate as a release.
