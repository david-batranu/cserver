#include <string.h>

#include "routes.h"


void handle_options(Request_t *req) {
    response_write(req, "HTTP/1.1 200 OK\r\nConnection: close\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: *\r\n\r\n");
    response_flush(req);
}

int handle_routes(Request_t *req, queries *queries, Route *routes) {
    int handled = 0;
    int i = 0;

    RouteHandlerParams_t routeHandlerParams;
    routeHandlerParams.req = req;
    routeHandlerParams.queries = queries;

    while(i < NR_ROUTES) {
        if (req->method != RM_UNK && routes[i].handler != 0 && req->method == routes[i].method && strncmp(req->uri, routes[i].path, routes[i].size) == 0) {
            routeHandlerParams.route = &routes[i];
            routes[i].handler(&routeHandlerParams);
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
