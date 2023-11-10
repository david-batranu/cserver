#ifndef QUERY_HANDLERS_H
#define QUERY_HANDLERS_H

#include "defs.h"
#include "request.h"
#include "queries.h"
#include "utils.h"

void write_articles_prepared_paginate(Request_t *, int page_number, sqlite3_stmt *query);
void write_user_sources_prepared(Request_t *, int user_id, sqlite3_stmt *query);
void write_user_articles_prepared_paginate(Request_t *, int user_id, int page_number, sqlite3_stmt *query);
void write_source_articles_prepared_paginate(Request_t *, int source_id, int page_number, sqlite3_stmt *query);
void write_greeting_prepared(Request_t *, char* user_name, sqlite3_stmt *query);

#endif
