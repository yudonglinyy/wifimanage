#ifndef _VPN_AUTH_
#define _VPN_AUTH_ 


typedef struct reqtokendata
{
	char *ip;
    char *mac;
	char *hash;
	char *version;
	char *reqtime;
} ReqTokendata;

typedef struct resptokendata
{
	int status;
    char *url;
	char *key;
	char *msg;
} RespTokendata;


void request_vpn_cert(const char *url);
void get_vpndail_data(const char *upfile);


#endif