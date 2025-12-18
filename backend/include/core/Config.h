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
    // 迷宫配置
    constexpr int MAZE_WIDTH = 25;
    constexpr int MAZE_HEIGHT = 25;
    constexpr int CELL_SIZE = 32;

    // 游戏配置
    constexpr float TICK_RATE = 60.0f;
    constexpr int INITIAL_ZOMBIE_HEALTH = 100;

    // 网络配置
    constexpr int SERVER_PORT = 8080;

    // 实体速度
    constexpr float ZOMBIE_SPEED = 2.0f;
    constexpr float DAVE_SPEED = 1.5f;
}

#endif // CONFIG_H
