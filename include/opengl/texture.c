#include "texture.h"
#include <string.h>
#include "error.h"

Texture Texture_Init(const char* imageFile, const char* texType, GLuint slot) {
    Texture texture;

    texture.type = malloc(strlen(texType) + 1);
    if (!texture.type) {
        MSG_FATAL(imageFile, 1, "could not allocate memory for texture type");
        exit(1);
    }
    strcpy(texture.type, texType);

    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* bytes = stbi_load(imageFile, &widthImg, &heightImg, &numColCh, 0);
    if (!bytes) {
        MSG_ERROR(imageFile, 1, "failed to load texture: '%s", stbi_failure_reason());
        exit(1);
    }

    glGenTextures(1, &texture.ID);
    glActiveTexture(GL_TEXTURE0 + slot);
    texture.unit = slot;
    glBindTexture(GL_TEXTURE_2D, texture.ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum format;
    if (numColCh == 4) format = GL_RGBA;
    else if (numColCh == 3) format = GL_RGB;
    else if (numColCh == 1) format = GL_RED;
    else {
        MSG_ERROR(imageFile, 1, "unsupported color channel count '%d", numColCh);
        stbi_image_free(bytes);
        exit(1);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, widthImg, heightImg, 0, format, GL_UNSIGNED_BYTE, bytes);
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