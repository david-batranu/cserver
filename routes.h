#ifndef ROUTES_H
#define ROUTES_H

#include "queries.h"

#define NR_ROUTES 5

typedef struct Route Route;
struct Route {
    short unsigned int size;
    const char *method;
    const char *path;
    const char *scan;
    void (*handler)(int sockfd, char *uri, char *resp, queries *q, Route *route);
};

void make_route(
    Route *route,
    char *method,
    char *path,
    char *scan,
    void (*handler)(int, char *, char *, queries *, Route *)
);

int handle_routes(
    int sockfd,
    char *method,
    char *uri,
    char *resp,
    queries *queries,
    Route *routes
);
#endif
