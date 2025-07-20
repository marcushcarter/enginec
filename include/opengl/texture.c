#include "opengl/texture.h"

Texture Texture_Init(const char* image, GLenum texType, GLuint slot, GLenum format, GLenum pixelType) {
    Texture texture;

    texture.type = texType;
    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

    glGenTextures(1, &texture.ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    texture.unit = slot;
    glBindTexture(texType, texture.ID);

    glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(texType, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
    glGenerateMipmap(texType);

    stbi_image_free(bytes);
    glBindTexture(texType, 0);

    return texture;
}

void Texture_texUnit(Shader* shader, const char* uniform, GLuint unit) {
    GLuint tex0Uni = glGetUniformLocation(shader->ID, uniform);
    Shader_Activate(shader);
    glUniform1i(tex0Uni, unit);
}

void Texture_Bind(Texture* texture) {
    glActiveTexture(GL_TEXTURE0 + texture->unit);
    glBindTexture(texture->type, texture->ID);
}

void Texture_Unbind(Texture* texture) {
    glBindTexture(texture->type, 0);
}

void Texture_Delete(Texture* texture) {
    glDeleteTextures(1, &texture->ID);
}