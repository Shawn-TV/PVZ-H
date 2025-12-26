/**
 * @file WallNut.cpp
 * @brief 坚果墙实现
 */

#include "../../include/plants/WallNut.h"

WallNut::WallNut(float x, float y)
    : Plant(x, y, PlantType::WALL_NUT) {

    // 坚果墙属性 - 超高生命值
    health_ = 800.0f;
    maxHealth_ = 800.0f;

    // 坚果墙不攻击
    attackCooldown_ = 0;
    attackRange_ = 0;
    attackDamage_ = 0;
    attackDirection_ = Direction::NONE;

    // 坚果墙体型较大
    setSize(40, 40);

    // 初始化动画
    initializeAnimations();
}

WallNut::~WallNut() {
}

void WallNut::performAttack() {
    // 坚果墙不攻击，不做任何事
}

int WallNut::getDamageState() const {
    float healthPercent = health_ / maxHealth_;

    if (healthPercent > 0.66f) {
        return 0;  // 完好
    } else if (healthPercent > 0.33f) {
        return 1;  // 轻伤
    } else {
        return 2;  // 重伤
    }
}

void WallNut::initializeAnimations() {
    // ===== 坚果墙动画（根据受损程度不同） =====

    // 完好状态（循环，轻微摇晃）
    AnimationClip* healthyAnim = new AnimationClip("healthy", true);
    healthyAnim->addFrame("assets/images/plants/wallnut/healthy/frame_0.png", 0.3f);
    healthyAnim->addFrame("assets/images/plants/wallnut/healthy/frame_1.png", 0.3f);
    healthyAnim->addFrame("assets/images/plants/wallnut/healthy/frame_2.png", 0.3f);
    animationController_.registerAnimation(healthyAnim);

    // 轻伤状态（循环）
    AnimationClip* damagedAnim = new AnimationClip("damaged", true);
    damagedAnim->addFrame("assets/images/plants/wallnut/damaged/frame_0.png", 0.3f);
    damagedAnim->addFrame("assets/images/plants/wallnut/damaged/frame_1.png", 0.3f);
    damagedAnim->addFrame("assets/images/plants/wallnut/damaged/frame_2.png", 0.3f);
    animationController_.registerAnimation(damagedAnim);

    // 重伤状态（循环）
    AnimationClip* criticalAnim = new AnimationClip("critical", true);
    criticalAnim->addFrame("assets/images/plants/wallnut/critical/frame_0.png", 0.3f);
    criticalAnim->addFrame("assets/images/plants/wallnut/critical/frame_1.png", 0.3f);
    criticalAnim->addFrame("assets/images/plants/wallnut/critical/frame_2.png", 0.3f);
    animationController_.registerAnimation(criticalAnim);

    // 默认播放完好状态
    animationController_.play("healthy");
}

void WallNut::updateAnimation() {
    // 根据生命值百分比选择动画
    int damageState = getDamageState();

    switch (damageState) {
        case 0:  // 完好
            if (!animationController_.isPlaying("healthy")) {
                animationController_.play("healthy");
            }
            break;

        case 1:  // 轻伤
            if (!animationController_.isPlaying("damaged")) {
                animationController_.play("damaged");
            }
            break;

        case 2:  // 重伤
            if (!animationController_.isPlaying("critical")) {
                animationController_.play("critical");
            }
            break;
    }
}
