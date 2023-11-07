#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "utils.h"
#include "queries.h"
#include "routes.h"
#include "query_handlers.h"

void route_handler_articles_paged(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    sscanf(uri, route->scan, val_page_number);
    clean_str_number(val_page_number, val_clean_page_number);
    write_articles_prepared_paginate(sockfd, resp, str_to_int(val_clean_page_number), queries->prep_query_all_articles_paginate);
}

void route_handler_user_sources(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char val_userid[BUFFER_SIZE] = {0};
    char val_clean_userid[BUFFER_SIZE] = {0};
    sscanf(uri, route->scan, val_userid);
    clean_str_number(val_userid, val_clean_userid);
    write_user_sources_prepared(sockfd, resp, str_to_int(val_clean_userid), queries->prep_query_user_sources);
}

void route_handler_user_articles_paged(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    char val_userid[BUFFER_SIZE] = {0};
    char val_clean_userid[BUFFER_SIZE] = {0};
    sscanf(uri, route->scan, val_userid, val_page_number);
    clean_str_number(val_userid, val_clean_userid);
    clean_str_number(val_page_number, val_clean_page_number);
    write_user_articles_prepared_paginate(sockfd, resp, str_to_int(val_clean_userid), str_to_int(val_clean_page_number), queries->prep_query_user_articles_paginate);
}

void route_handler_source_articles_paged(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    char val_sourceid[BUFFER_SIZE] = {0};
    char val_clean_sourceid[BUFFER_SIZE] = {0};
    sscanf(uri, route->scan, val_sourceid, val_page_number);
    clean_str_number(val_sourceid, val_clean_sourceid);
    clean_str_number(val_page_number, val_clean_page_number);
    write_source_articles_prepared_paginate(sockfd, resp, str_to_int(val_clean_sourceid), str_to_int(val_clean_page_number), queries->prep_query_source_articles_paginate);
}

void route_handler_greet(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char greet_name[BUFFER_SIZE] = {0};
    char clean_greet_name[BUFFER_SIZE] = {0};
    sscanf(uri,route->scan, greet_name);
    clean_user_string(greet_name, clean_greet_name);
    printf("greet: %s (%s)\n", greet_name, clean_greet_name);
    /* if (db_connected) { */
    /*     write_greeting(sockfd, resp, clean_greet_name, db); */
    /* } */
}
