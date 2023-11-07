#ifndef DEFS_H
#define DEFS_H

#define BUFFER_SIZE 1024
#define RESP_BUFFER_SIZE 1024*64

#define JSON_RESP_HEADER "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: application/json\r\n\r\n{\"results\":["
#define JSON_RESP_FOOTER "]}"

typedef struct mybuff mybuff;
struct mybuff {
    char *buffer;
    int buffer_size;
    void (*callback)(void *);
    int sockfd;
    char *p;
};

#endif