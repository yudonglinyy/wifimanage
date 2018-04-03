#ifndef _BASE64_H
#define _BASE64_H

#include <sys/types.h>

char *base64_encode(char *base64, const u_char * bindata, int binlength );
int base64_decode( const char * base64, unsigned char * bindata );
void base64_file_encode(FILE * fp_in, FILE * fp_out);
void base64_file_decode(FILE * fp_in, FILE * fp_out);

#endif