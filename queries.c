#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "defs.h"
#include "queries.h"
#include "utils.h"


void db_prepare_queries(sqlite3 *db, queries *q) {
    char *query_greet_user = db_query_read_from_file("queries/greet_user.sql");
    char *query_all_articles = db_query_read_from_file("queries/all_articles.sql");
    char *query_all_articles_paginate = db_query_read_from_file("queries/all_articles_paginate.sql");
    char *query_user_sources = db_query_read_from_file("queries/user_sources.sql");
    char *query_search_all_articles_paginate = db_query_read_from_file("queries/search_all_articles_paginate.sql");
    char *query_source_articles_paginate = db_query_read_from_file("queries/source_articles_paginate.sql");
    char *query_search_source_articles_paginate = db_query_read_from_file("queries/search_source_articles_paginate.sql");

    char *query_user_articles_paginate = db_query_read_from_file("queries/user_articles_paginate.sql");
    char *query_search_user_articles_paginate = db_query_read_from_file("queries/search_user_articles_paginate.sql");

    sqlite3_prepare_v3(db, query_greet_user, strlen(query_greet_user),
            SQLITE_PREPARE_PERSISTENT, &q->prep_query_greet_user,
            NULL);

    sqlite3_prepare_v3(db, query_all_articles, strlen(query_all_articles),
            SQLITE_PREPARE_PERSISTENT, &q->prep_query_all_articles,
            NULL);

    sqlite3_prepare_v3(
            db, query_all_articles_paginate, strlen(query_all_articles_paginate),
            SQLITE_PREPARE_PERSISTENT, &q->prep_query_all_articles_paginate, NULL);

    sqlite3_prepare_v3(db, query_search_all_articles_paginate,
            strlen(query_search_all_articles_paginate),
            SQLITE_PREPARE_PERSISTENT,
            &q->prep_query_search_all_articles_paginate, NULL);

    sqlite3_prepare_v3(db, query_user_sources, strlen(query_user_sources),
            SQLITE_PREPARE_PERSISTENT, &q->prep_query_user_sources,
            NULL);

    sqlite3_prepare_v3(
            db, query_user_articles_paginate, strlen(query_user_articles_paginate),
            SQLITE_PREPARE_PERSISTENT, &q->prep_query_user_articles_paginate, NULL);

    sqlite3_prepare_v3(db, query_search_user_articles_paginate,
            strlen(query_search_user_articles_paginate),
            SQLITE_PREPARE_PERSISTENT,
            &q->prep_query_search_user_articles_paginate, NULL);

    sqlite3_prepare_v3(db, query_source_articles_paginate,
            strlen(query_source_articles_paginate),
            SQLITE_PREPARE_PERSISTENT,
            &q->prep_query_source_articles_paginate, NULL);

    sqlite3_prepare_v3(db, query_search_source_articles_paginate,
            strlen(query_search_source_articles_paginate),
            SQLITE_PREPARE_PERSISTENT,
            &q->prep_query_search_source_articles_paginate, NULL);
}

char* db_query_read_from_file(char *fname) {
    long size = 0;

    FILE *fp;
    char buffer[BUFFER_SIZE];
    char *output = 0;

    fp = fopen(fname, "r");

    printf("Reading SQL file: %s... ", fname);

    if (fp != NULL) {
        size = get_file_size(fp);
        output = (char*)malloc(size + 1);
        output[0] = 0;
        while (fgets(buffer, BUFFER_SIZE, fp)) {
            mystrcat(output, buffer);
        };
        fclose(fp);
        printf("OK.\n");
    } else {
        printf("FAIL!\n");
    }

    return output;
}

void db_finalize_queries(queries *q) {
    sqlite3_finalize(q->prep_query_greet_user);
    sqlite3_finalize(q->prep_query_all_articles);
    sqlite3_finalize(q->prep_query_all_articles_paginate);
    sqlite3_finalize(q->prep_query_search_all_articles_paginate);
    sqlite3_finalize(q->prep_query_user_sources);
    sqlite3_finalize(q->prep_query_user_articles_paginate);
    sqlite3_finalize(q->prep_query_search_user_articles_paginate);
    sqlite3_finalize(q->prep_query_source_articles_paginate);
    sqlite3_finalize(q->prep_query_search_source_articles_paginate);
}
