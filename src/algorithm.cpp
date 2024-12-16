/*
 * @Author: Kainan Yang ykn0309@whu.edu.cn
 * @Date: 2024-12-03 20:32:18
 * @LastEditors: Kainan Yang ykn0309@whu.edu.cn
 * @LastEditTime: 2024-12-16 11:22:01
 * @FilePath: /sqlite-graph/src/algorithm.cpp
 * @Description: 
 * 
 */
#include"algorithm.h"
#include<queue>
#include<unordered_set>

std::vector<std::string> BFS::runBFS() {
    sqlite3_int64 start = startNodeId;
    std::queue<Node*>q;
    std::unordered_set<Node*>visited;
    Node *startNode = nodeMap->find(start);
    
    q.push(startNode);
    visited.insert(startNode);
    
    std::vector<std::string>result;
    while (!q.empty()) {
        Node *node = q.front();
        q.pop();
        std::string label = graph->getNodeLabelById(node->iNode);
        result.push_back(label);
        for (sqlite3_int64 neighbor_id : node->outNode) {
            Node *neighbor = nodeMap->find(neighbor_id);
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                q.push(neighbor);
            }
        }
    }
    return result;
}