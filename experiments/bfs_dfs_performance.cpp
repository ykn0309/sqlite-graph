#include"sqlite3.h"
#include<iostream>
#include<vector>
#include <chrono>

int init(sqlite3 **db, std::string &db_name) {
    int rc = sqlite3_open(db_name.c_str(), db);
    return rc;
}

int load_extension(sqlite3 *db, char *errMsg) {
    sqlite3_enable_load_extension(db, 1);
    int rc = sqlite3_load_extension(db, "./graph.so", 0, &errMsg);
    return rc;
}

int executeBFS(sqlite3 *db, std::string start_node, char *errMsg) {
    std::string sql = "SELECT bfs(\'" + start_node + "\');";
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    return rc;
}

int executeDFS(sqlite3 *db, std::string start_node, char *errMsg) {
    std::string sql = "SELECT dfs(\'" + start_node + "\');";
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    return rc;
}

void deInit(sqlite3 *db) {
    sqlite3_close(db);
}

int main() {
    int round_num = 5;
    std::vector<std::string> db_names = {
        // "facebook_5.db"
        // "facebook_10.db"
        // "facebook_50.db"
        // "facebook_100.db"
        // "facebook_500.db"
        // "facebook_1000.db"
        // "facebook_2000.db"
        "facebook_4000.db"
    };
    std::vector<std::vector<double>> BFS_results;
    std::vector<std::vector<double>> DFS_results;
    char *errMsg = nullptr;

    int num = db_names.size();
    for (int i = 0; i < num; i++) {
        sqlite3 *db;
        std::string db_name = db_names[i];
        int rc = init(&db, db_name);
        if (rc) {
            std::cerr << "无法打开数据库: " << sqlite3_errmsg(db) << std::endl;
            return rc;
        }
        rc = load_extension(db, errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "加载扩展失败: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        } else {
            std::cout << "扩展加载成功！" << std::endl;
        }
        std::string createGraph_sql = "SELECT createGraph('nodes', 'edges', 'label', 'attribute', 'label', 'attribute', 'from_node', 'to_node');";
        rc = sqlite3_exec(db, createGraph_sql.c_str(), 0, 0, NULL);
        if (rc != SQLITE_OK) {
            std::cerr << "createGraph()执行失败 " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }

        std::vector<double> BFS_result;
        std::vector<double> DFS_result;
        for (int j = 0; j < round_num; j++) {
            auto start = std::chrono::high_resolution_clock::now();
            rc = executeBFS(db, "0", errMsg);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            double seconds = duration.count();
            BFS_result.push_back(seconds);

            start = std::chrono::high_resolution_clock::now();
            rc = executeDFS(db, "0", errMsg);
            end = std::chrono::high_resolution_clock::now();
            duration = end - start;
            seconds = duration.count();
            DFS_result.push_back(seconds);
        }
        BFS_results.push_back(BFS_result);
        DFS_results.push_back(DFS_result);
        deInit(db);
    }

    std::vector<double> BFS_averages;
    std::vector<double> DFS_averages;
    for (int i = 0; i < num; i++) {
        double average_BFS = 0;
        double average_DFS = 0;
        for (int j = 0; j < round_num; j++) {
            average_BFS += BFS_results[i][j];
            average_DFS += DFS_results[i][j];
        }
        average_BFS /= round_num;
        average_DFS /= round_num;
        BFS_averages.push_back(average_BFS);
        DFS_averages.push_back(average_DFS);
    }

    for (int i = 0; i < num; i++) {
        std::cout << BFS_averages[i] << " ";
    }
    std::cout << std::endl;

    for (int i = 0; i < num; i++) {
        std::cout << DFS_averages[i] << " ";
    }
}