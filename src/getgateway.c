#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "strip.h"
#include "cJSON.h"
#include "cmdcall.h"
#include "getgateway.h"
#include "ucicmd.h"

#define SIZE 4096

char *getgateway()
{
	char buffer[SIZE];
	char wan_type[100];
	char *gateway;

	uci_get("router.system.wan_type", buffer, SIZE);
	strip(buffer, wan_type, sizeof(wan_type));
	if (!strcmp(wan_type, "static")) {
		gateway = from_type_get_gateway("wan");
	} else if (!strcmp(wan_type, "dhcp")) {
		gateway = from_type_get_gateway("wan");
	} else if (!strcmp(wan_type, "3g")) {
		int res = cmdcall_no_output("ifconfig 3g-wan");
		if (res) {
			gateway = from_type_get_gateway("wan");
		} else {
			gateway = getCmdResult("ip route|grep 'dev 3g-wan'|head -n 1|cut -d\" \" -f3", buffer, SIZE);
		}
	} else if (!strcmp(wan_type, "wifi")) {
		gateway = from_type_get_gateway("wwan");
	} else {
		LOG("wan_type is wrong");
	}

	return gateway;
}

char *from_type_get_gateway(const char *ifname)
{
	if (!ifname) {
		LOG("ifname is NULL");
	}
	
	char buffer[SIZE];
	char cmd[SIZE];
	char *gatewaystr = NULL;

	sprintf(cmd, "ubus call network.interface.%s status", ifname);
	getCmdResult(cmd, buffer, SIZE);

	cJSON *json;
	if(!(json = cJSON_Parse(buffer)))
		LOG("Error before:[%s]",cJSON_GetErrorPtr());

	cJSON *routelist = cJSON_GetObjectItem(json,"route");
	if (!routelist) {
		LOG("get gateway fail");
	}
	for ( routelist=routelist->child; routelist != NULL; routelist=routelist->next) {
		gatewaystr = cJSON_GetObjectItem(routelist, "nexthop")->valuestring;
		// printf("%s\n", gatewaystr);                                              //test
		if (gatewaystr[0] != '0')
			break;
	}
	if (!routelist) {
		return NULL;
	}
	char *gateway = strdup(gatewaystr);
	cJSON_Delete(json);
	return gateway;
}