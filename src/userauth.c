#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"
#include "curl/curl.h"
#include "curlhttp.h"
#include "jsonvalue.h"
#include "strip.h"

#define SIZE 1024
#define MINSIZE 256
#define MAXSIZE 4096

const char *responsefile = "/tmp/response_data";

typedef struct authdata
{
	char *info;
    char *hash;
	char *key;
	char *serverid;
} AUTHDATA;


typedef struct RespAuthdata
{
    int status;
    char *msg;
    char *ip;
    char *cid;
} RespAuthdata;


void display_usage()
{
    fprintf(stderr, "usage: userauth [option]\n"
                    "option:\n"
                    "\t -i, --info: \n" 
                    "\t -a, --hash: \n"
                    "\t -u, --key: \n"
                    "\t -s, --serverid: \n"
                    "\t -f, --file: save the cid and ip info\n"
                    "\t -u, --url: \n"
                    "\t -h, --help\n");
    exit(EXIT_FAILURE);
}


int parse_auth_response(char *response, RespAuthdata *pitem)
{
    cJSON *json;

    if(!(json = cJSON_Parse(response)))
        LOG("Error before:[%s]",cJSON_GetErrorPtr());

    // int count = cJSON_GetArraySize(json);
    
    pitem->status = atoi(cJSON_value(json, "status", string));
    pitem->msg = strdup(cJSON_value(json, "msg", string));
    pitem->ip = strdup(cJSON_value(json, "ip", string));
    pitem->cid = strdup(cJSON_value(json, "cid", string));

    cJSON_Delete(json);

    return pitem->status;
}


/*save the cid in the line and the ip in the second line*/ 
void save_cid_ip_to_file(const char *filename, const char *cid, const char *ip)
{
    FILE *fp;

    if ((fp = fopen(filename, "w")) == NULL) {
        LOG("fopen fail");
    }
    fprintf(fp, "%s\n%s", cid, ip);
    if (fclose(fp) == EOF) {
        LOG("fclose fail");
    }
    return;
}


void free_resp_auth_data(RespAuthdata item)
{
    free(item.msg);
    free(item.ip);
    free(item.cid);
    return;
}


int main(int argc, char* const *argv)
{
    
	int ch;
    int option_index = 0;
    int code;
    char post_data[MAXSIZE], response[SIZE];
    char savefilename[SIZE];
	char url[SIZE]="https://ip/auth/api/v1/connect";


    AUTHDATA item;
    RespAuthdata resp_auth_item;
    memset((void *)&item, 0, sizeof(item));
    memset((void *)&resp_auth_item, 0, sizeof(resp_auth_item));

	if (argc <= 7 && argc != 2) {
        display_usage();
    }

	static struct option long_options[] =
    {
        {"info", required_argument, NULL, 'i'},
        {"hash", required_argument, NULL, 'a'},
        {"key", required_argument, NULL, 'k'}, 
        {"serverid", required_argument, NULL, 's'},
        {"file", required_argument, NULL,'f'},
        {"url", required_argument, NULL,'u'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0},
    }; 

    while ((ch = getopt_long(argc, argv, "i:a:k:s:f:u:h:", long_options, &option_index )) != -1)
    {
        switch (ch) {
        case 'i':
            item.info = optarg;
            break;
        case 'a':
            item.hash = optarg;
            break;
        case 'k':
            item.key = optarg;
            break;
        case 's':
            item.serverid = optarg;
            break;
        case 'f':
            sprintf(savefilename, "%s", optarg);
            break;
        case 'u':
            strcpy(url, optarg);
            break;
        case '?':
            printf("无效的选项字符 \' %c \'\n", (char)ch);
            exit(EXIT_FAILURE);
        case ':':
            printf("缺少选项参数！\n");
            exit(EXIT_FAILURE);
        case 'h':
        default:
            display_usage();
        }
    }

    // base64_encode(post_data_encrt, (u_char *)post_data, strlen(post_data));
    sprintf(post_data, "info=%s&hash=%s&key=%s&serverid=%s", item.info, item.hash, item.key, item.serverid);
    if (curlhttp_string(url, post_data, "POST", response, SIZE)) {
        exit(1);
    }

    if (strlen(response) == 0) {
        LOG("response file is none, curl %s {hour, num} fail", url);
    }

    // base64_decode(response, (u_char *)buf);
    strip(response, response, SIZE);
printf("%s\n", response);
    code = parse_auth_response(response, &resp_auth_item);

    save_cid_ip_to_file(savefilename, resp_auth_item.cid, resp_auth_item.ip);

    if (0 == code) {
        printf("success:%s(code:%d)\n", resp_auth_item.msg, code);
        return 0;
    } else {
        printf("fail:%s(code:%d)\n", resp_auth_item.msg, code);
        return 1;
    }

    free_resp_auth_data(resp_auth_item);
    return 0;
}

