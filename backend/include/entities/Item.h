/**
 * @file Item.h
 * @brief 道具基类
 *
 * 道具类型：
 * - 撑杆跳套装（PoleVaultKit）：让僵尸变身为撑杆跳僵尸，跑动速度更快
 * - 铁桶（Bucket）：增加防御值，保护僵尸
 * - 生命药水（HealthPotion）：恢复生命值
 */

#ifndef ITEM_H
#define ITEM_H

#include "Entity.h"
#include <string>

/**
 * 道具类型枚举
 */
enum class ItemType {
    POLE_VAULT_KIT,     // 撑杆跳套装
    BUCKET,             // 铁桶
    HEALTH_POTION,      // 生命药水（红色）
    SPEED_POTION        // 速度药水（绿色）
};

/**
 * 道具基类
 */
class Item : public Entity {
public:
    Item(float x, float y, ItemType itemType);
    virtual ~Item();

    // 实现Entity接口
    void update(float deltaTime) override;
    void onCollision(Entity* other) override;
    void initializeAnimations() override;
    void updateAnimation() override;
    std::string toJson() const override;

    // 道具特定方法
    ItemType getItemType() const { return itemType_; }
    bool isPickedUp() const { return pickedUp_; }
    void setPickedUp(bool picked) { pickedUp_ = picked; }

    // 拾取免疫（防止掉落后立即被拾取）
    bool canBePickedUp() const { return pickupImmunityTimer_ <= 0 && !pickedUp_; }
    void setPickupImmunity(float seconds) { pickupImmunityTimer_ = seconds; }

    /**
     * 应用道具效果到僵尸
     * @param zombie 拾取道具的僵尸
     * @return 是否成功应用（是否消耗道具）
     */
    virtual bool applyEffect(class Zombie* zombie) = 0;

protected:
    ItemType itemType_;     // 道具类型
    bool pickedUp_;         // 是否已被拾取
    float lifetime_;        // 存在时间（某些道具可能会消失）
    float maxLifetime_;     // 最大存在时间（-1表示永久）
    float pickupImmunityTimer_;  // 拾取免疫时间（秒）
};

/**
 * 撑杆跳套装
 * 效果：让僵尸变身为撑杆跳僵尸，移动速度更快，动画改为跑动
 */
class PoleVaultKit : public Item {
public:
    PoleVaultKit(float x, float y);
    ~PoleVaultKit() override;

    bool applyEffect(class Zombie* zombie) override;
    void initializeAnimations() override;
};

/**
 * 铁桶
 * 效果：增加防御值，保护僵尸头部
 */
class Bucket : public Item {
public:
    Bucket(float x, float y, float armorValue = 200.0f);  // 支持自定义护甲值
    ~Bucket() override;

    bool applyEffect(class Zombie* zombie) override;
    void initializeAnimations() override;

    // 铁桶属性
    float getBucketArmor() const { return bucketArmor_; }
    void setBucketArmor(float armor) { bucketArmor_ = armor; }

private:
    float bucketArmor_;     // 铁桶提供的护甲值
};

/**
 * 生命药水（红色）
 * 效果：恢复40%最大生命值
 */
class HealthPotion : public Item {
public:
    HealthPotion(float x, float y);
    ~HealthPotion() override;

    bool applyEffect(class Zombie* zombie) override;
    void initializeAnimations() override;

    // 药水属性
    float getHealPercentage() const { return healPercentage_; }

private:
    float healPercentage_;  // 恢复的生命值百分比（0.4 = 40%）
};

/**
 * 速度药水（绿色）
 * 效果：速度提升至1.5倍，持续30秒，重复拾取叠加持续时间
 */
class SpeedPotion : public Item {
public:
    SpeedPotion(float x, float y);
    ~SpeedPotion() override;

    bool applyEffect(class Zombie* zombie) override;
    void initializeAnimations() override;

    // 药水属性
    float getSpeedMultiplier() const { return speedMultiplier_; }
    float getDuration() const { return duration_; }

private:
    float speedMultiplier_;  // 速度倍率（1.5 = 1.5倍）
    float duration_;         // 持续时间（30秒）
};

#endif // ITEM_H
