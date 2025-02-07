#ifndef CYPHER_H
#define CYPHER_H

#include<iostream>
#include<vector>
#include<set>
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
        type(type), constrainType(constrainType), constrain(constrain) {}
};

class Parser {
public:
    std::string zCypher; // cypher string
    CypherNode *head; // head of linked list

    Parser::Parser(std::string zCypher): zCypher(zCypher) {
        head = nullptr;
    }

    // return type of constrain
    int whichConstrainType(std::string constrain) {

    }

    // if parses successfully return GRAPH_SUCCESS, else return GRAPH_FAILED
    int parse() {
        int zCypher_len = zCypher.length();
        if (zCypher[0] != '(') return GRAPH_FAILED;
        int i = 1;
        std::string constrain;
        while (1) {
            if (zCypher[i] == ')') {
                int constrain_type = whichConstrainType(constrain);
                CypherNode *cnode = new CypherNode(NODE, constrain_type, constrain);
                head = cnode;
                break;
            }
            constrain += zCypher[i];
            i++;
        }
    }
};

#endif