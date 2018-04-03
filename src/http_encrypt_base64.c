#include <stdio.h>
#include <assert.h>
#include "curl/curl.h"
#include "log.h"
#include "base64.h"
#include "des_aes.h"
#include "http_encrypt_base64.h"


#define MAXSIZE 16384


int urlencode(const char *in, char *out, int len)
{
	assert(in != NULL);
	assert(out != NULL);

	char *tmp = NULL;
	CURL *curl = NULL;

	if(!(curl=curl_easy_init())) {
		return 1;
	}

	if (!(tmp=curl_easy_escape(curl, in, strlen(in)))) {
		return 1;
	}

	snprintf(out, len, "%s", tmp);
    
    curl_free(tmp);
	curl_easy_cleanup(curl);
	return 0;
}



/* text -> encrypt_bin -> base64_text -> urlencode_text*/
void http_data_encrypt(char *srctext, char *destext, char *mcrypt, char *mode, u_char *key, u_char *iv)
{
	char buf[MAXSIZE];
	u_char en_bin[MAXSIZE];
	memset(buf, 0, MAXSIZE);

	assert(srctext && destext && mode && key);

	int bin_len = do_encrypt((u_char *)srctext, en_bin, mcrypt, mode, key, iv, MAXSIZE);
	base64_encode(buf, en_bin, bin_len);
	if (urlencode(buf, destext, MAXSIZE)) {
		LOG("urlencode fail(%s)", buf);
	}
	return;
}



/* base64_text  -> decrypt_bin-> text*/
void http_data_decrypt(char *srctext, char *destext, char *mcrypt, char *mode, u_char *key, u_char *iv)
{
	int inbin_len;

	if (!(srctext && destext && mode && key)) {
		LOG("point is null");
	}

	u_char inbin[MAXSIZE];
	memset(inbin, 0, MAXSIZE);
	inbin_len = base64_decode(srctext, inbin);
	do_decrypt(inbin, inbin_len, (u_char *)destext, mcrypt, mode, key, iv);

	return;
}