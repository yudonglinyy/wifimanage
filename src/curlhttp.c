#include <stdio.h>
#include "log.h"
#include "curl/curl.h"
#include "curlhttp.h"

size_t WriteFileCallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    St_Stream *pstream = (St_Stream *)stream;
    int written = fwrite(ptr, size, nmemb, (FILE *)(pstream->stream));
    return written;
}


size_t WriteStringCallback(void *ptr, size_t size, size_t nmemb, void *pst)
{
    St_Stream *pstring = (St_Stream *)pst;
    char *pbuf = (char *)(pstring->stream);

    if (nmemb * size >= pstring->len) {
        LOG("pstring buf is too small to write the response data");
    }

    memset(pbuf, 0, pstring->len);
    snprintf(pbuf, nmemb * size + 1, "%s", (char *)ptr);

    return strlen(pbuf);
}

int curlhttp(const char *url, const char *postdata, const char *mode, const char *filename)
{
    int default_timeout = 15;
    return curlhttp_file_timeout(url, postdata, mode, filename, default_timeout);
}

int curlhttp_string(const char *url, const char *postdata, const char *mode, char *ptr, int len)
{
    int default_timeout = 15;
    return curlhttp_string_timeout(url, postdata, mode, ptr, len, default_timeout);
}


int curlhttp_file_timeout(const char *url, const char *postdata, const char *mode, const char *filename, int timeout)
{
    int resnum;
    FILE *fp = NULL;

    if (!filename) {
        LOG("filename is NULL");
    }
    if (!(fp = fopen(filename, "w+"))) {
        LOG("file %s open fail", filename);
    }

    resnum = curlhttp_base(url, postdata, mode, (void *)fp, 0, timeout, WriteFileCallback);

    fclose(fp);

    return resnum;
}


int curlhttp_string_timeout(const char *url, const char *postdata, const char *mode, char *ptr, int ptrlen, int timeout)
{
    if (!ptr) {
        LOG("ptr is NULL");
    }
    return curlhttp_base(url, postdata, mode, (void *)ptr, ptrlen, timeout, WriteStringCallback);
}


int curlhttp_base(const char *url, const char *postdata, const char *mode, void *ptr, int ptrlen, int timeout, WriteFun callback)
{
    char *urlencode_postdata = NULL;
    CURL *curl_handle;
    CURLcode res;
    St_Stream st;
    st.stream = ptr;
    st.len = ptrlen;
    
    int resnum = 0;
    long http_code = 0;

    if (!ptr) {
        LOG("ptr is NULL");
    }

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();

    if(!curl_handle) {
        LOG("curl_easy_init fail");
    }

    if (!strcmp("post", mode) || !strcmp("POST", mode)) {
        // curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, strlen(postdata));

        printf("%s\n", postdata);
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postdata);
    }

    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, "charset:utf-8");

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&st);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, timeout);

    res = curl_easy_perform(curl_handle);
    curl_easy_getinfo (curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

    if(res != CURLE_OK) {
        resnum = 1;
        RETURN_MSG(1, "url:%s, errmsg: %s", url, curl_easy_strerror(res));
    } else if (http_code != 200 || res == CURLE_ABORTED_BY_CALLBACK) {
        //fail
        resnum = 1;
        RETURN_MSG(1, "http_code:%ld, url:%s", http_code, url);
    }

    curl_easy_cleanup(curl_handle);
    curl_slist_free_all(headers);
    curl_free(urlencode_postdata);

    curl_global_cleanup();

    return resnum;
}

