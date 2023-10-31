#include <sqlite3.h>
#include <stdio.h>



int connect_db(char *filename, sqlite3 **db) {
    int rc;

    rc = sqlite3_open(filename, db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return 0;
    }

    return 1;
}


int query_db(
        sqlite3 *db,
        char* query,
        int (*callback)(void *, int, char**, char**),
        void * callback_argument) {
    int rc;
    char *zErrMsg = 0;

    rc = sqlite3_exec(
        db,
        query,
        callback,
        callback_argument,
        &zErrMsg
    );

    if(rc != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }

    return 0;
}
