#ifndef QUERY_HANDLERS_H
#define QUERY_HANDLERS_H

#include "defs.h"
#include "queries.h"
#include "utils.h"

void write_articles_prepared_paginate(int sockfd, char* resp, int page_number, sqlite3_stmt *query);
void write_user_sources_prepared(int sockfd, char* resp, int user_id, sqlite3_stmt *query);
void write_user_articles_prepared_paginate(int sockfd, char* resp, int user_id, int page_number, sqlite3_stmt *query);
void write_source_articles_prepared_paginate(int sockfd, char* resp, int source_id, int page_number, sqlite3_stmt *query);

#endif
