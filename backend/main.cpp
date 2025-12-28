/**
 * @file main.cpp
 * @brief 游戏主入口文件
 *
 * 负责：
 * - 初始化游戏系统
 * - 运行游戏主循环
 */

#include "include/core/Game.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

/**
 * 简单的自动测试模式
 * 僵尸自动寻路到出口
 */
void runAutoTest(Game& game) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  自动测试模式" << std::endl;
    std::cout << "  僵尸将自动移动到出口" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 简单的自动移动逻辑
    int moveCounter = 0;

    while (game.isRunning() && game.getStatus() == GameStatus::PLAYING) {
        moveCounter++;

        // 每隔一段时间改变移动方向（模拟玩家输入）
        int direction = (moveCounter / 30) % 4;  // 每30帧改变一次方向

        switch (direction) {
            case 0:
                game.moveZombieRight();
                break;
            case 1:
                game.moveZombieDown();
                break;
            case 2:
                game.moveZombieLeft();
                break;
            case 3:
                game.moveZombieUp();
                break;
        }

        // 每隔一段时间尝试攻击
        if (moveCounter % 20 == 0) {
            game.startAttack();
        } else if (moveCounter % 20 == 5) {
            game.stopAttack();
        }

        // 让游戏循环处理一帧
        std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS

        // 每100帧输出一次状态
        if (moveCounter % 100 == 0) {
            std::cout << "自动测试进行中... (帧数: " << moveCounter << ")" << std::endl;
        }
    }
}

/**
 * 交互式测试模式
 * 显示简单的命令行界面，等待用户输入命令
 */
void runInteractiveTest(Game& game) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  交互式测试模式" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "命令列表：" << std::endl;
    std::cout << "  w/a/s/d - 移动僵尸" << std::endl;
    std::cout << "  space   - 攻击" << std::endl;
    std::cout << "  q       - 退出游戏" << std::endl;
    std::cout << "  h       - 显示帮助" << std::endl;
    std::cout << "========================================\n" << std::endl;

    char command;
    while (game.isRunning() && std::cin >> command) {
        switch (command) {
            case 'w':
            case 'W':
                game.moveZombieUp();
                std::cout << "向上移动" << std::endl;
                break;

            case 's':
            case 'S':
                game.moveZombieDown();
                std::cout << "向下移动" << std::endl;
                break;

            case 'a':
            case 'A':
                game.moveZombieLeft();
                std::cout << "向左移动" << std::endl;
                break;

            case 'd':
            case 'D':
                game.moveZombieRight();
                std::cout << "向右移动" << std::endl;
                break;

            case ' ':
                game.startAttack();
                std::cout << "攻击！" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                game.stopAttack();
                break;

            case 'q':
            case 'Q':
                std::cout << "退出游戏..." << std::endl;
                game.quit();
                break;

            case 'h':
            case 'H':
                std::cout << "\n命令: w(上) a(左) s(下) d(右) space(攻击) q(退出)\n" << std::endl;
                break;

            default:
                std::cout << "未知命令。输入 'h' 查看帮助。" << std::endl;
                break;
        }

        // 让游戏更新一帧
        std::this_thread::sleep_for(std::chrono::milliseconds(16));

        // 检查游戏状态
        if (game.getStatus() == GameStatus::WIN || game.getStatus() == GameStatus::LOSE) {
            break;
        }
    }
}

/**
 * 网络模式（模式3）
 * 读取stdin输入并转发到游戏（用于WebSocket桥接）
 */
void runNetworkMode(Game& game) {
    std::cout << "网络模式启动 - 监听stdin输入..." << std::endl;

    // 网络模式默认使用AI控制Dave
    // 多人模式时，前端会发送 'm' 命令来启用玩家控制
    // 单人模式时，Dave保持AI控制，可以自动追踪和种植

    char command;
    while (game.isRunning() && std::cin.get(command)) {
        // 跳过换行符
        if (command == '\n' || command == '\r') {
            continue;
        }

        // 处理命令
        switch (command) {
            case 'w':
            case 'W':
                game.moveZombieUp();
                break;

            case 's':
            case 'S':
                game.moveZombieDown();
                break;

            case 'a':
            case 'A':
                game.moveZombieLeft();
                break;

            case 'd':
            case 'D':
                game.moveZombieRight();
                break;

            case 'x':
            case 'X':
                // 停止移动
                game.stopZombie();
                break;

            case ' ':
                // 攻击
                game.startAttack();
                // 短暂延迟后停止攻击
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                game.stopAttack();
                break;

            case 'c':
            case 'C':
                // 撑杆跳
                game.triggerPoleVaultJump();
                break;

            case 'p':
                // 暂停/恢复游戏（只用小写p，大写P留给种植命令）
                game.togglePause();
                break;

            // ==================== 戴夫控制（多人模式） ====================
            case 'i':
            case 'I':
                // 戴夫向上移动
                game.moveDaveUp();
                break;

            case 'k':
            case 'K':
                // 戴夫向下移动
                game.moveDaveDown();
                break;

            case 'j':
            case 'J':
                // 戴夫向左移动
                game.moveDaveLeft();
                break;

            case 'l':
            case 'L':
                // 戴夫向右移动
                game.moveDaveRight();
                break;

            case 'o':
            case 'O':
                // 戴夫停止移动
                game.stopDave();
                break;

            case 'm':
            case 'M':
                // 启用戴夫玩家控制模式
                game.setDavePlayerControlled(true);
                break;

            case 'n':
            case 'N':
                // 禁用戴夫玩家控制模式（恢复AI控制）
                game.setDavePlayerControlled(false);
                break;

            // ==================== 戴夫种植（多人模式） ====================
            // 旧格式：1-4 在Dave当前位置种植
            case '1':
                game.davePlantAtPosition(0);
                break;

            case '2':
                game.davePlantAtPosition(1);
                break;

            case '3':
                game.davePlantAtPosition(2);
                break;

            case '4':
                game.davePlantAtPosition(3);
                break;

            // 新格式：P<type>,<x>,<y> 在指定位置种植
            case 'P': {
                // 读取剩余的命令参数
                std::string params;
                std::getline(std::cin, params);

                // 解析参数: <type>,<x>,<y>
                int plantType = -1, gridX = -1, gridY = -1;
                if (sscanf(params.c_str(), "%d,%d,%d", &plantType, &gridX, &gridY) == 3) {
                    game.davePlantAtGridPosition(plantType, gridX, gridY);
                }
                break;
            }

            case 'q':
            case 'Q':
                game.quit();
                break;

            default:
                // 忽略未知命令
                break;
        }

        // 检查游戏状态
        if (game.getStatus() == GameStatus::WIN || game.getStatus() == GameStatus::LOSE) {
            break;
        }
    }
}

/**
 * 主函数
 */
int main(int argc, char* argv[]) {
    // 初始化随机数种子
    srand(static_cast<unsigned int>(time(nullptr)));

    std::cout << R"(
   ______ _    __ ____                __  ___
  / ____/| |  / //_  /               /  |/  /____ _ ____  ___
 / /_    | | / /  / /  _____ _____  / /|_/ // __ `//_  / / _ \
/ __/    | |/ /  / /_ /____//____/ / /  / // /_/ /  / /_/  __/
/_/       |___/  /____/              /_/  /_/ \__,_/  /___/\___/

         植物大战僵尸 - 迷宫冒险游戏
    )" << std::endl;

    // 创建游戏实例
    Game game;

    // 初始化游戏
    try {
        game.initialize();
    } catch (const std::exception& e) {
        std::cerr << "游戏初始化失败: " << e.what() << std::endl;
        return 1;
    }

    // 选择测试模式
    std::cout << "\n请选择模式：" << std::endl;
    std::cout << "  1 - 自动测试模式（僵尸自动移动）" << std::endl;
    std::cout << "  2 - 交互式模式（手动输入命令）" << std::endl;
    std::cout << "  3 - 网络模式（WebSocket桥接）" << std::endl;
    std::cout << "选择 (1/2/3): ";

    int choice;
    std::cin >> choice;

    // 清除输入缓冲区中的换行符
    std::cin.ignore();

    // 启动游戏
    std::thread gameThread([&game]() {
        game.run();
    });

    // 根据选择运行不同的测试模式
    switch (choice) {
        case 1:
            runAutoTest(game);
            break;
        case 2:
            runInteractiveTest(game);
            break;
        case 3:
            runNetworkMode(game);
            break;
        default:
            std::cout << "无效选择，使用自动测试模式" << std::endl;
            runAutoTest(game);
            break;
    }

    // 等待游戏线程结束
    if (gameThread.joinable()) {
        gameThread.join();
    }

    // 清理
    game.shutdown();

    std::cout << "\n感谢游玩！再见！\n" << std::endl;
    return 0;
}
