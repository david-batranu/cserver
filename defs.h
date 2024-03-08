#ifndef DEFS_H
#define DEFS_H

#define BUFFER_SIZE 1024
#define RESP_BUFFER_SIZE 1024*64
#define FILE_BUFFER_SIZE 1024*64

#define HEADER_SIZE 64
#define RESPONSE_SIZE 4096

#define JSON_RESP_HEADER "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-type: application/json\r\n\r\n{\"results\":["
#define JSON_RESP_FOOTER "]}"

#define HTML_RESP_HEADER "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-type: text/html\r\n\r\n"


typedef struct mybuff mybuff;
struct mybuff {
    char *buffer;
    int buffer_size;
    void (*callback)(void *);
    int sockfd;
    char *p;
};

#endif
