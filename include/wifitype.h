#ifndef _WIFITYPE_H
#define _WIFITYPE_H

typedef struct  wifiedit_argv
{
	char *bssid;
	char *device;
	char *action;
	char *ssid;
	char *encryption;
	char *key;	
} WIFIEDIT_T;

typedef struct  dial_argv
{
	char *bssid;
	char *device;
	char *action;
	char *ssid;
	char *encryption;
	char *key;	
} DIAL_T;

typedef struct  wifiscan_argv
{
	char *device;
} WIFISCAN_T;

typedef struct vpn_info
{
	int level;
	int mtu;
	char mac[18];
	char server[100];
	char username[100];
	char password[100];
	char proto[10];
	char psk[100];
} VPN_INFO;


int wifiedit_main(int argc_n, WIFIEDIT_T *pArgv);
int dial_main(int argc_n, DIAL_T *pArgv);
int wifiscan_main(int argc_n, WIFISCAN_T *pArgv);




#endif