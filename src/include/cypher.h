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

struct NNode {
    Graph *graph;
    sqlite3_int64 iNode; // node id
    Node *node; // corresponding Node instance of an NNode object
    std::set<NEdge*> inEdge; // set of in edges
    std::set<NEdge*> outEdge; // set of out edges
    CypherNode *cNode; // CypherNode that NNode belongs to
    bool valid;
    
    NNode(Graph *graph, sqlite3_int64 id, CypherNode *cNode): graph(graph), iNode(id), cNode(cNode) {
        if (cNode->nodeFilter(id) != GRAPH_SUCCESS) {
            valid = 0;
        } else {
            valid = 1;
            node = graph->nodeMap->find(iNode);
            CypherNode *nextCNode = cNode->next;
            if (nextCNode == nullptr);
            else { // cNode has next CypherNode
                std::map<sqlite3_int64, NEdge*> *edgeMap = &nextCNode->edgeMap;
                for (sqlite3_int64 iEdge : node->outEdge) {
                    NEdge *ne = new NEdge(graph, iEdge, cNode->next);
                    if (!ne->valid) { // ne doen't meet the constrain
                        delete ne;
                    } else { // ne meets the constrain
                        ne->from_node = this;
                        (*edgeMap)[iEdge] = ne; // insert ne to edgeMap
                        outEdge.insert(ne);
                    }
                }
                if (outEdge.empty()) {
                    valid = 0;
                }
            }
        }
    }
};

struct NEdge {
    Graph *graph;
    sqlite3_int64 iEdge; // edge id
    Edge *edge; // corresponding Edge instance of an Edge object
    NNode *from_node; // from node
    NNode *to_node; // to node
    CypherNode *cNode; // CypherNode that NEdge belongs to
    bool valid;

    NEdge(Graph *graph, sqlite3_int64 id, CypherNode *cNode): graph(graph), iEdge(id), cNode(cNode) {
        if (cNode->edgeFilter(id) != GRAPH_SUCCESS) {
            valid = 0;
        } else {
            valid = 1;
            edge = graph->edgeMap->find(iEdge);
            CypherNode *nextCNode = cNode->next;
            std::map<sqlite3_int64, NNode*> *nodeMap = &nextCNode->nodeMap;
            sqlite3_int64 iNode = edge->toNode;
            if (nodeMap->find(iNode) != nodeMap->end()) { // NNode instance of iNode aready exists
                NNode *nn = (*nodeMap)[iNode];
                nn->inEdge.insert(this);
                to_node = nn;
            } else { // NNode instace of iNode doesn't exists
                NNode *nn = new NNode(graph, iNode, cNode->next);
                if (!nn->valid) {
                    delete nn;
                    valid = 0;
                } else {
                    nn->inEdge.insert(this);
                    (*nodeMap)[iNode] = nn;
                    to_node = nn;
                }
            }
        }
    }
};

struct CypherNode {
    int type; // NODE or EDGE
    bool certain;
    int constrainType; // NOCONSTRAIN or DEFINITE or ATTRIBUTE
    std::string constrain;
    std::vector<std::string>key; // ATTRIBUTE constrain type, key list of json
    std::vector<std::string>value; // ATTRIBUTE constrain type, value list of json
    int key_num;
    union
    {
        std::map<sqlite3_int64, NNode*> nodeMap;
        std::map<sqlite3_int64, NEdge*> edgeMap;
    };
    CypherNode *prev;
    CypherNode *next;
    Graph *graph;

    CypherNode(int type, int constrainType, std::string constrain): 
     type(type), certain(0), constrainType(constrainType), constrain(constrain), prev(nullptr), next(nullptr) {
        if (constrainType == ATTRIBUTE) {
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
            key_num = key.size();
        } else {
            key_num = -1;
        }
     }
    
     // judge whether a node meets the constrain
    int nodeFilter(sqlite3_int64 nodeId) {
        if (constrainType == NOCONSTRAIN) {
            return GRAPH_SUCCESS;
        } else if (constrainType == DEFINITE) {
            sqlite3_int64 id = std::stoll(constrain);
            int rc = (id == nodeId) ? GRAPH_SUCCESS : GRAPH_FAILED;
            return rc;
        } else { // ATTRIBUTE
            std::string attribute = graph->getNodeAttributeById(nodeId);
            json data = json::parse(attribute);
            for (int i = 0; i < key_num; i++) {
                std::string k = key[i];
                std::string v = value[i];
                if (!data.contains(k)) {
                    return GRAPH_FAILED;
                }
                auto v_type = data[k].type();
                if (v_type == json::value_t::string) {
                    std::string v_str = data[k];
                    if (v_str != v) {
                        return GRAPH_FAILED;
                        break;
                    }
                } else if (v_type == json::value_t::number_integer) {
                    int v_int = data[k];
                    if (v_int != std::stoi(v)) {
                        return GRAPH_FAILED;
                        break;
                    }
                } else if (v_type == json::value_t::number_float) {
                    double v_float = data[k];
                    if (v_float != std::stod(v)) {
                        return GRAPH_FAILED;
                        break;
                    }
                } else {
                    std::cerr << "ERROR: Incorrect type of value." << std::endl;
                    return GRAPH_FAILED;
                }
            }
            return GRAPH_SUCCESS;
        }
    }

    // judge whether a edge meets the constrain
    int edgeFilter(sqlite3_int64 edgeId) {
        if (constrainType == NOCONSTRAIN) {
            return GRAPH_SUCCESS;
        } else if (constrainType == DEFINITE) {
            sqlite3_int64 id = std::stoll(constrain);
            int rc = (id == edgeId) ? GRAPH_SUCCESS : GRAPH_FAILED;
            return rc;
        } else { // ATTRIBUTE
            std::string attribute = graph->getNodeAttributeById(edgeId);
            json data = json::parse(attribute);
            for (int i = 0; i < key_num; i++) {
                std::string k = key[i];
                std::string v = value[i];
                if (!data.contains(k)) {
                    return GRAPH_FAILED;
                }
                auto v_type = data[k].type();
                if (v_type == json::value_t::string) {
                    std::string v_str = data[k];
                    if (v_str != v) {
                        return GRAPH_FAILED;
                        break;
                    }
                } else if (v_type == json::value_t::number_integer) {
                    int v_int = data[k];
                    if (v_int != std::stoi(v)) {
                        return GRAPH_FAILED;
                        break;
                    }
                } else if (v_type == json::value_t::number_float) {
                    double v_float = data[k];
                    if (v_float != std::stod(v)) {
                        return GRAPH_FAILED;
                        break;
                    }
                } else {
                    std::cerr << "ERROR: Incorrect type of value." << std::endl;
                    return GRAPH_FAILED;
                }
            }
            return GRAPH_SUCCESS;
        }
    }
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
                        if (constrain_type == LABEL) {
                            std::string label = constrain.substr(1, constrain.length() - 2); // remove '"'
                            sqlite3_int64 id = graph->getNodeIdByLabel(label);
                            constrain = std::to_string(id);
                            constrain_type = DEFINITE;
                        } else if (constrain_type == ATTRIBUTE) {
                            constrain = constrain.substr(1, constrain.length() - 2); // remove '{' and '}'
                        }
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
                            if (constrain_type == LABEL) {
                                std::string label = constrain.substr(1, constrain.length() - 2); // remove '"'
                                sqlite3_int64 id = graph->getEdgeIdByLabel(label);
                                constrain = std::to_string(id);
                                constrain_type = DEFINITE;
                            } else if (constrain_type == ATTRIBUTE) {
                                constrain = constrain.substr(1, constrain.length() - 2); // remove '{' and '}'
                            }
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
        
    }

    int query(std::string var, std::set<sqlite3_int64> &set) {
        int index = parser->var_map[var];
        CypherNode *cnode = findCypherNode(index);
        if (cnode->type == NODE) {
            for (auto it = cnode->nodeMap.begin(); it != cnode->nodeMap.end(); it++) {
                set.insert(it->first);
            }
        } else {
            for (auto it = cnode->edgeMap.begin(); it != cnode->edgeMap.end(); it++) {
                set.insert(it->first);
            }
        }
        return cnode->type;
    }
};

#endif