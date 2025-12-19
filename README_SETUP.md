# PVZ Maze Game - Setup and Run Instructions

## 游戏架构

This game consists of three parts:
1. **C++ Backend** - Game logic and physics engine (in `backend/`)
2. **Node.js WebSocket Bridge** - Connects backend to frontend (in `server/`)
3. **Phaser Frontend** - Web-based game rendering (in `frontend/`)

## Requirements

- C++ compiler (g++ with C++17 support)
- CMake 3.10+
- Node.js 14+
- npm

## Quick Start

### Step 1: Build the C++ Backend

```bash
cd backend/build
cmake ..
make
```

This will create the `pvz_game` executable.

### Step 2: Start the WebSocket Bridge Server

Open a new terminal:

```bash
cd server
npm install  # If not already done
node bridge.js
```

You should see:
```
WebSocket服务器启动在端口 8080
启动游戏进程: ../backend/build/pvz_game
```

### Step 3: Start the Frontend Development Server

Open another terminal:

```bash
cd frontend
npm install  # If not already done
npm run dev
```

Vite will start and show you a localhost URL (usually http://localhost:5173)

### Step 4: Play the Game!

1. Open your browser to the Vite URL (http://localhost:5173)
2. The game will automatically connect to the WebSocket server
3. You'll see the maze with your zombie character

## Controls

- **WASD** or **Arrow Keys**: Move zombie
- **Mouse Left Click** or **Space**: Attack plants/Dave
- **Ctrl**: Pole vault jump (when equipped with pole vault kit)

## Game Objective

- Control the zombie from the entrance (green circle) to the exit (red circle)
- Avoid or fight Dave (blue circle)
- Collect items (yellow):
  - 🪣 Bucket: Adds armor
  - 🦘 Pole Vault Kit: Increases speed and enables jumping
  - 💊 Health Potion: Restores health
- **Win**: Reach the exit
- **Lose**: Zombie health reaches 0

## Architecture Details

### Backend (C++)
- Runs at 60 FPS fixed timestep
- Outputs JSON game state to stdout every 33ms (30 FPS for network)
- Receives input commands via stdin
- Features:
  - Maze generation with recursive backtracking
  - A* pathfinding for Dave AI
  - AABB collision detection
  - Equipment system (bucket armor, pole vault)
  - Animation system

### Bridge Server (Node.js)
- Spawns the C++ game process
- Reads JSON from game stdout
- Broadcasts to all WebSocket clients
- Forwards client input to game stdin

### Frontend (Phaser)
- Connects to WebSocket server
- Receives game state updates
- Renders maze, entities, health bars
- Camera follows zombie with smooth interpolation
- Sends input to server

## Troubleshooting

### "Cannot connect to server"
- Make sure the bridge server is running (`node bridge.js` in server/ directory)
- Check that port 8080 is not in use

### Backend crashes immediately
- Check that the executable was built successfully
- Try running `./pvz_game` directly in backend/build/ to see error messages

### Frontend shows black screen
- Open browser console (F12) to check for errors
- Ensure WebSocket connection is established

### Zombie health starts at 100 instead of 200
- Config has been updated to 200
- Rebuild the backend: `cd backend/build && make`

## Development

### Rebuilding Backend
```bash
cd backend/build
make
# If major changes, clean rebuild:
rm -rf * && cmake .. && make
```

### Restarting Servers
- Stop bridge server: Ctrl+C in its terminal
- Stop frontend: Ctrl+C in its terminal
- Restart both to pick up changes

## Network Protocol

### Messages from Backend to Frontend:

**MAZE_DATA** (once at start):
```json
{
  "type": "MAZE_DATA",
  "data": {
    "gridWidth": 21,
    "gridHeight": 31,
    "cellSize": 80,
    "entrance": {"x": 120, "y": 120},
    "exit": {"x": 1560, "y": 2360},
    "walls": [{"x": 0, "y": 0}, ...]
  }
}
```

**ENTITIES** (30 FPS):
```json
{
  "type": "ENTITIES",
  "data": [
    {
      "id": 1,
      "type": "zombie",
      "x": 120.5,
      "y": 200.3,
      "health": 180,
      "maxHealth": 200,
      "form": "normal",
      "armor": 0
    },
    ...
  ]
}
```

**GAME_STATE** (30 FPS):
```json
{
  "type": "GAME_STATE",
  "data": {
    "status": "playing",  // "menu", "playing", "paused", "win", "lose"
    "timestamp": 1234567890
  }
}
```

### Messages from Frontend to Backend:

- `MOVE_UP` / `MOVE_DOWN` / `MOVE_LEFT` / `MOVE_RIGHT`
- `STOP_MOVE`
- `ATTACK`
- `POLE_VAULT`

## Credits

- Game Design: Plants vs Zombies inspired
- Engine: C++ backend, Phaser 3 frontend
- Network: WebSocket with Node.js bridge
