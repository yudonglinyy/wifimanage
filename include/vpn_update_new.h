#ifndef _VPN_UPDATE_NEW_H
#define _VPN_UPDATE_NEW_H

#include <stdbool.h>

#define UsingVpnList    1
#define AllVpnList      2
#define DdnsList        3


typedef struct ddnslist
{
  char * mac;
  char *ip;
  char *domain;
  char *status;
} DDNSLIST;

typedef struct vpnlist
{
    char *mac;
    char *ip;
    char *username;
    char *password;
    char *dialmethod;
    char *speed;
    char *updatetimes;
} VPNLIST;


int vpn_update();
int update_from_type(int type);
DDNSLIST * parse_ddns_data(char *buf, int *pcount);
VPNLIST * parse_vpn_data(char *buf, int *pcount);

int ping_vpn(VPNLIST *p);
int ping_ddns(DDNSLIST *p);

void set_post_data_vpn(VPNLIST *p, char *item_post_data, int len);
void set_post_data_ddns(DDNSLIST *p, char *item_post_data, int len);
void set_post_data_ddns_ip(DDNSLIST *p, char *item_post_data, int len);

int action_base64_curlhttp(const char *url, const char *action, const char *post_data);

void free_ddnslist(DDNSLIST *p, int count);
void free_vpnlist(VPNLIST *plist, int count);

	
#endif