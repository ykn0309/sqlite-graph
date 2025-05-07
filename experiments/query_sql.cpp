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

int executeCypher(sqlite3 *db, std::string sql) {
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, nullptr);
    return rc;
}

void deInit(sqlite3 *db) {
    sqlite3_close(db);
}

int main() {
    int round_num = 5;
    std::vector<std::string> db_names = {
        // "facebook_500.db"
        // "facebook_1000.db"
        // "facebook_2000.db"
        "facebook_4000.db"
    };
    std::vector<std::string> sqls = {
        "select cypher('(\"0\")-->(x)', 'x');",
        "select cypher('(\"0\")-->()-->(x)', 'x');",
        "select cypher('(\"0\")-->()-->()-->(x)', 'x');",
        "select cypher('(\"0\")-->()-->()-->()-->(x)', 'x');",
        "select cypher('(\"0\")-->()-->()-->()-->()-->(x)', 'x');"
    };
    std::vector<std::vector<double>> cypher_results;
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

        std::vector<double> cypher_result;
        for (int k = 0; k < sqls.size(); k++) {
            double sum_seconds = 0;
            for (int j = 0; j < round_num; j++) {
                auto start = std::chrono::high_resolution_clock::now();
                rc = executeCypher(db, sqls[k]);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                double seconds = duration.count();
                sum_seconds += seconds;
            }
            cypher_result.push_back(sum_seconds / round_num);
        }
        cypher_results.push_back(cypher_result);
        deInit(db);
    }

    std::vector<double> result = cypher_results[0];
    for (double t : result) {
        std::cout << t << ",";
    }
}