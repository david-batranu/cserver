#ifndef REQUEST_H
#define REQUEST_H

#define RM_GET 1
#define RM_POST 2
#define RM_UNK 0

typedef struct Request Request_t;
typedef struct ResponseBuffer ResponseBuffer_t;


struct ResponseBuffer {
    char *buffer;
    int buffer_size;
    int sockfd;
    char *p;
};


struct Request {
    int sockfd;
    int method;
    char *uri;
    char *buffer;
    ResponseBuffer_t *resp_buffer;
};

void request_init(Request_t *req, ResponseBuffer_t *resp_buffer, char *response_buffer, char *request_buffer, int sockfd, char* req_method, char* uri);
void response_write(Request_t *req, char* src);
void response_flush(Request_t *req);

#endif
