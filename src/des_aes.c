#include <stdio.h>
#include <sys/types.h>
#include <openssl/des.h>
#include <openssl/evp.h>

#include "log.h"
#include "des_aes.h"

typedef const EVP_CIPHER * (*CRYPTFLAG)(void);


CRYPTFLAG des_get_flag(char *mode, u_char *iv)
{
	CRYPTFLAG flag;

	if (!strcasecmp(mode, "ecb")) {
	    iv = NULL;
	    flag = EVP_des_ecb;
	}
	else if (!strcasecmp(mode, "cbc")) {
		if (!iv) LOG("CBC must give a IV"); //CBC必须有向量
	    flag = EVP_des_cbc;
	}
	else if (!strcasecmp(mode, "cfb")) {
	    if (!iv) LOG("CFB must give a IV"); //CFB必须有向量
	    flag = EVP_des_cfb;
	}
	else if (!strcasecmp(mode, "ofb")) {
		if (!iv) LOG("OFB must give a IV"); //OFB必须有向量
		flag = EVP_des_ofb;
	}
	else {
		LOG("mode is unknow, must be ecb/cbc/cfb/ofb");
	}

	if (iv) {
		if (strlen((char *)iv) < 8) {
			LOG("iv size must be greater than 8");
		}
	}
	
	return flag;
}


CRYPTFLAG des3_get_flag(char *mode, u_char *iv)
{
	CRYPTFLAG flag;

	if (!strcasecmp(mode, "ecb")) {
	    iv = NULL;
	    flag = EVP_des_ede3_ecb;
	}
	else if (!strcasecmp(mode, "cbc")) {
		if (!iv) LOG("CBC must give a IV"); //CBC必须有向量
	    flag = EVP_des_ede3_cbc;
	}
	else if (!strcasecmp(mode, "cfb")) {
	    if (!iv) LOG("CFB must give a IV"); //CFB必须有向量
	    flag = EVP_des_ede3_cfb;
	}
	else if (!strcasecmp(mode, "ofb")) {
		if (!iv) LOG("OFB must give a IV"); //OFB必须有向量
		flag = EVP_des_ede3_ofb;
	}
	else {
		LOG("mode is unknow, must be ecb/cbc/cfb/ofb");
	}

	if (iv) {
		if (strlen((char *)iv) < 8) {
			LOG("iv size must be greater than 8");
		}
	}
	
	return flag;
}


CRYPTFLAG aes_128_get_flag(char *mode, u_char *iv)
{
	CRYPTFLAG flag;

	if (!strcasecmp(mode, "ecb")) {
	    iv = NULL;
	    flag = EVP_aes_128_ecb;
	}
	else if (!strcasecmp(mode, "cbc")) {
		if (!iv) LOG("CBC must give a IV"); //CBC必须有向量
	    flag = EVP_aes_128_cbc;
	}
	else if (!strcasecmp(mode, "cfb")) {
	    if (!iv) LOG("CFB must give a IV"); //CFB必须有向量
	    flag = EVP_aes_128_cfb;
	}
	else if (!strcasecmp(mode, "ofb")) {
		if (!iv) LOG("OFB must give a IV"); //OFB必须有向量
		flag = EVP_aes_128_ofb;
	}
	else {
		LOG("mode is unknow, must be ecb/cbc/cfb/ofb");
	}

	if (iv) {
		if (strlen((char *)iv) < 16) {
			LOG("iv size must be greater than 16");
		}
	}

	return flag;
}


CRYPTFLAG aes_192_get_flag(char *mode, u_char *iv)
{
	CRYPTFLAG flag;

	if (!strcasecmp(mode, "ecb")) {
	    iv = NULL;
	    flag = EVP_aes_192_ecb;
	}
	else if (!strcasecmp(mode, "cbc")) {
		if (!iv) LOG("CBC must give a IV"); //CBC必须有向量
	    flag = EVP_aes_192_cbc;
	}
	else if (!strcasecmp(mode, "cfb")) {
	    if (!iv) LOG("CFB must give a IV"); //CFB必须有向量
	    flag = EVP_aes_192_cfb;
	}
	else if (!strcasecmp(mode, "ofb")) {
		if (!iv) LOG("OFB must give a IV"); //OFB必须有向量
		flag = EVP_aes_192_ofb;
	}
	else {
		LOG("mode is unknow, must be ecb/cbc/cfb/ofb");
	}

	if (iv) {
		if (strlen((char *)iv) < 24) {
			LOG("iv size must be greater than 24");
		}
	}

	return flag;
}


CRYPTFLAG aes_256_get_flag(char *mode, u_char *iv)
{
	CRYPTFLAG flag;

	if (!strcasecmp(mode, "ecb")) {
	    iv = NULL;
	    flag = EVP_aes_256_ecb;
	}
	else if (!strcasecmp(mode, "cbc")) {
		if (!iv) LOG("CBC must give a IV"); //CBC必须有向量
	    flag = EVP_aes_256_cbc;
	}
	else if (!strcasecmp(mode, "cfb")) {
	    if (!iv) LOG("CFB must give a IV"); //CFB必须有向量
	    flag = EVP_aes_256_cfb;
	}
	else if (!strcasecmp(mode, "ofb")) {
		if (!iv) LOG("OFB must give a IV"); //OFB必须有向量
		flag = EVP_aes_256_ofb;
	}
	else {
		LOG("mode is unknow, must be ecb/cbc/cfb/ofb");
	}

	if (iv) {
		if (strlen((char *)iv) < 32) {
			LOG("iv size must greater than 32");
		}
	}

	return flag;
}


CRYPTFLAG crypt_mode_flag(char *mcrypt, char *mode, u_char *key, u_char *iv)
{
	CRYPTFLAG flag;

	if (!strcasecmp(mcrypt, "des")) {
		if (8 != strlen((char *)key)) {
			LOG("DES key[\"%s\"] size must be exactly 8 bytes long", key);
		}
		flag = des_get_flag(mode, iv);
	} else if (!strcasecmp(mcrypt, "3des")) {
		if (16 != strlen((char *)key) && 24 != strlen((char *)key)) {
			LOG("3DES key size must be exactly 16/24 bytes long");
		}
		flag = des3_get_flag(mode, iv);
	} else if (!strcasecmp(mcrypt, "aes_128")) {
		flag = aes_128_get_flag(mode, iv);
	} else if (!strcasecmp(mcrypt, "aes_192")) {
		flag = aes_192_get_flag(mode, iv);
	} else if (!strcasecmp(mcrypt, "aes_256")) {
		flag = aes_256_get_flag(mode, iv);
	} else {
		LOG("mcrypt is unknow, must be des/3des/aes_128/aes_192/aes_256");
	}

	return flag;
}


int do_encrypt_from_mode(u_char *intext, u_char *outbuf, u_char *key, u_char *iv, const EVP_CIPHER *flag)
{
	int outlen, tmplen;

	EVP_CIPHER_CTX *ctx;
	ctx = EVP_CIPHER_CTX_new();
	u_char key2[1024];
	
	if (strlen((char *)key) > sizeof(key2)) {
		LOG("key2 size is too small");
	}

	memset(key2, 0, sizeof(key2));
	memcpy(key2, key, strlen((char *)key));

	EVP_EncryptInit_ex(ctx, flag, NULL, key2, iv);

	if (!EVP_EncryptUpdate(ctx, outbuf, &outlen, intext, strlen((char *)intext))) {
	 	/* Error */
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}

	if (!EVP_EncryptFinal_ex(ctx, outbuf + outlen, &tmplen)) {
		/* Error */
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}

	outlen += tmplen;
	EVP_CIPHER_CTX_free(ctx);

	return outlen;
}


int do_encrypt(u_char *srctext, u_char *outbin, char *mcrypt, char *mode, u_char *key, u_char *iv, int outbin_size)
{
	int len;

	CRYPTFLAG flag = crypt_mode_flag(mcrypt, mode, key, iv);

	if (!(len=do_encrypt_from_mode(srctext, outbin, key, iv, (*flag)()))) {
		LOG("encrypt fail");
	}

	if (len > outbin_size) {
		LOG("outbin size is too small");
	}

	return len;
}


int do_decrypt_from_mode(u_char *intbin, int inbin_len, u_char *outbuf, u_char *key, u_char *iv, const EVP_CIPHER *flag)
{
	int outlen, tmplen;

	EVP_CIPHER_CTX *ctx;
	ctx = EVP_CIPHER_CTX_new();

	u_char key2[1024];
	
	if (strlen((char *)key) > sizeof(key2)) {
		LOG("key2 size is too small");
	}

	memset(key2, 0, sizeof(key2));
	memcpy(key2, key, strlen((char *)key));

	EVP_DecryptInit_ex(ctx, flag, NULL, key2, iv);

	if (!EVP_DecryptUpdate(ctx, outbuf, &outlen, intbin, inbin_len)) {
		/* Error */
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}

	if (!EVP_DecryptFinal_ex(ctx, outbuf + outlen , &tmplen)) {
		/* Error */
		EVP_CIPHER_CTX_free(ctx);
		return 0;
	}

	/*the tmplen data is messy code, should set '\0' to make sure the data right */
	outlen += tmplen;
	outbuf[outlen] = '\0';

	EVP_CIPHER_CTX_free(ctx);

	return outlen;
}


int do_decrypt(u_char *inbin, int inbin_len, u_char *outtext, char *mcrypt, char *mode, u_char *key, u_char *iv)
{
	int len;
	CRYPTFLAG flag;

	flag = crypt_mode_flag(mcrypt, mode, key, iv);

	if (!(len=do_decrypt_from_mode(inbin, inbin_len, outtext, key, iv, (*flag)()))) {
		LOG("decrypt fail");
	}

	return len;
}


void printfhelp(u_char *p, int len)
{
	for (int i=0; i<len; i++)
	{
		printf("%d ", p[i]);
	}
	printf("\n");
	
}