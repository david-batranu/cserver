#ifndef ROUTES_H
#define ROUTES_H

#include "request.h"
#include "queries.h"

#define NR_ROUTES 10

typedef struct Route Route;
typedef struct RouteHandlerParams RouteHandlerParams_t;

struct RouteHandlerParams {
    Request_t *req;
    queries *queries;
    Route *route;
};

struct Route {
    short unsigned int size;
    int method;
    const char *path;
    const char *scan;
    void (*handler)(RouteHandlerParams_t *);
};

int handle_routes(Request_t *, queries *, Route *);
#endif
