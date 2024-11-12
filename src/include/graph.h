#ifndef TYPES_H
#define TYPES_H

#include<unordered_map>
#include<set>
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
    std::set<sqlite3_int64>inNode; // In nodes
    std::set<sqlite3_int64>outNode; // Out nodes
    std::set<sqlite3_int64>inEdge; // In edges
    std::set<sqlite3_int64>outEdge; // Out edges
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

struct NodeMap {
    std::unordered_map<sqlite3_int64, Node*>map;

    unsigned int nNode; // Number of nodes

    // Find node by id. If node exists, return a pointer to node. Else, return nullptr.
    Node* find(sqlite3_int64 id) {
        auto it = map.find(id);
        if (it != map.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    // Insert node. If success, return 1. Else, return 0.
    int insert(sqlite3_int64 id, bool allow_duplicate) {
        if (id < 0) {
            std::cout<<"Node id can't smaller than 0.\n";
            return 0;
        }
        if (find(id) != nullptr) {
            if (!allow_duplicate) {
                std::cout<<"Node "<<id<<" already exists.\n";
                return 0;
            } else {
                return 1;
            }
        }
        Node *node = new Node(id);
        map[id] = node;
        nNode++;
        return 1;
    }

    // Remove node. If success, return 1. Else, return 0.
    int remove(sqlite3_int64 id) {
        if (id < 0) {
            std::cout<<"Node id can't smaller than 0.\n";
            return 0;
        }
        if (find(id) == nullptr) {
            std::cout<<"Node "<<id<<" doesn't exists.\n";
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

struct EdgeMap {
    std::unordered_map<sqlite3_int64, Edge*>map;

    unsigned int nEdge; // Number of nodes

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
        // Ensure edge id >= 0
        if (id < 0) {
            std::cout<<"Edge id can't smaller than 0.\n";
            return 0;
        }
        // Ensure from id and to id >= 0
        if (from < 0 || to < 0) {
            std::cout<<"Edge id can't smaller than 0.\n";
            return 0;
        }
        if (find(id) != nullptr) {
            std::cout<<"Edge "<<id<<" already exists.\n";
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
        // Ensure edge id >= 0
        if (id < 0) {
            std::cout<<"Edge id can't smaller than 0.\n";
            return 0;
        }
        if (find(id) == nullptr) {
            std::cout<<"Edge "<<id<<" doesn't exist.\n";
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
        sqlite3_int64 graph_id;
        std::string graph_name;
        bool persistence;
        NodeMap *nodeMap;
        EdgeMap *edgeMap;
        std::string node_table;
        std::string edge_table;
    
    public:
        // Delete defaute constructor
        Graph() = delete;

        // Temporary graph constructor
        Graph(sqlite3_int64 id, std::string graph_name): graph_id(id), graph_name(graph_name), persistence(0){
            nodeMap = new NodeMap();
            edgeMap = new EdgeMap();
        }

        // Persistence graph constructor
        Graph(sqlite3_int64 id, std::string graph_name, std::string node_table, std::string edge_table)
        : graph_id(id), graph_name(graph_name), persistence(1), node_table(node_table), edge_table(edge_table) {
            nodeMap = new NodeMap();
            edgeMap = new EdgeMap();
        }

        // Return number of nodes
        unsigned int getNNode() {
            return nodeMap->getNNode();
        }

        // Return number of edges
        unsigned int getNEdge() {
            return edgeMap->getNEdge();
        }
        
        int addNode(sqlite3_int64 id) {
            return nodeMap->insert(id, 0);
        }

        // If node id exists, return 1.
        int sameAddNode(sqlite3_int64 id) {
            return nodeMap->insert(id, 1);
        }

        // Remove node id and its edges, also update its adj nodes.
        int removeNode(sqlite3_int64 id) {
            Node *node = nodeMap->find(id);

            if (!node) return 0;

            for (sqlite3_int64 i : node->inEdge) {
                Edge *e = edgeMap->find(i);
                sqlite3_int64 iNode = e->fromNode;
                Node *n = nodeMap->find(iNode);
                n->outNode.erase(id);
                n->outEdge.erase(i);
                edgeMap->remove(i);
            }

            for (sqlite3_int64 i : node->outEdge) {
                Edge *e = edgeMap->find(id);
                sqlite3_int64 iNode = e->toNode;
                Node *n = nodeMap->find(iNode);
                n->inNode.erase(id);
                n->inEdge.erase(i);
                edgeMap->remove(i);
            }
            
            return nodeMap->remove(id);
        }

        int addEdge(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
            // Check if from-node exists
            if (nodeMap->find(from) == nullptr) {
                std::cout<<"In-node doesn't exist.\n";
                return 0;
            }
            // Check if to-node exists
            if (nodeMap->find(to) == nullptr) {
                std::cout<<"Out-node doesn't exist.\n";
                return 0;
            }

            // Add edge
            safeAddEdge(id, from, to);
        }

        // No check for from-node and to-node before add edge.
        int safeAddEdge(sqlite3_int64 id, sqlite3_int64 from, sqlite3_int64 to) {
            if (edgeMap->insert(id, from, to)) {
                // add attributes of from-node and to-node
                Node *from_node = nodeMap->find(from);
                Node *to_node = nodeMap->find(to);
                from_node->outEdge.insert(to);
                from_node->outEdge.insert(id);
                to_node->inNode.insert(from);
                to_node->inEdge.insert(id);
                return 1;
            } else {
                return 0;
            }
        }

        // Remove edge id and update related in-node and out-node.
        int removeEdge(sqlite3_int64 id) {
            Edge *edge = edgeMap->find(id);
            if (!edge) return 0;
            Node *from = nodeMap->find(edge->fromNode);
            Node *to = nodeMap->find(edge->toNode);
            from->outNode.erase(to->iNode);
            from->outEdge.erase(id);
            to->inNode.erase(from->iNode);
            to->inEdge.erase(id);
            return edgeMap->remove(id);
        }

        // Return a vector containing all nodes' pointer
        std::vector<Node*> nodeList() {
            std::vector<Node*>list;
            for (auto it = nodeMap->map.begin(); it != nodeMap->map.end(0); it++) {
                Node *n = it->second;
                list.push_back(n);
            }
            return list;
        }
};

#endif