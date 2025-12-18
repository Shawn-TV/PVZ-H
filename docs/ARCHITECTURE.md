# PVZ(H) 架构设计文档

## 系统架构

### 总体架构

```
┌─────────────────────────────────────────┐
│            前端 (Browser)                │
│  ┌─────────────────────────────────┐   │
│  │  游戏渲染引擎 (Canvas 2D)        │   │
│  │  - 迷宫渲染                      │   │
│  │  - 实体渲染                      │   │
│  │  - 摄像机系统                    │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │  UI系统                          │   │
│  │  - HUD (生命值、道具栏)          │   │
│  │  - 菜单界面                      │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │  输入处理                        │   │
│  │  - 键盘/鼠标监听                 │   │
│  └─────────────────────────────────┘   │
└──────────────┬──────────────────────────┘
               │ WebSocket
               │ (JSON消息)
               │
┌──────────────▼──────────────────────────┐
│          后端 (C++ Server)               │
│  ┌─────────────────────────────────┐   │
│  │  游戏核心                        │   │
│  │  - 游戏循环                      │   │
│  │  - 状态管理                      │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │  迷宫系统                        │   │
│  │  - 迷宫生成算法                  │   │
│  │  - 地图数据                      │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │  实体管理                        │   │
│  │  - 僵尸 (玩家控制)               │   │
│  │  - 植物 (静态/攻击)              │   │
│  │  - 戴夫 (AI控制)                 │   │
│  │  - 道具                          │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │  AI系统                          │   │
│  │  - A*寻路算法                    │   │
│  │  - 路径规划                      │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │  物理引擎                        │   │
│  │  - 碰撞检测                      │   │
│  │  - 移动计算                      │   │
│  └─────────────────────────────────┘   │
└─────────────────────────────────────────┘
```

## 核心模块详解

### 1. 游戏核心 (Core)

#### Game 类
- 负责整个游戏的生命周期管理
- 运行游戏主循环（固定60 tick/秒）
- 协调各个子系统

```cpp
Game::run() {
    while (running) {
        float deltaTime = timer.getDeltaTime();

        // 处理输入
        processInput();

        // 更新游戏逻辑
        update(deltaTime);

        // 发送状态到客户端
        broadcastState();

        // 帧率控制
        limitFrameRate();
    }
}
```

#### GameState 类
- 跟踪游戏状态（菜单、游戏中、暂停、结束）
- 管理游戏数据（分数、时间、玩家生命值）

### 2. 迷宫系统 (Maze)

#### 迷宫生成算法

**深度优先搜索 (DFS)**
1. 从随机单元格开始
2. 随机选择未访问的邻居
3. 打通两个单元格之间的墙
4. 递归访问邻居
5. 回溯到有未访问邻居的单元格

**Prim算法**
1. 从随机单元格开始
2. 将邻居加入列表
3. 从列表中随机选择一个单元格
4. 打通到已访问单元格的墙
5. 重复直到所有单元格被访问

#### 迷宫数据结构

```cpp
class Maze {
    vector<vector<Cell>> cells;  // 2D网格
    Vector2D entrance;           // 入口位置
    Vector2D exit;               // 出口位置
    vector<Plant*> plants;       // 植物列表
    vector<Item*> items;         // 道具列表
};
```

### 3. 实体系统 (Entities)

#### 实体继承层次

```
Entity (抽象基类)
├── Zombie (玩家控制)
├── Plant (植物基类)
│   ├── PeaShooter (豌豆射手)
│   ├── CherryBomb (樱桃炸弹)
│   └── WallNut (坚果墙)
├── Dave (NPC)
└── Item (道具基类)
    ├── Bucket (铁桶)
    └── HealthPotion (生命药水)
```

#### 更新循环

```cpp
void Game::update(float deltaTime) {
    // 更新玩家
    zombie->update(deltaTime);

    // 更新植物（攻击逻辑）
    for (auto plant : plants) {
        plant->update(deltaTime);
    }

    // 更新戴夫（AI移动和攻击）
    dave->update(deltaTime);

    // 检测碰撞
    checkCollisions();

    // 移除死亡实体
    removeDeadEntities();
}
```

### 4. AI系统 (A* 寻路)

#### A*算法实现

```
1. 初始化：
   - openList = [起点]
   - closedList = []

2. 循环直到找到终点或openList为空：
   - 从openList中取出f值最小的节点current
   - 如果current是终点，重构路径并返回
   - 将current加入closedList

   - 对于current的每个邻居neighbor：
     - 如果neighbor在closedList中，跳过
     - 计算新的g值 = current.g + 移动代价
     - 如果neighbor不在openList中或新g值更小：
       - 更新neighbor的g、h、f值
       - 设置neighbor的父节点为current
       - 将neighbor加入openList

3. 如果openList为空，返回空路径（无解）
```

#### 启发式函数（曼哈顿距离）

```cpp
float heuristic(Vector2D a, Vector2D b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}
```

### 5. 物理系统 (Physics)

#### 碰撞检测

**AABB (Axis-Aligned Bounding Box)**
```cpp
bool checkAABB(Rect a, Rect b) {
    return a.x < b.x + b.width &&
           a.x + a.width > b.x &&
           a.y < b.y + b.height &&
           a.y + a.height > b.y;
}
```

**圆形碰撞**
```cpp
bool checkCircle(Circle a, Circle b) {
    float distance = (a.center - b.center).length();
    return distance < (a.radius + b.radius);
}
```

### 6. 网络通信 (Network)

#### 通信协议（JSON）

**客户端 -> 服务器**
```json
{
    "type": "PLAYER_MOVE",
    "data": {
        "direction": "up|down|left|right"
    }
}

{
    "type": "PLAYER_ACTION",
    "data": {
        "action": "use_item",
        "slot": 0
    }
}
```

**服务器 -> 客户端**
```json
{
    "type": "GAME_STATE_UPDATE",
    "data": {
        "zombie": {
            "x": 100,
            "y": 150,
            "health": 80
        },
        "plants": [...],
        "dave": {...},
        "items": [...]
    }
}
```

### 7. 前端渲染系统

#### 摄像机跟随

```javascript
camera.update() {
    // 目标位置：僵尸位置 - 半个屏幕尺寸
    targetX = zombie.x - viewWidth / 2;
    targetY = zombie.y - viewHeight / 2;

    // 平滑插值
    camera.x += (targetX - camera.x) * smoothFactor;
    camera.y += (targetY - camera.y) * smoothFactor;

    // 限制边界
    camera.x = clamp(camera.x, 0, mazeWidth - viewWidth);
    camera.y = clamp(camera.y, 0, mazeHeight - viewHeight);
}
```

#### 渲染流程

```javascript
render() {
    // 1. 清空画布
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // 2. 应用摄像机变换
    ctx.save();
    ctx.translate(-camera.x, -camera.y);

    // 3. 渲染迷宫（只渲染可见部分）
    renderVisibleMaze();

    // 4. 渲染实体
    renderEntities();

    // 5. 恢复变换
    ctx.restore();

    // 6. 渲染HUD（固定在屏幕上）
    renderHUD();
}
```

## 数据流

### 游戏循环数据流

```
用户输入 -> 前端输入处理 -> WebSocket -> 后端接收
                                            ↓
                                      更新游戏状态
                                            ↓
                                    实体更新、AI计算
                                            ↓
                                      碰撞检测
                                            ↓
                                    序列化游戏状态
                                            ↓
前端渲染 <- 前端状态更新 <- WebSocket <- 后端发送
```

## 性能优化考虑

### 后端优化
1. **空间分区**：使用网格将迷宫划分，加速碰撞检测和实体查询
2. **状态差异更新**：只发送变化的实体数据，减少网络带宽
3. **路径缓存**：缓存戴夫的路径，只在必要时重新计算

### 前端优化
1. **视锥剔除**：只渲染摄像机可见范围内的实体
2. **精灵图集**：使用图集减少HTTP请求
3. **对象池**：重用对象，减少GC压力
4. **请求动画帧**：使用requestAnimationFrame优化渲染

## 扩展性设计

### 添加新植物
1. 继承Plant基类
2. 实现update()方法（攻击逻辑）
3. 在MazeGenerator中注册
4. 添加前端精灵资源

### 添加新道具
1. 继承Item基类
2. 实现use()方法（效果逻辑）
3. 在道具生成器中注册
4. 添加前端图标和效果

### 添加新游戏模式
1. 创建新的GameState子类
2. 实现特定的游戏规则
3. 修改Game类支持模式切换
