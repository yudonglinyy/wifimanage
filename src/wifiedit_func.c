#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "log.h"
#include "cJSON.h"
#include "strip.h"
#include "cmdcall.h"
#include "wifitype.h"
#include "ucicmd.h"


void newSsid(const char *wifiPath, const char *bssid, const char *device, const char *ssid, const char *encryption, const char *key);


int wifiedit_main(int argc_n, WIFIEDIT_T *pArgv)
{
	const char *wifiPath = "/etc/config/conf/wifi/";
	const char *wfPath = "/tmp/wf/";

	if (access(wifiPath, F_OK) != 0)
	{
		LOG("have to %s", wifiPath);
	}

	if (access(wfPath, F_OK) != 0 )
	{
		if (mkdir(wfPath, 0644) == -1)
			LOG("mkdir fail"); 
	}
	
	char cmd[1024];
	char *wifiBssidFile = StringJoin(wifiPath, pArgv->bssid);
	char *wfBssidFile = StringJoin(wfPath, pArgv->bssid);

	/* del */
	if (strcmp(pArgv->action,"del") == 0)
	{
		if( !access(wifiBssidFile, F_OK) )
		{
			remove(wifiBssidFile);
		}
		if( !access(wfBssidFile, F_OK) )
		{
			remove(wfBssidFile);
		}
		
		RETURN_MSG(0, "del success");
	}
	/*new or edit*/
	else if (strcmp(pArgv->action,"new") == 0 || strcmp(pArgv->action,"edit") == 0)
	{
		FILE *fp;
		if((fp = fopen(wfBssidFile,"w")) == NULL)
		{
			LOG("open file error");
		}
		fclose(fp);
		newSsid(wfPath, pArgv->bssid, pArgv->device, pArgv->ssid, pArgv->encryption, pArgv->key);
		RETURN_MSG(0, "new success");
	}
	/*conn*/
	else if (strcmp(pArgv->action,"conn") == 0)
	{
		if ((access(wifiBssidFile, F_OK) == 0) || (access(wfBssidFile, F_OK) == 0))
		{
			memset(cmd, 0, sizeof(cmd));

			uci_add("router.wifi.sta=%s", pArgv->device);
			uci_add("router.wifi.enable=1");
			
			cmdcall_no_output("killall wifimon"); 
			DIAL_T dial_argv;
			dial_argv.bssid = pArgv->bssid;
			dial_main(1, &dial_argv);
		}
		else
		{
			RETURN_MSG(1, "conn fail");
			return 255;   //error
		}		
	}
	/*check*/
	else if (strcmp(pArgv->action,"check") == 0)
	{
		if ((access(wifiBssidFile, F_OK) == 0) || (access(wfBssidFile, F_OK) == 0))    //when file is access will return 0 
		{
			RETURN_MSG(0, "file is access");
		}
		else
		{
			RETURN_MSG(1, "Check Error, No Such File\n");
		}	
	}
	else
	{
		RETURN_MSG(1, "%s is unknow\n", pArgv->action);
		return 1;
	}
	free(wifiBssidFile);
	free(wfBssidFile);
	
	return 0;
}



void newSsid(const char *wifiPath, const char *bssid, const char *device, const char *ssid, const char *encryption, const char *key)
{
	char cmd[1024];
	memset(cmd,0,sizeof(cmd));

	uci_add_confdir(wifiPath, "%s.wificlient_chocobo=wifi-iface", bssid);
	uci_add_confdir(wifiPath, "%s.wificlient_chocobo.network=wwan", bssid);
	uci_add_confdir(wifiPath, "%s.wificlient_chocobo.device=%s", bssid, device);
	uci_add_confdir(wifiPath, "%s.wificlient_chocobo.mode=sta", bssid);
	uci_add_confdir(wifiPath, "%s.wificlient_chocobo.ssid=%s", bssid, ssid);
	uci_add_confdir(wifiPath, "%s.wificlient_chocobo.bssid=%s", bssid, bssid);
	uci_add_confdir(wifiPath, "%s.wificlient_chocobo.encryption=%s", bssid, encryption);

	memset(cmd,0,sizeof(cmd));
	if(strcmp(encryption, "none") == 0)
	{
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.encryption=none", bssid);
	}
	else if(strcmp(encryption, "wep") == 0)
	{
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.key=1", bssid);
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.key1=%s", bssid, encryption);
	}
	else
	{
		uci_add_confdir(wifiPath, "%s.wificlient_chocobo.key=%s", bssid, key);
	}

	return;
}

