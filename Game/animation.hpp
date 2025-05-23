#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <vector>
#include <string>
#include <map>
#include "texture_manager.hpp"
#include "imgui.h"

struct FrameResult {
    GLuint textureID;
    ImVec2 uv0;
    ImVec2 uv1;
};

struct Animation {
    std::vector<std::string> textureKeys;
    float frameDuration;
    bool loop;
    int frameCount = 1;
    bool isSpriteSheet = false;
    int startFrame = 0;

    Animation(const std::vector<std::string>& keys, float duration, bool loop = true,
        int count = 1, bool sheet = false, int start = 0)
        : textureKeys(keys), frameDuration(duration), loop(loop),
        frameCount(count), isSpriteSheet(sheet), startFrame(start) {
    }
};

class AnimationController {
public:
    AnimationController(TextureManager& textureManager);
    void addAnimation(const std::string& name, const std::vector<std::string>& textureKeys,
        float frameDuration, bool loop = true,
        int frameCount = 1, bool isSpriteSheet = false, int startFrame = 0);
    void playAnimation(const std::string& name, float currentTime);
    FrameResult getCurrentFrame() const;
    void update(float currentTime);
    bool isPlaying(const std::string& name) const; // Add this declaration
    bool hasFinished(const std::string& name, float currentTime) const;

private:
    TextureManager& textureManager;
    std::map<std::string, Animation> animations;
    std::string currentAnimation;
    float animationStartTime;
    size_t currentFrameIndex;
};

#endif // ANIMATION_HPP
