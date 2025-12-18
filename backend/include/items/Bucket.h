/**
 * @file Bucket.h
 * @brief 铁桶道具
 *
 * 功能：
 * - 增加僵尸防御力
 * - 提供额外护甲值
 * - 可装备在头部
 */

#ifndef BUCKET_H
#define BUCKET_H

#include "../entities/Item.h"

class Bucket : public Item {
public:
    Bucket(float x, float y);
    ~Bucket();

    void use(class Zombie* zombie) override;

private:
    int armorValue_;
};

#endif // BUCKET_H
