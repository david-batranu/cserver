#include <arpa/inet.h>
#include <sys/socket.h>
/* #include <netinet/in.h> */
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
/* #include <sqlite3.h> */
#include "dbutil.c"
#include "queries.h"
#include "routes.h"
#include "defs.h"
#include "utils.h"
#include "route_handlers.h"
#include "query_handlers.h"

#define PORT 8080

#define GET "GET"
#define POST "POST"

static volatile sig_atomic_t keepRunning = 1;


void resp_ok(char* resp, char* content_type, char* extra_headers, char* body) {
    char basic[] =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-type: %s\r\n"
        "%s"
        "\r\n"
        "%s";
    sprintf(resp, basic, content_type, extra_headers, body);
}

void resp_404(char* resp) {
    char basic[] =
        "HTTP/1.1 404 Not Found\r\n"
        "Connection: close\r\n"
        "\r\n";
    strcat(resp, basic);
}




void write_default(int sockfd, char* resp) {
    resp_ok(resp, "text/html", "", "<html>Hello, world!</html>");
    write(sockfd, resp, strlen(resp));
}


void write_404(int sockfd, char* resp) {
    resp_404(resp);
    write(sockfd, resp, strlen(resp));
}

void write_favicon(int sockfd, char* resp) {
  char extra_headers[HEADER_SIZE] = {0};

  FILE *fp;
  fp = fopen("favicon.ico", "r");
  if (fp != NULL) {

    sprintf(
        extra_headers,
        "Content-length: %ld\r\n",
        get_file_size(fp)
        );

    resp_ok(resp, "image/x-icon", extra_headers, "");

    /* send ok */
    write(sockfd, resp, strlen(resp));

    /* send rest of file */
    send_file(fp, sockfd);
    fclose(fp);
    /* printf("write: favicon.ico\n"); */
  }
  else {
    perror("opening favicon.ico");
    write_404(sockfd, resp);
  }

}


static int db_callback_greeting(void *buffer, int num_columns, char **columns, char **column_names) {
    /* printf("Got %d columns!\n", num_columns); */
    if (num_columns == 1) {
        /* printf("Columns: %s\n", columns[0]); */
        sprintf((char *)buffer, "%s", columns[0]);
    }
    return 0;
}


int create_socket() {
    int socket_reuse = 1;
    int sockfd, set_socket_reuse_addr;

    /* create a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("webserver (socket)");
    }
    printf("socket created successfully!\n");

    /* set socket options */
    set_socket_reuse_addr = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&socket_reuse, sizeof(socket_reuse));
    if (set_socket_reuse_addr < 0) {
        perror("webserver (setsockopt: SO_REUSEADDR)");
    }

#ifdef SO_REUSEPORT
    int set_socket_reuse_port = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&socket_reuse, sizeof(socket_reuse));
    if (set_socket_reuse_port < 0) {
        perror("webserver (setsockopt: SO_REUSEPORT)");
    }
#endif

    return sockfd;
}

void signalHandler(int dummy) {
    if (keepRunning == 1){
        keepRunning = 0;
    } else {
        exit(0);
    }
}

void sigpipe_handler(int dummy) {
    printf("SIGPIPE caught!");
}


void handle_login(int sockfd, char* req, char* resp) {
    char userid[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    char clean_userid[BUFFER_SIZE];

    char *body_begins;
    body_begins = strstr(req, "\n\r\n");

    sscanf(body_begins, "%1000[^:]:%1000s", userid, password);
    clean_user_string(userid, clean_userid);
    printf("user: %s password: %s \n", clean_userid, password);
}


int main() {
    int handled_route = 0;

    Route routes[NR_ROUTES];

    queries queries;

    sqlite3 *db;
    int db_connected = 0;
    int sockfd;

    char request_buffer[BUFFER_SIZE];
    char response_buffer[RESP_BUFFER_SIZE];

    /* prepare the address to bind the socket to */
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    /* prepare client address */
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    /* make_route(&routes[0], "/login", '\0', &route_handler_login); */
    /* make_route(&routes[0], "POST", "/login", '\0', &route_handler_login); */
    make_route(&routes[0], "GET", "/articles-paged/", "/articles-paged/%1000s", &route_handler_articles_paged);
    make_route(&routes[1], "GET", "/user-sources/", "/user-sources/%1000s", &route_handler_user_sources);
    make_route(&routes[2], "GET", "/user-articles-paged/", "/user-articles-paged/%1000[^/]/%1000[^'/']s", &route_handler_user_articles_paged);
    make_route(&routes[3], "GET", "/source-articles-paged/", "/source-articles-paged/%1000[^/]/%1000[^'/']s", &route_handler_source_articles_paged);
    make_route(&routes[4], "GET", "/greet/", "/greet/%128s", &route_handler_greet);
    printf("ROUTE: %s | %s | %i\n", routes[0].path, routes[0].scan, routes[0].size);
    db_connected = connect_db("main.db", &db);

    /* create a socket */
    sockfd = create_socket();
    if (sockfd == -1) {
        return 1;
    }

    /* create the address to bind the socket to */
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* bind the socket to the address */
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)
    {
        perror("webserver (bind)");
        return 1;
    }

    printf("socket successfully bound to address!\n");

    /* Listen for incoming connections */
    if (listen(sockfd, SOMAXCONN) != 0)
    {
        perror("webserver (listen)");
        return 1;
    }

    printf("server listening for connectins!\n");

    db_prepare_queries(db, &queries);

    /* Handle Ctrl+C */
    signal(SIGINT, signalHandler);
    signal(SIGPIPE, sigpipe_handler);

    while(keepRunning) {
        int newsockfd, sockn, valread;
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];

        /* Accept incoming connections */
        newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
        if (newsockfd < 0)
        {
            perror("webserver (accept)");
            continue;
        }
        /* n_requests++; */
        /* printf("[%i] connection accepted\n", n_requests); */

        /* Get client address */
        sockn = getpeername(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            perror("webserver (getpeername)");
            continue;
        }

        /* Read from the socket */
        valread = read(newsockfd, request_buffer, BUFFER_SIZE);
        if (valread < 0)
        {
            perror("webserver (read)");
            continue;
        }

        /* Read the request */
        /* method[0] = uri[0] = version[0] = '\0'; */
        sscanf(request_buffer, "%s %s %s", method, uri, version);


        /*
        printf(
                "[%s:%u] %s %s %s\n",
                inet_ntoa(client_addr.sin_addr),
                ntohs(client_addr.sin_port),
                method,
                version,
                uri
              );
        */
        handled_route = handle_routes(newsockfd, method, uri, response_buffer, &queries, routes);

        if (strcmp(uri, "/favicon.ico") == 0) {
            write_favicon(newsockfd, response_buffer);
            /* write_404(newsockfd, response_buffer); */
            handled_route = 1;
        }

        if (!handled_route) {
            printf("Could not handle route: %s\n", uri);
            write_default(newsockfd, response_buffer);
        }

        bzero(response_buffer, BUFFER_SIZE);
        close(newsockfd);
    }

    printf("EXITING...\n");
    close(sockfd);
    db_finalize_queries(&queries);
    sqlite3_close_v2(db);
    return 0;
}
