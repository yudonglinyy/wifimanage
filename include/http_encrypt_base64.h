#ifndef _HTTP_ENCRYPT_BASE64_H
#define _HTTP_ENCRYPT_BASE64_H

typedef unsigned char u_char;

int urlencode(const char *in, char *out, int len);
void http_data_encrypt(char *srctext, char *destext, char *mcrypt, char *mode, u_char *key, u_char *iv);
void http_data_decrypt(char *srctext, char *destext, char *mcrypt, char *mode, u_char *key, u_char *iv);

#endif
