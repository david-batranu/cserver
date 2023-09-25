#include <arpa/inet.h>
#include <sys/socket.h>
// #include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
// #include <sqlite3.h>
#include "dbutil.c"

#define PORT 8080
#define BUFFER_SIZE 1024
#define HEADER_SIZE 64
#define RESPONSE_SIZE 4096

static volatile sig_atomic_t keepRunning = 1;

void escape_quotes(char *in) {
    int i = 0;
    for (; in[i]; i++) {
        if (in[i] == '"') {
            in[i] = '\'';
        }
    }
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


long get_file_size(FILE *fp) {
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return size;
}


void send_file(FILE* fp, int sockfd) {
    char buffer[BUFFER_SIZE] = {0};

    for (;;) {
        int nread = fread(buffer, 1, BUFFER_SIZE, fp);
        // printf("Bytes read %d \n", nread);

        if (nread > 0) {
            if (write(sockfd, buffer, nread) == -1) {
                perror("sending file...");
            }
        }

        else if (nread < BUFFER_SIZE) {
            if (feof(fp)) {
                // printf("End of file.\n");
                break;
            }
            else if (ferror(fp)) {
                fprintf(stderr, "Error reading file.\n");
            }
            break;
        }

        bzero(buffer, BUFFER_SIZE);
    }
}


void write_default(int sockfd, char* resp) {
    resp_ok(resp, "text/html", "", "<html>Hello, world!</html>");
    write(sockfd, resp, strlen(resp));
}


void write_favicon(int sockfd, char* resp) {
    char extra_headers[HEADER_SIZE] = {0};

    FILE *fp;
    fp = fopen("favicon.ico", "r");
    if (fp == NULL) {
        perror("opening favicon.ico");
    }

    sprintf(
            extra_headers,
            "Content-length: %ld\r\n",
            get_file_size(fp)
           );

    resp_ok(resp, "image/x-icon", extra_headers, "");

    // send ok
    write(sockfd, resp, strlen(resp));

    // send rest of file
    send_file(fp, sockfd);

    // printf("write: favicon.ico\n");
}


static int db_callback_greeting(void *buffer, int num_columns, char **columns, char **column_names) {
    // printf("Got %d columns!\n", num_columns);
    if (num_columns == 1) {
        // printf("Columns: %s\n", columns[0]);
        sprintf((char *)buffer, "%s", columns[0]);
    }
    return 0;
};


void write_greeting(int sockfd, char* resp, char* name, sqlite3 *db) {

    char query[BUFFER_SIZE] = {0};
    char greeting[BUFFER_SIZE] = {0};
    char body[RESPONSE_SIZE] = {0};

    sprintf(query, "select greeting from user_greeting where name == '%s';", name);

    // printf("query: %s\n", query);

    if (query_db(db, query, db_callback_greeting, greeting) != 0 || strlen(greeting) == 0) {
        sprintf(greeting, "Who?");
    }

    sprintf(body, "<html><head><title>%s</title></head><body><h1>%s</h1></body></html>\n", greeting, greeting);
    resp_ok(resp, "text/html", "", body);
    write(sockfd, resp, strlen(resp));

    // bzero(query, BUFFER_SIZE);
    // bzero(greeting, BUFFER_SIZE);
    // bzero(body, RESPONSE_SIZE);

}

static int db_callback_articles(void *buffer, int num_columns, char **columns, char **column_names) {
    // int *fd = (int*) sockfd;
    // (*ptr)++;
    escape_quotes(columns[1]);
    char article[BUFFER_SIZE*1024];
    sprintf(article, "{\"uri\":\"%s\",\"title\": \"%s\",\"date\":\"%s\"},", columns[0], columns[1], columns[2]);

    strncat(buffer, article, strlen(article));

    // write(*fd, article, strlen(article));

    // printf("count: %i", (int**)buffer);
    // printf("Got %d columns!\n", num_columns);
    // if (num_columns == 1) {
    //     // printf("Columns: %s\n", columns[0]);
    //     sprintf((char *)buffer, "%s", columns[0]);
    // }
    return 0;
};

void write_articles(int sockfd, char* resp, sqlite3 *db) {
    char query[BUFFER_SIZE] = {0};

    char response[BUFFER_SIZE*1024];
    response[0] = '\0';

    char basic[] =
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-type: application/json\r\n"
        "\r\n"
        "{\"results\":[";

    write(sockfd, basic, strlen(basic));

    sprintf(query, "SELECT uri, quote(title), datetime(pubdate, 'unixepoch') FROM Articles ORDER BY -pubdate LIMIT 10;");

    if (query_db(db, query, db_callback_articles, response) != 0) {
        printf("Articles query error");
    }

    write(sockfd, response, strlen(response));

    write(sockfd, "]}", 2);
}


int create_socket() {
    // create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("webserver (socket)");
    }
    printf("socket created successfully!\n");

    // set socket options
    int socket_reuse = 1;
    int set_socket_reuse_addr = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&socket_reuse, sizeof(socket_reuse));
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


void clean_user_string(char *in, char *out) {
    int i = 0;
    int j = 0;
    int ch;
    for (;i < strlen(in); i++) {
        ch = in[i];
        if ((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122)) {
            out[j] = ch;
            j++;
        }
    }
}


int main() {

    // Handle Ctrl+C
    signal(SIGINT, signalHandler);

    sqlite3 *db;
    int db_connected = 0;

    db_connected = connect_db("main.db", &db);

    // int n_requests = 0;
    char request_buffer[BUFFER_SIZE];
    char response_buffer[RESPONSE_SIZE];

    // create a socket
    int sockfd = create_socket();
    if (sockfd == -1) {
        return 1;
    }

    // create the address to bind the socket to
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Create client address
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    // bind the socket to the address
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)
    {
        perror("webserver (bind)");
        return 1;
    }

    printf("socket successfully bound to address!\n");

    // Listen for incoming connections
    if (listen(sockfd, SOMAXCONN) != 0)
    {
        perror("webserver (listen)");
        return 1;
    }

    printf("server listening for connectins!\n");

    while(keepRunning) {
        // Accept incoming connections
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
        if (newsockfd < 0)
        {
            perror("webserver (accept)");
            continue;
        }
        // n_requests++;
        // printf("[%i] connection accepted\n", n_requests);

        // Get client address
        int sockn = getpeername(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            perror("webserver (getpeername)");
            continue;
        }

        // Read from the socket
        int valread = read(newsockfd, request_buffer, BUFFER_SIZE);
        if (valread < 0)
        {
            perror("webserver (read)");
            continue;
        }

        // Read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE] = {0};
        sscanf(request_buffer, "%s %s %s", method, uri, version);


        // printf(
        //         "[%s:%u] %s %s %s\n",
        //         inet_ntoa(client_addr.sin_addr),
        //         ntohs(client_addr.sin_port),
        //         method,
        //         version,
        //         uri
        //       );


        if (strcmp(uri, "/favicon.ico") == 0) {
            write_favicon(newsockfd, response_buffer);
        }
        else if (strcmp(uri, "/articles") == 0) {
            write_articles(newsockfd, response_buffer, db);
        }
        else if (strncmp(uri, "/greet/", 6) == 0) {
            char greet_name[BUFFER_SIZE] = {0};
            char clean_greet_name[BUFFER_SIZE] = {0};
            sscanf(uri, "/greet/%s", greet_name);
            clean_user_string(greet_name, clean_greet_name);
            // printf("greet: %s (%s)\n", greet_name, clean_greet_name);
            if (db_connected) {
                write_greeting(newsockfd, response_buffer, clean_greet_name, db);
            }
        }
        else {
            write_default(newsockfd, response_buffer);
        }

        bzero(response_buffer, BUFFER_SIZE);
        close(newsockfd);
    }

    printf("EXITING...\n");
    close(sockfd);

    return 0;
}
