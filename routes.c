#include <stdio.h>
#include <string.h>

#include "request.h"
#include "routes.h"


void handle_options(Request_t *req) {
    response_write(req, "HTTP/1.1 200 OK\r\nConnection: close\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: *\r\n\r\n");
    response_flush(req);
}

int handle_routes(Request_t *req, queries *queries, Route *routes) {
    int handled = 0;
    int i = 0;
    while(i < NR_ROUTES) {
        if (req->method != RM_UNK && routes[i].handler != 0 && req->method == routes[i].method && strncmp(req->uri, routes[i].path, routes[i].size) == 0) {
            routes[i].handler(req, queries, &routes[i]);
            handled = 1;
            break;
        }
        else if (req->method == RM_OPTIONS && routes[i].handler != 0 && strncmp(req->uri, routes[i].path, routes[i].size) == 0) {
            handle_options(req);
            handled = 1;
            break;
        }
        i++;
    }
    return handled;
}

void make_route(Route *route, int method, char *path, char *scan, void (*handler)(Request_t *, queries *, Route *)) {
    route->method = method;
    route->path = path;
    route->scan = scan;
    route->size = strlen(path);
    route->handler = handler;
}
