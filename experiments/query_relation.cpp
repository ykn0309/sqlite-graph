#include"sqlite3.h"
#include<iostream>
#include<vector>
#include <chrono>

int init(sqlite3 **db, std::string &db_name) {
    int rc = sqlite3_open(db_name.c_str(), db);
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
        "facebook_500.db",
        "facebook_1000.db",
        "facebook_2000.db",
        "facebook_4000.db"
    };
    std::vector<std::string> sqls = {
        "SELECT e1.to_node AS first_degree_neighbors_count FROM edges e1 WHERE e1.from_node = '0';",
        "SELECT e2.to_node AS second_degree_neighbors_count FROM edges e1 JOIN edges e2 ON e1.to_node = e2.from_node WHERE e1.from_node = '0';",
        "SELECT e3.to_node AS third_degree_neighbors_count FROM edges e1 JOIN edges e2 ON e1.to_node = e2.from_node JOIN edges e3 ON e2.to_node = e3.from_node WHERE e1.from_node = '0';",
        "SELECT e4.to_node AS fourth_degree_neighbors_count FROM edges e1 JOIN edges e2 ON e1.to_node = e2.from_node JOIN edges e3 ON e2.to_node = e3.from_node JOIN edges e4 ON e3.to_node = e4.from_node WHERE e1.from_node = '0'; ",
        "SELECT e5.to_node AS fifth_degree_neighbors_count FROM edges e1 JOIN edges e2 ON e1.to_node = e2.from_node JOIN edges e3 ON e2.to_node = e3.from_node JOIN edges e4 ON e3.to_node = e4.from_node JOIN edges e5 ON e4.to_node = e5.from_node WHERE e1.from_node = '0';"
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

    for (int i = 0; i < num; i++) {
        std::vector<double> result = cypher_results[i];
        for (double t : result) {
            std::cout << t << ",";
        }
        std::cout << std::endl;
    }
}