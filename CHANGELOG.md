# Changelog

All notable project-level changes are tracked here.

## Unreleased

- Reworked the README into a structured English-first, Chinese-second project
  overview.
- Added release documentation for the packaged Windows build.
- Added release screenshots to the README project homepage.
- Added repository files: `LICENSE` and this changelog.
- Cleaned up minor C++ warnings and completed serialization for Repeater and
  Speed Potion entities.
- Added macOS packaging resources and platform-specific Electron release
  resources.
- Removed vendored `server/node_modules/` files from version control.
- Removed long-term project-maintenance notes because this repository is a
  course project snapshot.
- Expanded `.gitignore` for generated desktop packages and dependency folders.

## 1.0.0

- Packaged Windows release prepared as `PVZ Maze Edition.exe`.
- Packaged macOS Apple Silicon release prepared as
  `PVZ-Maze-Edition-macOS-arm64.dmg`.
- Includes single-player zombie maze escape and local split-screen multiplayer.
- Includes React/Phaser frontend, C++17 backend, WebSocket browser bridge, and
  Electron desktop shell.
