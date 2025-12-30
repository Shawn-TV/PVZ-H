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
    // 1. 创建迷宫
    maze_ = std::make_unique<Maze>(
        Config::MAZE_GRID_WIDTH,
        Config::MAZE_GRID_HEIGHT,
        Config::MAZE_CELL_SIZE
    );
    maze_->generate();

    // 2. 创建实体管理器
    entityManager_ = std::make_unique<EntityManager>();

    // 3. 创建玩家僵尸（在入口处）
    Vector2D entrancePos = maze_->getEntrancePosition();
    zombie_ = new Zombie(entrancePos.x, entrancePos.y);
    zombie_->setHealth(200.0f);
    zombie_->setMaxHealth(200.0f);
    zombie_->setEntityManager(entityManager_.get());
    zombie_->setMaze(maze_.get());
    entityManager_->addZombie(zombie_);

    // 4. 创建戴夫NPC（在迷宫中央附近）
    float daveX = maze_->getPixelWidth() / 2.0f;
    float daveY = maze_->getPixelHeight() / 2.0f;
    dave_ = new Dave(daveX, daveY, maze_.get());
    dave_->setHealth(200.0f);
    dave_->setMaxHealth(200.0f);
    dave_->setTarget(zombie_);
    dave_->setEntityManager(entityManager_.get());
    entityManager_->addDave(dave_);

    // 5. 生成道具
    spawnItems();

    // 6. 初始化时间
    lastFrameTime_ = std::chrono::steady_clock::now();

    // 设置游戏状态为进行中
    status_ = GameStatus::PLAYING;
    running_ = true;

    // 7. 发送迷宫初始化数据给前端
    outputMazeInit();
}

void Game::outputMazeInit() const {
    // 发送迷宫数据给前端
    if (maze_) {
        std::string mazeJson = GameStateSerializer::serializeMaze(maze_.get());
        std::cout << "{\"type\":\"MAZE_INIT\",\"data\":" << mazeJson << "}" << std::endl;
    }
}

void Game::spawnItems() {
    // 获取道具生成点
    std::vector<Vector2D> spawnPositions = maze_->getItemSpawnPositions();

    // 在每个生成点随机生成道具
    for (size_t i = 0; i < spawnPositions.size(); ++i) {
        const Vector2D& pos = spawnPositions[i];

        // 随机决定道具类型 (25% 铁桶, 25% 撑杆跳, 25% 生命药水, 25% 速度药水)
        int randomType = rand() % 4;

        Item* item = nullptr;
        if (randomType == 0) {
            item = new Bucket(pos.x, pos.y);
        } else if (randomType == 1) {
            item = new PoleVaultKit(pos.x, pos.y);
        } else if (randomType == 2) {
            item = new HealthPotion(pos.x, pos.y);
        } else {
            item = new SpeedPotion(pos.x, pos.y);
        }

        if (item) {
            entityManager_->addItem(item);
        }
    }
}

// ==================== 游戏主循环 ====================

void Game::run() {
    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1.0f / TARGET_FPS;

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
    static float lastJsonTime = 0;
    float currentTime = gameTime_;

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
    if (dave_ && dave_->isAlive() && dave_->isPlayerControlled()) {
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
    }
}

void Game::davePlantAtPosition(int plantType) {
    if (dave_ && dave_->isAlive() && dave_->isPlayerControlled()) {
        dave_->plantAtCurrentPosition(plantType);
    }
}

void Game::davePlantAtGridPosition(int plantType, int gridX, int gridY) {
    if (dave_ && dave_->isAlive() && dave_->isPlayerControlled()) {
        dave_->plantAtGridPosition(plantType, gridX, gridY);
    }
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
    }
}

void Game::resumeGame() {
    if (status_ == GameStatus::PAUSED) {
        status_ = GameStatus::PLAYING;
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
                attackCooldown = 0;
            }
        }
    }
}

void Game::checkWinCondition() {
    if (!zombie_ || !zombie_->isAlive()) return;

    // 检查僵尸是否到达出口
    Vector2D exitPos = maze_->getExitPosition();
    float distToExit = zombie_->getPosition().distance(exitPos);

    // 如果僵尸在出口附近（小于一个格子的距离）
    if (distToExit < maze_->getCellSize()) {
        status_ = GameStatus::WIN;
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
        running_ = false;
    }
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
    // EntityManager会自动清理实体
    entityManager_.reset();
    maze_.reset();

    zombie_ = nullptr;
    dave_ = nullptr;
}
