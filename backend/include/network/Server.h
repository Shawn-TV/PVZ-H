/**
 * @file Server.h
 * @brief WebSocket服务器
 *
 * 功能：
 * - 与前端建立WebSocket连接
 * - 接收玩家输入
 * - 发送游戏状态更新
 * - 处理连接和断开
 */

#ifndef SERVER_H
#define SERVER_H

#include <string>

class Game;

class Server {
public:
    Server(int port);
    ~Server();

    void start();
    void stop();
    void broadcast(const std::string& message);

private:
    int port_;
    bool running_;
    Game* game_;

    void handleMessage(const std::string& message);
    // TODO: WebSocket连接管理
};

#endif // SERVER_H
