#ifdef __cplusplus
extern "C" {
#endif
#include<assert.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"sqlite3ext.h"
#include"sqlite3.h"
#ifdef __cplusplus
}
#endif
#include"types.h"

SQLITE_EXTENSION_INIT1

struct Graph {
    Node **adj_list; // adj_list
    int nNode; // Number of nodes
};

Graph *graph;

// Node **adj_list; // adj_list
// static int nNode; // number of nodes

// Free the memory of adj_list
static void free_adj_list() {
    int n = graph->nNode;
    for (int i = 0; i < n; i++) {
        Node *cur = graph->adj_list[i];
        while (cur) {
            Node *next = cur->next;
            free(cur);
            cur = next;
        }
    }
    free(graph->adj_list);
}

// Insert adj node into adj list.
static void insert_adj_node(sqlite3_int64 from, sqlite3_int64 to) {
    Node *adj_node = (Node*)malloc(sizeof(Node));
    memset(adj_node, 0, sizeof(Node));
    adj_node->iNode = to;
    adj_node->next = graph->adj_list[from];
    graph->adj_list[from] = adj_node;
}

// Generate a string to present the adj list and return a pointer to the string.
static char* str_adj_list() {
    size_t buffer_size = 1024;
    char *result = (char *)malloc(buffer_size);
    if (!result) {
        return NULL; // Memory allocation failed
    }

    result[0] = '\0'; // Initialize as empty string

    char line[256];  // Buffer to store each line of adjacency info
    int n = graph->nNode;
    for (int i = 0; i < n; i++) {
        snprintf(line, sizeof(line), "Node %lld: ", (sqlite3_int64)i);
        strcat(result, line);

        // Traverse the adjacency list for the current node
        Node *current = graph->adj_list[i];
        if (!current) {
            strcat(result, "None");
        } else {
            while (current) {
                snprintf(line, sizeof(line), "%lld -> ", current->iNode);
                strcat(result, line);
                current = current->next;
            }
            // Remove the trailing " -> " and replace with a newline
            size_t len = strlen(result);
            result[len - 4] = '\n';  // Replace last " -> " with a newline
            result[len - 3] = '\0';  // End string correctly
        }

        // Check if the result buffer needs to be expanded
        if (strlen(result) + 256 > buffer_size) {
            buffer_size *= 2;
            result = (char *)realloc(result, buffer_size);
            if (!result) {
                return NULL; // Memory reallocation failed
            }
        }
    }

    return result;  // Return the dynamically generated string
}

static void create_adj_list(sqlite3_context *context, int argc, sqlite3_value **argv) {
    assert(argc == 2);
    const char *node_table = (const char*)sqlite3_value_text(argv[0]);
    const char *edge_table = (const char*)sqlite3_value_text(argv[1]);
    
    int rc;
    char sql[128];
    snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM %s;", node_table);
    sqlite3 *db = sqlite3_context_db_handle(context);
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_result_error(context, "Failed to prepare node count query.", -1);
        return;
    }
    int node_count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        node_count = sqlite3_column_int(stmt, 0);
        graph->nNode = node_count;
    }
    sqlite3_finalize(stmt);
    if (node_count <= 0) {
        sqlite3_result_error(context, "Invalid node count.", -1);
        return;
    }

    // If create_adj_list() is called more than once, adj_list has already been allocated memory.
    // So we should free the previous memory.
    if (graph->adj_list) {
        free_adj_list();
    }

    graph->adj_list = (Node**)malloc(node_count * sizeof(Node*));
    memset(graph->adj_list, 0, node_count * sizeof(Node*));
    if(!graph->adj_list) {
        sqlite3_result_error(context, "Memory allocation failed.", -1);
        return;
    }

    snprintf(sql, sizeof(sql), "SELECT from_node, to_node FROM %s;", edge_table);
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        free_adj_list(); // free the memory of adj list
        sqlite3_result_error(context, "Failed to prepare edge query.", -1);
        return;
    }

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_int64 from = sqlite3_column_int64(stmt, 0);
        sqlite3_int64 to = sqlite3_column_int64(stmt, 1);
        assert(from < node_count && to < node_count);
        insert_adj_node(from, to);
    }
    sqlite3_finalize(stmt);
    sqlite3_result_text(context, "createAdjList OK.", -1, SQLITE_STATIC);
}

static void show_adj_list(sqlite3_context *context, int argc, sqlite3_value **argv) {
    char *adj_list_str = str_adj_list();
    // sqlite3_result_text(context, adj_list_str, -1, SQLITE_TRANSIENT);
    printf("%s", adj_list_str);
    free(adj_list_str);
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
  graph = (Graph*)malloc(sizeof(Graph));
  memset(graph, 0, sizeof(Graph));
  sqlite3_create_function(db, "createAdjList", 2, SQLITE_UTF8, 0, create_adj_list, 0, 0);
  sqlite3_create_function(db, "showAdjList", 0, SQLITE_UTF8, 0, show_adj_list, 0, 0);
  return rc;
}

#ifdef __cplusplus
}
#endif