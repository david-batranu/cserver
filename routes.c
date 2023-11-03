#include <stdio.h>
#include <string.h>
#include "routes.h"


int handle_routes(int sockfd, char *method, char *uri, char *resp, queries *queries, Route *routes) {
    int handled = 0;
    int i = 0;
    while(i < NR_ROUTES) {
        if (strcmp(method, routes[i].method) == 0 && strncmp(uri, routes[i].path, routes[i].size) == 0) {
            routes[i].handler(sockfd, uri, resp, queries, &routes[i]);
            handled = 1;
            break;
        }
        i++;
    }
    return handled;
}

void make_route(Route *route, char *method, char *path, char *scan, void (*handler)(int sockfd, char *uri, char *resp, queries *q, Route *route)) {
    route->method = method;
    route->path = path;
    route->scan = scan;
    route->size = strlen(path);
    route->handler = handler;
}
