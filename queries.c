#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

#define QUERY_ALL_ARTICLES "SELECT uri, title, pubdate FROM Articles ORDER BY -pubdate;"
#define QUERY_ALL_ARTICLES_PAGINATE "SELECT uri, title, pubdate FROM Articles WHERE id NOT IN (SELECT id FROM Articles ORDER BY -pubdate LIMIT ?) ORDER BY -pubdate limit ?;"

#define QUERY_SOURCE_ARTICLES "SELECT uri, title, pubdate FROM Articles WHERE sourceid = ? ORDER BY -pubdate;"
#define QUERY_SOURCE_ARTICLES_PAGINATE "SELECT uri, title, pubdate FROM Articles WHERE sourceid = :SourceID AND id NOT IN (SELECT id FROM Articles WHERE sourceid = :SourceID ORDER BY -pubdate LIMIT :PageOffset) ORDER BY -pubdate limit :PageSize;"

#define QUERY_USER_SOURCES "SELECT uri, title FROM Sources where id IN (SELECT sourceid FROM UserSources WHERE userid = ?) ORDER BY id;"
#define QUERY_USER_ARTICLES_PAGINATE "SELECT uri, title, pubdate FROM Articles WHERE id NOT IN (SELECT id FROM Articles WHERE sourceid IN (SELECT sourceid FROM UserSources WHERE userid = :UserID) ORDER BY -pubdate LIMIT :PageOffset) AND sourceid IN (SELECT sourceid FROM UserSources WHERE userid = :UserID) ORDER BY -pubdate LIMIT :PageSize;"

#define QUERY_PAGE_SIZE 10


typedef struct {
    sqlite3_stmt *prep_query_all_articles;
    sqlite3_stmt *prep_query_all_articles_paginate;
    sqlite3_stmt *prep_query_user_sources;
    sqlite3_stmt *prep_query_user_articles_paginate;
    sqlite3_stmt *prep_query_source_articles_paginate;
} queries;


void db_prepare_queries(sqlite3 *db, queries *q) {

    sqlite3_prepare_v3(
        db,
        QUERY_ALL_ARTICLES,
        strlen(QUERY_ALL_ARTICLES),
        SQLITE_PREPARE_PERSISTENT,
        &q->prep_query_all_articles,
        NULL
    );

    sqlite3_prepare_v3(
        db,
        QUERY_ALL_ARTICLES_PAGINATE,
        strlen(QUERY_ALL_ARTICLES_PAGINATE),
        SQLITE_PREPARE_PERSISTENT,
        &q->prep_query_all_articles_paginate,
        NULL
    );

    sqlite3_prepare_v3(
        db,
        QUERY_USER_SOURCES,
        strlen(QUERY_USER_SOURCES),
        SQLITE_PREPARE_PERSISTENT,
        &q->prep_query_user_sources,
        NULL
    );

    sqlite3_prepare_v3(
        db,
        QUERY_USER_ARTICLES_PAGINATE,
        strlen(QUERY_USER_ARTICLES_PAGINATE),
        SQLITE_PREPARE_PERSISTENT,
        &q->prep_query_user_articles_paginate,
        NULL
    );

    sqlite3_prepare_v3(
        db,
        QUERY_SOURCE_ARTICLES_PAGINATE,
        strlen(QUERY_SOURCE_ARTICLES_PAGINATE),
        SQLITE_PREPARE_PERSISTENT,
        &q->prep_query_source_articles_paginate,
        NULL
    );

}


void db_finalize_queries(queries *q) {
    sqlite3_finalize(q->prep_query_all_articles);
    sqlite3_finalize(q->prep_query_all_articles_paginate);
    sqlite3_finalize(q->prep_query_user_sources);
    sqlite3_finalize(q->prep_query_user_articles_paginate);
    sqlite3_finalize(q->prep_query_source_articles_paginate);
}
