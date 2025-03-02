#ifndef CYPHER_H
#define CYPHER_H

#include<iostream>
#include<vector>
#include<set>
#include<unordered_map>
#include"json.hpp"
#include"defs.h"
#include"graph.h"
#include"graph_manager.h"

#ifdef __cplusplus
extern "C" {
#endif
#include"sqlite3.h"
#ifdef __cplusplus
}
#endif

using json = nlohmann::json;

struct NNode;
struct NEdge;
struct CypherNode;

struct NNode {
    Graph *graph;
    sqlite3_int64 iNode; // node id
    Node *node; // corresponding Node instance of an NNode object
    std::set<NEdge*> inEdge; // set of in edges
    std::set<NEdge*> outEdge; // set of out edges
    CypherNode *cNode; // CypherNode that NNode belongs to
    bool valid;
    
    NNode(Graph *graph, sqlite3_int64 id, CypherNode *cNode);
};

struct NEdge {
    Graph *graph;
    sqlite3_int64 iEdge; // edge id
    Edge *edge; // corresponding Edge instance of an Edge object
    NNode *from_node; // from node
    NNode *to_node; // to node
    CypherNode *cNode; // CypherNode that NEdge belongs to
    bool valid;

    NEdge(Graph *graph, sqlite3_int64 id, CypherNode *cNode);
};


struct CypherNode {
    int type; // NODE or EDGE
    bool isVariable;
    int constrainType; // NOCONSTRAIN or DEFINITE or ATTRIBUTE
    std::string constrain;
    std::vector<std::string>key; // ATTRIBUTE constrain type, key list of json
    std::vector<std::string>value; // ATTRIBUTE constrain type, value list of json
    int key_num;
    std::unordered_map<sqlite3_int64, NNode*> *nodeMap;
    std::unordered_map<sqlite3_int64, NEdge*> *edgeMap;
    CypherNode *prev;
    CypherNode *next;
    Graph *graph;
    std::set<sqlite3_int64> removed_nodes; // nodes that don't meet the constrain will be put into this set to avoid another constrain judge

    CypherNode(int type, int constrainType, std::string constrain, Graph *graph): 
     type(type), isVariable(0), constrainType(constrainType), constrain(constrain), graph(graph), prev(nullptr), next(nullptr) {
        nodeMap = new std::unordered_map<sqlite3_int64, NNode*>;
        edgeMap = new std::unordered_map<sqlite3_int64, NEdge*>;
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
    
    ~CypherNode() {
        for (auto it = nodeMap->begin(); it != nodeMap->end(); it++) {
            NNode *nnode = it->second;
            delete nnode;
        }
        for (auto it = edgeMap->begin(); it != edgeMap->end(); it++) {
            NEdge *nedge = it->second;
            delete nedge;
        }
        delete nodeMap;
        delete edgeMap;
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


NNode::NNode(Graph *graph, sqlite3_int64 id, CypherNode *cNode): graph(graph), iNode(id), cNode(cNode) {
    if (cNode->nodeFilter(id) != GRAPH_SUCCESS) {
        valid = 0;
    } else {
        valid = 1;
        node = graph->nodeMap->find(iNode);
        CypherNode *nextCNode = cNode->next;
        if (nextCNode == nullptr);
        else { // cNode has next CypherNode
            std::unordered_map<sqlite3_int64, NEdge*> *edgeMap = nextCNode->edgeMap;
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

NEdge::NEdge(Graph *graph, sqlite3_int64 id, CypherNode *cNode): graph(graph), iEdge(id), cNode(cNode) {
    if (cNode->edgeFilter(id) != GRAPH_SUCCESS) {
        valid = 0;
    } else {
        valid = 1;
        edge = graph->edgeMap->find(iEdge);
        CypherNode *nextCNode = cNode->next;
        std::unordered_map<sqlite3_int64, NNode*> *nodeMap = nextCNode->nodeMap;
        sqlite3_int64 iNode = edge->toNode;
        if (nodeMap->find(iNode) != nodeMap->end()) { // NNode instance of iNode aready exists
            NNode *nn = (*nodeMap)[iNode];
            nn->inEdge.insert(this);
            to_node = nn;
        } else if (nextCNode->removed_nodes.find(iNode) != nextCNode->removed_nodes.end()) { // nodes that don't meet the constrain
            valid = 0;
        } else { // NNode instace of iNode doesn't exists
            NNode *nn = new NNode(graph, iNode, nextCNode);
            if (!nn->valid) {
                nextCNode->removed_nodes.insert(iNode);
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

class Parser {
public:
    std::string zCypher; // cypher string
    CypherNode *head; // head of linked list
    std::unordered_map<std::string, int> var_map; // map variable name to cyphernode index
    std::vector<CypherNode*> variableCNode; // CypherNodes that constrain type is variable
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
                p->isVariable = 1;
                std::string constrain = p->constrain;
                p->constrain = "";
                var_map[constrain] = i;
                variableCNode.push_back(p);
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
                CypherNode *cnode = new CypherNode(status, constrain_type, constrain, graph);
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
                        CypherNode *cnode = new CypherNode(status, constrain_type, constrain, graph);
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
                    CypherNode *cnode =  new CypherNode(status, NOCONSTRAIN, constrain, graph);
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
                            CypherNode *cnode = new CypherNode(status, constrain_type, constrain, graph);
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

    CypherNode *findCypherNodeByIndex(int k) {
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

    void dfs(NNode *nn, NEdge *ne, std::vector<sqlite3_int64> &result) {
        bool add = 0;
        if (nn == nullptr) { // NEdge
            if (ne->cNode->isVariable) {
                result.push_back(ne->iEdge);
                add = 1;
            }
            dfs(ne->to_node, nullptr, result);
        } else { // NNode
            if (nn->cNode->isVariable) {
                result.push_back(nn->iNode);
                add = 1;
            }
            if (nn->cNode->next == nullptr) {
                results.push_back(result);
                if (add) {
                    result.pop_back();
                }
                return;
            }
            for (auto it = nn->outEdge.begin(); it != nn->outEdge.end(); it++) {
                dfs(nullptr, *it, result);
            }
        }
        if (add) {
            result.pop_back();
        }
    }

public:
    std::vector<std::vector<sqlite3_int64>> results;

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

    void execute() {
        std::unordered_map<sqlite3_int64, Node*> *map = &graph->nodeMap->map;
        for (auto it = map->begin(); it != map->end(); it++) {
            NNode *nn = new NNode(graph, it->first, head);
            if (!nn->valid) {
                delete nn;
            } else {
                (*head->nodeMap)[it->first] = nn;
            }
        }
    }

    void buildResults() {
        for (auto it = head->nodeMap->begin(); it != head->nodeMap->end(); it++) {
            std::vector<sqlite3_int64> result;
            dfs(it->second, nullptr, result);
        }
    }

    Graph* getGraph() {
        return graph;
    }

    CypherNode* findCypherNodeByColumnId(int i) {
        int var_len = parser->variableCNode.size();
        if (i >= 0 && i < var_len) {
            return parser->variableCNode[i];
        } else {
            std::cerr << "ERROR: Column id out of bound." << std::endl;
            return nullptr;
        }
    }

    int query(std::string var, std::set<sqlite3_int64> &set) {
        int index = parser->var_map[var];
        CypherNode *cnode = findCypherNodeByIndex(index);
        if (cnode->type == NODE) {
            for (auto it = cnode->nodeMap->begin(); it != cnode->nodeMap->end(); it++) {
                set.insert(it->first);
            }
        } else {
            for (auto it = cnode->edgeMap->begin(); it != cnode->edgeMap->end(); it++) {
                set.insert(it->first);
            }
        }
        return cnode->type;
    }
};

struct CypherVtab {
    sqlite3_vtab base;
    Cypher *cypher;
};

struct CypherVtabCursor {
    sqlite3_vtab_cursor base;
    sqlite3_int64 iRowid;
};

static int cyphervtabCreate(sqlite3 *db,
    void *pAux,
    int argc, const char *const*argv,
    sqlite3_vtab **ppVtab,
    char **pzErr
) {
    CypherVtab *pNew;
    int rc;
    std::string sql = "CREATE TABLE cypher_result(";
    for (int i = 4; i < argc; i++) {
        sql += argv[i];
        if (i != argc - 1) {
            sql += ",";
        }
    }
    sql += ")";
    rc = sqlite3_declare_vtab(db, sql.c_str());
    if (rc == SQLITE_OK) {
        pNew = new(std::nothrow) CypherVtab();
        *ppVtab = (sqlite3_vtab*)pNew;
        if (pNew == nullptr) return SQLITE_NOMEM;
        GraphManager &graphManager = GraphManager::getGraphManager();
        Graph *graph = graphManager.getGraph();
        std::string zCypher = argv[3];
        zCypher = zCypher.substr(1, zCypher.length() - 2);
        Cypher *cypher = new Cypher(graph, zCypher);
        pNew->cypher = cypher;
        if (cypher->parse() == GRAPH_FAILED) return SQLITE_ERROR;
        cypher->execute();
        cypher->buildResults();
    }
    return rc;
}

static int cyphervtabConnect(sqlite3 *db,
    void *pAux,
    int argc, const char *const*argv,
    sqlite3_vtab **ppVtab,
    char **pzErr
) {
    CypherVtab *pNew;
    int rc;
    std::string sql = "CREATE TABLE cypher_result(";
    for (int i = 1; i < argc; i++) {
        sql += argv[i];
        if (i != argc - 1) {
            sql += ",";
        }
    }
    sql += ")";
    rc = sqlite3_declare_vtab(db, sql.c_str());
    if (rc == SQLITE_OK) {
        pNew = new(std::nothrow) CypherVtab();
        *ppVtab = (sqlite3_vtab*)pNew;
        if (pNew == nullptr) return SQLITE_NOMEM;
        GraphManager &graphManager = GraphManager::getGraphManager();
        Graph *graph = graphManager.getGraph();
        std::string zCypher = argv[0];
        Cypher *cypher = new Cypher(graph, zCypher);
        pNew->cypher = cypher;
        if (cypher->parse() == GRAPH_FAILED) return SQLITE_ERROR;
        cypher->execute();
        cypher->buildResults();
    }
    return rc;
}

static int cyphervtabDisconnect(sqlite3_vtab *pVtab) {\
    CypherVtab *p = (CypherVtab*)pVtab;
    Cypher *cypher = p->cypher;
    delete cypher;
    delete p;
    return SQLITE_OK;
}

static int cyphervtabDestroy(sqlite3_vtab *pVtab) {\
    CypherVtab *p = (CypherVtab*)pVtab;
    Cypher *cypher = p->cypher;
    delete cypher;
    delete p;
    return SQLITE_OK;
}

static int cyphervtabOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor) {
    CypherVtabCursor *pCur;
    pCur = new(std::nothrow) CypherVtabCursor();
    if (pCur == nullptr) return SQLITE_NOMEM;
    *ppCursor = &pCur->base;
    return SQLITE_OK;
}

static int cyphervtabClose(sqlite3_vtab_cursor *cur) {
    CypherVtabCursor *pCur = (CypherVtabCursor*)cur;
    delete pCur;
    return SQLITE_OK;
}

static int cyphervtabNext(sqlite3_vtab_cursor *cur){
    CypherVtabCursor *pCur = (CypherVtabCursor*)cur;
    pCur->iRowid++;
    return SQLITE_OK;
}

static int cyphervtabColumn(
    sqlite3_vtab_cursor *cur,   /* The cursor */
    sqlite3_context *ctx,       /* First argument to sqlite3_result_...() */
    int i                       /* Which column to return */
) {
    CypherVtabCursor *pCur = (CypherVtabCursor*)cur;
    CypherVtab *p = (CypherVtab*)cur->pVtab;
    std::vector<std::vector<sqlite3_int64>> *results = &p->cypher->results;
    sqlite3_int64 id = (*results)[pCur->iRowid][i];
    Graph *graph = p->cypher->getGraph();
    CypherNode *cnode = p->cypher->findCypherNodeByColumnId(i);
    std::string label;
    if (cnode->type == NODE) {
        label = graph->getNodeLabelById(id);
    } else {
        label = graph->getEdgeLabelById(id);
    }
    sqlite3_result_text(ctx, label.c_str(), label.length(), SQLITE_TRANSIENT);
    return SQLITE_OK;
}

static int cyphervtabRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
    CypherVtabCursor *pCur = (CypherVtabCursor*)cur;
    *pRowid = pCur->iRowid;
    return SQLITE_OK;
}

static int cyphervtabEof(sqlite3_vtab_cursor *cur) {
    CypherVtabCursor *pCur = (CypherVtabCursor*)cur;
    CypherVtab *p = (CypherVtab*)cur->pVtab;
    int row_num = p->cypher->results.size();
    return pCur->iRowid >= row_num;
}

static int cyphervtabFilter(
    sqlite3_vtab_cursor *pVtabCursor, 
    int idxNum, const char *idxStr,
    int argc, sqlite3_value **argv
) {
    CypherVtabCursor *pCur = (CypherVtabCursor*)pVtabCursor;
    pCur->iRowid = 0;
    return SQLITE_OK;
}

static int cyphervtabBestIndex(
    sqlite3_vtab *tab,
    sqlite3_index_info *pIdxInfo
) {
    return SQLITE_OK;
}

static sqlite3_module cyphervtabModule = {
    /* iVersion    */ 0,
    /* xCreate     */ cyphervtabCreate,
    /* xConnect    */ cyphervtabConnect,
    /* xBestIndex  */ cyphervtabBestIndex,
    /* xDisconnect */ cyphervtabDisconnect,
    /* xDestroy    */ cyphervtabDestroy,
    /* xOpen       */ cyphervtabOpen,
    /* xClose      */ cyphervtabClose,
    /* xFilter     */ cyphervtabFilter,
    /* xNext       */ cyphervtabNext,
    /* xEof        */ cyphervtabEof,
    /* xColumn     */ cyphervtabColumn,
    /* xRowid      */ cyphervtabRowid,
    /* xUpdate     */ 0,
    /* xBegin      */ 0,
    /* xSync       */ 0,
    /* xCommit     */ 0,
    /* xRollback   */ 0,
    /* xFindMethod */ 0,
    /* xRename     */ 0,
    /* xSavepoint  */ 0,
    /* xRelease    */ 0,
    /* xRollbackTo */ 0,
    /* xShadowName */ 0,
    /* xIntegrity  */ 0
};

#endif