/**
 * @file Protocol.h
 * @brief 通信协议定义
 *
 * 功能：
 * - 定义前后端通信消息格式
 * - JSON序列化/反序列化
 * - 消息类型定义
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

namespace Protocol {
    // 消息类型
    enum class MessageType {
        // 客户端 -> 服务器
        PLAYER_MOVE,
        PLAYER_ACTION,

        // 服务器 -> 客户端
        GAME_STATE_UPDATE,
        ENTITY_UPDATE,
        GAME_OVER,
        GAME_WIN
    };

    // 消息结构
    struct Message {
        MessageType type;
        std::string data;
    };

    // 序列化和反序列化
    std::string serialize(const Message& msg);
    Message deserialize(const std::string& json);
}

#endif // PROTOCOL_H
