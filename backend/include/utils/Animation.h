/**
 * @file Animation.h
 * @brief 动画系统
 *
 * 负责：
 * - 管理多帧动画播放
 * - 帧切换和循环
 * - 动画状态管理
 */

#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include <vector>

/**
 * 单个动画帧
 */
struct AnimationFrame {
    std::string framePath;  // 帧图片路径（相对于assets目录）
    float duration;         // 该帧持续时间（秒）

    AnimationFrame(const std::string& path, float dur = 0.1f)
        : framePath(path), duration(dur) {}
};

/**
 * 动画剪辑（一组连续的帧）
 */
class AnimationClip {
public:
    AnimationClip(const std::string& name, bool loop = true);
    ~AnimationClip();

    // 添加帧
    void addFrame(const std::string& framePath, float duration = 0.1f);
    void addFrames(const std::vector<std::string>& framePaths, float frameDuration = 0.1f);

    // 获取当前帧
    const AnimationFrame* getCurrentFrame() const;
    int getCurrentFrameIndex() const { return currentFrameIndex_; }
    int getFrameCount() const { return frames_.size(); }

    // 更新动画
    void update(float deltaTime);

    // 重置动画
    void reset();

    // 设置/获取属性
    void setLoop(bool loop) { loop_ = loop; }
    bool isLoop() const { return loop_; }
    bool isFinished() const { return finished_; }
    std::string getName() const { return name_; }

private:
    std::string name_;                      // 动画名称
    std::vector<AnimationFrame> frames_;    // 帧列表
    int currentFrameIndex_;                 // 当前帧索引
    float currentFrameTime_;                // 当前帧已播放时间
    bool loop_;                             // 是否循环
    bool finished_;                         // 是否播放完成
};

/**
 * 动画控制器
 * 管理实体的所有动画剪辑，控制播放、切换
 */
class AnimationController {
public:
    AnimationController();
    ~AnimationController();

    // 注册动画剪辑
    void registerAnimation(AnimationClip* clip);

    // 播放动画
    void play(const std::string& animationName, bool forceRestart = false);

    // 停止当前动画
    void stop();

    // 暂停/继续
    void pause();
    void resume();

    // 更新（在Entity::update中调用）
    void update(float deltaTime);

    // 获取当前动画帧
    const AnimationFrame* getCurrentFrame() const;

    // 获取当前动画名称
    std::string getCurrentAnimationName() const;

    // 检查是否在播放某个动画
    bool isPlaying(const std::string& animationName) const;

    // 检查当前动画是否完成
    bool isCurrentAnimationFinished() const;

private:
    std::vector<AnimationClip*> animations_;  // 所有动画剪辑
    AnimationClip* currentAnimation_;         // 当前播放的动画
    bool paused_;                             // 是否暂停

    // 查找动画
    AnimationClip* findAnimation(const std::string& name);
};

#endif // ANIMATION_H
