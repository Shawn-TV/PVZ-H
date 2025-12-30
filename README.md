# PVZ Maze Edition

植物大战僵尸迷宫版 - 扮演僵尸逃离迷宫的冒险游戏

## 项目结构

```
PVZ-H/
├── backend/                    # C++ 游戏后端
│   ├── include/
│   │   ├── ai/                 # A*寻路算法
│   │   ├── core/               # 游戏核心 (Game, Config)
│   │   ├── entities/           # 实体类 (Zombie, Dave, Plant, Item, Projectile)
│   │   ├── maze/               # 迷宫生成
│   │   ├── network/            # 游戏状态序列化
│   │   ├── physics/            # 碰撞检测、向量运算
│   │   ├── plants/             # 植物类型 (PeaShooter, CherryBomb, WallNut)
│   │   └── utils/              # 工具类 (Animation, Timer, Random)
│   ├── src/                    # 源文件实现
│   ├── main.cpp                # 程序入口
│   └── CMakeLists.txt          # CMake 构建配置
│
├── frontend/                   # React + Phaser 前端
│   ├── src/
│   │   ├── components/         # React 组件
│   │   │   ├── GameContainer.tsx   # 游戏容器
│   │   │   └── LoginScreen.tsx     # 主菜单
│   │   ├── scenes/
│   │   │   └── GameScene.js    # Phaser 游戏场景
│   │   ├── network/
│   │   │   ├── client.js       # WebSocket 客户端
│   │   │   └── electronClient.js   # Electron IPC 客户端
│   │   └── utils/
│   │       └── InterpolationManager.js  # 动画插值
│   ├── electron/               # Electron 桌面应用
│   │   ├── main.cjs            # 主进程
│   │   └── preload.cjs         # 预加载脚本
│   ├── public/assets/          # 游戏资源 (图片、音效)
│   └── package.json
│
└── server/                     # Node.js 桥接服务器
    └── bridge.js               # WebSocket 桥接 (浏览器模式)
```

## 编译与运行

### 方式一：浏览器模式

**1. 编译后端**
```bash
cd backend
mkdir -p build && cd build
cmake ..
make
```

**2. 启动桥接服务器**
```bash
cd server
npm install
node bridge.js
```

**3. 启动前端开发服务器**
```bash
cd frontend
npm install
npm run dev
```

**4. 打开浏览器访问** `http://localhost:5173`

### 方式二：Electron 桌面应用

**1. 编译后端**（同上）

**2. 启动 Electron 应用**
```bash
cd frontend
npm install
npm run electron:dev
```

### 方式三：生产构建

```bash
# 后端
cd backend/build && cmake .. && make

# 前端
cd frontend && npm run build

# Electron 打包
cd frontend && npm run electron:build
```

## 游戏操作

| 按键 | 功能 |
|------|------|
| WASD / 方向键 | 移动僵尸 |
| C | 撑杆跳（需装备撑杆） |
| M | 小地图（僵尸视角） |
| Tab | 小地图（戴夫视角，多人模式） |
| ESC | 暂停游戏 |

### 多人模式额外按键
| 按键 | 功能 |
|------|------|
| IJKL | 移动戴夫 |
| Q | 打开/关闭种植菜单 |
| 1-4 | 选择植物 |
| 鼠标点击 | 种植植物 |

## 技术栈

- **后端**: C++17, CMake
- **前端**: React, TypeScript, Phaser 3, Tailwind CSS
- **通信**: WebSocket (浏览器) / IPC (Electron)
- **桌面**: Electron

## 许可证

MIT License
