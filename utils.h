#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "defs.h"

void clean_user_string(char *in, char *out);
void clean_str_number(char *in, char *out);
int str_to_int(char *str);

void on_resp_buffer_full(void *buff);
void myrespstrcat(mybuff *buff, char* src);
char *escape_quotes(char *in);
char* mystrcat( char* dest, char* src );
long get_file_size(FILE *fp);
void send_file(FILE* fp, int sockfd);

#endif
