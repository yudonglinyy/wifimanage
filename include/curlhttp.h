#ifndef _CURLHTTP_H
#define _CURLHTTP_H

typedef struct st_stream
{
	void *stream;
	int len;
}St_Stream;

typedef size_t (*WriteFun)(void *ptr, size_t size, size_t nmemb, void *stream);

size_t WriteFileCallback(void *ptr, size_t size, size_t nmemb, void *stream);
size_t WriteStringCallback(void *ptr, size_t size, size_t nmemb, void *pst);
int curlhttp(const char *url, const char *postdata, const char *mode, const char *filename);
int curlhttp_string(const char *url, const char *postdata, const char *mode, char *ptr, int len);
int curlhttp_file_timeout(const char *url, const char *postdata, const char *mode, const char *filename, int timeout);
int curlhttp_string_timeout(const char *url, const char *postdata, const char *mode, char *ptr, int ptrlen, int timeout);
int curlhttp_base(const char *url, const char *postdata, const char *mode, void *ptr, int ptrlen, int timeout, WriteFun callback);

#endif