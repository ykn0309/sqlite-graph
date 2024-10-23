#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif
#include"sqlite3.h"
#ifdef __cplusplus
}
#endif

struct Node {
    sqlite3_int64 iNode; //Node id
    Node *next; // Next node
};

struct Edge {
    sqlite3_int64 iEdge; // Edge id
    sqlite3_int64 inNode; // In-node id
    sqlite3_int64 outNode; // Out-node id
};

#endif