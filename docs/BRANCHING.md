# Branch Cleanup Plan

The repository currently has several one-off experiment branches. The clean
long-term shape should be:

- `main`: stable default branch for the current playable project.
- `release/v1.0.0`: optional branch or tag for the packaged Windows build.
- Short-lived feature branches for future work.

## Recommended Consolidation

Use `claude/add-dave-animation-4Ck4r` as the source branch for the first stable
`main` branch. It contains the most complete game client, Electron packaging,
asset set, and packaging fixes.

After `main` is created and verified, the following remote branches can be
archived or deleted from GitHub:

- `claude/pvz-maze-game-setup-bJlKN`
- `claude/improve-animation-clarity-kLuYs`
- `claude/fix-game-graphics-controls-bMnap`
- `claude/add-game-login-interface-OjwFJ`
- `claude/add-dave-animation-4Ck4r`

Do this only after confirming that `main` contains the desired final state and
any release tags point at the correct commit.

## Suggested Commands

Create or update a clean default branch:

```bash
git fetch origin
git checkout cleanup/project-polish
git checkout -B main
git push -u origin main
```

Set `main` as the default branch in GitHub repository settings.

Delete old remote experiment branches after verification:

```bash
git push origin --delete claude/pvz-maze-game-setup-bJlKN
git push origin --delete claude/improve-animation-clarity-kLuYs
git push origin --delete claude/fix-game-graphics-controls-bMnap
git push origin --delete claude/add-game-login-interface-OjwFJ
git push origin --delete claude/add-dave-animation-4Ck4r
```

This cleanup branch does not delete remote branches by itself.
