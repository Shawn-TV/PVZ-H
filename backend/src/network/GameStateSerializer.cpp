/**
 * @file GameStateSerializer.cpp
 * @brief 游戏状态序列化器实现
 */

#include "../../include/network/GameStateSerializer.h"
#include "../../include/core/Game.h"
#include "../../include/entities/EntityManager.h"
#include "../../include/entities/Entity.h"
#include "../../include/entities/Zombie.h"
#include "../../include/entities/Dave.h"
#include "../../include/entities/Plant.h"
#include "../../include/entities/Item.h"
#include "../../include/maze/Maze.h"
#include <sstream>
#include <iomanip>
#include <cmath>

std::string GameStateSerializer::floatToStr(float value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

std::string GameStateSerializer::escapeJson(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default: oss << c; break;
        }
    }
    return oss.str();
}

std::string GameStateSerializer::serializeEntity(const Entity* entity) {
    if (!entity) return "null";

    std::ostringstream json;
    json << "{";
    json << "\"id\":" << entity->getId() << ",";
    json << "\"type\":\"";

    // 类型字符串
    switch (entity->getType()) {
        case EntityType::ZOMBIE: json << "zombie"; break;
        case EntityType::DAVE: json << "dave"; break;
        case EntityType::PLANT: json << "plant"; break;
        case EntityType::ITEM: json << "item"; break;
        case EntityType::PROJECTILE: json << "projectile"; break;
        case EntityType::EXPLOSION: json << "explosion"; break;
        default: json << "unknown"; break;
    }
    json << "\",";

    json << "\"x\":" << floatToStr(entity->getPosition().x) << ",";
    json << "\"y\":" << floatToStr(entity->getPosition().y) << ",";
    json << "\"vx\":" << floatToStr(entity->getVelocity().x) << ",";
    json << "\"vy\":" << floatToStr(entity->getVelocity().y) << ",";
    json << "\"health\":" << floatToStr(entity->getHealth()) << ",";
    json << "\"maxHealth\":" << floatToStr(entity->getMaxHealth()) << ",";
    json << "\"width\":" << floatToStr(entity->getWidth()) << ",";
    json << "\"height\":" << floatToStr(entity->getHeight()) << ",";
    json << "\"active\":" << (entity->isAlive() ? "true" : "false");

    // 特定类型的额外信息
    if (entity->getType() == EntityType::ZOMBIE) {
        const Zombie* zombie = static_cast<const Zombie*>(entity);
        json << ",\"form\":";
        switch (zombie->getForm()) {
            case ZombieForm::NORMAL: json << "\"normal\""; break;
            case ZombieForm::POLE_VAULTER: json << "\"pole_vaulter\""; break;
            case ZombieForm::BUCKET: json << "\"bucket\""; break;
        }
        json << ",\"armor\":" << floatToStr(zombie->getArmor());
        json << ",\"hasShield\":" << (zombie->hasShield() ? "true" : "false");
    } else if (entity->getType() == EntityType::PLANT) {
        const Plant* plant = static_cast<const Plant*>(entity);
        json << ",\"plantType\":";
        switch (plant->getPlantType()) {
            case PlantType::PEA_SHOOTER: json << "\"pea_shooter\""; break;
            case PlantType::CHERRY_BOMB: json << "\"cherry_bomb\""; break;
            case PlantType::WALL_NUT: json << "\"wall_nut\""; break;
        }
    } else if (entity->getType() == EntityType::ITEM) {
        const Item* item = static_cast<const Item*>(entity);
        json << ",\"itemType\":";
        switch (item->getItemType()) {
            case ItemType::BUCKET: json << "\"bucket\""; break;
            case ItemType::POLE_VAULT_KIT: json << "\"pole_vault_kit\""; break;
            case ItemType::HEALTH_POTION: json << "\"health_potion\""; break;
        }
    }

    json << "}";
    return json.str();
}

std::string GameStateSerializer::serializeEntities(const std::vector<Entity*>& entities) {
    std::ostringstream json;
    json << "[";

    bool first = true;
    for (const Entity* entity : entities) {
        if (!entity || !entity->isAlive()) continue;

        if (!first) json << ",";
        json << serializeEntity(entity);
        first = false;
    }

    json << "]";
    return json.str();
}

std::string GameStateSerializer::serializeMaze(const Maze* maze) {
    if (!maze) return "null";

    std::ostringstream json;
    json << "{";
    json << "\"gridWidth\":" << maze->getGridWidth() << ",";
    json << "\"gridHeight\":" << maze->getGridHeight() << ",";
    json << "\"cellSize\":" << maze->getCellSize() << ",";
    json << "\"pixelWidth\":" << maze->getPixelWidth() << ",";
    json << "\"pixelHeight\":" << maze->getPixelHeight() << ",";

    // 入口和出口位置
    Vector2D entrance = maze->getEntrancePosition();
    Vector2D exit = maze->getExitPosition();
    json << "\"entrance\":{\"x\":" << floatToStr(entrance.x) << ",\"y\":" << floatToStr(entrance.y) << "},";
    json << "\"exit\":{\"x\":" << floatToStr(exit.x) << ",\"y\":" << floatToStr(exit.y) << "},";

    // 迷宫墙壁数据（简化版，只发送墙壁列表）
    json << "\"walls\":[";
    bool firstWall = true;
    for (int y = 0; y < maze->getGridHeight(); ++y) {
        for (int x = 0; x < maze->getGridWidth(); ++x) {
            if (maze->isWall(x, y)) {
                if (!firstWall) json << ",";
                json << "{\"x\":" << x << ",\"y\":" << y << "}";
                firstWall = false;
            }
        }
    }
    json << "]";

    json << "}";
    return json.str();
}

std::string GameStateSerializer::serializeGameState(const Game* game) {
    if (!game) return "null";

    std::ostringstream json;
    json << "{";
    json << "\"status\":\"";

    switch (game->getStatus()) {
        case GameStatus::MENU: json << "menu"; break;
        case GameStatus::PLAYING: json << "playing"; break;
        case GameStatus::PAUSED: json << "paused"; break;
        case GameStatus::WIN: json << "win"; break;
        case GameStatus::LOSE: json << "lose"; break;
    }
    json << "\",";

    json << "\"timestamp\":" << std::time(nullptr);

    json << "}";
    return json.str();
}
