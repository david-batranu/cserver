#ifndef ROUTES_H
#define ROUTES_H

#include "request.h"
#include "queries.h"

#define NR_ROUTES 10

typedef struct Route Route;
struct Route {
    short unsigned int size;
    int method;
    const char *path;
    const char *scan;
    void (*handler)(Request_t *, queries *, Route *);
};

void make_route(
    Route *route,
    int method,
    char *path,
    char *scan,
    void (*handler)(Request_t *, queries *, Route *)
);

int handle_routes(Request_t *, queries *, Route *);
#endif
