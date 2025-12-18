/**
 * @file Timer.h
 * @brief 计时器工具
 *
 * 功能：
 * - 计算帧间隔时间（deltaTime）
 * - FPS统计
 * - 游戏时间管理
 */

#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
public:
    Timer();

    void start();
    void tick();
    float getDeltaTime() const;
    float getFPS() const;

private:
    std::chrono::high_resolution_clock::time_point startTime_;
    std::chrono::high_resolution_clock::time_point lastTime_;
    float deltaTime_;
    float fps_;
};

#endif // TIMER_H
