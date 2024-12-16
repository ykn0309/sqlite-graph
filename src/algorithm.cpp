/*
 * @Author: Kainan Yang ykn0309@whu.edu.cn
 * @Date: 2024-12-03 20:32:18
 * @LastEditors: Kainan Yang ykn0309@whu.edu.cn
 * @LastEditTime: 2024-12-16 20:27:10
 * @FilePath: /sqlite-graph/src/algorithm.cpp
 * @Description: 
 * 
 */
#include"algorithm.h"


std::vector<std::string> BFS::runBFS() {
    sqlite3_int64 start = startNodeId;
    std::queue<Node*> q;
    std::unordered_set<Node*> visited;
    NodeMap *nodeMap = graph->nodeMap;
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

std::vector<std::string> DFS::runDFS() {
    sqlite3_int64 start = startNodeId;
    std::stack<Node*> st;
    std::unordered_set<Node*> visited;
    NodeMap *nodeMap = graph->nodeMap;
    Node *startNode = nodeMap->find(start);

    st.push(startNode);
    visited.insert(startNode);

    std::vector<std::string>result;
    while (!st.empty()) {
        Node *node = st.top();
        st.pop();
        std::string label = graph->getNodeLabelById(node->iNode);
        result.push_back(label);
        for (sqlite3_int64 neighbor_id : node->outNode) {
            Node *neighbor = nodeMap->find(neighbor_id);
            if (visited.find(node) == visited.end()) {
                visited.insert(neighbor);
                st.push(neighbor);
            }
        }
    }
    return result;
}

using PDI = std::pair<double, sqlite3_int64>;

void Dijkstra::runDijkstra() {
    std::priority_queue<PDI, std::vector<PDI>, std::greater<>> pq;
    dist[startNodeId] = 0;
    pq.push({0, startNodeId});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;

        Node *node = graph->nodeMap->find(u);
        for (sqlite3_int64 edge_id : node->outEdge) {
            double w = graph->getEdgeWeight(edge_id, weight_alias);
            Edge *e = graph->edgeMap->find(edge_id);
            sqlite3_int64 v = e->toNode;
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
}