/**
 * @file EntityManager.cpp
 * @brief 实体管理器实现
 */

#include "../../include/entities/EntityManager.h"
#include "../../include/physics/Collision.h"
#include <algorithm>
#include <sstream>

EntityManager::EntityManager()
    : zombie_(nullptr), dave_(nullptr) {
}

EntityManager::~EntityManager() {
    clear();
}

void EntityManager::update(float deltaTime) {
    // 更新所有实体
    for (auto* entity : allEntities_) {
        if (entity && entity->isAlive()) {
            entity->update(deltaTime);
        }
    }

    // 检测碰撞
    checkCollisions();

    // 清理死亡实体
    removeDeadEntities();
}

void EntityManager::checkCollisions() {
    // 遍历所有实体对，检测碰撞
    for (size_t i = 0; i < allEntities_.size(); ++i) {
        Entity* entityA = allEntities_[i];
        if (!entityA || !entityA->isAlive()) continue;

        for (size_t j = i + 1; j < allEntities_.size(); ++j) {
            Entity* entityB = allEntities_[j];
            if (!entityB || !entityB->isAlive()) continue;

            // 检测碰撞
            if (checkCollision(entityA, entityB)) {
                // 通知双方发生碰撞
                entityA->onCollision(entityB);
                entityB->onCollision(entityA);
            }
        }
    }
}

bool EntityManager::checkCollision(Entity* a, Entity* b) const {
    if (!a || !b) return false;

    // 使用AABB碰撞检测
    return Collision::checkAABB(
        a->getPosition(), a->getWidth(), a->getHeight(),
        b->getPosition(), b->getWidth(), b->getHeight()
    );
}

// ==================== 添加实体 ====================

void EntityManager::addZombie(Zombie* zombie) {
    if (zombie_) {
        // 如果已经存在僵尸，先删除旧的
        removeEntity(zombie_);
        delete zombie_;
    }
    zombie_ = zombie;
    registerEntity(zombie);
}

void EntityManager::addDave(Dave* dave) {
    if (dave_) {
        // 如果已经存在戴夫，先删除旧的
        removeEntity(dave_);
        delete dave_;
    }
    dave_ = dave;
    registerEntity(dave);
}

void EntityManager::addPlant(Plant* plant) {
    if (plant) {
        plants_.push_back(plant);
        registerEntity(plant);
    }
}

void EntityManager::addItem(Item* item) {
    if (item) {
        items_.push_back(item);
        registerEntity(item);
    }
}

void EntityManager::addEntity(Entity* entity) {
    if (!entity) return;

    switch (entity->getType()) {
        case EntityType::ZOMBIE:
            addZombie(dynamic_cast<Zombie*>(entity));
            break;
        case EntityType::DAVE:
            addDave(dynamic_cast<Dave*>(entity));
            break;
        case EntityType::PLANT:
            addPlant(dynamic_cast<Plant*>(entity));
            break;
        case EntityType::ITEM:
            addItem(dynamic_cast<Item*>(entity));
            break;
        default:
            registerEntity(entity);
            break;
    }
}

void EntityManager::registerEntity(Entity* entity) {
    if (entity) {
        allEntities_.push_back(entity);
    }
}

// ==================== 移除实体 ====================

void EntityManager::removeEntity(Entity* entity) {
    if (!entity) return;

    // 从总列表中移除
    auto it = std::find(allEntities_.begin(), allEntities_.end(), entity);
    if (it != allEntities_.end()) {
        allEntities_.erase(it);
    }

    // 从特定类型列表中移除
    switch (entity->getType()) {
        case EntityType::ZOMBIE:
            if (zombie_ == entity) {
                zombie_ = nullptr;
            }
            break;

        case EntityType::DAVE:
            if (dave_ == entity) {
                dave_ = nullptr;
            }
            break;

        case EntityType::PLANT: {
            auto plantIt = std::find(plants_.begin(), plants_.end(), dynamic_cast<Plant*>(entity));
            if (plantIt != plants_.end()) {
                plants_.erase(plantIt);
            }
            break;
        }

        case EntityType::ITEM: {
            auto itemIt = std::find(items_.begin(), items_.end(), dynamic_cast<Item*>(entity));
            if (itemIt != items_.end()) {
                items_.erase(itemIt);
            }
            break;
        }

        default:
            break;
    }
}

void EntityManager::removeDeadEntities() {
    // 收集需要删除的实体
    std::vector<Entity*> toRemove;

    for (auto* entity : allEntities_) {
        if (entity && !entity->isAlive()) {
            // 僵尸死亡不立即删除（用于游戏结束判定）
            if (entity->getType() == EntityType::ZOMBIE) {
                continue;
            }

            // 戴夫永远不会被删除
            if (entity->getType() == EntityType::DAVE) {
                continue;
            }

            toRemove.push_back(entity);
        }
    }

    // 删除实体
    for (auto* entity : toRemove) {
        removeEntity(entity);
        delete entity;
    }
}

// ==================== 查找实体 ====================

Entity* EntityManager::findEntityById(int id) const {
    for (auto* entity : allEntities_) {
        if (entity && entity->getId() == id) {
            return entity;
        }
    }
    return nullptr;
}

std::vector<Entity*> EntityManager::findEntitiesInRange(const Vector2D& position, float range) const {
    std::vector<Entity*> result;

    for (auto* entity : allEntities_) {
        if (entity && entity->isAlive()) {
            float distance = position.distance(entity->getPosition());
            if (distance <= range) {
                result.push_back(entity);
            }
        }
    }

    return result;
}

// ==================== 清空 ====================

void EntityManager::clear() {
    // 删除所有实体
    for (auto* entity : allEntities_) {
        delete entity;
    }

    allEntities_.clear();
    plants_.clear();
    items_.clear();
    zombie_ = nullptr;
    dave_ = nullptr;
}

// ==================== 统计 ====================

int EntityManager::getAliveEntityCount() const {
    int count = 0;
    for (auto* entity : allEntities_) {
        if (entity && entity->isAlive()) {
            count++;
        }
    }
    return count;
}

// ==================== 序列化 ====================

std::string EntityManager::toJson() const {
    std::stringstream ss;
    ss << "{";

    // 僵尸
    if (zombie_) {
        ss << "\"zombie\":" << zombie_->toJson() << ",";
    } else {
        ss << "\"zombie\":null,";
    }

    // 戴夫
    if (dave_) {
        ss << "\"dave\":" << dave_->toJson() << ",";
    } else {
        ss << "\"dave\":null,";
    }

    // 植物
    ss << "\"plants\":[";
    for (size_t i = 0; i < plants_.size(); ++i) {
        if (plants_[i]) {
            ss << plants_[i]->toJson();
            if (i < plants_.size() - 1) ss << ",";
        }
    }
    ss << "],";

    // 道具
    ss << "\"items\":[";
    for (size_t i = 0; i < items_.size(); ++i) {
        if (items_[i]) {
            ss << items_[i]->toJson();
            if (i < items_.size() - 1) ss << ",";
        }
    }
    ss << "]";

    ss << "}";
    return ss.str();
}
