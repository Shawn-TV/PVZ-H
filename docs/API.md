# API 文档

## 网络通信协议

本游戏使用 WebSocket 进行前后端通信，数据格式为 JSON。

## 连接信息

- **协议**: WebSocket (ws://)
- **默认端口**: 8080
- **URL**: `ws://localhost:8080`

## 消息格式

所有消息遵循统一格式：

```json
{
    "type": "MESSAGE_TYPE",
    "data": { /* 消息数据 */ }
}
```

---

## 客户端 -> 服务器 消息

### 1. 玩家移动 (PLAYER_MOVE)

玩家控制僵尸移动。

**消息类型**: `PLAYER_MOVE`

**数据格式**:
```json
{
    "type": "PLAYER_MOVE",
    "data": {
        "direction": "up" | "down" | "left" | "right"
    }
}
```

**字段说明**:
- `direction`: 移动方向
  - `"up"`: 向上
  - `"down"`: 向下
  - `"left"`: 向左
  - `"right"`: 向右

**示例**:
```json
{
    "type": "PLAYER_MOVE",
    "data": {
        "direction": "up"
    }
}
```

---

### 2. 玩家动作 (PLAYER_ACTION)

玩家执行动作（如使用道具）。

**消息类型**: `PLAYER_ACTION`

**数据格式**:
```json
{
    "type": "PLAYER_ACTION",
    "data": {
        "action": "use_item",
        "slot": 0
    }
}
```

**字段说明**:
- `action`: 动作类型
  - `"use_item"`: 使用道具
  - `"pickup_item"`: 拾取道具（自动触发，通常不需要手动发送）
- `slot`: 道具栏位置（0-5）

**示例**:
```json
{
    "type": "PLAYER_ACTION",
    "data": {
        "action": "use_item",
        "slot": 2
    }
}
```

---

### 3. 游戏控制 (GAME_CONTROL)

控制游戏状态（暂停、继续、重启等）。

**消息类型**: `GAME_CONTROL`

**数据格式**:
```json
{
    "type": "GAME_CONTROL",
    "data": {
        "command": "pause" | "resume" | "restart"
    }
}
```

**字段说明**:
- `command`: 控制命令
  - `"pause"`: 暂停游戏
  - `"resume"`: 继续游戏
  - `"restart"`: 重新开始

---

## 服务器 -> 客户端 消息

### 1. 游戏状态更新 (GAME_STATE_UPDATE)

服务器定期发送完整游戏状态（每帧）。

**消息类型**: `GAME_STATE_UPDATE`

**数据格式**:
```json
{
    "type": "GAME_STATE_UPDATE",
    "data": {
        "timestamp": 1234567890,
        "gameStatus": "playing",
        "zombie": {
            "x": 100.5,
            "y": 150.3,
            "health": 80,
            "maxHealth": 100,
            "armor": 30,
            "speed": 2.0,
            "inventory": [
                { "type": "bucket", "slot": 0 },
                { "type": "health_potion", "slot": 1 }
            ]
        },
        "dave": {
            "x": 200.0,
            "y": 180.0,
            "targetX": 100.5,
            "targetY": 150.3
        },
        "plants": [
            {
                "id": 1,
                "type": "peashooter",
                "x": 120,
                "y": 160,
                "direction": "right",
                "cooldown": 1.5
            },
            {
                "id": 2,
                "type": "cherrybomb",
                "x": 180,
                "y": 200,
                "timer": 3.2
            }
        ],
        "items": [
            {
                "id": 10,
                "type": "health_potion",
                "x": 150,
                "y": 170
            }
        ]
    }
}
```

**字段说明**:
- `timestamp`: 服务器时间戳
- `gameStatus`: 游戏状态（"menu", "playing", "paused", "win", "game_over"）
- `zombie`: 僵尸状态
  - `x`, `y`: 位置坐标
  - `health`: 当前生命值
  - `maxHealth`: 最大生命值
  - `armor`: 护甲值
  - `speed`: 当前速度
  - `inventory`: 道具栏
- `dave`: 戴夫状态
  - `x`, `y`: 当前位置
  - `targetX`, `targetY`: 目标位置（追踪点）
- `plants`: 植物数组
  - `id`: 唯一标识符
  - `type`: 植物类型
  - `x`, `y`: 位置
  - 其他字段根据植物类型变化
- `items`: 道具数组

---

### 2. 实体更新 (ENTITY_UPDATE)

增量更新，只发送变化的实体（优化版本）。

**消息类型**: `ENTITY_UPDATE`

**数据格式**:
```json
{
    "type": "ENTITY_UPDATE",
    "data": {
        "updated": [
            {
                "entityType": "zombie",
                "id": 0,
                "changes": {
                    "x": 105.2,
                    "y": 151.0,
                    "health": 75
                }
            }
        ],
        "removed": [
            {
                "entityType": "plant",
                "id": 2
            }
        ],
        "added": [
            {
                "entityType": "item",
                "id": 15,
                "type": "bucket",
                "x": 200,
                "y": 220
            }
        ]
    }
}
```

---

### 3. 迷宫数据 (MAZE_DATA)

游戏开始时发送迷宫结构（仅发送一次）。

**消息类型**: `MAZE_DATA`

**数据格式**:
```json
{
    "type": "MAZE_DATA",
    "data": {
        "width": 25,
        "height": 25,
        "entrance": { "x": 0, "y": 12 },
        "exit": { "x": 24, "y": 12 },
        "cells": [
            [0, 1, 0, 1, ...],  // 0=通道, 1=墙壁
            [0, 0, 0, 0, ...],
            ...
        ]
    }
}
```

**字段说明**:
- `width`, `height`: 迷宫尺寸
- `entrance`: 入口坐标
- `exit`: 出口坐标
- `cells`: 二维数组，0表示通道，1表示墙壁

---

### 4. 游戏事件 (GAME_EVENT)

特殊事件通知（拾取道具、受到攻击等）。

**消息类型**: `GAME_EVENT`

**数据格式**:
```json
{
    "type": "GAME_EVENT",
    "data": {
        "event": "item_pickup",
        "details": {
            "itemType": "health_potion",
            "itemId": 10
        }
    }
}
```

**事件类型**:
- `"item_pickup"`: 拾取道具
- `"take_damage"`: 受到伤害
  ```json
  {
      "event": "take_damage",
      "details": {
          "damage": 10,
          "source": "peashooter",
          "sourceId": 1
      }
  }
  ```
- `"plant_destroyed"`: 植物被摧毁
- `"explosion"`: 爆炸事件（樱桃炸弹）

---

### 5. 游戏结束 (GAME_OVER / GAME_WIN)

游戏结束通知。

**失败消息**:
```json
{
    "type": "GAME_OVER",
    "data": {
        "reason": "death",
        "score": 450,
        "time": 125.5
    }
}
```

**胜利消息**:
```json
{
    "type": "GAME_WIN",
    "data": {
        "score": 1750,
        "time": 98.2,
        "healthRemaining": 65,
        "itemsCollected": 8,
        "bonuses": {
            "speedBonus": 500,
            "healthBonus": 65,
            "noHitBonus": 0
        }
    }
}
```

---

## 数据类型定义

### 位置 (Position)
```typescript
{
    x: number,  // X坐标
    y: number   // Y坐标
}
```

### 实体类型 (EntityType)
```typescript
"zombie" | "dave" | "plant" | "item"
```

### 植物类型 (PlantType)
```typescript
"peashooter" | "cherrybomb" | "wallnut"
```

### 道具类型 (ItemType)
```typescript
"bucket" | "health_potion" | "speed_boost" | "shield"
```

### 游戏状态 (GameStatus)
```typescript
"menu" | "playing" | "paused" | "win" | "game_over"
```

---

## 通信时序

### 游戏开始流程

```
Client                          Server
  |                               |
  |------- Connect WS ----------->|
  |                               |
  |<------ MAZE_DATA -------------|  // 发送迷宫数据
  |                               |
  |<--- GAME_STATE_UPDATE --------|  // 初始游戏状态
  |                               |
  |====== 游戏循环开始 ======       |
  |                               |
```

### 游戏循环

```
Client                          Server
  |                               |
  |                               |--- Update Game Logic (60 FPS)
  |                               |
  |- PLAYER_MOVE ---------------->|
  |                               |
  |<--- GAME_STATE_UPDATE --------|
  |                               |
  |- PLAYER_ACTION --------------->|
  |                               |
  |<--- GAME_EVENT ---------------|  // 道具拾取事件
  |                               |
  |<--- GAME_STATE_UPDATE --------|
  |                               |
```

### 游戏结束流程

```
Client                          Server
  |                               |
  |                               |--- 检测到僵尸到达出口
  |                               |
  |<--- GAME_WIN -----------------|
  |                               |
  |-- GAME_CONTROL (restart) ---->|
  |                               |
  |<--- MAZE_DATA ---------------|  // 新游戏开始
  |                               |
```

---

## 错误处理

### 连接错误

如果WebSocket连接失败，客户端应：
1. 显示错误消息
2. 尝试重新连接（指数退避）
3. 最多重试5次

### 消息错误

如果收到格式错误的消息：
1. 记录错误日志
2. 忽略该消息
3. 继续处理后续消息

---

## 性能建议

1. **状态更新频率**: 60次/秒
2. **消息大小**: 保持每条消息 < 5KB
3. **压缩**: 可选使用WebSocket压缩
4. **增量更新**: 优先使用ENTITY_UPDATE而非完整状态更新
5. **批处理**: 合并多个小消息为单个消息

---

## 示例代码

### 客户端发送移动命令 (JavaScript)

```javascript
function sendMove(direction) {
    const message = {
        type: 'PLAYER_MOVE',
        data: {
            direction: direction
        }
    };
    websocket.send(JSON.stringify(message));
}

// 使用
sendMove('up');
```

### 服务器处理移动命令 (C++)

```cpp
void Server::handlePlayerMove(const json& data) {
    std::string direction = data["direction"];

    if (direction == "up") {
        zombie->moveUp();
    } else if (direction == "down") {
        zombie->moveDown();
    } else if (direction == "left") {
        zombie->moveLeft();
    } else if (direction == "right") {
        zombie->moveRight();
    }
}
```
