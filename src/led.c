#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "log.h"
#include <stdbool.h>
#include "strip.h"
#include "cmdcall.h"
#include "getwlanname.h"

// #define LOGPATH "led.log"


void writetofile(const char *pathname, const char *flag);
bool showVpnLevel(const char *proto, const char *level, const char *searchStr);
void lanLed();
void wifiLed();
void wanLed();
void threeGLed();


void writetofile(const char *pathname, const char *flag)   
{
	char path[100];
	sprintf(path, "/sys/class/leds/gl-mifi:green:%s/brightness", pathname);
	FILE *fp;
	if(NULL == (fp = fopen(path, "w")))
		LOG("%s", pathname);
	fwrite(flag, 1, 3, fp);
	fclose(fp);
	return;
}


bool showVpnLevel(const char *proto, const char *level, const char *searchStr)
{
	char buffer[1024] = "0";
	char cmd[1024];
	sprintf(cmd, "ifconfig %s-vpn%s 2> /dev/null", searchStr, level);
	getCmdResult(cmd, buffer, sizeof(buffer));
	if(*buffer != 0 && strstr(buffer, "inet addr:"))
	{
		writetofile("lan", "255");
		return true;
	}
	else
	{
		writetofile("lan", "0");
		return false;		
	}
}

void lanLed()
{
	char model[1024];
	getCmdResult("uci get router.vpn.model", model, sizeof(model));
	if(0 == strcmp(model, "openvpn"))
	{
		if(!cmdcall_no_output("ip link show tun0"))
		{
			writetofile("lan","255");
		}
		else
		{
			writetofile("lan","0");
		}
	}
	else
	{
		char level[1024];
		char proto[1024];
		getCmdResult("uci get router.vpn.level", level, sizeof(level));
		char cmd[1024];
		sprintf(cmd, "uci get network.vpn%s.proto", level);
		getCmdResult(cmd, proto, sizeof(proto));

		if (0 == strcmp(proto,"pptp"))
		{
			showVpnLevel(proto, level, "pptp");
		}
		else if(0 == strcmp(proto,"l2tp"))
		{
			showVpnLevel(proto, level, "l2tp");
		}
		else
		{
			writetofile("lan", "0");
		}

	}
}


void wifiLed()
{
	char buffer[1024];
	char cmd[1024];
	char devicename[1024];

	if ( !getwifidevice(devicename, sizeof(devicename)))
		LOG("get wifi devicename errno");
	sprintf(cmd, "uci get wireless.%s.disabled", devicename);
	int num =atoi(getCmdResult("uci get router.wifi.soft_ap_num", buffer, sizeof(buffer)));
	int num2 =atoi(getCmdResult(cmd, buffer, sizeof(buffer)));
	bool flag = false;
	
	if ((0 < num) && (0 == num2))
	{
		int i;
		for(i=0; i<num; i++)
		{
			if (0 == atoi(getCmdResult("uci get wireless.wifiap_$i.disabled", buffer, sizeof(buffer))))
				flag = true;
		}
	}

	if(flag)
		writetofile("wlan", "255");
	else
		writetofile("wlan", "0");
}


void wanLed()
{
	char buffer[1024];
	char test_ip[1024];
	getCmdResult("uci get router.@system[0].check_ip", test_ip, sizeof(test_ip));
	getCmdResult("route -n", buffer, sizeof(buffer));
	if (NULL == strstr(buffer, test_ip))
	{
		char cmd[1024];
		sprintf(cmd, "route add -net %s netmask 255.255.255.255 gateway `cat /tmp/tmp_gateway` && ping %s -c 1 -W 3", test_ip, test_ip);
		if (! cmdcall_no_output(cmd))
		{
			writetofile("wan", "255");
		}
		else
		{
			writetofile("wan", "0");
		}
	}
}

void threeGLed()
{
	char buffer[1024];
	int resNum = cmdcall_no_output("ip link show 3g-wan");
	getCmdResult("uci get router.@system[0].wan_type", buffer, sizeof(buffer));
	if (!resNum && (strcmp(buffer,"3g") == 0))
	{
		writetofile("net", "255");
	}
	else
	{
		writetofile("net", "0");
	}
}

int main(int argc, char const *argv[])
{
	while(1)
	{
		lanLed();
		wifiLed();
		wanLed();
		threeGLed();
		sleep(3);
	}
	return 0;		
}
