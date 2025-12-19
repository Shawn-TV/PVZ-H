/**
 * @file GameStateSerializer.h
 * @brief 游戏状态序列化器
 *
 * 功能：
 * - 将游戏状态序列化为JSON
 * - 用于网络传输
 */

#ifndef GAME_STATE_SERIALIZER_H
#define GAME_STATE_SERIALIZER_H

#include <string>
#include <sstream>
#include <vector>

class Game;
class Entity;
class Maze;

class GameStateSerializer {
public:
    /**
     * 序列化完整游戏状态
     * @param game 游戏实例
     * @return JSON字符串
     */
    static std::string serializeGameState(const Game* game);

    /**
     * 序列化迷宫数据（仅在初始化时发送）
     * @param maze 迷宫实例
     * @return JSON字符串
     */
    static std::string serializeMaze(const Maze* maze);

    /**
     * 序列化单个实体
     * @param entity 实体指针
     * @return JSON字符串
     */
    static std::string serializeEntity(const Entity* entity);

    /**
     * 序列化实体列表
     * @param entities 实体列表
     * @return JSON字符串
     */
    static std::string serializeEntities(const std::vector<Entity*>& entities);

private:
    // 辅助函数：转义JSON字符串
    static std::string escapeJson(const std::string& str);

    // 辅助函数：浮点数转字符串（保留2位小数）
    static std::string floatToStr(float value);
};

#endif // GAME_STATE_SERIALIZER_H
