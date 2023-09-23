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

#define PORT 8080
#define BUFFER_SIZE 1024
#define HEADER_SIZE 64


void resp_ok(char* resp, char* content_type, char* extra_headers) {
    char basic[] = 
    "HTTP/1.1 200 OK\r\n"
    "Connection: close\r\n"
    "Content-type: %s\r\n"
    "%s"
    "\r\n";
    sprintf(resp, basic, content_type, extra_headers);
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
        printf("Bytes read %d \n", nread);

        if (nread > 0) {
            if (write(sockfd, buffer, nread) == -1) {
                perror("sending favicon.ico");
            }
        }
        
        else if (nread < BUFFER_SIZE) {
            if (feof(fp)) {
                printf("End of file.\n");
            }
            else if (ferror(fp)) {
                printf("Error reading file.\n");
            }
            break;
        }

        bzero(buffer, BUFFER_SIZE);
    }
}


void write_favicon(int sockfd) {
    char resp[BUFFER_SIZE];
    char extra_headers[HEADER_SIZE];

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

    resp_ok(resp, "image/x-icon", extra_headers);

    // send ok
    write(sockfd, resp, strlen(resp));

    // send rest of file
    send_file(fp, sockfd);

    printf("write: favicon.ico\n");
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


int main()
{
    int n_requests = 0;
    char buffer[BUFFER_SIZE];
    char resp[] =
        "HTTP/1.1 200 OK\r\n"
        "Server: webserver-c\r\n"
        "Content-type: text/html\r\n"
        "\r\n"
        "<html>hello, world</html>\r\n";

    printf("hello, world!\n");

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

    for (;;)
    {
        // Accept incoming connections
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
        if (newsockfd < 0)
        {
            perror("webserver (accept)");
            continue;
        }
        n_requests++;
        printf("[%i] connection accepted\n", n_requests);

        // Get client address
        int sockn = getpeername(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            perror("webserver (getpeername)");
            continue;
        }

        // Read from the socket
        int valread = read(newsockfd, buffer, BUFFER_SIZE);
        if (valread < 0)
        {
            perror("webserver (read)");
            continue;
        }

        // Read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);


        printf(
            "[%s:%u] %s %s %s\n", 
            inet_ntoa(client_addr.sin_addr), 
            ntohs(client_addr.sin_port),
            method,
            version,
            uri
        );

        if (strcmp(uri, "/favicon.ico") == 0) {
            write_favicon(newsockfd);
        }
        else {
            // Write to the socket
            int valwrite = write(newsockfd, resp, strlen(resp));
            if (valwrite < 0)
            {
                perror("webserver (write)");
                continue;
            }
        }

        close(newsockfd);
    }

    return 0;
}
