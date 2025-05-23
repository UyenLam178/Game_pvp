#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <GL/glew.h>
#include <string>
#include <map>

class TextureManager {
public:
    TextureManager();
    ~TextureManager();

    // Tải texture từ file và lưu vào map với key
    GLuint loadTexture(const std::string& key, const std::string& filepath);
    // Lấy texture ID theo key
    GLuint getTexture(const std::string& key) const;
    // Xóa tất cả texture
    void clear();

private:
    std::map<std::string, GLuint> textures;
};

#endif // TEXTURE_MANAGER_HPP
