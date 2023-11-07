#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

void clean_user_string(char *in, char *out);
void clean_str_number(char *in, char *out);
int str_to_int(char *str);

void on_resp_buffer_full(void *buff);
void myrespstrcat(mybuff *buff, char* src);
char *escape_quotes(char *in);

#endif
