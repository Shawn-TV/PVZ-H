# Contributing

Thanks for helping improve PVZ Maze Edition.

## Development Flow

1. Create a feature branch from the stable default branch.
2. Keep gameplay changes, frontend UI changes, and packaging changes in
   separate commits when practical.
3. Build the C++ backend before testing the browser or Electron clients.
4. Run the relevant frontend checks before opening a pull request.
5. Do not commit generated folders or release binaries.

## Local Checks

Backend:

```bash
cd backend
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Frontend:

```bash
cd frontend
npm install
npm run typecheck
npm run build
```

Bridge server:

```bash
cd server
npm install
npm start
```

## Release Assets

Desktop binaries should be uploaded to GitHub Releases. Keep source history
focused on code, assets, docs, and lockfiles.
