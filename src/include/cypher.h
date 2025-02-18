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

using json = nlohmann::json;

struct CypherNode {
    int type; // NODE or EDGE
    bool certain;
    int constrainType; // NOCONSTRAIN or DEFINITE or ATTRIBUTE
    std::string constrain;
    std::set<sqlite3_int64> set; // node set or edge set
    CypherNode *prev;
    CypherNode *next;

    CypherNode(int type, int constrainType, std::string constrain): 
     type(type), certain(0), constrainType(constrainType), constrain(constrain), prev(nullptr), next(nullptr) {}
};

class Parser {
public:
    std::string zCypher; // cypher string
    CypherNode *head; // head of linked list
    std::unordered_map<std::string, int> var_map; // map variable name to cyphernode index
    Graph *graph;

    Parser(std::string zCypher, Graph *graph): zCypher(zCypher), graph(graph), head(nullptr) {}

    ~Parser() {
        CypherNode *p = head;
        CypherNode *q;
        while (p != nullptr) {
            q = p;
            p = p->next;
            delete q;
        }
    }

    // return type of constrain
    int whichConstrainType(std::string constrain) {
        int constrain_len = constrain.length();
        if (constrain == "") return NOCONSTRAIN; // no constrain
        else if (constrain[0] == '"' && constrain[constrain_len - 1] == '"') return LABEL; // label constrain
        else if (constrain[0] == '{' && constrain[constrain_len - 1] == '}') return ATTRIBUTE; // attribute constrain
        else if (constrain[0] >= '0' && constrain[0] <= '9') return DEFINITE; // id constrain
        else return VARIABLE;
    }

    void parseVarible() {
        CypherNode *p = head;
        int i = 0;
        while(p != nullptr) {
            if (p->constrainType == VARIABLE) {
                p->constrainType = NOCONSTRAIN;
                std::string constrain = p->constrain;
                p->constrain = "";
                var_map[constrain] = i;
            }
            p = p->next;
            i++;
        }
    }

    // if parses successfully, return GRAPH_SUCCESS, else return GRAPH_FAILED
    int parse() {
        int zCypher_len = zCypher.length();
        if (zCypher[0] != '(') return GRAPH_FAILED;
        int status = NODE;
        int i = 1;
        std::string constrain;
        while (1) {
            if (zCypher[i] == ')') {
                int constrain_type = whichConstrainType(constrain);
                if (constrain_type == LABEL) {
                    std::string label = constrain.substr(1, constrain.length() - 2); // remove '"'
                    sqlite3_int64 id = graph->getNodeIdByLabel(label);
                    constrain = std::to_string(id);
                    constrain_type = DEFINITE;
                } else if (constrain_type == ATTRIBUTE) {
                    constrain = constrain.substr(1, constrain.length() - 2); // remove '{' and '}'
                }
                CypherNode *cnode = new CypherNode(status, constrain_type, constrain);
                head = cnode;
                i++;
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
                    CypherNode *cnode =  new CypherNode(status, NOCONSTRAIN, constrain);
                    cnode->prev = cur;
                    cur->next = cnode;
                    cur = cur->next;
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
        parseVarible();
        return GRAPH_SUCCESS;
    }
};

class Cypher {
private:
    Graph *graph;
    std::string zCypher;
    Parser *parser;
    CypherNode *head;

    int backtraceNode(CypherNode *node, std::vector<sqlite3_int64> *to_be_removed) {
        if (node->next != nullptr) {
            for (auto it = node->set.begin(); it != node->set.end(); it++) {
                Node *n = graph->nodeMap->find(*it);
                if (n == nullptr) return GRAPH_FAILED;
                if (n->outNode.empty()) {
                    to_be_removed->push_back(*it);
                }
            }
        }

        // remove node in to_be_removed
        for (auto id : *to_be_removed) {
            node->set.erase(id);
        }

        CypherNode *edge = node->prev;
        if (edge == nullptr || edge->certain) return GRAPH_SUCCESS;
        std::vector<sqlite3_int64> edge_to_be_removed;
        for (sqlite3_int64 nodeId : *to_be_removed) {
            Node *n = graph->nodeMap->find(nodeId);
            if (n == nullptr) return GRAPH_FAILED;
            for (auto it = n->inEdge.begin(); it != n->inEdge.end(); it++) {
                edge_to_be_removed.push_back(*it);
            }
        }
        for (sqlite3_int64 edgeId : edge_to_be_removed) {
            edge->set.erase(edgeId);
        }
        return backtraceEdge(edge, &edge_to_be_removed);
    }

    int backtraceEdge(CypherNode *edge, std::vector<sqlite3_int64> *to_be_removed) {
        // remove edge in to_be_removed
        for (auto id: *to_be_removed) {
            edge->set.erase(id);
        }
        
        CypherNode *node = edge->prev;
        if (node->certain) return GRAPH_SUCCESS;
        std::set<sqlite3_int64> node_to_be_removed_set;
        std::vector<sqlite3_int64> node_to_be_removed;
        for (sqlite3_int64 edgeId : *to_be_removed) {
            Edge *e = graph->edgeMap->find(edgeId);
            if (e == nullptr) return GRAPH_FAILED;
            node_to_be_removed_set.insert(e->fromNode);
        }
        // add back nodes which should not be removed
        for (auto it = edge->set.begin(); it != edge->set.end(); it++) {
            Edge *e = graph->edgeMap->find(*it);
            if (e == nullptr) return GRAPH_FAILED;
            node_to_be_removed_set.erase(e->fromNode);
        }
        CypherNode *next_node = edge->next;
        for (auto it = next_node->set.begin(); it != next_node->set.end(); it++) {
            node_to_be_removed_set.insert(*it);
        }
        // build vector from set
        for (auto it = node_to_be_removed_set.begin(); it != node_to_be_removed_set.end(); it++) {
            node_to_be_removed.push_back(*it);
        }
        // remove nodes
        for (sqlite3_int64 nodeId : node_to_be_removed) {
            node->set.erase(nodeId);
        }
        return backtraceNode(node, &node_to_be_removed);
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
                    std::cerr << "ERROR: Cannot find this edge!" << std::endl;
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
            if (graph->nodeMap->find(nodeId) == nullptr) {
                std::cerr << "ERROR: Cannot find this node!" << std:: endl;
                return GRAPH_FAILED;
            }
            std::vector<sqlite3_int64> to_be_removed;
            for (auto it = node->set.begin(); it != node->set.end(); it++) {
                if (*it == nodeId) continue;
                to_be_removed.push_back(*it);
            }
            node->set.clear();
            node->set.insert(nodeId);
            if (!to_be_removed.empty()) {
                return backtraceNode(node, &to_be_removed);
            } else {
                return GRAPH_SUCCESS;
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
                    i++;
                    while (constrain[i] == ' ' || constrain[i] == ':') { // skip space and ':'
                        i++;
                    }
                    while (i < constrain_len && constrain[i] != ',' && constrain[i] != ' ') { //end of v
                        if (constrain[i] == '"') { // skip '"'
                            i++;
                            continue;
                        }
                        v += constrain[i];
                        i++;
                    }

                    key.push_back(k);
                    value.push_back(v);
                } else {
                    i++;
                    continue;
                }
            }

            std::vector<sqlite3_int64> to_be_removed;
            for (auto it = node->set.begin(); it != node->set.end(); it++) {
                sqlite3_int64 nodeId = *it;
                Node *n = graph->nodeMap->find(nodeId);
                if (n == nullptr) {
                    std::cerr << "ERROR: Cannot find this node!" << std::endl;
                    return GRAPH_FAILED;
                }
                std::string attribute = graph->getNodeAttributeById(nodeId);
                json data = json::parse(attribute);
                int constrain_num = key.size();
                for (int i = 0; i < constrain_num; i++) {
                    std::string k = key[i];
                    std::string v = value[i];
                    // ensure attribute of node has the key
                    if (!data.contains(k)) {
                        std::cerr << "ERROR: Cannot this key!" << std::endl;
                        return GRAPH_FAILED;
                    }

                    // add nodes that do not meet the constrains to to_be_removed list
                    auto v_type = data[k].type();
                    if (v_type == json::value_t::string) {
                        std::string v_str = data[k];
                        if (v_str != v) {
                            to_be_removed.push_back(nodeId);
                            break;
                        }
                    } else if (v_type == json::value_t::number_integer) {
                        int v_int = data[k];
                        if (v_int != std::stoi(v)) {
                            to_be_removed.push_back(nodeId);
                            break;
                        }
                    } else if (v_type == json::value_t::number_float) {
                        double v_float = data[k];
                        if (v_float != std::stod(v)) {
                            to_be_removed.push_back(nodeId);
                            break;
                        }
                    } else {
                        std::cerr << "ERROR: Incorrect type of value." << std::endl;
                        return GRAPH_FAILED;
                    }
                }
            }

            // if to_be_removed is not empty, then start backtrace
            if (!to_be_removed.empty()) {
                return backtraceNode(node, &to_be_removed);
            } else {
                return GRAPH_SUCCESS;
            }
        } else {
            return GRAPH_FAILED;
        }
    }

    int executeEdge(CypherNode *edge) {
        // initialize set of edge
        if (edge->prev == nullptr) {
            std::cerr << "ERROR: Edge should have an in-node." << std::endl;
            return GRAPH_FAILED;
        } else {
            CypherNode *node = edge->prev;
            for (auto it = node->set.begin(); it != node->set.end(); it++) {
                sqlite3_int64 nodeId = *it;
                Node *n = graph->nodeMap->find(nodeId);
                if (n == nullptr) {
                    std::cerr << "ERROR: Cannot find this node!" << std::endl;
                    return GRAPH_FAILED;
                }
                std::set<sqlite3_int64> *edge_set = &n->outEdge;
                for (auto e = edge_set->begin(); e != edge_set->end(); e++) {
                    edge->set.insert(*e);
                }
            }
        }

        // apply constrain to set of edge
        if (edge->constrainType == NOCONSTRAIN) {
            return GRAPH_SUCCESS;
        } else if (edge->constrainType == DEFINITE) {
            sqlite3_int64 edgeId = std::stoll(edge->constrain);
            if (graph->edgeMap->find(edgeId) == nullptr) {
                std::cerr << "ERROR: Cannot find this edge!" << std::endl;
                return GRAPH_FAILED;
            }
            std::vector<sqlite3_int64> to_be_removed;
            for (auto it = edge->set.begin(); it != edge->set.end(); it++) {
                if (*it == edgeId) continue;
                to_be_removed.push_back(*it);
            }
            edge->set.clear();
            edge->set.insert(edgeId);
            if (!to_be_removed.empty()) {
                return backtraceEdge(edge, &to_be_removed);
            } else {
                return GRAPH_SUCCESS;
            }
        } else if (edge->constrainType == ATTRIBUTE) {
            std::vector<std::string> key, value;
            std::string constrain = edge->constrain;
            int constrain_len = constrain.length();
            int i = 0;

            while (i < constrain_len) {
                if (constrain[i] == '"') {
                    i++;
                    std::string k, v;
                    while (constrain[i] != '"') {
                        k += constrain[i];
                        i++;
                    }
                    i++;
                    while (constrain[i] == ' ' || constrain[i] ==':') {
                        i++;
                    }
                    while (i < constrain_len && constrain[i] != ',' && constrain[i] != ' ') {
                        if (constrain[i] == '"') {
                            i++;
                            continue;
                        }
                        v += constrain[i];
                        i++;
                    }

                    key.push_back(k);
                    value.push_back(v);
                } else {
                    i++;
                    continue;
                }
            }

            std::vector<sqlite3_int64> to_be_removed;
            for (auto it = edge->set.begin(); it != edge->set.end(); it++) {
                sqlite3_int64 edgeId = *it;
                Edge *e = graph->edgeMap->find(edgeId);
                if (e == nullptr) {
                    std::cerr << "ERROR: Cannot find this edge!" << std::endl;
                    return GRAPH_FAILED;
                }
                std::string attribute = graph->getEdgeAttributeById(edgeId);
                json data = json::parse(attribute);
                int constrain_num = key.size();
                for (int i = 0; i < constrain_num; i++) {
                    std::string k = key[i];
                    std::string v = value[i];
                    if (!data.contains(k)) {
                        std::cerr << "ERROR: Cannot find this key!" << std::endl;
                        return GRAPH_FAILED;
                    }

                    auto v_type = data[k].type();
                    if (v_type == json::value_t::string) {
                        std::string v_str = data[k];
                        if (v_str != v) {
                            to_be_removed.push_back(edgeId);
                            break;
                        }
                    } else if (v_type == json::value_t::number_integer) {
                        int v_int = data[k];
                        if (v_int != stoi(v)) {
                            to_be_removed.push_back(edgeId);
                            break;
                        }
                    } else if (v_type == json::value_t::number_float) {
                        double v_float = data[k];
                        if (v_float != std::stod(v)) {
                            to_be_removed.push_back(edgeId);
                            break;
                        }
                    } else {
                        std::cerr << "ERROR: Incorrect type of value." << std::endl;
                        return GRAPH_FAILED;
                    }
                }
            }

            // if to_be_removed is not empty, then start backtrace
            if (!to_be_removed.empty()) {
                return backtraceEdge(edge, &to_be_removed);
            } else {
                return GRAPH_SUCCESS;
            }
        } else {
            return GRAPH_FAILED;
        }
    }

    CypherNode *findCypherNode(int k) {
        CypherNode *p = head;
        int i = 0;
        while (1) {
            if (i == k) return p;
            if (p->next != nullptr) {
                p = p->next;
                i++;
            } else {
                std::cerr << "ERROR: Null pointer error!" << std::endl;
                return nullptr;
            }
        }
    }

public:
    Cypher(Graph *graph, std::string zCypher): graph(graph), zCypher(zCypher), head(nullptr) {}
    
    ~Cypher() {
        delete parser;
    }
    
    int parse() {
        parser = new Parser(zCypher, graph);
        int rc = parser->parse();
        if (rc != GRAPH_SUCCESS) return GRAPH_FAILED;
        head = parser->head;
        return GRAPH_SUCCESS;
    }

    int execute() {
        CypherNode *cur = head;
        while (cur != nullptr) {
            if (cur->type == NODE) {
                int rc = executeNode(cur);
                if (rc != GRAPH_SUCCESS) {
                    return GRAPH_FAILED;
                }

                if (cur->constrainType == DEFINITE) {
                    cur->certain = 1;
                }
            } else { // EDGE
                int rc = executeEdge(cur);
                if (rc != GRAPH_SUCCESS) {
                    return GRAPH_FAILED;
                }

                if (cur->constrainType == DEFINITE) {
                    cur->certain = 1;
                }
            }
            cur = cur->next;
        }
        return GRAPH_SUCCESS;
    }

    int query(std::string var, std::set<sqlite3_int64> &set) {
        int index = parser->var_map[var];
        CypherNode *cnode = findCypherNode(index);
        set = cnode->set;
        return cnode->type;
    } 
};

#endif