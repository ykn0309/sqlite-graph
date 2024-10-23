#include<stdlib.h>
#include<string.h>
#include"types.h"

// Do BFS to a graph.
// param:
//      adj_list: Adj list of a graph
//      nNode: Number of nodes
//      src: Id of start node
// return:
//      A link list of Edge.
static Edge* bfs(Node** adj_list, int nNode, sqlite3_int64 src, Edge *edge) {
    Node *src_node = adj_list[src];
    char *visited;
    visited = (char*)malloc(nNode);
    memset(visited, 0, nNode);
    
}