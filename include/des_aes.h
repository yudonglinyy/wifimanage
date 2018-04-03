#ifndef _DES_AES_
#define _DES_AES_

#include <sys/types.h>

void check_key_size(u_char *key);

int do_encrypt(u_char *srctext, u_char *outbin, char *mcrypt, char *mode, u_char *key, u_char *iv, int outbin_size);

int do_decrypt(u_char *inbin, int inbin_len, u_char *outtext, char *mcrypt, char *mode, u_char *key, u_char *iv);


#endif

