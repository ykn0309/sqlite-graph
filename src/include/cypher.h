#ifndef CYPHER_H
#define CYPHER_H

#include<iostream>
#include<vector>
#include<set>
#include<unordered_map>
#include"json.hpp"
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

using json = nlohmann::json;

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
        // initialize set of node
        if (node->prev == nullptr) { // CypherNode is head
            std::unordered_map<sqlite3_int64, Node*> map = graph->nodeMap->map;
            for(auto it = map.begin(); it != map.end(); it++) {
                node->set.insert(it->first);
            }
        } else { // previous CypherNode is an edge
            CypherNode *edge = node->prev;
            for (auto it = edge->set.begin(); it != edge->set.end(); it++) {
                sqlite3_int64 edgeId = *it;
                Edge *e = graph->edgeMap->find(edgeId);
                if (e == nullptr) {
                    std::cerr << "ERROR: No this edge!" << std::endl;
                    return GRAPH_FAILED;
                }
                node->set.insert(e->toNode);
            }
        }
        
        // apply constrain to set of node
        if (node->constrainType == NOCONSTRAIN) {
            return GRAPH_SUCCESS;
        } else if (node->constrainType == DEFINITE) {
            sqlite3_int64 nodeId = std::stoll(node->constrain);
            if (graph->nodeMap->find(nodeId) != nullptr) {
                // the set has only 1 element and the only element is nodeId, then there is no need to change anything.
                if (node->set.size() == 1 && node->set.count(nodeId)) {
                    return GRAPH_SUCCESS;
                } else {
                    node->set.clear();
                    node->set.insert(nodeId);
                    return GRAPH_MODIFIED;
                }
            } else {
                std::cerr << "ERROR: No this node!" << std:: endl;
            }
        } else if (node->constrainType == ATTRIBUTE) {
            std::vector<std::string> key, value;
            std::string constrain = node->constrain;
            int constrain_len = constrain.length();
            int i = 0;

            while (i < constrain_len) {
                if (constrain[i] == '"') { // begin of a new k-v pair
                    i++;
                    std::string k, v;
                    while (constrain[i] != '"') { // end of k
                        k += constrain[i];
                        i++;
                    }
                    while (constrain[i] == ' ' || constrain[i] == ':') { // skip space and ':'
                        i++;
                    }
                    while (constrain[i] != ',' && constrain[i] != ' ') { //end of v
                        if (constrain[i] == '"'){ // skip '"'
                            i++;
                            continue;
                        }
                        v += constrain[i];
                        i++;
                    }

                    key.push_back(k);
                    value.push_back(v);
                } else {
                    continue;
                }
            }

            std::vector<sqlite3_int64> to_be_removed;
            for (auto it = node->set.begin(); it != node->set.end(); it++) {
                sqlite3_int64 nodeId = *it;
                Node *n = graph->nodeMap->find(nodeId);
                if (n == nullptr) {
                    std::cerr << "ERROR: No this node!" << std::endl;
                    return GRAPH_FAILED;
                }
                std::string attribute = "{" + graph->getNodeAttributeById(nodeId) + "}";
                json data = json::parse(attribute);
                int constrain_num = key.size();
                for (int i = 0; i < constrain_num; i++) {
                    std::string k = key[i];
                    std::string v = value[i];
                    if (!data.contains(k)) {
                        std::cerr << "ERROR: No this key!" << std::endl;
                        return GRAPH_FAILED;
                    }
                    // compare
                }
            }
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
                    // backtrace
                }
                /* 如果限制为一个结点，那么在此之前的所有CypherNode的set都确定
                  下来，不会再因为后面新的限制条件而发生改变 */
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