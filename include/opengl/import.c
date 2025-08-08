#include "import.h"
#include <stdlib.h>
#include <string.h>
#include "error.h"

int find_or_add_vertex(Vertex* vertices, int* verticesCount, Vertex v) {
    for (int i = 0; i < *verticesCount; i++) {
        if (memcmp(&vertices[i], &v, sizeof(Vertex)) == 0) {
            return i;
        }
    }

    int index = (*verticesCount)++;
    vertices[index] = v;
    return index;
}

int count_face_vertices(const char* line) {
    const char* ptr = line + 2;
    int count = 0;

    while (*ptr) {
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n') ptr++;
        if (*ptr == '\0') break;

        const char* token_start = ptr;

        while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\r' && *ptr != '\n') ptr++;

        char token[64];
        size_t len = ptr - token_start;
        if (len >= sizeof(token)) len = sizeof(token) - 1;
        memcpy(token, token_start, len);
        token[len] = '\0';

        int vi, vti, vni;
        if (sscanf(token, "%d/%d/%d", &vi, &vti, &vni) == 3 ||
            sscanf(token, "%d//%d", &vi, &vni) == 2 ||
            sscanf(token, "%d/%d", &vi, &vti) == 2 ||
            sscanf(token, "%d", &vi) == 1) {
            count++;
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
        MSG_ERROR(obj_path, 1, "could not open file");
        exit(1);
    }

    vec3* positions = (vec3*)malloc(sizeof(vec3) * 100000);
    int positionsCount = 0;
    if (!positions) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh positions");
        exit(1);
    }
    
    vec3* normals = (vec3*)malloc(sizeof(vec3) * 100000);
    int normalsCount = 0;
    if (!normals) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh normals");
        exit(1);
    }
    
    vec2* uvs = (vec2*)malloc(sizeof(vec2) * 100000);
    int uvsCount = 0;
    if (!uvs) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh uvs");
        exit(1);
    }

    Vertex* vertices = (Vertex*)malloc(sizeof(Vertex) * 100000);
    int verticesCount = 0;
    if (!vertices) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh vertices");
        exit(1);
    }
    
    GLuint* indices =  (GLuint*)malloc(sizeof(GLuint) * 100000);
    int indicesCount = 0;
    if (!indices) {
        MSG_FATAL(obj_path, 1, "could not allocate memory for mesh indices");
        exit(1);
    }

    const char** textures;
    int texturesCount = 0;

    char line[546];
    int lineNum = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        
        if ( 
            line[0] == '#' || 
            line[0] == '\n' || 
            strncmp(line, "o ", 2) == 0 || 
            strncmp(line, "s ", 2) == 0
        ) continue;

        if (strncmp(line, "mtllib ", 7) == 0) {
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
                MSG_ERROR(obj_path, lineNum, "broken position vertex");
                continue;
            }

        } else if (strncmp(line, "vt ", 3) == 0) {

            vec2 vt;
            if (sscanf(line, "vt %f %f", &vt[0], &vt[1]) == 2) {
                glm_vec2_copy(vt, uvs[uvsCount++]);
            } else {
                MSG_ERROR(obj_path, lineNum, "broken uv vertex");
                continue;
            }

        } else if (strncmp(line, "vn ", 3) == 0) {

            vec3 vn;
            if (sscanf(line, "vn %f %f %f", &vn[0], &vn[1], &vn[2]) == 3) {
                glm_vec3_copy(vn, normals[normalsCount++]);
            } else {
                MSG_ERROR(obj_path, lineNum, "broken normal vertex");
                continue;
            }

        } else if (strncmp(line, "f ", 2) == 0) {
            
            int faceVertCount = count_face_vertices(line);

            char* token = strtok(line+2, " \t\r\n");
            
            Vertex* verts = (Vertex*)malloc(sizeof(Vertex) * faceVertCount);
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
                    MSG_ERROR(obj_path, lineNum, "broken face vertex '%s'", token);
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
            line[strcspn(line, "\n")] = '\0';
            MSG_WARNING(obj_path, lineNum, "unsupported OBJ directive '%s'", line);
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

    MSG_INFO(obj_path, lineNum, "model loaded succesfully");
    return mesh;
}

const char** Mesh_getTexturesFromMTL(const char* mtl_path, int* outCount) {

    FILE* file = fopen(mtl_path, "r");
    if (!file) {
        MSG_ERROR(mtl_path, 1, "could not open file");
        exit(1);
    }
    
    const char** textures = (const char**)malloc(sizeof(char*) * 50);
    int count = 0;
    if (!textures) {
        MSG_FATAL(mtl_path, 1, "could not allocate memory for mesh textures");
        exit(1);
    }

    char line[256];
    int lineNum = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNum++;
        
        if ( 
            line[0] == '#' || 
            line[0] == '\n'
        ) continue;

        if (strncmp(line, "map_Kd ", 7) == 0) {

            char fileRelPath[256];
            sscanf(line, "map_Kd %s", fileRelPath);

            char texturePath[512];
            replacePathSuffix(mtl_path, fileRelPath, texturePath, sizeof(texturePath));

            textures[count++] = strdup(texturePath);
            textures[count++] = strdup("diffuse");

        } else if (strncmp(line, "map_Ks ", 7) == 0) {
            
            char fileRelPath[256];
            sscanf(line, "map_Ks %s", fileRelPath);

            char texturePath[512];
            replacePathSuffix(mtl_path, fileRelPath, texturePath, sizeof(texturePath));

            textures[count++] = strdup(texturePath);
            textures[count++] = strdup("specular");
        
        } else {
            line[strcspn(line, "\n")] = '\0';
            MSG_WARNING(mtl_path, lineNum, "unsupported MTL directive '%s'", line);
            continue;
        }
    }

    fclose(file);

    if (outCount) *outCount = count;
    return textures;
}
