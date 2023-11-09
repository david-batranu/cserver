#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"
#include "queries.h"
#include "utils.h"
#include "query_handlers.h"


void write_articles_prepared_paginate(int sockfd, char* resp, int page_number, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = JSON_RESP_HEADER;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    sqlite3_bind_int(query, 1, page_number * QUERY_PAGE_SIZE);
    sqlite3_bind_int(query, 2, QUERY_PAGE_SIZE);

    while (sqlite3_step(query) == SQLITE_ROW) {
        myrespstrcat(&buff, "{\"uri\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 0));
        myrespstrcat(&buff, "\",\"title\": \"");
        myrespstrcat(&buff, escape_quotes((char *)sqlite3_column_text(query, 1)));
        myrespstrcat(&buff, "\",\"date\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 2));
        myrespstrcat(&buff, "\"},");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, JSON_RESP_FOOTER);

    write(sockfd, buff.buffer, (buff.p - buff.buffer));
}

void write_user_sources_prepared(int sockfd, char* resp, int user_id, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = JSON_RESP_HEADER;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    sqlite3_bind_int(query, 1, user_id);

    while (sqlite3_step(query) == SQLITE_ROW) {
        myrespstrcat(&buff, "{\"uri\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 0));
        myrespstrcat(&buff, "\",\"title\": \"");
        myrespstrcat(&buff, escape_quotes((char *)sqlite3_column_text(query, 1)));
        myrespstrcat(&buff, "\"},");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, JSON_RESP_FOOTER);

    write(sockfd, buff.buffer, (buff.p - buff.buffer));
}

void write_user_articles_prepared_paginate(int sockfd, char* resp, int user_id, int page_number, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = JSON_RESP_HEADER;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":UserID"), user_id);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageOffset"), page_number * QUERY_PAGE_SIZE);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageSize"), QUERY_PAGE_SIZE);

    while (sqlite3_step(query) == SQLITE_ROW) {
        myrespstrcat(&buff, "{\"uri\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 0));
        myrespstrcat(&buff, "\",\"title\": \"");
        myrespstrcat(&buff, escape_quotes((char *)sqlite3_column_text(query, 1)));
        myrespstrcat(&buff, "\",\"date\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 2));
        myrespstrcat(&buff, "\"},");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, JSON_RESP_FOOTER);

    write(sockfd, buff.buffer, (buff.p - buff.buffer));
}

void write_source_articles_prepared_paginate(int sockfd, char* resp, int source_id, int page_number, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = JSON_RESP_HEADER;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":SourceID"), source_id);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageOffset"), page_number * QUERY_PAGE_SIZE);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageSize"), QUERY_PAGE_SIZE);

    while (sqlite3_step(query) == SQLITE_ROW) {
        myrespstrcat(&buff, "{\"uri\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 0));
        myrespstrcat(&buff, "\",\"title\": \"");
        myrespstrcat(&buff, escape_quotes((char *)sqlite3_column_text(query, 1)));
        myrespstrcat(&buff, "\",\"date\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 2));
        myrespstrcat(&buff, "\"},");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, JSON_RESP_FOOTER);

    write(sockfd, buff.buffer, (buff.p - buff.buffer));
}

void write_greeting_prepared(int sockfd, char* resp, char* user_name, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = HTML_RESP_HEADER;
    char body[RESPONSE_SIZE];
    int found = 0;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    sqlite3_bind_text(
        query, sqlite3_bind_parameter_index(query, ":UserName"),
        user_name, strlen(user_name), NULL);

    while (sqlite3_step(query) == SQLITE_ROW) {
      found = 1;
      sprintf(
          body,
          "<html><head><title>%s</title></head><body><h1>%s</h1></body></html>\n",
          (char *)sqlite3_column_text(query, 0),
          (char *)sqlite3_column_text(query, 0)
          );
    }

    if (found == 0) {
      sprintf(body, "Who?");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, body);
    write(sockfd, buff.buffer, (buff.p - buff.buffer));
}

