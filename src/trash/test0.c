// 2024.10.15
// 尝试创建一个sqlite扩展

#include<sqlite3ext.h>

SQLITE_EXTENSION_INIT1

static void sumFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
    if(argc > 0){
        int sum = 0;
        for(int i=0; i<argc; i++){
            sum += sqlite3_value_int(argv[i]);
        }
        sqlite3_result_int(context, sum);
    }
}

#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_test_init( /* <== Change this name, maybe */
  sqlite3 *db, 
  char **pzErrMsg, 
  const sqlite3_api_routines *pApi
){
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);
  /* insert code to initialize your extension here */
  sqlite3_create_function(db, "sum_test", -1, SQLITE_UTF8, 0, sumFunc, 0, 0);
  return rc;
}