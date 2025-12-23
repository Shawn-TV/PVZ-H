/**
 * @file Config.h
 * @brief 游戏配置常量
 *
 * 包含：
 * - 游戏平衡参数
 * - 网络配置
 * - 迷宫尺寸
 * - 实体属性默认值
 */

#ifndef CONFIG_H
#define CONFIG_H

namespace Config {
    // ==================== 迷宫配置 ====================

    // 迷宫网格尺寸（格子数）
    // 设计目标：足够大，让玩家需要约2分钟探索完成
    constexpr int MAZE_GRID_WIDTH = 15;      // 迷宫宽度（格子数）
    constexpr int MAZE_GRID_HEIGHT = 21;     // 迷宫高度（格子数）
    constexpr int MAZE_CELL_SIZE = 150;      // 每个格子的像素大小（足够容纳僵尸）

    // 迷宫像素尺寸（自动计算）
    constexpr int MAZE_PIXEL_WIDTH = MAZE_GRID_WIDTH * MAZE_CELL_SIZE;    // 2250像素
    constexpr int MAZE_PIXEL_HEIGHT = MAZE_GRID_HEIGHT * MAZE_CELL_SIZE;  // 3150像素

    // 道具生成点数量
    constexpr int ITEM_SPAWN_COUNT = 12;     // 迷宫中生成的道具数量

    // ==================== 游戏配置 ====================

    constexpr float TICK_RATE = 60.0f;       // 游戏更新频率（帧/秒）
    constexpr int INITIAL_ZOMBIE_HEALTH = 200;

    // ==================== 网络配置 ====================

    constexpr int SERVER_PORT = 8080;

    // ==================== 实体速度 ====================

    // 速度单位：像素/秒
    constexpr float ZOMBIE_SPEED = 75.0f;    // 僵尸移动速度（玩家控制）- 调整为1.5倍
    constexpr float DAVE_SPEED = 70.0f;      // 戴夫移动速度（NPC追踪）- 降低以给玩家逃脱机会

    // ==================== 植物配置 ====================

    constexpr int PLANT_HEALTH_PEASHOOTER = 100;   // 豌豆射手生命值
    constexpr int PLANT_HEALTH_REPEATER = 100;     // 双发射手生命值
    constexpr int PLANT_HEALTH_WALLNUT = 300;      // 坚果墙生命值
    constexpr float PLANT_SHOOT_COOLDOWN = 2.0f;   // 射击冷却时间（秒）
    constexpr float PEA_DAMAGE = 20.0f;            // 豌豆伤害
    constexpr float PEA_SPEED = 200.0f;            // 豌豆速度
    constexpr float ZOMBIE_EAT_DPS = 50.0f;        // 僵尸吃植物每秒伤害
    constexpr float DAVE_PLANT_COOLDOWN = 8.0f;    // 戴夫种植冷却时间（秒）

    // ==================== 摄像机配置 ====================

    // 摄像机视野大小（像素）
    constexpr int CAMERA_VIEW_WIDTH = 800;   // 摄像机可见宽度
    constexpr int CAMERA_VIEW_HEIGHT = 600;  // 摄像机可见高度
    constexpr float CAMERA_SMOOTH_FACTOR = 0.1f;  // 摄像机跟随平滑系数
}

#endif // CONFIG_H
