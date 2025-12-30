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

#include "../maze/Maze.h"
#include "../entities/EntityManager.h"
#include "../entities/Zombie.h"
#include "../entities/Dave.h"
#include "../entities/Item.h"
#include "Config.h"
#include <chrono>
#include <memory>

/**
 * 游戏状态枚举
 */
enum class GameStatus {
    MENU,       // 主菜单
    PLAYING,    // 游戏进行中
    PAUSED,     // 暂停
    WIN,        // 胜利
    LOSE        // 失败
};

/**
 * 游戏核心类
 */
class Game {
public:
    Game();
    ~Game();

    /**
     * 初始化游戏系统
     * - 创建迷宫
     * - 生成僵尸和戴夫
     * - 生成道具
     */
    void initialize();

    /**
     * 游戏主循环
     * 以固定60fps运行
     */
    void run();

    /**
     * 关闭游戏，清理资源
     */
    void shutdown();

    /**
     * 更新游戏逻辑（每帧调用）
     * @param deltaTime 时间增量（秒）
     */
    void update(float deltaTime);

    /**
     * 渲染游戏状态（输出到控制台，用于调试）
     */
    void render();

    // ==================== 输入处理 ====================

    /**
     * 处理玩家输入
     * WASD/方向键：移动
     * 鼠标：攻击
     * Ctrl：撑杆跳跳跃
     */
    void handleInput();

    // 僵尸移动输入
    void moveZombieUp();
    void moveZombieDown();
    void moveZombieLeft();
    void moveZombieRight();
    void stopZombie();

    // 戴夫移动输入（多人模式）
    void moveDaveUp();
    void moveDaveDown();
    void moveDaveLeft();
    void moveDaveRight();
    void stopDave();
    void setDavePlayerControlled(bool controlled);

    // 戴夫种植功能（多人模式）
    void davePlantAtPosition(int plantType);
    void davePlantAtGridPosition(int plantType, int gridX, int gridY);

    // 攻击输入
    void startAttack();
    void stopAttack();

    // 特殊技能
    void triggerPoleVaultJump();

    // ==================== 游戏状态 ====================

    GameStatus getStatus() const { return status_; }
    void setStatus(GameStatus newStatus) { status_ = newStatus; }

    bool isRunning() const { return running_; }
    void quit() { running_ = false; }

    // 暂停/恢复游戏
    void pauseGame();
    void resumeGame();
    void togglePause();
    bool isPaused() const { return status_ == GameStatus::PAUSED; }

    // ==================== 游戏逻辑 ====================

    /**
     * 检查胜利条件：僵尸到达出口
     */
    void checkWinCondition();

    /**
     * 检查失败条件：僵尸死亡
     */
    void checkLoseCondition();

    /**
     * 处理僵尸攻击
     */
    void processZombieAttack(float deltaTime);

    /**
     * 生成道具到迷宫中
     */
    void spawnItems();

    /**
     * 输出JSON格式的游戏状态（用于网络通信）
     */
    void outputGameStateJson() const;

    /**
     * 输出迷宫初始化数据（游戏开始时发送一次）
     */
    void outputMazeInit() const;

    /**
     * 获取Maze指针（用于序列化）
     */
    const Maze* getMaze() const { return maze_.get(); }

    /**
     * 获取EntityManager指针（用于序列化）
     */
    const EntityManager* getEntityManager() const { return entityManager_.get(); }

private:
    // 游戏状态
    GameStatus status_;
    bool running_;

    // 核心系统
    std::unique_ptr<Maze> maze_;
    std::unique_ptr<EntityManager> entityManager_;

    // 主要实体
    Zombie* zombie_;  // 玩家控制的僵尸
    Dave* dave_;      // NPC敌人

    // 时间管理
    std::chrono::steady_clock::time_point lastFrameTime_;
    float accumulator_;  // 用于固定时间步长

    // 输入状态
    bool attacking_;

    // 游戏统计
    float gameTime_;      // 游戏运行时间
    int itemsCollected_;  // 收集的道具数量
};

#endif // GAME_H
