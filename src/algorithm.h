/*
 * @Author: Kainan Yang ykn0309@whu.edu.cn
 * @Date: 2024-12-11 20:47:32
 * @LastEditors: Kainan Yang ykn0309@whu.edu.cn
 * @LastEditTime: 2024-12-11 21:53:13
 * @FilePath: /sqlite-graph/src/algorithm.h
 * @Description: 
 * 
 */
#include"graph.h"

class BFS {
private:
    NodeMap *nodeMap;
    sqlite3_int64 startNodeId;

public:
    BFS() = delete;
    
    BFS(Graph *graph, std::string start_label) {
        nodeMap = graph->nodeMap;
        sqlite3_int64 id = graph->getNodeIdByLabel(start_label);
        if (id != GRAPH_SUCCESS) {
            std::cerr << "Cannot find node by label." << std::endl;
        } else {
            startNodeId = id;
        }
    }

    void runBFS();
};