/*
 * @Author: Kainan Yang ykn0309@whu.edu.cn
 * @Date: 2024-12-11 20:47:32
 * @LastEditors: Kainan Yang ykn0309@whu.edu.cn
 * @LastEditTime: 2024-12-16 20:09:14
 * @FilePath: /sqlite-graph/src/algorithm.h
 * @Description: 
 * 
 */
#ifndef ALGORITHM_H
#define ALGORITHM_H

#include"graph.h"
#include<vector>
#include<limits>
#include<queue>
#include<stack>
#include<unordered_set>
#include<unordered_map>

class BFS {
private:
    sqlite3_int64 startNodeId;
    Graph *graph;

public:
    BFS() = delete;
    
    BFS(Graph *graph, std::string start_label): graph(graph) {
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
    sqlite3_int64 startNodeId;
    Graph *graph;

public:
    DFS() = delete;

    DFS(Graph *graph, std::string start_label): graph(graph) {
        sqlite3_int64 id = graph->getNodeIdByLabel(start_label);
        if (id != GRAPH_SUCCESS) {
            std::cerr << "Cannot find node by label." << std::endl;
        } else {
            startNodeId = id;
        }
    }

    std::vector<std::string> runDFS();
};

class Dijkstra {
private:
    Graph *graph;
    sqlite3_int64 startNodeId;
    std::string weight_alias;
    std::unordered_map<sqlite3_int64, double> dist;
    std::unordered_map<sqlite3_int64, sqlite3_int64> prev;

public:
    Dijkstra() = delete;

    Dijkstra(Graph *graph, std::string start_label, std::string weight_alias)
    : graph(graph), weight_alias(weight_alias) {
        NodeMap *nodeMap = graph->nodeMap;

        sqlite3_int64 id = graph->getNodeIdByLabel(start_label);
        if (id != GRAPH_SUCCESS) {
            std::cerr << "Cannot find node by label." << std::endl;
        } else {
            startNodeId = id;
        }
        
        for (auto it = nodeMap->map.begin(); it != nodeMap->map.end(); it++) {
            sqlite3_int64 node_id = it->first;
            dist.insert({node_id, std::numeric_limits<double>::max()});
            prev.insert({node_id, -1});
        }
    }

    void runDijkstra();
};

#endif