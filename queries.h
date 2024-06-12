#ifndef QUERIES_H
#define QUERIES_H

#include <sqlite3.h>
/* #include "sqlite/sqlite3.h" */

#define QUERY_PAGE_SIZE 50

typedef struct {
    char *query_greet_user;
    char *query_all_articles;
    char *query_all_articles_paginate;
    char *query_search_all_articles_paginate;
    char *query_user_sources;
    char *query_user_articles_paginate;
    char *query_search_user_articles_paginate;
    char *query_source_articles_paginate;
    char *query_search_source_articles_paginate;
} query_strings;

typedef struct {
    sqlite3_stmt *prep_query_greet_user;
    sqlite3_stmt *prep_query_all_articles;
    sqlite3_stmt *prep_query_all_articles_paginate;
    sqlite3_stmt *prep_query_search_all_articles_paginate;
    sqlite3_stmt *prep_query_user_sources;
    sqlite3_stmt *prep_query_user_articles_paginate;
    sqlite3_stmt *prep_query_search_user_articles_paginate;
    sqlite3_stmt *prep_query_source_articles_paginate;
    sqlite3_stmt *prep_query_search_source_articles_paginate;
} queries;

void db_read_queries(query_strings *qs);
void db_free_queries(query_strings *qs);
void db_prepare_queries(sqlite3 *db, queries *q, query_strings *qs);
void db_finalize_queries(queries *q);
char* db_query_read_from_file(char* fname);
#endif
