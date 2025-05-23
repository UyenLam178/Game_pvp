#include "animation.hpp"
#include <GLFW/glfw3.h>

bool AnimationController::isPlaying(const std::string& name) const {
    return currentAnimation == name;
}

AnimationController::AnimationController(TextureManager& tm)
    : textureManager(tm), animationStartTime(0.0f), currentFrameIndex(0) {
}

void AnimationController::addAnimation(const std::string& name, const std::vector<std::string>& textureKeys,
    float frameDuration, bool loop, int frameCount, bool isSpriteSheet, int startFrame) {
    animations.emplace(name, Animation(textureKeys, frameDuration, loop, frameCount, isSpriteSheet, startFrame));
}

void AnimationController::playAnimation(const std::string& name, float currentTime) {
    if (animations.find(name) != animations.end() && name != currentAnimation) {
        currentAnimation = name;
        animationStartTime = currentTime;
        currentFrameIndex = 0;
    }
}

FrameResult AnimationController::getCurrentFrame() const {
    if (currentAnimation.empty() || animations.find(currentAnimation) == animations.end()) {
        return { 0, ImVec2(0, 0), ImVec2(1, 1) };
    }

    const Animation& anim = animations.at(currentAnimation);
    GLuint texID = textureManager.getTexture(anim.textureKeys[0]);

    if (!anim.isSpriteSheet || anim.frameCount <= 1) {
        return { texID, ImVec2(0, 0), ImVec2(1, 1) };
    }

    float frameWidth = 1.0f / anim.frameCount;
    size_t actualFrame = anim.startFrame + currentFrameIndex;

    float u0 = frameWidth * actualFrame;
    float u1 = u0 + frameWidth;

    return { texID, ImVec2(u0, 0), ImVec2(u1, 1) };
}

void AnimationController::update(float currentTime) {
    if (currentAnimation.empty() || animations.find(currentAnimation) == animations.end()) return;

    const Animation& anim = animations.at(currentAnimation);
    float elapsed = currentTime - animationStartTime;

    size_t frameCount = anim.isSpriteSheet ? anim.frameCount : anim.textureKeys.size();
    size_t newIndex = static_cast<size_t>(elapsed / anim.frameDuration);

    if (anim.loop) {
        currentFrameIndex = newIndex % frameCount;
    }
    else {
        currentFrameIndex = std::min(newIndex, frameCount - 1);
    }
}

bool AnimationController::hasFinished(const std::string& name, float currentTime) const {
    if (animations.find(name) == animations.end()) return true;
    const Animation& anim = animations.at(name);
    if (anim.loop) return false;

    float elapsed = currentTime - animationStartTime;
    size_t frameCount = anim.isSpriteSheet ? anim.frameCount : anim.textureKeys.size();
    size_t maxFrameIndex = frameCount - 1;

    return static_cast<size_t>(elapsed / anim.frameDuration) > maxFrameIndex;
}
