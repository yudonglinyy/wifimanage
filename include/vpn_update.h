#ifndef _VPN_UPDATE_H
#define _VPN_UPDATE_H

#include "wifitype.h"

int vpn_update_data(char *data, int size);
int get_vpn_num();
int parse_data(char *data, struct vpn_info *vpn_arry, int *p_level_num);
int vpn_update();

#endif