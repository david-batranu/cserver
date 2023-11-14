#ifndef ROUTE_HANDLERS_H
#define ROUTE_HANDLERS_H

#include "request.h"
#include "queries.h"
#include "routes.h"

void route_handler_login(Request_t *, queries *queries, Route *route);
void route_handler_articles_paged(Request_t *, queries *queries, Route *route);
void route_handler_user_sources(Request_t *, queries *queries, Route *route);
void route_handler_user_articles_paged(Request_t *, queries *queries, Route *route);
void route_handler_source_articles_paged(Request_t *, queries *queries, Route *route);
void route_handler_greet(Request_t *, queries *queries, Route *route);

#endif
