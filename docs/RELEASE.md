# Release Notes

## Windows Release: PVZ Maze Edition.exe

The current packaged release build should be distributed through GitHub
Releases, not committed to the repository.

| Field | Value |
| --- | --- |
| Local file name | `PVZ Maze Edition.exe` |
| GitHub download name | `PVZ.Maze.Edition.exe` |
| Format | Windows PE32 GUI executable, Nullsoft Installer self-extracting archive |
| Size | 153,819,264 bytes / 146.69 MiB |
| SHA-256 | `fb8f9452ad71a54fe567a93fae0d1261b46d4d9774cd7af31061b456f714aaea` |

## Suggested GitHub Release

- Tag: `v1.0.0`
- Title: `PVZ Maze Edition v1.0.0`
- Asset: `PVZ Maze Edition.exe`

Suggested description:

```markdown
PVZ Maze Edition v1.0.0 is the first packaged Windows build.

Highlights:
- Play as the zombie and escape a generated maze.
- Avoid Dave, plant attacks, and maze hazards.
- Use pickups such as bucket armor, health potion, speed potion, and pole vault kit.
- Play single-player or local split-screen multiplayer.

Asset:
- PVZ Maze Edition.exe
- SHA-256: fb8f9452ad71a54fe567a93fae0d1261b46d4d9774cd7af31061b456f714aaea
```

## Release Checklist

1. Build the backend in release mode.
2. Build the frontend.
3. Package the Electron app for the target OS.
4. Run the packaged app locally.
5. Upload the binary to GitHub Releases.
6. Include the SHA-256 checksum in the release notes.
