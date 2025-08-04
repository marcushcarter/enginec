#include "import.h"
#include <stdlib.h>
#include <string.h>

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

int count_face_vertices(const char* line) {
    const char* ptr = line + 2;  // skip "f "
    int count = 0;

    while (*ptr) {
        // Skip any leading whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n') ptr++;
        if (*ptr == '\0') break;

        // Token start
        const char* token_start = ptr;

        // Move to end of token
        while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\r' && *ptr != '\n') ptr++;

        // Parse the token without modifying the string
        char token[64];
        size_t len = ptr - token_start;
        if (len >= sizeof(token)) len = sizeof(token) - 1;
        memcpy(token, token_start, len);
        token[len] = '\0';

        // Count if it's a valid vertex (vi, vi/vti, etc.)
        int vi, vti, vni;
        if (sscanf(token, "%d/%d/%d", &vi, &vti, &vni) == 3 ||
            sscanf(token, "%d//%d", &vi, &vni) == 2 ||
            sscanf(token, "%d/%d", &vi, &vti) == 2 ||
            sscanf(token, "%d", &vi) == 1) {
            count++;
        } else {
            printf("Warning: malformed face token '%s'\n", token);
        }
    }

    return count;
}


void replacePathSuffix(const char* path, const char* newsuffix, char* dest, int destsize) {
    const char* lastSlash = strrchr(path, '/');

    if (!lastSlash) {
        snprintf(dest, destsize, "%s", newsuffix);
        return;
    }

    int dirLen = lastSlash - path + 1;

    if (dirLen >= destsize) dirLen = destsize - 1;

    strncpy(dest, path, dirLen);
    dest[dirLen] = '\0';

    strncat(dest, newsuffix, destsize - strlen(dest) - 1);
}

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

    const char** textures;
    int texturesCount = 0;

    char line[546];
    int lineNum = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNum++;

        if (strncmp(line, "#", 1) == 0) {

            continue;

        } else if (strncmp(line, "mtllib ", 7) == 0) {
            char mtl_file[256] = {0};
            char mtl_filepath[256] = {0};

            sscanf(line, "mtllib %s", mtl_file);
            replacePathSuffix(obj_path, mtl_file, mtl_filepath, sizeof(mtl_filepath));

            textures = Mesh_getTexturesFromMTL(mtl_filepath, &texturesCount);

        } else if (strncmp(line, "v ", 2) == 0) {

            vec3 v;
            if (sscanf(line, "v %f %f %f", &v[0], &v[1], &v[2]) == 3) {
                glm_vec3_copy(v, positions[positionsCount++]);
            } else {
                printf("%s:%d:error: broken position vertex\n", obj_path, lineNum);
                continue;
            }

        } else if (strncmp(line, "vt ", 3) == 0) {

            vec2 vt;
            if (sscanf(line, "vt %f %f", &vt[0], &vt[1]) == 2) {
                glm_vec2_copy(vt, uvs[uvsCount++]);
            } else {
                printf("%s:%d:error: broken uv vertex\n", obj_path, lineNum);
                continue;
            }

        } else if (strncmp(line, "vn ", 3) == 0) {

            vec3 vn;
            if (sscanf(line, "vn %f %f %f", &vn[0], &vn[1], &vn[2]) == 3) {
                glm_vec3_copy(vn, normals[normalsCount++]);
            } else {
                printf("%s:%d:error: broken normal vertex\n", obj_path, lineNum);
                continue;
            }

        } else if (strncmp(line, "f ", 2) == 0) {
            
            int faceVertCount = count_face_vertices(line);

            char* token = strtok(line+2, " \t\r\n");
            
            Vertex* verts = malloc(sizeof(Vertex) * faceVertCount);
            int numVerts = 0;

            while (token != NULL) {

                int vi, vti, vni;

                if (sscanf(token, "%d/%d/%d", &vi, &vti, &vni) == 3) {

                    vi--; vti--; vni--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy(normals[vni], verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy(uvs[vti], verts[numVerts].texUV);

                } else if (sscanf(token, "%d//%d", &vi, &vni) == 2) {

                    vi--; vni--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy(normals[vni], verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy((vec2){0.0f,0.0f}, verts[numVerts].texUV);

                } else if (sscanf(token, "%d/%d", &vi, &vti) == 2) {

                    vi--; vni--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy((vec3){0.0f,0.0f,1.0f}, verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy(uvs[vti], verts[numVerts].texUV);

                } else if (sscanf(token, "%d", &vi) == 1) {

                    vi--;
                    glm_vec3_copy(positions[vi], verts[numVerts].position);
                    glm_vec3_copy((vec3){0.0f,0.0f,1.0f}, verts[numVerts].normal);
                    glm_vec3_copy((vec3){1.0f,1.0f,1.0f}, verts[numVerts].color);
                    glm_vec2_copy((vec2){0.0f,0.0f}, verts[numVerts].texUV);

                } else {
                    printf("%s:%d:error: broken face vertex\n", obj_path, lineNum);
                }

                token = strtok(NULL, " \t\r\n");
                numVerts++;
            }

            for (int i = 1; i < numVerts - 1; i++) {
                int i0 = find_or_add_vertex(vertices, &verticesCount, verts[0]);
                int i1 = find_or_add_vertex(vertices, &verticesCount, verts[i]);
                int i2 = find_or_add_vertex(vertices, &verticesCount, verts[i + 1]);

                indices[indicesCount++] = i1;
                indices[indicesCount++] = i0;
                indices[indicesCount++] = i2;
            }

            free(verts);
            
        } else {
            printf("%s:%d:warning: unknown command -> %s", obj_path, lineNum, line);
            continue;

        }

    }

    fclose(file);

    if (texturesCount == 0) {
        static const char* fallbackTextures[] = {
            "res/textures/null.jpg",
            "diffuse"
        };
        textures = fallbackTextures;
        texturesCount = 2;
    }
    
    mesh = Mesh_InitFromData(textures, 1, vertices, verticesCount, indices, indicesCount);

    free(positions);
    free(normals);
    free(uvs);
    free(vertices);
    free(indices);

    printf("succesfully loaded model -> %s\n", obj_path);
    return mesh;
}

const char** Mesh_getTexturesFromMTL(const char* mtl_path, int* outCount) {

    const int maxTextures = 50;
    const char** textures = malloc(maxTextures * sizeof(char*));
    if (!textures) {
        printf("error: could not allocate memory for textures\n");
        exit(1);
    }

    int count = 0;

    FILE* file = fopen(mtl_path, "r");
    if (!file) {
        printf("%s:1:error: could not open the file\n", mtl_path);
        exit(1);
    }

    char line[256];
    int lineNum = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        if (line[0] == '#') continue;

        if (strncmp(line, "map_Kd ", 7) == 0) {
            char fileRelPath[256] = {0};
            sscanf(line + 7, "%255[^\n]", fileRelPath);

            char texturePath[512];
            replacePathSuffix(mtl_path, fileRelPath, texturePath, sizeof(texturePath));

            textures[count++] = strdup(texturePath);
            textures[count++] = strdup("diffuse");
        }

        if (strncmp(line, "map_Ks ", 7) == 0) {
            char fileRelPath[256] = {0};
            sscanf(line + 7, "%255[^\n]", fileRelPath);

            char texturePath[512];
            replacePathSuffix(mtl_path, fileRelPath, texturePath, sizeof(texturePath));

            textures[count++] = strdup(texturePath);
            textures[count++] = strdup("specular");
        }
    }

    fclose(file);

    if (outCount) *outCount = count;
    return textures;
}
