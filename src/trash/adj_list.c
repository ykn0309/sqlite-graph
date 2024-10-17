#if !defined(SQLITEINT_H)
#include "sqlite3ext.h"
#endif
SQLITE_EXTENSION_INIT1
#include <string.h>
#include <assert.h>

typedef struct node node;
struct node {
    sqlite3_int64 iNode; //node id
    node *next;
};


typedef struct adj_vtab adj_vtab;
struct adj_vtab {
    sqlite3_vtab base;
    node **adj_list; // adj list
    sqlite3_int64 nNode; // number of nodes
    sqlite3_int64 nEdge; // number of edges
    char *node_table; // node table name
    char *edge_table; // edge table name
};

typedef struct adj_cursor adj_cursor;
struct adj_cursor {
    sqlite3_vtab_cursor base;
    sqlite3_int64 iRowid;
};

static int adj_connect(
    sqlite3 *db,
    void *pAux,
    int argc, 
    const char *const*argv,
    sqlite3_vtab **ppVtab,
    char **pzErr
) {
    if (argc < 3) {
        *pzErr = sqlite3_mprintf("wrong number of arguments");
        return SQLITE_ERROR;
    }

    adj_vtab *pVtab;
    int rc;
    char *node_table = argv[1]; // name of node table
    char *edge_table = argv[2]; // name of edge table

    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(id INTEGER, list TEXT)");
    if (rc != SQLITE_OK) return rc;
    // SQLITE_OK
    pVtab = sqlite3_malloc(sizeof(*pVtab));
    if (!pVtab) return SQLITE_NOMEM;
    memset(pVtab, 0, sizeof(*pVtab));
    pVtab->node_table = node_table;

    char sql[128];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s;", node_table);
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_free(pVtab->node_table);
        sqlite3_free(pVtab);
        *pzErr = sqlite3_mprintf("Cannot prepare statment: %s", sqlite3_errmsg(db));
        return rc;
    }


    *ppVtab = (sqlite3_vtab*)pVtab;
    return SQLITE_OK;
}

static int adj_create(
    sqlite3 *db,
    void *pAux,
    int argc, 
    const char *const*argv,
    sqlite3_vtab **ppVtab,
    char **pzErr
) {
    return adj_connect(db, pAux, argc, argv, ppVtab, pzErr);
}

static int adj_disconnect(sqlite3_vtab *pVtab) {
    adj_vtab *p = (adj_vtab*)pVtab;
    sqlite3_free(p);
    return SQLITE_OK;
}

static int adj_destroy(sqlite3_vtab *pVtab) {
    return adj_disconnect(pVtab);
}

static int adj_open(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor) {

}
