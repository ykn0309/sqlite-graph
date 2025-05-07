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

int executeDijkstra(sqlite3 *db, std::string start_node, std::string end_node) {
    std::string sql = "SELECT dijkstra(\'" + start_node + "\', \'" + end_node + "\', \'\');";
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, nullptr);
    return rc;
}

void deInit(sqlite3 *db) {
    sqlite3_close(db);
}

int main() {
    int round_num = 5;
    std::vector<std::string> db_names = {
        "facebook_5.db"
        // "facebook_10.db"
        // "facebook_50.db"
        // "facebook_100.db"
        // "facebook_500.db"
        // "facebook_1000.db"
        // "facebook_2000.db"
        // "facebook_4000.db"
    };
    std::string end_node_label;
    end_node_label = "4";
    // end_node_label = "9";
    // end_node_label = "49";
    // end_node_label = "99";
    // end_node_label = "1912";
    // end_node_label = "1912";
    // end_node_label = "3290";
    // end_node_label = "4038";
    std::vector<std::vector<double>> Dijkstra_results;
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

        std::vector<double> Dijkstra_result;
        for (int j = 0; j < round_num; j++) {
            auto start = std::chrono::high_resolution_clock::now();
            rc = executeDijkstra(db, "0", end_node_label);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            double seconds = duration.count();
            Dijkstra_result.push_back(seconds);
        }
        Dijkstra_results.push_back(Dijkstra_result);
        deInit(db);
    }

    std::vector<double> Dijkstra_averages;
    for (int i = 0; i < num; i++) {
        double average_Dijkstra = 0;
        for (int j = 0; j < round_num; j++) {
            average_Dijkstra += Dijkstra_results[i][j];
        }
        average_Dijkstra /= round_num;
        Dijkstra_averages.push_back(average_Dijkstra);
    }

    for (int i = 0; i < num; i++) {
        std::cout << std::endl << Dijkstra_averages[i] << " ";
    }
}