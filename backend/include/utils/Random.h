/**
 * @file Random.h
 * @brief 随机数生成工具
 *
 * 功能：
 * - 生成随机整数
 * - 生成随机浮点数
 * - 随机位置生成
 */

#ifndef RANDOM_H
#define RANDOM_H

#include <random>

class Random {
public:
    static void initialize();

    static int randomInt(int min, int max);
    static float randomFloat(float min, float max);
    static bool randomBool(float probability = 0.5f);

private:
    static std::mt19937 generator_;
};

#endif // RANDOM_H
