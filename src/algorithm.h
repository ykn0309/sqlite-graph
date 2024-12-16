/*
 * @Author: Kainan Yang ykn0309@whu.edu.cn
 * @Date: 2024-12-11 20:47:32
 * @LastEditors: Kainan Yang ykn0309@whu.edu.cn
 * @LastEditTime: 2024-12-16 11:39:59
 * @FilePath: /sqlite-graph/src/algorithm.h
 * @Description: 
 * 
 */
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include"graph.h"
#include<vector>

class BFS {
private:
    NodeMap *nodeMap;
    sqlite3_int64 startNodeId;
    Graph *graph;

public:
    BFS() = delete;
    
    BFS(Graph *graph, std::string start_label): graph(graph) {
        nodeMap = graph->nodeMap;
        sqlite3_int64 id = graph->getNodeIdByLabel(start_label);
        if (id != GRAPH_SUCCESS) {
            std::cerr << "Cannot find node by label." << std::endl;
        } else {
            startNodeId = id;
        }
    }

    std::vector<std::string> runBFS();
};

class DFS {
private:
    NodeMap *nodeMap;
    sqlite3_int64 startNodeId;
    Graph *graph;

public:
    DFS() = delete;

    DFS(Graph *graph, std::string start_label): graph(graph) {
        nodeMap = graph->nodeMap;
        sqlite3_int64 id = graph->getNodeIdByLabel(start_label);
        if (id != GRAPH_SUCCESS) {
            std::cerr << "Cannot find node by label." << std::endl;
        } else {
            startNodeId = id;
        }
    }

    std::vector<std::string> runDFS();
};

#endif