#include "texture_manager.hpp"
#include "stb_image.h"
#include <iostream>

TextureManager::TextureManager() {}

TextureManager::~TextureManager() {
    clear();
}

GLuint TextureManager::loadTexture(const std::string& key, const std::string& filepath) {
    // Kiểm tra nếu texture đã được tải
    auto it = textures.find(key);
    if (it != textures.end()) {
        return it->second;
    }

    std::cout << "Loading texture: " << filepath << "\n";
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
    if (!image) {
        std::cerr << "Failed to load texture: " << filepath << "\n";
        std::cerr << "Reason: " << stbi_failure_reason() << "\n";
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after loading texture: " << err << "\n";
    }

    stbi_image_free(image);
    textures[key] = textureID;
    return textureID;
}

GLuint TextureManager::getTexture(const std::string& key) const {
    auto it = textures.find(key);
    if (it != textures.end()) {
        return it->second;
    }
    return 0;
}

void TextureManager::clear() {
    for (auto& pair : textures) {
        glDeleteTextures(1, &pair.second);
    }
    textures.clear();
}
