#ifndef QUERIES_H
#define QUERIES_H

#include <sqlite3.h>

#define QUERY_PAGE_SIZE 10

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

void db_prepare_queries(sqlite3 *db, queries *q);
void db_finalize_queries(queries *q);
long db_query_read_from_file(char* fname, char* output);
#endif
