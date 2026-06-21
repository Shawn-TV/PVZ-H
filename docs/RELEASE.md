# Release Notes

Release binaries and screenshots should be distributed through GitHub Releases,
not committed to the repository.

## GitHub Release

- Tag: `v1.0.0`
- Title: `PVZ Maze Edition v1.0.0`

## Binary Assets

| Platform | GitHub asset | Format | Size | SHA-256 |
| --- | --- | --- | --- | --- |
| Windows | `PVZ-Maze-Edition-Windows.exe` | Windows PE32 GUI executable, Nullsoft Installer self-extracting archive | 153,819,264 bytes / 146.69 MiB | `fb8f9452ad71a54fe567a93fae0d1261b46d4d9774cd7af31061b456f714aaea` |
| macOS Apple Silicon | `PVZ-Maze-Edition-macOS-arm64.dmg` | Unsigned macOS DMG with bundled arm64 backend | 199,452,995 bytes / 190.21 MiB | `992844f646bda975b2f80914d8e654982cbc1b018aa4bfdcb887914284caf7f5` |

The macOS build is unsigned. On a fresh machine, users may need to open it via
System Settings > Privacy & Security or by control-clicking the app and choosing
Open.

## Screenshot Assets

| Asset | Description |
| --- | --- |
| `screenshot-single-player.png` | Single-player maze gameplay |
| `screenshot-minimap.png` | Single-player minimap overlay |
| `screenshot-multiplayer.png` | Local split-screen multiplayer |

## Suggested Release Description

```markdown
PVZ Maze Edition v1.0.0 is the first packaged release.

Downloads:
- Windows: PVZ-Maze-Edition-Windows.exe
- macOS Apple Silicon: PVZ-Maze-Edition-macOS-arm64.dmg

Highlights:
- Play as the zombie and escape a generated maze.
- Avoid Dave, plant attacks, and maze hazards.
- Use pickups such as bucket armor, health potion, speed potion, and pole vault kit.
- Play single-player or local split-screen multiplayer.

Checksums:
- Windows SHA-256: fb8f9452ad71a54fe567a93fae0d1261b46d4d9774cd7af31061b456f714aaea
- macOS SHA-256: 992844f646bda975b2f80914d8e654982cbc1b018aa4bfdcb887914284caf7f5

Note:
- The macOS build is unsigned. Use Control-click > Open or approve it in
  System Settings > Privacy & Security if Gatekeeper blocks first launch.
```

## Release Checklist

1. Build the backend in release mode.
2. Build the frontend.
3. Package the Electron app for the target OS.
4. Run the packaged app locally.
5. Mount or launch the packaged artifact to verify it starts.
6. Upload binaries and screenshots to GitHub Releases.
7. Include SHA-256 checksums in the release notes.
