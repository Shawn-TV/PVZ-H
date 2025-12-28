/**
 * @file Game.cpp
 * @brief 游戏核心管理类实现
 */

#include "../../include/core/Game.h"
#include "../../include/core/Config.h"
#include "../../include/network/GameStateSerializer.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <sstream>

Game::Game()
    : status_(GameStatus::MENU),
      running_(false),
      zombie_(nullptr),
      dave_(nullptr),
      accumulator_(0),
      attacking_(false),
      gameTime_(0),
      itemsCollected_(0) {
}

Game::~Game() {
    shutdown();
}

// ==================== 初始化 ====================

void Game::initialize() {
    std::cout << "==================================" << std::endl;
    std::cout << "  PVZ迷宫游戏 - 初始化中..." << std::endl;
    std::cout << "==================================" << std::endl;

    // 1. 创建迷宫
    std::cout << "生成迷宫..." << std::endl;
    maze_ = std::make_unique<Maze>(
        Config::MAZE_GRID_WIDTH,
        Config::MAZE_GRID_HEIGHT,
        Config::MAZE_CELL_SIZE
    );
    maze_->generate();
    std::cout << "迷宫生成完成！" << std::endl;
    std::cout << "  - 尺寸: " << maze_->getGridWidth() << "x" << maze_->getGridHeight() << " 格子" << std::endl;
    std::cout << "  - 像素尺寸: " << maze_->getPixelWidth() << "x" << maze_->getPixelHeight() << std::endl;

    // 2. 创建实体管理器
    entityManager_ = std::make_unique<EntityManager>();

    // 3. 创建玩家僵尸（在入口处）
    Vector2D entrancePos = maze_->getEntrancePosition();
    zombie_ = new Zombie(entrancePos.x, entrancePos.y);
    zombie_->setHealth(200.0f);  // 设置初始生命值为200
    zombie_->setMaxHealth(200.0f);
    zombie_->setEntityManager(entityManager_.get());
    zombie_->setMaze(maze_.get());  // 设置迷宫引用用于墙壁碰撞检测
    entityManager_->addZombie(zombie_);
    std::cout << "玩家僵尸已创建！" << std::endl;
    std::cout << "  - 位置: (" << entrancePos.x << ", " << entrancePos.y << ")" << std::endl;
    std::cout << "  - 生命值: " << zombie_->getHealth() << std::endl;

    // 4. 创建戴夫NPC（在迷宫中央附近）
    float daveX = maze_->getPixelWidth() / 2.0f;
    float daveY = maze_->getPixelHeight() / 2.0f;
    dave_ = new Dave(daveX, daveY, maze_.get());
    dave_->setHealth(200.0f);  // 设置初始生命值为200
    dave_->setMaxHealth(200.0f);
    dave_->setTarget(zombie_);
    dave_->setEntityManager(entityManager_.get());
    entityManager_->addDave(dave_);
    std::cout << "戴夫NPC已创建！" << std::endl;
    std::cout << "  - 位置: (" << daveX << ", " << daveY << ")" << std::endl;
    std::cout << "  - 生命值: " << dave_->getHealth() << std::endl;

    // 5. 生成道具
    spawnItems();

    // 6. 初始化时间
    lastFrameTime_ = std::chrono::steady_clock::now();

    std::cout << "\n游戏初始化完成！" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "\n操作说明：" << std::endl;
    std::cout << "  WASD/方向键：移动僵尸" << std::endl;
    std::cout << "  鼠标左键：攻击" << std::endl;
    std::cout << "  Ctrl：撑杆跳跳跃（需装备）" << std::endl;
    std::cout << "  ESC：退出游戏" << std::endl;
    std::cout << "\n目标：到达迷宫出口！" << std::endl;
    std::cout << "==================================" << std::endl;

    // 设置游戏状态为进行中
    status_ = GameStatus::PLAYING;
    running_ = true;

    // 输出迷宫数据JSON（仅在初始化时发送一次）
    std::string mazeJson = GameStateSerializer::serializeMaze(maze_.get());
    std::cout << "{\"type\":\"MAZE_DATA\",\"data\":" << mazeJson << "}" << std::endl;
}

void Game::spawnItems() {
    std::cout << "生成道具..." << std::endl;

    // 获取道具生成点
    std::vector<Vector2D> spawnPositions = maze_->getItemSpawnPositions();

    int bucketCount = 0;
    int poleVaultCount = 0;
    int healthPotionCount = 0;

    // 在每个生成点随机生成道具
    for (size_t i = 0; i < spawnPositions.size(); ++i) {
        const Vector2D& pos = spawnPositions[i];

        // 随机决定道具类型 (33% 铁桶, 33% 撑杆跳, 33% 生命药水)
        int randomType = rand() % 3;

        Item* item = nullptr;
        if (randomType == 0) {
            item = new Bucket(pos.x, pos.y);
            bucketCount++;
        } else if (randomType == 1) {
            item = new PoleVaultKit(pos.x, pos.y);
            poleVaultCount++;
        } else {
            item = new HealthPotion(pos.x, pos.y);
            healthPotionCount++;
        }

        if (item) {
            entityManager_->addItem(item);
        }
    }

    std::cout << "道具生成完成！" << std::endl;
    std::cout << "  - 铁桶: " << bucketCount << " 个" << std::endl;
    std::cout << "  - 撑杆跳套装: " << poleVaultCount << " 个" << std::endl;
    std::cout << "  - 生命药水: " << healthPotionCount << " 个" << std::endl;
}

// ==================== 游戏主循环 ====================

void Game::run() {
    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1.0f / TARGET_FPS;

    std::cout << "\n游戏开始运行！" << std::endl;
    std::cout << "目标FPS: " << TARGET_FPS << std::endl;

    // 重新输出迷宫数据（确保在网络模式选择后前端能收到）
    if (maze_) {
        std::string mazeJson = GameStateSerializer::serializeMaze(maze_.get());
        std::cout << "{\"type\":\"MAZE_DATA\",\"data\":" << mazeJson << "}" << std::endl;
    }

    while (running_) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime_).count();
        lastFrameTime_ = currentTime;

        // 限制最大时间步长（防止大延迟）
        if (deltaTime > 0.25f) {
            deltaTime = 0.25f;
        }

        // 固定时间步长更新
        accumulator_ += deltaTime;

        while (accumulator_ >= FRAME_TIME) {
            // 处理输入
            handleInput();

            // 更新游戏逻辑
            if (status_ == GameStatus::PLAYING) {
                update(FRAME_TIME);
            }

            accumulator_ -= FRAME_TIME;
        }

        // 渲染（调试输出）
        render();

        // 控制帧率
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // 检查游戏结束
        if (status_ == GameStatus::WIN || status_ == GameStatus::LOSE) {
            // 确保发送最终游戏状态
            outputGameStateJson();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            break;
        }
    }

    std::cout << "\n游戏结束！" << std::endl;
}

void Game::update(float deltaTime) {
    // 更新游戏时间
    gameTime_ += deltaTime;

    // 更新所有实体
    entityManager_->update(deltaTime);

    // 检查碰撞
    entityManager_->checkCollisions();

    // 僵尸攻击由Zombie类自动处理（遇到戴夫或植物时自动攻击）

    // 移除死亡实体
    entityManager_->removeDeadEntities();

    // 检查胜利条件
    checkWinCondition();

    // 检查失败条件
    checkLoseCondition();
}

void Game::render() {
    // 每秒输出一次调试信息
    static float lastDebugTime = 0;
    static float lastJsonTime = 0;

    float currentTime = gameTime_;

    // 每秒输出调试信息
    if (currentTime - lastDebugTime >= 1.0f) {
        printDebugInfo();
        lastDebugTime = currentTime;
    }

    // 每 1/60 秒（60fps）输出JSON状态给前端
    if (currentTime - lastJsonTime >= 0.01667f) {
        outputGameStateJson();
        lastJsonTime = currentTime;
    }
}

// ==================== 输入处理 ====================

void Game::handleInput() {
    // 这里是简化版本，实际需要集成输入库（如SDL、GLFW等）
    // 目前留空，由main.cpp调用具体的输入函数
}

void Game::moveZombieUp() {
    if (zombie_ && zombie_->isAlive()) {
        zombie_->moveUp();
    }
}

void Game::moveZombieDown() {
    if (zombie_ && zombie_->isAlive()) {
        zombie_->moveDown();
    }
}

void Game::moveZombieLeft() {
    if (zombie_ && zombie_->isAlive()) {
        zombie_->moveLeft();
    }
}

void Game::moveZombieRight() {
    if (zombie_ && zombie_->isAlive()) {
        zombie_->moveRight();
    }
}

void Game::stopZombie() {
    if (zombie_ && zombie_->isAlive()) {
        zombie_->stopMoving();
    }
}

// ==================== 戴夫控制（多人模式） ====================

void Game::moveDaveUp() {
    std::cerr << "[DEBUG moveDaveUp] called, dave_=" << (dave_ ? "valid" : "null") << std::endl;
    if (dave_) {
        std::cerr << "[DEBUG moveDaveUp] isAlive=" << dave_->isAlive()
                  << ", isPlayerControlled=" << dave_->isPlayerControlled() << std::endl;
    }
    if (dave_ && dave_->isAlive() && dave_->isPlayerControlled()) {
        std::cerr << "[DEBUG moveDaveUp] calling dave_->moveUp()" << std::endl;
        dave_->moveUp();
    }
}

void Game::moveDaveDown() {
    if (dave_ && dave_->isAlive() && dave_->isPlayerControlled()) {
        dave_->moveDown();
    }
}

void Game::moveDaveLeft() {
    if (dave_ && dave_->isAlive() && dave_->isPlayerControlled()) {
        dave_->moveLeft();
    }
}

void Game::moveDaveRight() {
    if (dave_ && dave_->isAlive() && dave_->isPlayerControlled()) {
        dave_->moveRight();
    }
}

void Game::stopDave() {
    if (dave_ && dave_->isAlive() && dave_->isPlayerControlled()) {
        dave_->stopMoving();
    }
}

void Game::setDavePlayerControlled(bool controlled) {
    if (dave_) {
        dave_->setPlayerControlled(controlled);
        if (controlled) {
            std::cout << "戴夫切换为玩家控制模式" << std::endl;
        } else {
            std::cout << "戴夫切换为AI控制模式" << std::endl;
        }
    }
}

void Game::davePlantAtPosition(int plantType) {
    if (dave_ && dave_->isAlive() && dave_->isPlayerControlled()) {
        dave_->plantAtCurrentPosition(plantType);
    }
}

void Game::davePlantAtGridPosition(int plantType, int gridX, int gridY) {
    std::cerr << "[DEBUG] davePlantAtGridPosition: plantType=" << plantType
              << ", gridX=" << gridX << ", gridY=" << gridY << std::endl;

    if (!dave_) {
        std::cerr << "[DEBUG] 种植失败: dave_ 为空!" << std::endl;
        return;
    }
    if (!dave_->isAlive()) {
        std::cerr << "[DEBUG] 种植失败: Dave 已死亡!" << std::endl;
        return;
    }
    if (!dave_->isPlayerControlled()) {
        std::cerr << "[DEBUG] 种植失败: Dave 不是玩家控制模式! isPlayerControlled="
                  << dave_->isPlayerControlled() << std::endl;
        return;
    }

    std::cerr << "[DEBUG] 条件检查通过，调用 plantAtGridPosition" << std::endl;
    dave_->plantAtGridPosition(plantType, gridX, gridY);
}

void Game::startAttack() {
    attacking_ = true;
}

void Game::stopAttack() {
    attacking_ = false;
}

void Game::triggerPoleVaultJump() {
    if (zombie_ && zombie_->isAlive() && zombie_->hasPoleVault() && !zombie_->hasPoleVaultJumped()) {
        // 执行撑杆跳跃
        zombie_->performPoleVaultJump();
    }
}

void Game::pauseGame() {
    if (status_ == GameStatus::PLAYING) {
        status_ = GameStatus::PAUSED;
        std::cout << "游戏已暂停" << std::endl;
    }
}

void Game::resumeGame() {
    if (status_ == GameStatus::PAUSED) {
        status_ = GameStatus::PLAYING;
        std::cout << "游戏已恢复" << std::endl;
    }
}

void Game::togglePause() {
    if (status_ == GameStatus::PLAYING) {
        pauseGame();
    } else if (status_ == GameStatus::PAUSED) {
        resumeGame();
    }
}

// ==================== 游戏逻辑 ====================

void Game::processZombieAttack(float deltaTime) {
    if (!zombie_ || !zombie_->isAlive()) return;

    // 获取僵尸附近的敌人（植物和戴夫）
    float attackRange = 50.0f;  // 攻击范围
    float attackDamage = 10.0f;  // 攻击伤害

    // 检查是否能攻击戴夫
    if (dave_ && dave_->isAlive()) {
        float distToDave = zombie_->getPosition().distance(dave_->getPosition());
        if (distToDave <= attackRange) {
            // 每0.5秒攻击一次
            static float attackCooldown = 0;
            attackCooldown += deltaTime;
            if (attackCooldown >= 0.5f) {
                dave_->takeDamage(attackDamage);
                std::cout << "僵尸攻击戴夫！戴夫生命值: " << dave_->getHealth() << std::endl;
                attackCooldown = 0;
            }
        }
    }

    // TODO: 攻击植物（需要添加植物实体）
}

void Game::checkWinCondition() {
    if (!zombie_ || !zombie_->isAlive()) return;

    // 检查僵尸是否到达出口
    Vector2D exitPos = maze_->getExitPosition();
    float distToExit = zombie_->getPosition().distance(exitPos);

    // 如果僵尸在出口附近（小于一个格子的距离）
    if (distToExit < maze_->getCellSize()) {
        status_ = GameStatus::WIN;
        std::cout << "\n" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "          恭喜！你赢了！" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "游戏时间: " << static_cast<int>(gameTime_) << " 秒" << std::endl;
        std::cout << "收集道具: " << itemsCollected_ << " 个" << std::endl;
        std::cout << "剩余生命值: " << zombie_->getHealth() << std::endl;
        std::cout << "========================================" << std::endl;
        running_ = false;
    }
}

void Game::checkLoseCondition() {
    // 游戏刚开始时不检查失败条件，给予1秒的缓冲时间
    // 防止因为初始化顺序问题导致的误判
    if (gameTime_ < 1.0f) {
        return;
    }

    // 检查僵尸是否死亡
    if (!zombie_ || !zombie_->isAlive()) {
        status_ = GameStatus::LOSE;
        std::cout << "\n" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "          游戏失败！" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "游戏时间: " << static_cast<int>(gameTime_) << " 秒" << std::endl;
        std::cout << "========================================" << std::endl;
        running_ = false;
    }
}

void Game::printDebugInfo() const {
    std::cout << "\n--- 游戏状态 ---" << std::endl;
    std::cout << "时间: " << static_cast<int>(gameTime_) << "s" << std::endl;

    if (zombie_) {
        std::cout << "僵尸: 生命 " << zombie_->getHealth() << "/" << zombie_->getMaxHealth()
                  << " | 位置 (" << static_cast<int>(zombie_->getPosition().x) << ", "
                  << static_cast<int>(zombie_->getPosition().y) << ")"
                  << " | 护甲 " << zombie_->getArmor()
                  << " | 形态 " << static_cast<int>(zombie_->getForm())
                  << std::endl;
    }

    if (dave_) {
        std::cout << "戴夫: 生命 " << dave_->getHealth() << "/" << dave_->getMaxHealth()
                  << " | 位置 (" << static_cast<int>(dave_->getPosition().x) << ", "
                  << static_cast<int>(dave_->getPosition().y) << ")" << std::endl;
    }

    Vector2D exitPos = maze_->getExitPosition();
    if (zombie_) {
        float distToExit = zombie_->getPosition().distance(exitPos);
        std::cout << "距离出口: " << static_cast<int>(distToExit) << " 像素" << std::endl;
    }

    std::cout << "实体数量: " << entityManager_->getEntityCount() << std::endl;
    std::cout << "----------------" << std::endl;
}

void Game::outputGameStateJson() const {
    // 输出游戏状态JSON（发送到WebSocket桥接服务器）

    // 1. 输出实体列表（使用各实体的toJson()方法以获取完整数据）
    if (entityManager_) {
        std::vector<Entity*> allEntities = entityManager_->getAllEntities();

        std::ostringstream entitiesJson;
        entitiesJson << "[";
        bool first = true;
        for (const Entity* entity : allEntities) {
            if (!entity || !entity->isAlive()) continue;
            if (!first) entitiesJson << ",";
            entitiesJson << entity->toJson();
            first = false;
        }
        entitiesJson << "]";

        std::cout << "{\"type\":\"ENTITIES\",\"data\":" << entitiesJson.str() << "}" << std::endl;
    }

    // 2. 输出游戏状态
    std::string gameStateJson = GameStateSerializer::serializeGameState(this);
    std::cout << "{\"type\":\"GAME_STATE\",\"data\":" << gameStateJson << "}" << std::endl;
}

// ==================== 清理 ====================

void Game::shutdown() {
    std::cout << "正在关闭游戏..." << std::endl;

    // EntityManager会自动清理实体
    entityManager_.reset();
    maze_.reset();

    zombie_ = nullptr;
    dave_ = nullptr;

    std::cout << "游戏已关闭。" << std::endl;
}
