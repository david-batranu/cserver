#include <string.h>
#include <unistd.h>

#include "defs.h"
#include "request.h"


void request_init(Request_t *req, ResponseBuffer_t *resp_buffer, char *response_buffer, char *request_buffer, int sockfd, char* req_method, char* uri) {
    req->sockfd = sockfd;
    req->uri = uri;
    req->buffer = request_buffer;

    resp_buffer->sockfd = sockfd;
    resp_buffer->buffer = response_buffer;
    resp_buffer->p = response_buffer;
    resp_buffer->buffer_size = RESP_BUFFER_SIZE;

    req->resp_buffer = resp_buffer;

    if (strncmp(req_method, "GET", 3) == 0) {
        req->method = RM_GET;
    }
    else if (strncmp(req_method, "POST", 4) == 0) {
        req->method = RM_POST;
    }
    else if (strncmp(req_method, "OPTIONS", 7) == 0) {
        req->method = RM_OPTIONS;
    }
    else {
        req->method = RM_UNK;
    }

}


void request_resp_buffer_full(ResponseBuffer_t *b) {
    /* printf("Flush resp buffer!\n"); */
    /* mybuff *b = (mybuff *)buff; */
    write(b->sockfd, b->buffer, b->buffer_size);
    b->buffer[0] = '\0';
    b->p = b->buffer;
}


void response_write(Request_t *req, char* src) {
    int remaining = req->resp_buffer->buffer_size - (req->resp_buffer->p - req->resp_buffer->buffer);
    while (*src && remaining) {
        *req->resp_buffer->p++ = *src++;;
        remaining--;
    }

    if (!remaining && *src) {
        --req->resp_buffer->p;
        request_resp_buffer_full(req->resp_buffer);
        response_write(req, src);
    }
}

void response_flush(Request_t *req) {
    write(req->sockfd, req->resp_buffer->buffer, (req->resp_buffer->p - req->resp_buffer->buffer));
}
