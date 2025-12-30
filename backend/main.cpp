/**
 * @file main.cpp
 * @brief 游戏主入口文件
 */

#include "include/core/Game.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

/**
 * 网络模式
 * 读取stdin输入并转发到游戏（用于WebSocket桥接）
 */
void runNetworkMode(Game& game) {
    char command;
    while (game.isRunning() && std::cin.get(command)) {
        if (command == '\n' || command == '\r') {
            continue;
        }

        switch (command) {
            // 僵尸移动
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
                game.stopZombie();
                break;

            // 僵尸攻击
            case ' ':
                game.startAttack();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                game.stopAttack();
                break;

            // 撑杆跳
            case 'c':
            case 'C':
                game.triggerPoleVaultJump();
                break;

            // 暂停
            case 'p':
                game.togglePause();
                break;

            // 戴夫移动（多人模式）
            case 'i':
            case 'I':
                game.moveDaveUp();
                break;
            case 'k':
            case 'K':
                game.moveDaveDown();
                break;
            case 'j':
            case 'J':
                game.moveDaveLeft();
                break;
            case 'l':
            case 'L':
                game.moveDaveRight();
                break;
            case 'o':
            case 'O':
                game.stopDave();
                break;

            // 戴夫控制模式切换
            case 'm':
            case 'M':
                game.setDavePlayerControlled(true);
                break;
            case 'n':
            case 'N':
                game.setDavePlayerControlled(false);
                break;

            // 戴夫种植
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
            case 'P': {
                std::string params;
                std::getline(std::cin, params);
                int plantType = -1, gridX = -1, gridY = -1;
                char comma1, comma2;
                std::istringstream iss(params);
                if (iss >> plantType >> comma1 >> gridX >> comma2 >> gridY &&
                    comma1 == ',' && comma2 == ',') {
                    game.davePlantAtGridPosition(plantType, gridX, gridY);
                }
                break;
            }

            // 退出
            case 'q':
            case 'Q':
                game.quit();
                break;

            default:
                break;
        }

        if (game.getStatus() == GameStatus::WIN || game.getStatus() == GameStatus::LOSE) {
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(nullptr)));

    Game game;

    try {
        game.initialize();
    } catch (const std::exception& e) {
        return 1;
    }

    std::thread gameThread([&game]() {
        game.run();
    });

    runNetworkMode(game);

    if (gameThread.joinable()) {
        gameThread.join();
    }

    game.shutdown();

    return 0;
}
