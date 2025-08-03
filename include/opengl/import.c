#include "import.h"

int find_or_add_vertex(Vertex* vertices, int* verticesCount, Vertex v) {
    for (int i = 0; i < *verticesCount; i++) {
        if (memcmp(&vertices[i], &v, sizeof(Vertex)) == 0) {
            return i;  // Already exists
        }
    }

    // Not found, add new
    int index = (*verticesCount)++;
    vertices[index] = v;
    return index;
}

int check_duplicate_vert(Vertex* check, int count, Vertex* reference) {
    for (int i = 0; i < count; i++) {
        if (memcmp(&check[i], reference, sizeof(Vertex)) == 0) {
            return i;
        }
    }
    return -1;
}

// Texture* Import_loadTexturesFromMTL(const char* mtl_path, int* outCount) {



//     Texture* textures = malloc(sizeof(Texture) * count);
//     if (!textures) return NULL;
// }

Mesh Import_loadMeshFromOBJ(const char* obj_path) {
    Mesh mesh;

    FILE* file = fopen(obj_path, "r");
    if (!file) {
        printf("%s:1:error: could not open the file\n", obj_path);
        return mesh;
    }

    vec3* positions = malloc(sizeof(vec3) * 100000);
    int positionsCount = 0;
    
    vec3* normals = malloc(sizeof(vec3) * 100000);
    int normalsCount = 0;
    
    vec2* uvs = malloc(sizeof(vec2) * 100000);
    int uvsCount = 0;

    Vertex* vertices = malloc(sizeof(Vertex) * 100000);
    int verticesCount = 0;
    
    GLuint* indices =  malloc(sizeof(GLuint) * 100000);
    int indicesCount = 0;

    // Texture textures[10];
    // int texturesCount = 0;

    char line[256];
    int lineNum = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNum++;

        if (strncmp(line, "#", 1) == 0) {
            continue;
        } else if (strncmp(line, "v ", 2) == 0) {
            vec3 v;
            if (sscanf(line, "v %f %f %f", &v[0], &v[1], &v[2]) == 3) {
                // printf("v %f %f %f\n", v[0], v[1], v[2]);
                glm_vec3_copy(v, positions[positionsCount++]);
            } else {
                printf("%s:%d:error: broken position vertex\n", obj_path, lineNum);
                continue;
            }
        } else if (strncmp(line, "vt ", 3) == 0) {
            vec2 vt;
            if (sscanf(line, "vt %f %f", &vt[0], &vt[1]) == 2) {
                // printf("vt %f %f\n", vt[0], vt[1]);
                glm_vec2_copy(vt, uvs[uvsCount++]);
            } else {
                printf("%s:%d:error: broken uv vertex\n", obj_path, lineNum);
                continue;
            }
        } else if (strncmp(line, "vn ", 3) == 0) {
            vec3 vn;
            if (sscanf(line, "vn %f %f %f", &vn[0], &vn[1], &vn[2]) == 3) {
                // printf("vn %f %f %f\n", vn[0], vn[1], vn[2]);
                glm_vec3_copy(vn, normals[normalsCount++]);
            } else {
                printf("%s:%d:error: broken normal vertex\n", obj_path, lineNum);
                continue;
            }
        } else if (strncmp(line, "f ", 2) == 0) {
            char* token = strtok(line+2, " \t\r\n");

            Vertex verts[10];
            int numVerts = 0;

            while (token != NULL) {

                int vi, vti, vni;

                if (sscanf(token, "%d/%d/%d", &vi, &vti, &vni) == 3) {

                    vi--; vti--; vni--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy(normals[vni], verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy(uvs[vti], verts[numVerts].texUV);
                    // glm_vec3_copy((vec3){0.0f,0.0f,1.0f}, verts[numVerts].normal);
                    // glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    // glm_vec2_copy((vec2){0.0f,0.0f}, verts[numVerts].texUV);

                } else if (sscanf(token, "%d//%d", &vi, &vni) == 2) {

                    vi--; vni--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy(normals[vni], verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy((vec2){0.0f,0.0f}, verts[numVerts].texUV);

                } else if (sscanf(token, "%d", &vi) == 1) {

                    vi--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy((vec3){0.0f,0.0f,1.0f}, verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy((vec2){0.0f,0.0f}, verts[numVerts].texUV);

                } else {
                    printf("%s:%d:error: broken face vertex\n", obj_path, lineNum);
                }

                // printf("%d/%d/%d\n", vi, vti, vni);

                token = strtok(NULL, " \t\r\n");
                numVerts++;
            }

            for (int i = 1; i < numVerts - 1; i++) {
                int i0 = find_or_add_vertex(vertices, &verticesCount, verts[0]);
                int i1 = find_or_add_vertex(vertices, &verticesCount, verts[i]);
                int i2 = find_or_add_vertex(vertices, &verticesCount, verts[i + 1]);

                // indices[indicesCount++] = i0;
                indices[indicesCount++] = i1;
                indices[indicesCount++] = i0;
                indices[indicesCount++] = i2;
            }


            // indices

            // i, i+1, inc, i+2,
            // i, i+2, inc, i+3,

            
        } else if (strncmp(line, "mtllib ", 7) == 0) {
            
        } else if (strncmp(line, "usemtl ", 7) == 0) {
            continue;            
        } else if (strncmp(line, "vp ", 3) == 0) {
            printf("%s:%d:warning: perameter space vertexes not supported\n", obj_path, lineNum);
            continue;
        } else {
            printf("%s:%d:warning: unknown command -> %s", obj_path, lineNum, line);
            continue;

        }

    }


    fclose(file);

    // Texture textures[0];
    // textures[0] = Texture_Init("res/models/Gun/Gun.png", "diffuse", 0);

    
    // Texture textures[1];
    // textures[0] = Texture_Init("res/textures/brick.jpg", "diffuse", 0);
    // textures[1] = Texture_Init("res/textures/box_specular.png", "specular", 1);

    const char* textures[] = { "res/textures/brick.jpg", "diffuse" };
    mesh = Mesh_InitFromData(textures, 1, vertices, verticesCount, indices, indicesCount);

    free(positions);
    free(normals);
    free(uvs);
    free(vertices);
    free(indices);

    return mesh;
}