#ifndef CYPHER_H
#define CYPHER_H

#include<iostream>
#include<vector>
#include<set>
#include<unordered_map>
#include"defs.h"
#include"graph.h"

#ifdef __cplusplus
extern "C" {
#endif
#include"sqlite3.h"
#ifdef __cplusplus
}
#endif

#define NODE 0
#define EDGE 1
#define NOCONSTRAIN 0 // no constrain
#define DEFINITE 1 // definite label
#define ATTRIBUTE 2 // attribute constrain

class CypherNode {
public:
    int type; // NODE or EDGE
    int constrainType; // NOCONSTRAIN or DEFINITE or ATTRIBUTE
    std::string constrain;
    std::set<sqlite3_int64> set; // node set or edge set
    CypherNode *prev;
    CypherNode *next;

    CypherNode::CypherNode(int type, int constrainType, std::string constrain): 
     type(type), constrainType(constrainType), constrain(constrain) {
        prev = nullptr;
        next = nullptr;
    }
};

class Parser {
public:
    std::string zCypher; // cypher string
    CypherNode *head; // head of linked list

    Parser::Parser(std::string zCypher): zCypher(zCypher), head(nullptr) {}

    // return type of constrain
    int whichConstrainType(std::string constrain) {
        if (constrain == "") return NOCONSTRAIN;
        else if (constrain[0] == '"') return ATTRIBUTE;
        else return DEFINITE;
    }

    // if parses successfully return GRAPH_SUCCESS, else return GRAPH_FAILED
    int parse() {
        int zCypher_len = zCypher.length();
        if (zCypher[0] != '(') return GRAPH_FAILED;
        int status = NODE;
        int i = 1;
        std::string constrain;
        while (1) {
            if (zCypher[i] == ')') {
                int constrain_type = whichConstrainType(constrain);
                CypherNode *cnode = new CypherNode(status, constrain_type, constrain);
                head = cnode;
                break;
            }
            constrain += zCypher[i];
            i++;
        }

        CypherNode *cur = head;

        while (i < zCypher_len) {
            if (zCypher[i] == '(') {
                if (status != EDGE) {
                    std::cerr << "Error: Should be an edge!" << std::endl;
                    return GRAPH_FAILED;
                } else {
                    status = NODE;
                }
                i++;
                constrain = "";
                while (1) {
                    if (zCypher[i] == ')') {
                        int constrain_type = whichConstrainType(constrain);
                        CypherNode *cnode = new CypherNode(status, constrain_type, constrain);
                        cnode->prev = cur;
                        cur->next = cnode;
                        cur = cur->next;
                        i++;
                        break;
                    }
                    constrain += zCypher[i];
                    i++;
                }
            } else if (zCypher[i] == '-') {
                if (status != NODE) {
                    std::cerr << "Error: Should be an node!" << std::endl;
                } else {
                    status = EDGE;
                }
                i++;
                constrain = "";
                if (zCypher[i] == '-' && zCypher[i+1] == '>') {
                    i += 2;
                    continue;
                } else if (zCypher[i] == '[') {
                    while (1) {
                        if (zCypher[i] == ']') {
                            int constrain_type = whichConstrainType(constrain);
                            CypherNode *cnode = new CypherNode(status, constrain_type, constrain);
                            cnode->prev = cur;
                            cur->next = cnode;
                            cur = cur->next;
                            if (zCypher[i+1] != '-' || zCypher[i+2] != '>') {
                                std::cerr << "Error: Should be -> !" << std::endl;
                                return GRAPH_FAILED;
                            }
                            i += 3;
                            break;
                        }
                        constrain += zCypher[i];
                        i++;
                    }
                } else {
                    return GRAPH_FAILED;
                }
            } else {
                return GRAPH_FAILED;
            }
        }
        return GRAPH_SUCCESS;
    }
};

class Cypher {
private:
    Graph *graph;
    std::string zCypher;
    CypherNode *head;

public:
    Cypher::Cypher(Graph *graph, std::string zCypher): graph(graph), zCypher(zCypher), head(nullptr) {}
    
    int parse() {
        Parser *parser = new Parser(zCypher);
        int rc = parser->parse();
        if (rc != GRAPH_SUCCESS) return GRAPH_FAILED;
        head = parser->head;
        return GRAPH_SUCCESS;
    }

    int executeNode(CypherNode *node) {
        if (node->prev == nullptr) {
            std::unordered_map<sqlite3_int64, Node*> map = graph->nodeMap->map;
            for(auto it = map.begin(); it != map.end(); it++) {
                node->set.insert(it->first);
            }
        }
        if (node->constrainType == NOCONSTRAIN) {
            return GRAPH_SUCCESS;
        } else if (node->constrainType == DEFINITE) {
            sqlite3_int64 nodeId = std::stoll(node->constrain);
            if (graph->nodeMap->find(nodeId) != nullptr) {
                
            } else {
                std::cerr << "ERROR: No this node!" << std:: endl;
            }
        } else if (node->constrainType == ATTRIBUTE) {

        } else {
            return GRAPH_FAILED;
        }
    }

    int executeEdge(CypherNode *edge) {

    }

    int execute() {
        CypherNode *cur = head;
        CypherNode *certain = nullptr;
        while (cur != nullptr) {
            if (cur->type == NODE) {
                int rc = executeNode(cur);
                if (rc == GRAPH_MODIFIED) {
                    //
                }
                if (cur->constrainType == DEFINITE) {
                    certain = cur;
                }
            } else if (cur->type == EDGE) {
                int rc = executeEdge(cur);
                if (rc == GRAPH_MODIFIED) {
                    //
                }
                if (cur->constrainType == DEFINITE) {
                    certain = cur;
                }
            } else {
                std::cerr << "ERROR: Type error!" << std::endl;
                return GRAPH_FAILED;
            }
            cur = cur->next;
        }
    }
};

#endif