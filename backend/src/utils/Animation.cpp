/**
 * @file Animation.cpp
 * @brief 动画系统实现
 */

#include "../../include/utils/Animation.h"
#include <algorithm>

// ==================== AnimationClip ====================

AnimationClip::AnimationClip(const std::string& name, bool loop)
    : name_(name), currentFrameIndex_(0), currentFrameTime_(0.0f),
      loop_(loop), finished_(false) {
}

AnimationClip::~AnimationClip() {
}

void AnimationClip::addFrame(const std::string& framePath, float duration) {
    frames_.emplace_back(framePath, duration);
}

void AnimationClip::addFrames(const std::vector<std::string>& framePaths, float frameDuration) {
    for (const auto& path : framePaths) {
        addFrame(path, frameDuration);
    }
}

const AnimationFrame* AnimationClip::getCurrentFrame() const {
    if (frames_.empty()) return nullptr;
    return &frames_[currentFrameIndex_];
}

void AnimationClip::update(float deltaTime) {
    if (frames_.empty() || finished_) return;

    currentFrameTime_ += deltaTime;

    // 检查是否需要切换到下一帧
    while (currentFrameTime_ >= frames_[currentFrameIndex_].duration) {
        currentFrameTime_ -= frames_[currentFrameIndex_].duration;
        currentFrameIndex_++;

        // 检查是否播放完成
        if (currentFrameIndex_ >= static_cast<int>(frames_.size())) {
            if (loop_) {
                currentFrameIndex_ = 0;  // 循环
            } else {
                currentFrameIndex_ = frames_.size() - 1;  // 停在最后一帧
                finished_ = true;
                break;
            }
        }
    }
}

void AnimationClip::reset() {
    currentFrameIndex_ = 0;
    currentFrameTime_ = 0.0f;
    finished_ = false;
}

// ==================== AnimationController ====================

AnimationController::AnimationController()
    : currentAnimation_(nullptr), paused_(false) {
}

AnimationController::~AnimationController() {
    // 清理所有动画剪辑
    for (auto* anim : animations_) {
        delete anim;
    }
    animations_.clear();
}

void AnimationController::registerAnimation(AnimationClip* clip) {
    if (clip) {
        animations_.push_back(clip);

        // 如果还没有当前动画，设置为第一个
        if (!currentAnimation_ && !animations_.empty()) {
            currentAnimation_ = animations_[0];
        }
    }
}

void AnimationController::play(const std::string& animationName, bool forceRestart) {
    AnimationClip* anim = findAnimation(animationName);

    if (anim) {
        // 如果已经在播放这个动画，且不强制重启，则不做处理
        if (currentAnimation_ == anim && !forceRestart) {
            return;
        }

        currentAnimation_ = anim;

        if (forceRestart || currentAnimation_->isFinished()) {
            currentAnimation_->reset();
        }

        paused_ = false;
    }
}

void AnimationController::stop() {
    if (currentAnimation_) {
        currentAnimation_->reset();
    }
    paused_ = false;
}

void AnimationController::pause() {
    paused_ = true;
}

void AnimationController::resume() {
    paused_ = false;
}

void AnimationController::update(float deltaTime) {
    if (currentAnimation_ && !paused_) {
        currentAnimation_->update(deltaTime);
    }
}

const AnimationFrame* AnimationController::getCurrentFrame() const {
    if (currentAnimation_) {
        return currentAnimation_->getCurrentFrame();
    }
    return nullptr;
}

std::string AnimationController::getCurrentAnimationName() const {
    if (currentAnimation_) {
        return currentAnimation_->getName();
    }
    return "";
}

bool AnimationController::isPlaying(const std::string& animationName) const {
    if (currentAnimation_) {
        return currentAnimation_->getName() == animationName;
    }
    return false;
}

bool AnimationController::isCurrentAnimationFinished() const {
    if (currentAnimation_) {
        return currentAnimation_->isFinished();
    }
    return true;
}

AnimationClip* AnimationController::findAnimation(const std::string& name) {
    for (auto* anim : animations_) {
        if (anim->getName() == name) {
            return anim;
        }
    }
    return nullptr;
}
