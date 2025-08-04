#ifndef IMPORT_H
#define IMPORT_H

#include "mesh.h"
#include "vector.h"
int find_or_add_vertex(Vertex* vertices, int* verticesCount, Vertex v);
int check_duplicate_vert(Vertex* check, int count, Vertex* reference);
void replacePathSuffix(const char* path, const char* newsuffix, char* dest, int destsize);
int count_face_vertices(const char* line);

Mesh Import_loadMeshFromOBJ(const char* obj_path);
const char** Mesh_getTexturesFromMTL(const char* mtl_path, int* outCount);

#endif