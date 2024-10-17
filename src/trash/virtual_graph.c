#include<sqlite3ext.h>
#include<sqlite3.h>
#include<string.h>

SQLITE_EXTENSION_INIT1

struct node_vtab
{
    sqlite3_vtab base;
    const char *table_name; // table name
    unsigned int nNode; // count of nodes in table
};

// struct edge_vtab
// {
//     sqlite3_vtab base;
//     unsigned int nEdge;
// };

struct node_cursor
{
    sqlite3_vtab_cursor base;
    int current; // current position of cursor
    node_vtab *pVtab; // pointer to vtab
};

// struct edge_cursor
// {
//     sqlite3_vtab_cursor base;
// };

typedef struct node_vtab node_vtab;
typedef struct edge_vtab edge_vtab;
typedef struct node_cursor node_cursor;
typedef struct edge_cursor edge_cursor;

static int node_create( sqlite3* db,
                        void *pAux,
                        int argc,
                        const char* const* argv,
                        sqlite3_vtab **ppVtab,
                        char **pzErr) {
    // argv[0] is the name of the module being invoked, argv[1] is table name
    if (argc < 2) {
        *pzErr = sqlite3_mprintf("Wrong number of arguments!");
        return SQLITE_ERROR;
    }

    node_vtab *vtab = sqlite3_malloc(sizeof(node_vtab));
    if (!vtab) return SQLITE_NOMEM;
    memset(vtab, 0, sizeof(node_vtab));
    const char *table_name = argv[1];
    vtab->table_name = strdup(table_name);
    if (!vtab->table_name) {
        sqlite3_free(vtab);
        return SQLITE_NOMEM;
    }

    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM %s;", table_name);
    sqlite3_stmt *stmt;
    int rc;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_free(vtab->table_name);
        sqlite3_free(vtab);
        *pzErr = sqlite3_mprintf("Cannot prepare statment: %s", sqlite3_errmsg(db));
        return rc;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        vtab->nNode = sqlite3_column_int(stmt, 0);
    } else {
        vtab->nNode = 0;
    }
    sqlite3_finalize(stmt);

    // define virtual table
    rc = sqlite3_declare_vtab(db, "CREATE TABLE x(id INTEGER, name TEXT)");
    if (rc != SQLITE_OK) {
        sqlite3_free(vtab->table_name);
        sqlite3_free(vtab);
        *pzErr = sqlite3_mprintf("sqlite3_declare_vtab failed: %s", sqlite3_errmsg(db));
        return rc;
    }

    *ppVtab = (sqlite3_vtab*)vtab;
    return SQLITE_OK;
}

static int node_connect(sqlite3 *db,
                        void *pAux,
                        int argc,
                        const char* const* argv,
                        sqlite3_vtab **ppVtab,
                        char **pzErr) {
    return node_create(db, pAux, argc, argv, ppVtab, pzErr);
}

static int node_disconnect(sqlite3_vtab *pVtab) {
    node_vtab *vtab = (node_vtab *)pVtab;
    if (vtab->table_name) {
        sqlite3_free(vtab->table_name);
    }
    sqlite3_free(vtab);
    return SQLITE_OK;
}

static int node_destroy(sqlite3_vtab *pVtab) {
    return node_disconnect(pVtab);
}

static int node_open(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor) {
    node_cursor *cursor = (node_cursor *)sqlite3_malloc(sizeof(node_cursor));
    if (!cursor) return SQLITE_NOMEM;
    memset(cursor, 0, sizeof(node_cursor));
    cursor->current = 0;
    cursor->pVtab = (node_vtab *)pVtab;
    *ppCursor = (sqlite3_vtab_cursor *)cursor;
    return SQLITE_OK;
}

static int node_close(sqlite3_vtab_cursor *cur) {
    node_cursor *cursor = (node_cursor *)cur;
    sqlite3_free(cursor);
    return SQLITE_OK;
}

static int node_filter(sqlite3_vtab_cursor *pCursor, int idxNum, const char *idxStr,
                      int argc, sqlite3_value **argv) {
    node_cursor *cursor = (node_cursor *)pCursor;
    cursor->current = 0; // 从第一个结点开始
    return SQLITE_OK;
}

static int node_next(sqlite3_vtab_cursor *cur) {
    node_cursor *cursor = (node_cursor *)cur;
    cursor->current++;
    return SQLITE_OK;
}

static int node_eof(sqlite3_vtab_cursor *cur) {
    node_cursor *cursor = (node_cursor *)cur;
    
    node_vtab *vtab = cursor->pVtab;
    return cursor->current >= vtab->nNode;  
}

static int node_column(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i) {
    node_cursor *cursor = (node_cursor *)cur;
    node_vtab *vtab = cursor->pVtab;

    // 查询源表的当前结点
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id, name FROM %s LIMIT 1 OFFSET %d;",
             vtab->table_name, cursor->current);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(sqlite3_db_handle(cursor->pVtab), sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_result_null(ctx);
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        if (i == 0) { // id 列
            sqlite3_result_int(ctx, sqlite3_column_int(stmt, 0));
        } else if (i == 1) { // name 列
            const unsigned char *name = sqlite3_column_text(stmt, 1);
            sqlite3_result_text(ctx, (const char *)name, -1, SQLITE_TRANSIENT);
        } else {
            sqlite3_result_null(ctx);
        }
    } else {
        sqlite3_result_null(ctx);
    }

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}



