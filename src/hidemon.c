#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "log.h"
#include "cJSON.h"
#include <stdbool.h>
#include "strip.h"
#include "getwlanname.h"
#include "cmdcall.h"
#include "getiponline.h"
#include "ucicmd.h"
#include "jsonvalue.h"



int main(int argc, char const *argv[])
{	
	int count;
	int flag = 0;
	cJSON *json;
	const char *wifiPath = "/etc/config/conf/wifi/";
	const char *startwifimon = "wifimon";
	const char *ssid=NULL, *encryption=NULL, *key=NULL;
	char device[1024], cmd[1024];

	if (argc != 2) {
		LOG("parameter error");
	}
	
	if(!(json = cJSON_Parse(argv[1])))
		LOG("Error before:[%s]",cJSON_GetErrorPtr());

	count = cJSON_GetArraySize(json);
	if (count != 3)
	{
		RETURN_MSG(1, "Error.\nUsage:\n\
				$1  ssid\n\
				$2  encryption\n\
				$3  key\n");
		exit(1);
	}


	ssid = safe_trip(cJSON_value(json, "ssid", string));
	encryption = safe_trip(cJSON_value(json, "encryption", string));
	key = cJSON_value(json, "key", string);

	getwifidevice(device, sizeof(device));
	char *wlanstr = getwlanname();


	uci_del("wireless.wificlient_chocobo");
	uci_add("wireless.wificlient_chocobo=wifi-iface");
	uci_add("wireless.wificlient_chocobo.network=wwan");
	uci_add("wireless.wificlient_chocobo.device=%s", device);
	uci_add("wireless.wificlient_chocobo.mode=sta");
	uci_add("wireless.wificlient_chocobo.ssid=%s", ssid);
	uci_add("wireless.wificlient_chocobo.encryption=%s", encryption);
	uci_add("wireless.wificlient_chocobo.key=%s", key);

	cmdcall_no_output("/etc/init.d/network reload");
	sleep(15);


	if(isDevHasIp(wlanstr))
	{
		char buffer[1024];
		char hidessid[18];
		
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "iw %s scan|grep associated", wlanstr);   //test

		FILE *pHideScan = popen(cmd, "r");
		if(NULL == pHideScan)
			LOG("popen");
		memset(buffer, 0, sizeof(buffer));
		if (fread(buffer,1,sizeof(buffer),pHideScan) == 0) {
			LOG("%s result is null", cmd);
		}

		strncpy(hidessid, buffer+4,17);
		hidessid[17] = '\0';

		char *hidewifiBssidFile = StringJoin(wifiPath, hidessid);

		FILE *fp;
		if ((fp = fopen(hidewifiBssidFile,"w")) == NULL)
		{
			LOG("%s", hidewifiBssidFile);
		}
		fclose(fp);

		free(hidewifiBssidFile);


		cmdcall_no_output("killall wifimon"); 
		
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo=wifi-iface", hidessid);
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.network=wwan", hidessid);
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.device=%s", hidessid, device);
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.mode=sta", hidessid);
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.ssid=%s", hidessid, ssid);
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.encryption=%s", hidessid, encryption);
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.key=%s", hidessid, key);
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.bssid=%s", hidessid, hidessid);

		pid_t pid;
		switch (pid = fork()) {
		case -1:
			LOG("handle error");
		case 0:
			execlp(startwifimon, startwifimon, (char *)NULL);
			break;
		default:
			flag = 1;
		}
	}
	else
	{
		uci_del("wireless.wificlient_chocobo");
		cmdcall_no_output("/etc/init.d/network reload");
	}

	free(wlanstr);
	free((char*)ssid);
	free((char*)encryption);
	free((char*)key);

	if (flag) {
		RETURN_MSG(0, "hidemon success");
		return 0;
	} else {
		RETURN_MSG(1, "hidemon fail");
		return 1;
	}
	
	
}

