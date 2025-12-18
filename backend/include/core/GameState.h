/**
 * @file GameState.h
 * @brief 游戏状态管理
 *
 * 负责：
 * - 跟踪游戏当前状态（菜单、游戏中、暂停、游戏结束等）
 * - 状态转换逻辑
 * - 游戏数据（分数、时间等）
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H

enum class GameStatus {
    MENU,
    PLAYING,
    PAUSED,
    WIN,
    GAME_OVER
};

class GameState {
public:
    GameState();

    void reset();
    void update(float deltaTime);

private:
    // TODO: 当前状态
    // TODO: 玩家生命值
    // TODO: 游戏时间
    // TODO: 分数
};

#endif // GAMESTATE_H
