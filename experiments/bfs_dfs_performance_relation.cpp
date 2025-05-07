#include "sqlite3.h"
#include <iostream>
#include <string>
#include <unordered_set>
#include <queue>
#include <stack>
#include <chrono>

// 查询一个结点的所有邻居（即以这个节点为 from_node 的所有 to_node）
std::vector<std::string> getNeighbors(sqlite3* db, const std::string& label) {
    std::vector<std::string> neighbors;
    const char* sql = "SELECT to_node FROM edges WHERE from_node = ?";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, label.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* neighbor = sqlite3_column_text(stmt, 0);
            if (neighbor) {
                neighbors.emplace_back(reinterpret_cast<const char*>(neighbor));
            }
        }
    }

    sqlite3_finalize(stmt);
    return neighbors;
}

void bfs(sqlite3* db, const std::string& start_label) {
    std::unordered_set<std::string> visited;
    std::queue<std::string> q;

    q.push(start_label);
    visited.insert(start_label);

    // std::cout << "BFS: ";

    while (!q.empty()) {
        std::string current = q.front();
        q.pop();
        // std::cout << current << " ";

        auto neighbors = getNeighbors(db, current);
        for (const auto& neighbor : neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                q.push(neighbor);
            }
        }
    }

    // std::cout << std::endl;
}

void dfs(sqlite3* db, const std::string& start_label) {
    std::unordered_set<std::string> visited;
    std::stack<std::string> s;

    s.push(start_label);

    // std::cout << "DFS: ";

    while (!s.empty()) {
        std::string current = s.top();
        s.pop();

        if (visited.find(current) != visited.end()) continue;

        visited.insert(current);
        // std::cout << current << " ";

        auto neighbors = getNeighbors(db, current);
        // 倒序入栈，确保遍历顺序和 BFS 一致方向（可选）
        for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
            if (visited.find(*it) == visited.end()) {
                s.push(*it);
            }
        }
    }

    // std::cout << std::endl;
}


int main() {
    sqlite3* db;
    // std::string db_path = "facebook_5.db";
    // std::string db_path = "facebook_10.db";
    // std::string db_path = "facebook_50.db";
    // std::string db_path = "facebook_100.db";
    // std::string db_path = "facebook_500.db";
    // std::string db_path = "facebook_1000.db";
    // std::string db_path = "facebook_2000.db";
    std::string db_path = "facebook_4000.db";
    if (sqlite3_open(db_path.c_str(), &db)) {
        std::cerr << "打开数据库失败: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    std::string start_label = "0"; // 假设从 label 为 "0" 的结点开始
    
    std::vector<double> bfs_times;

    std::vector<double> dfs_times;
    for (int i = 0; i < 5; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        bfs(db, start_label);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        double seconds = duration.count();
        bfs_times.push_back(seconds);

        start = std::chrono::high_resolution_clock::now();
        dfs(db, start_label);
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        seconds = duration.count();
        dfs_times.push_back(seconds);
    }

    sqlite3_close(db);

    double average_bfs = 0;
    double average_dfs = 0;
    for (int i = 0; i < 5; i++) {
        average_bfs += bfs_times[i];
        average_dfs += dfs_times[i];
    }
    average_bfs /= 5;
    average_dfs /= 5;

    std::cout << db_path << std::endl << average_bfs << std::endl << average_dfs;

    return 0;
}
