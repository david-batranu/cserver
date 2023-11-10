#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"
#include "queries.h"
#include "utils.h"
#include "query_handlers.h"


void write_articles_prepared_paginate(Request_t *req, int page_number, sqlite3_stmt *query) {
    response_write(req, JSON_RESP_HEADER);

    sqlite3_bind_int(query, 1, page_number * QUERY_PAGE_SIZE);
    sqlite3_bind_int(query, 2, QUERY_PAGE_SIZE);

    while (sqlite3_step(query) == SQLITE_ROW) {
        response_write(req, "{\"uri\":\"");
        response_write(req, (char *)sqlite3_column_text(query, 0));
        response_write(req, "\",\"title\": \"");
        response_write(req, escape_quotes((char *)sqlite3_column_text(query, 1)));
        response_write(req, "\",\"date\":\"");
        response_write(req, (char *)sqlite3_column_text(query, 2));
        response_write(req, "\"},");
    }

    sqlite3_reset(query);

    response_write(req, JSON_RESP_FOOTER);
    response_flush(req);
}

void write_user_sources_prepared(Request_t *req, int user_id, sqlite3_stmt *query) {
    response_write(req, JSON_RESP_HEADER);

    sqlite3_bind_int(query, 1, user_id);

    while (sqlite3_step(query) == SQLITE_ROW) {
        response_write(req, "{\"uri\":\"");
        response_write(req, (char *)sqlite3_column_text(query, 0));
        response_write(req, "\",\"title\": \"");
        response_write(req, escape_quotes((char *)sqlite3_column_text(query, 1)));
        response_write(req, "\"},");
    }

    sqlite3_reset(query);

    response_write(req, JSON_RESP_FOOTER);
    response_flush(req);
}

void write_user_articles_prepared_paginate(Request_t *req, int user_id, int page_number, sqlite3_stmt *query) {
    response_write(req, JSON_RESP_HEADER);

    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":UserID"), user_id);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageOffset"), page_number * QUERY_PAGE_SIZE);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageSize"), QUERY_PAGE_SIZE);

    while (sqlite3_step(query) == SQLITE_ROW) {
        response_write(req, "{\"uri\":\"");
        response_write(req, (char *)sqlite3_column_text(query, 0));
        response_write(req, "\",\"title\": \"");
        response_write(req, escape_quotes((char *)sqlite3_column_text(query, 1)));
        response_write(req, "\",\"date\":\"");
        response_write(req, (char *)sqlite3_column_text(query, 2));
        response_write(req, "\"},");
    }

    sqlite3_reset(query);

    response_write(req, JSON_RESP_FOOTER);
    response_flush(req);
}

void write_source_articles_prepared_paginate(Request_t *req, int source_id, int page_number, sqlite3_stmt *query) {
    response_write(req, JSON_RESP_HEADER);

    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":SourceID"), source_id);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageOffset"), page_number * QUERY_PAGE_SIZE);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageSize"), QUERY_PAGE_SIZE);

    while (sqlite3_step(query) == SQLITE_ROW) {
        response_write(req, "{\"uri\":\"");
        response_write(req, (char *)sqlite3_column_text(query, 0));
        response_write(req, "\",\"title\": \"");
        response_write(req, escape_quotes((char *)sqlite3_column_text(query, 1)));
        response_write(req, "\",\"date\":\"");
        response_write(req, (char *)sqlite3_column_text(query, 2));
        response_write(req, "\"},");
    }

    sqlite3_reset(query);

    response_write(req, JSON_RESP_FOOTER);
    response_flush(req);
}

void write_greeting_prepared(Request_t *req, char* user_name, sqlite3_stmt *query) {
    int found = 0;

    response_write(req, HTML_RESP_HEADER);

    sqlite3_bind_text(
        query, sqlite3_bind_parameter_index(query, ":UserName"),
        user_name, strlen(user_name), NULL);

    while (sqlite3_step(query) == SQLITE_ROW) {
      found = 1;
      response_write(req, "<html><head><title>");
      response_write(req, (char *)sqlite3_column_text(query, 0));
      response_write(req, "</title></head><body><h1>");
      response_write(req, (char *)sqlite3_column_text(query, 0));
      response_write(req, "</body></html>\n");
    }

    if (found == 0) {
      response_write(req, "<html><body>Who?</body></html>\n");
    }

    sqlite3_reset(query);

    response_flush(req);
}

