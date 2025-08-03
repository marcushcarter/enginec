#include "sprite.h"

Sprite Sprite_Init(const char** filepaths, int count) {
    Sprite sprite;

    int width, height, channels;
    stbi_uc* pixels = stbi_load(filepaths[0], &width, &height, &channels, 4);
    stbi_image_free(pixels);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, count, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    for (int i = 0; i < count; i++) {
        int widthImg, heightImg, numColCh;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* bytes = stbi_load(filepaths[i], &widthImg, &heightImg, &numColCh, 4);
        if (!bytes) {
            fprintf(stderr, "Failed to load imageFile: %s\nReason: %s\n", filepaths[i], stbi_failure_reason());
            exit(1); // or return a default texture
        }
        unsigned char* resized = malloc(width * height * 4);
        stbir_resize_uint8(bytes, widthImg, heightImg, 0, resized, width, height, 0, 4);

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, resized);

        stbi_image_free(bytes);
        free(resized);
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    sprite.textureArray = tex;
    sprite.width = width;
    sprite.height = height;
    sprite.count = count;

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


    return sprite;
}

// void Sprite_Draw(Sprite* sprite, Shader* shader, SpriteInstance* inst, VAO* vao) {
//     Shader_Activate(shader);
//     VAO_Bind(vao);
//     glBindTexture(GL_TEXTURE_2D_ARRAY, sprite->textureArray);

//     // Construct transformation matrix
//     mat4 model = GLM_MAT4_IDENTITY_INIT;
//     glm_translate(model, (vec3){ inst->position[0], inst->position[1], 0.0f });
//     glm_translate(model, (vec3){ 0.5f * inst->size[0], 0.5f * inst->size[1], 0.0f });
//     glm_rotate_z(model, glm_rad(inst->rotation), model);
//     glm_translate(model, (vec3){ -0.5f * inst->size[0], -0.5f * inst->size[1], 0.0f });
//     glm_scale(model, (vec3){ inst->size[0], inst->size[1], 1.0f });

    
//     mat4 proj = GLM_MAT4_IDENTITY_INIT;
//     glm_ortho(0.0f, 1600, 1000, 0.0f, -1.0f, 1.0f, proj); // Top-left is (0,0)
//     glUniformMatrix4fv(glGetUniformLocation(shader->ID, "uProjection"), 1, GL_FALSE, (float*)proj);

//     // Set uniforms
//     glUniformMatrix4fv(glGetUniformLocation(shader->ID, "uModel"), 1, GL_FALSE, (float*)model);
//     glUniform4fv(glGetUniformLocation(shader->ID, "uColor"), 1, (float*)inst->color);
//     glUniform1i(glGetUniformLocation(shader->ID, "uLayer"), inst->layer);
//     glUniform1i(glGetUniformLocation(shader->ID, "uTexArray"), 0); // texture unit 0

//     glDrawArrays(GL_TRIANGLES, 0, 6);

//     glBindVertexArray(0);
// }

void Sprite_Draw(Sprite* sprite, Shader* shader, SpriteInstance* inst, mat4 projection, VAO* vao) {
    Shader_Activate(shader);
    VAO_Bind(vao);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sprite->textureArray);

    glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, (float*)projection);
    glUniform2f(glGetUniformLocation(shader->ID, "position"), inst->position[0], inst->position[1]);
    glUniform2f(glGetUniformLocation(shader->ID, "size"), inst->size[0], inst->size[1]);
    glUniform1f(glGetUniformLocation(shader->ID, "rotation"), inst->rotation);
    glUniform1i(glGetUniformLocation(shader->ID, "layer"), inst->layer);
    glUniform4f(glGetUniformLocation(shader->ID, "spriteColor"), inst->color[0], inst->color[1], inst->color[2], inst->color[3]);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
