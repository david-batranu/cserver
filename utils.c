#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"
#include "utils.h"

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

void on_resp_buffer_full(void *buff) {
    /* printf("Flush resp buffer!\n"); */
    mybuff *b = (mybuff *)buff;
    write(b->sockfd, b->buffer, b->buffer_size);
    b->buffer[0] = '\0';
    b->p = b->buffer;
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

