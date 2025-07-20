#include "opengl/texture.h"
#include <string.h>

Texture Texture_Init(const char* image, char* texType, GLuint slot, GLenum format, GLenum pixelType) {
    Texture texture;

    texture.type = malloc(strlen(texType) + 1);  // +1 for null terminator
    if (!texture.type) {
        fprintf(stderr, "Failed to allocate memory for texture type.\n");
        exit(1); // or handle it however u want
    }
    strcpy(texture.type, texType);

    // texture.type = GL_TEXTURE_2D;
    // strncpy(texture.type, texType, sizeof(texture.type) - 1);
    // texture.type[sizeof(texture.type) - 1] = '\0';
    // strcpy(texture.type, texType);
    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* bytes = stbi_load(image, &widthImg, &heightImg, &numColCh, 0);

    glGenTextures(1, &texture.ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    texture.unit = slot;
    glBindTexture(GL_TEXTURE_2D, texture.ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(bytes);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void Texture_texUnit(Shader* shader, const char* uniform, GLuint unit) {
    GLuint tex0Uni = glGetUniformLocation(shader->ID, uniform);
    Shader_Activate(shader);
    glUniform1i(tex0Uni, unit);
}

void Texture_Bind(Texture* texture) {
    glActiveTexture(GL_TEXTURE0 + texture->unit);
    glBindTexture(GL_TEXTURE_2D, texture->ID);
}

void Texture_Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture_Delete(Texture* texture) {
    glDeleteTextures(1, &texture->ID);
}