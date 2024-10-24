#ifdef __cplusplus
extern "C" {
#endif
#include"sqlite3ext.h"
#include"sqlite3.h"
#ifdef __cplusplus
}
#endif
#include<string>
#include<cassert>
#include"types.h"

SQLITE_EXTENSION_INIT1

Graph *graph;

// Create a Graph object from edge table
// argv: table name, edge id, from node, to node
static void createGraphFromEdgeTable(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 4);
    std::string table_name = (const char*)sqlite3_value_text(argv[0]);
    std::string id_column_name = (const char*)sqlite3_value_text(argv[1]);
    std::string from_column_name = (const char*)sqlite3_value_text(argv[2]);
    std::string to_column_name = (const char*)sqlite3_value_text(argv[3]);
    int rc = SQLITE_OK;
    std::string sql;
    sql = "SELECT " + id_column_name + ", " + from_column_name + ", " + to_column_name + " from " + table_name;
    sqlite3 *db = sqlite3_context_db_handle(context);
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_result_error(context, "Failed to prepare query.", -1);
        return;
    }
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_int64 iEdge = sqlite3_column_int64(stmt, 0); // Edge id
        sqlite3_int64 from_node = sqlite3_column_int64(stmt, 1); // From-node id
        sqlite3_int64 to_node = sqlite3_column_int64(stmt, 2); // To-node id

    }
}

#ifdef _WIN32
__declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int sqlite3_graph_init(
    sqlite3 *db, 
    char **pzErrMsg, 
    const sqlite3_api_routines *pApi
){
    int rc = SQLITE_OK;
    SQLITE_EXTENSION_INIT2(pApi);
    /* insert code to initialize your extension here */
    graph = new Graph();
    sqlite3_create_function(db, "createAdjList", 2, SQLITE_UTF8, 0, createGraphFromEdgeTable, 0, 0);
    // sqlite3_create_function(db, "showAdjList", 0, SQLITE_UTF8, 0, show_adj_list, 0, 0);
    return rc;
}

#ifdef __cplusplus
}
#endif