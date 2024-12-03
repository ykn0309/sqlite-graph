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

SQLITE_EXTENSION_INIT1

#include"graph_manager.h"
#include"graph.h"

GraphManager &graphManager = GraphManager::getGraphManager();

/// @brief 创建一个图
/// @param argc 参数数量，必须等于8
static void createGraph(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 8);
    
    std::string node_table = (const char*)sqlite3_value_text(argv[0]);
    std::string edge_table = (const char*)sqlite3_value_text(argv[1]);
    std::string node_label_alias = (const char*)sqlite3_value_text(argv[2]);
    std::string node_attribute_alias = (const char*)sqlite3_value_text(argv[3]);
    std::string edge_label_alias = (const char*)sqlite3_value_text(argv[4]);
    std::string edge_attribute_alias = (const char*)sqlite3_value_text(argv[5]);
    std::string from_node_alias = (const char*)sqlite3_value_text(argv[6]);
    std::string to_node_alias = (const char*)sqlite3_value_text(argv[7]);
    BindingInfo *binding_info = new BindingInfo(node_table, edge_table, node_label_alias, node_attribute_alias,
                                                edge_label_alias, edge_attribute_alias, from_node_alias, to_node_alias);
    
    sqlite3 *db = sqlite3_context_db_handle(context);
    graphManager.newGraph(db, binding_info);
}

// Print adjadency table of the graph
static void printAdjTable(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 0);

    Graph *graph = graphManager.getGraph();
    if (graph == nullptr) {
        sqlite3_result_error(context, "Error: graph cannot be null.\n", -1);
    }
    std::vector<Node*> nodeList = graphManager.getGraph()->nodeList();
    std::string result;

    for (Node* n : nodeList) {
        result += std::to_string(n->iNode) + ": \n";

        // In nodes
        std::string line = "  in: ";
        if (!n->inNode.size()) {
            line += "null";
        } else {
            for (sqlite3_int64 id : n->inNode) {
                line += std::to_string(id) + ", ";
            }
            line.resize(line.size() - 2); // Delete the last comma.
        }
        result += line + "\n";

        // Out nodes
        line = "  out: ";
        if (!n->outNode.size()) {
            line += "null";
        } else {
            for (sqlite3_int64 id : n->outNode) {
                line += std::to_string(id) + ", ";
            }
            line.resize(line.size() - 2); // Delete the last comma.
        }
        result += line + "\n";
    }
    sqlite3_result_text(context, result.c_str(), result.length(), SQLITE_TRANSIENT);
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
    sqlite3_create_function(db, "createGraph", 8, SQLITE_UTF8, 0, createGraph, 0, 0);
    sqlite3_create_function(db, "showAdjTable", 0, SQLITE_UTF8, 0, printAdjTable, 0, 0);
    return rc;
}

#ifdef __cplusplus
}
#endif