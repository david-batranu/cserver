#ifndef ROUTE_HANDLERS_H
#define ROUTE_HANDLERS_H

#include "routes.h"


void route_handler_hello(RouteHandlerParams_t *params);
void route_handler_login(RouteHandlerParams_t *params);
void route_handler_articles_paged(RouteHandlerParams_t *params);
void route_handler_user_sources(RouteHandlerParams_t *params);
void route_handler_user_articles_paged(RouteHandlerParams_t *params);
void route_handler_search_user_articles_paged(RouteHandlerParams_t *params);
void route_handler_source_articles_paged(RouteHandlerParams_t *params);
void route_handler_greet(RouteHandlerParams_t *params);

#endif
