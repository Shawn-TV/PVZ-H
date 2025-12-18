/**
 * @file Game.h
 * @brief 游戏核心管理类
 *
 * 负责：
 * - 游戏主循环
 * - 游戏状态管理
 * - 各个子系统的协调
 * - 游戏时间管理
 */

#ifndef GAME_H
#define GAME_H

class Game {
public:
    Game();
    ~Game();

    void initialize();
    void run();
    void shutdown();

private:
    // TODO: 游戏状态
    // TODO: 迷宫对象
    // TODO: 实体管理器
    // TODO: 游戏循环控制
};

#endif // GAME_H
