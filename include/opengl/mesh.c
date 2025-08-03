#include "mesh.h"

Mesh Mesh_Init(VertexVector vertices, GLuintVector indices, TextureVector textures) {
    Mesh mesh;

    // mesh.vertices = malloc(sizeof(Vertex) * vertices.size);
    // memcpy(mesh.vertices, vertices.data, sizeof(Vertex) * vertices.size);

    // mesh.indices = malloc(sizeof(GLuint) * indices.size);
    // memcpy(mesh.indices, indices.data, sizeof(GLuint) * indices.size);

    // mesh.textures = malloc(sizeof(Texture) * textures.size);
    // memcpy(mesh.textures, textures.data, sizeof(Texture) * textures.size);

    mesh.vertices = vertices;
    mesh.indices = indices;
    mesh.textures = textures;

    VAO VAO1 = VAO_Init();
    VAO_Bind(&VAO1);
    VBO VBO1 = VBO_Init(&vertices);
    EBO EBO1 = EBO_Init(&indices);
    VAO_LinkAttrib(&VBO1, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    VAO_LinkAttrib(&VBO1, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    VAO_LinkAttrib(&VBO1, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    VAO_LinkAttrib(&VBO1, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
    VAO_Unbind();
    VBO_Unbind();
    EBO_Unbind();

    mesh.vao = VAO1;

    return mesh;
}


Mesh Mesh_InitFromData(const char** texbuffer, int texcount, Vertex* vertices, int vertcount, GLuint* indices, int indcount) {

    VertexVector verts;
    GLuintVector inds;
    TextureVector texs;

    Texture textures[texcount];
    for (int i = 0; i < texcount; i++) {
        textures[i] = Texture_Init(texbuffer[i*2], texbuffer[i*2+1], i);
    }

    VertexVector_Copy(vertices, vertcount, &verts);
    GLuintVector_Copy(indices, indcount, &inds);
    TextureVector_Copy(textures, texcount, &texs);

    Mesh mesh = Mesh_Init(verts, inds, texs);

    return mesh;
}

// Mesh Mesh_InitFromData(Texture* textures, int texcount, Vertex* vertices, int vertexcount, GLuint* indices, int indexcount) {

//     VertexVector verts;
//     VertexVector_Copy(vertices, vertexcount, &verts);

//     GLuintVector inds;
//     GLuintVector_Copy(indices, indexcount, &inds);

//     TextureVector texs;
//     TextureVector_Copy(textures, texcount, &texs);

//     Mesh mesh = Mesh_Init(&verts, &inds, &texs);

//     return mesh;
// }

void Mesh_Draw(Mesh* mesh, Shader* shader, Camera* camera) {
    Shader_Activate(shader);
    VAO_Bind(&mesh->vao);

    

    unsigned int numDiffuse = 0;
    unsigned int numSpecular = 0;

    if (mesh->textures.data) {
        for (unsigned int i = 0; i < mesh->textures.size; i++) {
            char num[16];
            char uniformName[256];
            char* type = mesh->textures.data[i].type;
            if (strcmp(type, "diffuse") == 0) {
                sprintf(num, "%d", numDiffuse++);
            } else if (strcmp(type, "specular") == 0) {
                sprintf(num, "%d", numSpecular++);
            }
            snprintf(uniformName, sizeof(uniformName), "%s%s", type, num);
            Texture_texUnit(shader, uniformName, i);
            Texture_Bind(&mesh->textures.data[i]);
        }
    }
    glUniform3fv(glGetUniformLocation(shader->ID, "camPos"), 1, (float*)camera->Position); 
    Camera_Matrix(camera, shader, "camMatrix");

    glDrawElements(GL_TRIANGLES, mesh->indices.size, GL_UNSIGNED_INT, 0);
}