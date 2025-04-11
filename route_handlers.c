#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "utils.h"
#include "queries.h"
#include "routes.h"
#include "query_handlers.h"


void route_handler_hello(RouteHandlerParams_t *params) {
    response_write(params->req, HTML_RESP_HEADER);
    response_write(params->req, "<html><body><h1>Hello!!!</h1></body></html>");
    response_flush(params->req);
}

void route_handler_login(RouteHandlerParams_t *params) {
    char userid[BUFFER_SIZE] = {'\0'};
    char password[BUFFER_SIZE] = {'\0'};
    char clean_userid[BUFFER_SIZE] = {'\0'};

    char *body_begins;
    body_begins = strstr(params->req->buffer, "\n\r\n");

    printf("userid: %s, password: %s, clean_userid: %s\n", userid, password, clean_userid);

    sscanf(body_begins, "%1000[^:]:%1000s", userid, password);
    clean_user_string(userid, clean_userid);
    printf("user: %s password: %s \n", clean_userid, password);
    response_write(params->req, HTML_RESP_HEADER);
    response_write(params->req, "<html><body><h1>Hello!!!</h1></body></html>");
    response_flush(params->req);
}

void route_handler_articles_paged(RouteHandlerParams_t *params) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    sscanf(params->req->uri, params->route->scan, val_page_number);
    clean_str_number(val_page_number, val_clean_page_number);
    write_articles_prepared_paginate(params->req, str_to_int(val_clean_page_number), params->queries->prep_query_all_articles_paginate);
}

void route_handler_user_sources(RouteHandlerParams_t *params) {
    char val_userid[BUFFER_SIZE] = {0};
    char val_clean_userid[BUFFER_SIZE] = {0};
    sscanf(params->req->uri, params->route->scan, val_userid);
    clean_str_number(val_userid, val_clean_userid);
    write_user_sources_prepared(params->req, str_to_int(val_clean_userid), params->queries->prep_query_user_sources);
}

void route_handler_user_articles_paged(RouteHandlerParams_t *params) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    char val_userid[BUFFER_SIZE] = {0};
    char val_clean_userid[BUFFER_SIZE] = {0};
    sscanf(params->req->uri, params->route->scan, val_userid, val_page_number);
    clean_str_number(val_userid, val_clean_userid);
    clean_str_number(val_page_number, val_clean_page_number);
    write_user_articles_prepared_paginate(params->req, str_to_int(val_clean_userid), str_to_int(val_clean_page_number), params->queries->prep_query_user_articles_paginate);
}

void route_handler_search_user_articles_paged(RouteHandlerParams_t *params) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    char val_userid[BUFFER_SIZE] = {0};
    char val_clean_userid[BUFFER_SIZE] = {0};
    char val_search_string[BUFFER_SIZE] = {0};
    char val_clean_search_string[BUFFER_SIZE] = {0};
    sscanf(params->req->uri, params->route->scan, val_userid, val_search_string, val_page_number);
    clean_str_number(val_userid, val_clean_userid);
    clean_str_number(val_page_number, val_clean_page_number);
    clean_string(val_search_string, val_clean_search_string);
    write_search_user_articles_prepared_paginate(params->req, val_clean_search_string, str_to_int(val_clean_userid), str_to_int(val_clean_page_number), params->queries->prep_query_search_user_articles_paginate);
}

void route_handler_source_articles_paged(RouteHandlerParams_t *params) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    char val_sourceid[BUFFER_SIZE] = {0};
    char val_clean_sourceid[BUFFER_SIZE] = {0};
    sscanf(params->req->uri, params->route->scan, val_sourceid, val_page_number);
    clean_str_number(val_sourceid, val_clean_sourceid);
    clean_str_number(val_page_number, val_clean_page_number);
    write_source_articles_prepared_paginate(params->req, str_to_int(val_clean_sourceid), str_to_int(val_clean_page_number), params->queries->prep_query_source_articles_paginate);
}

void route_handler_greet(RouteHandlerParams_t *params) {
    char greet_name[BUFFER_SIZE] = {0};
    char clean_greet_name[BUFFER_SIZE] = {0};
    sscanf(params->req->uri, params->route->scan, greet_name);
    clean_user_string(greet_name, clean_greet_name);
    write_greeting_prepared(params->req, clean_greet_name, params->queries->prep_query_greet_user);
}
