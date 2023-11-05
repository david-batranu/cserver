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

#define PORT 8080
#define BUFFER_SIZE 1024
#define RESP_BUFFER_SIZE 1024*64
#define HEADER_SIZE 64
#define RESPONSE_SIZE 4096
#define FILE_BUFFER_SIZE 1024*64

#define JSON_RESP_HEADER "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: application/json\r\n\r\n{\"results\":["
#define JSON_RESP_FOOTER "]}"

#define GET "GET"
#define POST "POST"

static volatile sig_atomic_t keepRunning = 1;

typedef struct {
    char *buffer;
    int buffer_size;
    void (*callback)(void *);
    int sockfd;
    char *p;
} mybuff;

/* https://beribey.medium.com/why-string-concatenation-so-slow-745f79e22eeb */
char* mystrcat( char* dest, char* src ) {
     while (*dest) dest++;
     while ((*dest++ = *src++));
     return --dest;
}

void myrespstrcat(mybuff *buff, char* src) {
    int remaining = buff->buffer_size - (buff->p - buff->buffer);
    while (*src && remaining) {
        *buff->p++ = *src++;;
        remaining--;
    }

    if (!remaining && *src) {
        --buff->p;
        (*buff->callback)(buff);
        myrespstrcat(buff, src);
    }

    /* while ((*buff->p++ = *src++)); */
    /* return --buff->p; */
}

char *escape_quotes(char *in) {
    int i = 0;
    for (; in[i]; i++) {
        if (in[i] == '"') {
            in[i] = '\'';
        }
    }
    return in;
}

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


long get_file_size(FILE *fp) {
    long size;
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return size;
}


void send_file(FILE* fp, int sockfd) {
    char buffer[FILE_BUFFER_SIZE];

    for (;;) {
        int nread = fread(buffer, 1, FILE_BUFFER_SIZE, fp);
        /* printf("Bytes read %d \n", nread); */
        if (nread > 0) {
            if (write(sockfd, buffer, nread) == -1) {
                perror("sending file...");
            }
        }

        else if (nread < FILE_BUFFER_SIZE) {
            if (feof(fp)) {
                /* printf("End of file.\n"); */
                break;
            }
            else if (ferror(fp)) {
                fprintf(stderr, "Error reading file.\n");
            }
            break;
        }

        /* bzero(buffer, BUFFER_SIZE); */
    }
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


void write_greeting(int sockfd, char* resp, char* name, sqlite3 *db) {

    char query[BUFFER_SIZE] = {0};
    char greeting[BUFFER_SIZE] = {0};
    char body[RESPONSE_SIZE] = {0};

    sprintf(query, "select greeting from user_greeting where name == '%s';", name);

    if (query_db(db, query, db_callback_greeting, greeting) != 0 || strlen(greeting) == 0) {
        sprintf(greeting, "Who?");
    }

    sprintf(body, "<html><head><title>%s</title></head><body><h1>%s</h1></body></html>\n", greeting, greeting);
    resp_ok(resp, "text/html", "", body);
    write(sockfd, resp, strlen(resp));
}

void on_resp_buffer_full(void *buff) {
    /* printf("Flush resp buffer!\n"); */
    mybuff *b = (mybuff *)buff;
    write(b->sockfd, b->buffer, b->buffer_size);
    b->buffer[0] = '\0';
    b->p = b->buffer;
}

void write_articles_prepared(int sockfd, char* resp, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = JSON_RESP_HEADER;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    while (sqlite3_step(query) == SQLITE_ROW) {
        myrespstrcat(&buff, "{\"uri\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 0));
        myrespstrcat(&buff, "\",\"title\": \"");
        myrespstrcat(&buff, escape_quotes((char *)sqlite3_column_text(query, 1)));
        myrespstrcat(&buff, "\",\"date\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 2));
        myrespstrcat(&buff, "\"},");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, JSON_RESP_FOOTER);

    write(sockfd, buff.buffer, (buff.p - buff.buffer));
}

void write_articles_prepared_paginate(int sockfd, char* resp, int page_number, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = JSON_RESP_HEADER;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    sqlite3_bind_int(query, 1, page_number * QUERY_PAGE_SIZE);
    sqlite3_bind_int(query, 2, QUERY_PAGE_SIZE);

    while (sqlite3_step(query) == SQLITE_ROW) {
        myrespstrcat(&buff, "{\"uri\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 0));
        myrespstrcat(&buff, "\",\"title\": \"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 1));
        myrespstrcat(&buff, "\"},");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, JSON_RESP_FOOTER);

    write(sockfd, buff.buffer, (buff.p - buff.buffer));
}

void write_user_articles_prepared_paginate(int sockfd, char* resp, int user_id, int page_number, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = JSON_RESP_HEADER;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":UserID"), user_id);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageOffset"), page_number * QUERY_PAGE_SIZE);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageSize"), QUERY_PAGE_SIZE);

    while (sqlite3_step(query) == SQLITE_ROW) {
        myrespstrcat(&buff, "{\"uri\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 0));
        myrespstrcat(&buff, "\",\"title\": \"");
        myrespstrcat(&buff, escape_quotes((char *)sqlite3_column_text(query, 1)));
        myrespstrcat(&buff, "\",\"date\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 2));
        myrespstrcat(&buff, "\"},");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, JSON_RESP_FOOTER);

    write(sockfd, buff.buffer, (buff.p - buff.buffer));
}

void write_source_articles_prepared_paginate(int sockfd, char* resp, int source_id, int page_number, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = JSON_RESP_HEADER;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":SourceID"), source_id);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageOffset"), page_number * QUERY_PAGE_SIZE);
    sqlite3_bind_int(query, sqlite3_bind_parameter_index(query, ":PageSize"), QUERY_PAGE_SIZE);

    while (sqlite3_step(query) == SQLITE_ROW) {
        myrespstrcat(&buff, "{\"uri\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 0));
        myrespstrcat(&buff, "\",\"title\": \"");
        myrespstrcat(&buff, escape_quotes((char *)sqlite3_column_text(query, 1)));
        myrespstrcat(&buff, "\",\"date\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 2));
        myrespstrcat(&buff, "\"},");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, JSON_RESP_FOOTER);

    write(sockfd, buff.buffer, (buff.p - buff.buffer));
}

void write_user_sources_prepared(int sockfd, char* resp, int user_id, sqlite3_stmt *query) {
    mybuff buff;
    char basic[] = JSON_RESP_HEADER;

    buff.buffer = resp;
    buff.p = resp;
    buff.buffer_size = RESP_BUFFER_SIZE;
    buff.callback = &on_resp_buffer_full;
    buff.sockfd = sockfd;

    myrespstrcat(&buff, basic);

    sqlite3_bind_int(query, 1, user_id);

    while (sqlite3_step(query) == SQLITE_ROW) {
        myrespstrcat(&buff, "{\"uri\":\"");
        myrespstrcat(&buff, (char *)sqlite3_column_text(query, 0));
        myrespstrcat(&buff, "\",\"title\": \"");
        myrespstrcat(&buff, escape_quotes((char *)sqlite3_column_text(query, 1)));
        myrespstrcat(&buff, "\"},");
    }

    sqlite3_reset(query);

    myrespstrcat(&buff, JSON_RESP_FOOTER);

    write(sockfd, buff.buffer, (buff.p - buff.buffer));
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

void clean_user_string(char *in, char *out) {
    int i = 0;
    int j = 0;
    int ch;
    for (;i < strlen(in); i++) {
        ch = in[i];
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
            out[j] = ch;
            j++;
        }
    }
}

void clean_str_number(char *in, char *out) {
    int i = 0;
    int j = 0;
    int ch;
    for (;i < strlen(in); i++) {
        ch = in[i];
        if (ch >= '0' && ch <= '9') {
            out[j] = ch;
            j++;
        }
    }
}

int str_to_int(char *str) {
    int mul = 1;
    int len_str = strlen(str);
    int i = len_str - 1;
    int result = 0;

    for (;i > 0; i--) {
        mul *= 10;
    }

    for (;i < len_str; i++) {
        result += mul * (str[i] - '0');
        mul /= 10;
    }

    return result;
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

/* void route_handler_login(int sockfd, char *uri, char *resp, queries *queries, Route *route) { */
/*     handle_login(sockfd, request_buffer, response_buffer); */
/* } */

void route_handler_articles_paged(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    sscanf(uri, route->scan, val_page_number);
    clean_str_number(val_page_number, val_clean_page_number);
    write_articles_prepared_paginate(sockfd, resp, str_to_int(val_clean_page_number), queries->prep_query_all_articles_paginate);
}

void route_handler_user_sources(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char val_userid[BUFFER_SIZE] = {0};
    char val_clean_userid[BUFFER_SIZE] = {0};
    sscanf(uri, route->scan, val_userid);
    clean_str_number(val_userid, val_clean_userid);
    write_user_sources_prepared(sockfd, resp, str_to_int(val_clean_userid), queries->prep_query_user_sources);
}

void route_handler_user_articles_paged(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    char val_userid[BUFFER_SIZE] = {0};
    char val_clean_userid[BUFFER_SIZE] = {0};
    sscanf(uri, route->scan, val_userid, val_page_number);
    clean_str_number(val_userid, val_clean_userid);
    clean_str_number(val_page_number, val_clean_page_number);
    write_user_articles_prepared_paginate(sockfd, resp, str_to_int(val_clean_userid), str_to_int(val_clean_page_number), queries->prep_query_user_articles_paginate);
}

void route_handler_source_articles_paged(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char val_page_number[BUFFER_SIZE] = {0};
    char val_clean_page_number[BUFFER_SIZE] = {0};
    char val_sourceid[BUFFER_SIZE] = {0};
    char val_clean_sourceid[BUFFER_SIZE] = {0};
    sscanf(uri, route->scan, val_sourceid, val_page_number);
    clean_str_number(val_sourceid, val_clean_sourceid);
    clean_str_number(val_page_number, val_clean_page_number);
    write_source_articles_prepared_paginate(sockfd, resp, str_to_int(val_clean_sourceid), str_to_int(val_clean_page_number), queries->prep_query_source_articles_paginate);
}

void route_handler_greet(int sockfd, char *uri, char *resp, queries *queries, Route *route) {
    char greet_name[BUFFER_SIZE] = {0};
    char clean_greet_name[BUFFER_SIZE] = {0};
    sscanf(uri,route->scan, greet_name);
    clean_user_string(greet_name, clean_greet_name);
    printf("greet: %s (%s)\n", greet_name, clean_greet_name);
    /* if (db_connected) { */
    /*     write_greeting(sockfd, resp, clean_greet_name, db); */
    /* } */
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
