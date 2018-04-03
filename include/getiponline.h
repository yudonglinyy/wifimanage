#ifndef _GETIPONLINE_H
#define _GETIPONLINE_H

#include <stdbool.h>

bool getIpOnline(const char *wlanstr);
int get_ip(const char *dev, char *ip);
int get_if(const char *dev, char *ip);
int get_ip_netmask(const char *dev, char *ip);
int get_mac(const char *dev, char *addr);
bool get_addr(const char *dev, char *addr, int flag);
bool isDevUp(const char *dev);
bool isDevHasIp(const char *dev);
int getdevlist();

#endif