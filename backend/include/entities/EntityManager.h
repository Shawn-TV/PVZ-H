/**
 * @file EntityManager.h
 * @brief 实体管理器
 *
 * 负责：
 * - 管理所有游戏实体（僵尸、植物、戴夫、道具等）
 * - 统一更新和渲染调度
 * - 碰撞检测
 * - 实体生命周期管理
 */

#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "Entity.h"
#include "Zombie.h"
#include "Dave.h"
#include "Plant.h"
#include "Item.h"
#include <vector>
#include <string>

class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    // 更新所有实体
    void update(float deltaTime);

    // 碰撞检测
    void checkCollisions();

    // 添加实体
    void addZombie(Zombie* zombie);
    void addDave(Dave* dave);
    void addPlant(Plant* plant);
    void addItem(Item* item);
    void addEntity(Entity* entity);  // 通用添加

    // 移除实体
    void removeEntity(Entity* entity);
    void removeDeadEntities();  // 清理所有已死亡的实体

    // 获取实体
    Zombie* getZombie() const { return zombie_; }
    Dave* getDave() const { return dave_; }
    const std::vector<Plant*>& getPlants() const { return plants_; }
    const std::vector<Item*>& getItems() const { return items_; }
    const std::vector<Entity*>& getAllEntities() const { return allEntities_; }

    // 查找实体
    Entity* findEntityById(int id) const;
    std::vector<Entity*> findEntitiesInRange(const Vector2D& position, float range) const;

    // 清空所有实体
    void clear();

    // 统计信息
    size_t getEntityCount() const { return allEntities_.size(); }
    int getAliveEntityCount() const;

    // 序列化所有实体
    std::string toJson() const;

private:
    // 玩家和NPC（唯一）
    Zombie* zombie_;
    Dave* dave_;

    // 植物列表
    std::vector<Plant*> plants_;

    // 道具列表
    std::vector<Item*> items_;

    // 所有实体的总列表（包括上述所有）
    std::vector<Entity*> allEntities_;

    // 辅助函数
    void registerEntity(Entity* entity);
    void unregisterEntity(Entity* entity);
    bool checkCollision(Entity* a, Entity* b) const;
};

#endif // ENTITYMANAGER_H
