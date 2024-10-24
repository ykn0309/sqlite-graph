#ifndef TYPES_H
#define TYPES_H

#include<unordered_map>
#include<vector>

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
    std::vector<sqlite3_int64>inNode; // In nodes
    std::vector<sqlite3_int64>outNode; // Out nodes
    std::vector<sqlite3_int64>inEdge; // In edges
    std::vector<sqlite3_int64>outEdge; // Out edges
    Node() :iNode(-1) {}
    Node(sqlite3_int64 id): iNode(id) {}
};

struct Edge {
    sqlite3_int64 iEdge; // Edge id
    sqlite3_int64 inNode; // In-node id
    sqlite3_int64 outNode; // Out-node id
};

struct NodeMap {
    std::unordered_map<sqlite3_int64, Node*>map;
    unsigned int nNode; // Number of nodes

    // Insert node. If success, return 1. Else, return 0.
    int insert(sqlite3_int64 id) {

    }

    // Find node by id. If node exists, return a pointer to node. Else, return nullptr.
    Node* find(sqlite3_int64 id) {

    }
};

struct EdgeMap {
    std::unordered_map<sqlite3_int64, Edge*>map;
    unsigned int nEdge; // Number of nodes
};

struct Graph {
    NodeMap *nodeMap;
    EdgeMap *edgeMap;
    Node **adj_list; // adj_list
    int nNode; // Number of nodes
    
    Graph(): nodeMap(nullptr), edgeMap(nullptr), adj_list(nullptr), nNode(0) {}
    
    int addNode(sqlite3_int64 id) {

    }

    int addEdge(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {

    }
};

#endif