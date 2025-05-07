#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <limits>
#include <stack>
#include <chrono>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace std::chrono;

struct Edge {
    string to;
};

vector<Edge> get_neighbors(sqlite3* db, const string& from_label) {
    vector<Edge> neighbors;
    sqlite3_stmt* stmt;

    const char* sql = "SELECT to_node FROM edges WHERE from_node = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, from_label.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* to = sqlite3_column_text(stmt, 0);
            if (to) {
                neighbors.push_back({reinterpret_cast<const char*>(to)});
            }
        }
    }
    sqlite3_finalize(stmt);
    return neighbors;
}

bool dijkstra(sqlite3* db, const string& start, const string& end,
              vector<string>& path, int& total_steps) {
    unordered_map<string, int> dist;
    unordered_map<string, string> prev;
    unordered_map<string, bool> visited;

    queue<string> q;
    q.push(start);
    dist[start] = 0;

    while (!q.empty()) {
        string current = q.front(); q.pop();
        if (visited[current]) continue;
        visited[current] = true;

        if (current == end) break;

        for (const Edge& edge : get_neighbors(db, current)) {
            if (!dist.count(edge.to)) {
                dist[edge.to] = dist[current] + 1;
                prev[edge.to] = current;
                q.push(edge.to);
            }
        }
    }

    if (!dist.count(end)) {
        return false;
    }

    total_steps = dist[end];

    string node = end;
    stack<string> reverse_path;
    while (node != start) {
        reverse_path.push(node);
        node = prev[node];
    }
    reverse_path.push(start);

    while (!reverse_path.empty()) {
        path.push_back(reverse_path.top());
        reverse_path.pop();
    }

    return true;
}

int main() {
    vector<const char*> db_paths = {
        "facebook_5.db",
        "facebook_10.db",
        "facebook_50.db",
        "facebook_100.db",
        "facebook_500.db",
        "facebook_1000.db",
        "facebook_2000.db",
        "facebook_4000.db"
    };

    vector<string> end_nodes = {
        "4", "9", "49", "99", "1912", "1912", "3290", "4038"
    };

    const string start_label = "0";

    ofstream csv("experiments/dijkstra.csv");
    csv << "database,run_index,time_seconds,path_length\n";

    for (size_t i = 0; i < db_paths.size(); ++i) {
        const char* db_path = db_paths[i];
        const string& end_label = end_nodes[i];

        cout << "\n测试数据库: " << db_path << "，从 " << start_label << " 到 " << end_label << "...\n";

        double total_time = 0;
        int last_cost = -1;

        for (int run = 0; run < 5; ++run) {
            sqlite3* db;
            if (sqlite3_open(db_path, &db)) {
                cerr << "无法打开数据库: " << sqlite3_errmsg(db) << endl;
                continue;
            }

            vector<string> path;
            int cost;
            auto start_time = high_resolution_clock::now();
            bool success = dijkstra(db, start_label, end_label, path, cost);
            auto end_time = high_resolution_clock::now();

            double seconds = duration_cast<microseconds>(end_time - start_time).count() / 1000000.0;
            total_time += seconds;

            last_cost = success ? cost : -1;

            csv << db_path << "," << run + 1 << "," << fixed << setprecision(6)
                << seconds << "," << last_cost << "\n";

            sqlite3_close(db);
        }

        double avg_time = total_time / 5.0;
        // cout << fixed << setprecision(3);
        cout << "平均耗时: " << avg_time << " s\n";
        if (last_cost != -1)
            cout << "最短路径步数: " << last_cost << endl;
    }

    csv.close();
    cout << "\n✅ 所有结果已保存到 dijkstra_relation.csv\n";
    return 0;
}
