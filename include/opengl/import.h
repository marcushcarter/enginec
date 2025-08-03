#ifndef IMPORT_H
#define IMPORT_H

#include "mesh.h"
#include "vector.h"
int find_or_add_vertex(Vertex* vertices, int* verticesCount, Vertex v);
int check_duplicate_vert(Vertex* check, int count, Vertex* reference);

Mesh Import_loadMeshFromOBJ(const char* obj_path);

#endif