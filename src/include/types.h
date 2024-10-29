#ifndef TYPES_H
#define TYPES_H

#include<unordered_map>
#include<vector>
#include<iostream>

#ifdef __cplusplus
extern "C" {
#endif
#include"sqlite3.h"
#ifdef __cplusplus
}
#endif

struct Node {
    sqlite3_int64 iNode; //Node id
    std::vector<sqlite3_int64>inNode; // In nodes
    std::vector<sqlite3_int64>outNode; // Out nodes
    std::vector<sqlite3_int64>inEdge; // In edges
    std::vector<sqlite3_int64>outEdge; // Out edges
    Node(): iNode(-1) {}
    Node(sqlite3_int64 id): iNode(id) {}
};

struct Edge {
    sqlite3_int64 iEdge; // Edge id
    sqlite3_int64 fromNode; // From-node id
    sqlite3_int64 toNode; // To-node id
    Edge(): iEdge(-1), fromNode(-1), toNode(-1) {}
    Edge(sqlite3_int64 id, sqlite3_int64 in, sqlite3_int64 out): iEdge(id), fromNode(id), toNode(id) {} 
};

class NodeMap {
    private:
        std::unordered_map<sqlite3_int64, Node*>map;
        unsigned int nNode; // Number of nodes

    public:
        // Find node by id. If node exists, return a pointer to node. Else, return nullptr.
        Node* find(sqlite3_int64 id) {
            std::unordered_map<sqlite3_int64, Node*>::iterator it = map.find(id);
            if (it != map.end()) {
                return it->second;
            } else {
                return nullptr;
            }
        }

        // Insert node. If success, return 1. Else, return 0.
        int insert(sqlite3_int64 id) {
            if (find(id) != nullptr) {
                std::cout<<"Node "<<id<<" already exists.";
                return 0;
            }
            Node *node = new Node(id);
            map[id] = node;
            nNode++;
            return 1;
        }

        // Remove node. If success, return 1. Else, return 0.
        int remove(sqlite3_int64 id) {
            if (find(id) == nullptr) {
                std::cout<<"Node "<<id<<" doesn't exists.";
                return 0;
            } else {
                Node *node = map[id];
                map.erase(id);
                delete node;
                nNode--;
                return 1;
            }
        }

        // Return number of nodes
        unsigned int getNNode() {
            return nNode;
        }
};

class EdgeMap {
    private:
        std::unordered_map<sqlite3_int64, Edge*>map;
        unsigned int nEdge; // Number of nodes
    public:
        // Return number of edges
        unsigned int getNEdge() {
            return nEdge;
        }

        // Find edge by id
        Edge* find(sqlite3_int64 id) {
            std::unordered_map<sqlite3_int64, Edge*>::iterator it = map.find(id);
            if (it != map.end()) {
                return it->second;
            } else {
                return nullptr;
            }
        }

        // Insert edge
        int insert(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
            if (find(id) != nullptr) {
                std::cout<<"Edge "<<id<<" already exists.";
                return 0;
            } else {
                Edge *edge = new Edge(id, from, to); // Node in and Node out exist
                map[id] = edge;
                nEdge++;
                return 1;
            }
        }

        // remove edge
        int remove(sqlite3_int64 id) {
            if (find(id) == nullptr) {
                std::cout<<"Edge "<<id<<" doesn't exist.";
                return 0;
            } else {
                Edge *edge = map[id];
                map.erase(id);
                delete edge;
                nEdge--;
                return 1;
            }
        }
};

class Graph {
    private:
        NodeMap *nodeMap;
        EdgeMap *edgeMap;
    
    public:
        Graph(): nodeMap(nullptr), edgeMap(nullptr){}
        
        int addNode(sqlite3_int64 id) {
            if (id < 0) {
                std::cout<<"Node id can't smaller than 0.";
                return 0;
            }
            return nodeMap->insert(id);
        }

        int removeNode(sqlite3_int64 id) {
            if (id < 0) {
                std::cout<<"Node id can't smaller than 0.";
                return 0;
            }
            return nodeMap->remove(id);
        }

        int addEdge(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
            // Ensure edge id >= 0
            if (id < 0) {
                std::cout<<"Edge id can't smaller than 0.";
                return 0;
            }
            // Ensure from id and to id >= 0
            if (from < 0 || to < 0) {
                std::cout<<"Node id can't smaller than 0.";
                return 0;
            }
            // Check if from-node exists
            if (nodeMap->find(from) == nullptr) {
                std::cout<<"In-node doesn't exist.";
                return 0;
            }
            // Check if to-node exists
            if (nodeMap->find(to) == nullptr) {
                std::cout<<"Out-node doesn't exist.";
                return 0;
            }

            // Add edge
            return edgeMap->insert(id, from, to);
        }

        int removeEdge(sqlite3_int64 id) {
            // Ensure edge id >= 0
            if (id < 0) {
                std::cout<<"Edge id can't smaller than 0.";
                return 0;
            }

            return edgeMap->remove(id);
        }
};

#endif