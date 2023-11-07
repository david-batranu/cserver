#ifndef ROUTE_HANDLERS_H
#define ROUTE_HANDLERS_H

#include "queries.h"
#include "routes.h"

void route_handler_articles_paged(int sockfd, char *uri, char *resp, queries *queries, Route *route);
void route_handler_user_sources(int sockfd, char *uri, char *resp, queries *queries, Route *route);
void route_handler_user_articles_paged(int sockfd, char *uri, char *resp, queries *queries, Route *route);
void route_handler_source_articles_paged(int sockfd, char *uri, char *resp, queries *queries, Route *route);
void route_handler_greet(int sockfd, char *uri, char *resp, queries *queries, Route *route);

#endif
