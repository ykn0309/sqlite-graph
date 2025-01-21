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
#include<stack>

SQLITE_EXTENSION_INIT1

#include"graph_manager.h"
#include"graph.h"
#include"algorithm.h"

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


/// @brief 向图中添加一个结点
/// @param argc 参数数量，必须等于2
static void addNode(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 2);
    std::string label = (const char*)sqlite3_value_text(argv[0]);
    std::string attribute = (const char*)sqlite3_value_text(argv[1]);
    Graph *graph = graphManager.getGraph();
    graph->addNode(label, attribute);
}

/// @brief 向图中删除一个结点
/// @param argc 参数数量，必须等于1
static void removeNode(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    std::string label = (const char*)sqlite3_value_text(argv[0]);
    Graph *graph = graphManager.getGraph();
    graph->removeNode(label);
}

/// @brief 向图中添加一条边
/// @param argc 参数数量，必须等于4
static void addEdge(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 4);
    std::string from_label = (const char*)sqlite3_value_text(argv[0]);
    std::string to_label = (const char*)sqlite3_value_text(argv[1]);
    std::string label = (const char*)sqlite3_value_text(argv[2]);
    std::string attribute = (const char*)sqlite3_value_text(argv[3]);
    Graph *graph = graphManager.getGraph();
    graph->addEdge(label, attribute, from_label, to_label);
}

/// @brief 向图中删除一条边
/// @param argc 参数数量，必须等于1
static void removeEdge(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    std::string label = (const char*)sqlite3_value_text(argv[0]);
    Graph *graph = graphManager.getGraph();
    graph->removeEdge(label);
}

static void bfs(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    std::string start_label = (const char*)sqlite3_value_text(argv[0]);
    Graph *graph = graphManager.getGraph();
    BFS b = BFS(graph, start_label);
    std::vector<std::string> result = b.runBFS();
    std::string s;
    for (auto node : result) {
        s += node + ", ";
    }
    s.resize(s.size() - 2);
    s += "\n";
    sqlite3_result_text(context, s.c_str(), s.length(), SQLITE_TRANSIENT);
}

static void dfs(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 1);
    std::string start_label = (const char*)sqlite3_value_text(argv[0]);
    Graph *graph = graphManager.getGraph();
    DFS d = DFS(graph, start_label);
    std::vector<std::string> result = d.runDFS();
    std::string s;
    for (auto node : result) {
        s += node + ", ";
    }
    s.resize(s.size() - 2);
    s += "\n";
    sqlite3_result_text(context, s.c_str(), s.length(), SQLITE_TRANSIENT);
}

static void dijkstra(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 3);
    std::string start_label = (const char*)sqlite3_value_text(argv[0]);
    std::string end_label = (const char*)sqlite3_value_text(argv[1]);
    std::string weight_alias = (const char*)sqlite3_value_text(argv[2]);
    Graph *graph = graphManager.getGraph();
    Dijkstra dijkstra = Dijkstra(graph, start_label, weight_alias);
    dijkstra.runDijkstra();
    sqlite3_int64 start_id = dijkstra.startNodeId;
    sqlite3_int64 end_id = graph->getNodeIdByLabel(end_label);
    std::stack<sqlite3_int64> st;
    st.push(end_id);
    while (st.top() != start_id) {
        sqlite3_int64 top = st.top();
        st.push(dijkstra.prev[top]);
    }
    std::string out;
    while (!st.empty()) {
        std::string label = graph->getNodeLabelById(st.top());
        out += label + "->";
        st.pop();
    }
    out.erase(out.size() - 2);
    std::cout << out << std::endl;
    std::cout << dijkstra.dist[end_id];
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
    sqlite3_create_function(db, "addNode", 2, SQLITE_UTF8, 0, addNode, 0, 0);
    sqlite3_create_function(db, "removeNode", 1, SQLITE_UTF8, 0, removeNode, 0, 0);
    sqlite3_create_function(db, "addEdge", 4, SQLITE_UTF8, 0, addEdge, 0, 0);
    sqlite3_create_function(db, "removeEdge", 1, SQLITE_UTF8, 0, removeEdge, 0, 0);
    sqlite3_create_function(db, "bfs", 1, SQLITE_UTF8, 0, bfs, 0, 0);
    sqlite3_create_function(db, "dfs", 1, SQLITE_UTF8, 0, dfs, 0, 0);
    sqlite3_create_function(db, "dijkstra", 3, SQLITE_UTF8, 0, dijkstra, 0, 0);
    return rc;
}

#ifdef __cplusplus
}
#endif