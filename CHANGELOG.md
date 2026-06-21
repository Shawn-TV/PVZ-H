# Changelog

All notable project-level changes are tracked here.

## Unreleased

- Reworked the README into a structured English-first, Chinese-second project
  overview.
- Added release documentation for the packaged Windows build.
- Added branch cleanup guidance for consolidating the experimental Claude
  branches into a stable default branch.
- Added repository maintenance files: `LICENSE`, `CONTRIBUTING.md`, and this
  changelog.
- Cleaned up minor C++ warnings and completed serialization for Repeater and
  Speed Potion entities.
- Removed vendored `server/node_modules/` files from version control.
- Expanded `.gitignore` for generated desktop packages and dependency folders.

## 1.0.0

- Packaged Windows release prepared as `PVZ Maze Edition.exe`.
- Includes single-player zombie maze escape and local split-screen multiplayer.
- Includes React/Phaser frontend, C++17 backend, WebSocket browser bridge, and
  Electron desktop shell.
