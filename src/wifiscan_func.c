#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "log.h"
#include "cJSON.h"
#include "strip.h"
#include "cmdcall.h"
#include "wifitype.h"


// #define LOGPATH "wifiscan.log"


char* getIwScanJson(FILE *pIwScan);


int wifiscan_main(int argc_n, WIFISCAN_T *pArgv)
{
	char cmd[1024];
	
	sprintf(cmd, "ifconfig %s", pArgv->device);
	if (cmdcall_no_output(cmd))
	{
		LOG("Device not found");
	}

	sprintf(cmd, "iw %s scan", pArgv->device);
	FILE *pIwScan;
	if((pIwScan =popen(cmd, "r")) == NULL)
		LOG("popen iw scan error ");

	char *jsonbuffer = getIwScanJson(pIwScan);
	printf("%s\n", jsonbuffer);

	return 0; 
}


/* make iw scan result into json */
char* getIwScanJson(FILE *pIwScan)
{
	char buffer[1024];
	char mac6[18];
	char ssid[100];

	memset(buffer,0,sizeof(buffer));
	if((fgets(buffer,sizeof(buffer),pIwScan)) == NULL)
		LOG("iw scan Fail");

	cJSON *json = cJSON_CreateObject();
	cJSON *array = NULL;
	cJSON *obj = NULL;
	cJSON_AddItemToObject(json,"result",array=cJSON_CreateArray());

	if(NULL != strstr(buffer,"Usage"))    //when inputs is wrong, will return json
	{
		LOG("scan error! confirm your wifi device is online");
	}

	do{
		while(buffer != strstr(buffer, "BSS"))     //get the wifi mac
		{
			memset(buffer,0,sizeof(buffer));
			if( NULL == fgets(buffer,sizeof(buffer),pIwScan))   //result is EOF, will return json
			{
				cJSON_AddItemToObject(json,"status",cJSON_CreateNumber(0));
				char *jsonstr = cJSON_Print(json);
				cJSON_Delete(json);
				return jsonstr;
			}
		}
		strncpy(mac6, buffer+4, 17);
		mac6[17] = '\0';
		while(NULL == strstr(buffer, "SSID"))     //get SSID
		{
			memset(buffer,0,sizeof(buffer));
			if (fgets(buffer,sizeof(buffer),pIwScan) == NULL) {
				LOG("parse wifiscan jsondata fail");
			}
		}
		memset(ssid, 0, sizeof(ssid));
		char ssidbuff[1024];
		strip(buffer,ssidbuff, sizeof(ssidbuff));
		strcpy(ssid, ssidbuff+6);

		cJSON_AddItemToArray(array,obj=cJSON_CreateObject());
		cJSON_AddItemToObject(obj,"mac",cJSON_CreateString(mac6));
		cJSON_AddItemToObject(obj,"ssid",cJSON_CreateString(ssid));
		memset(buffer,0,sizeof(buffer));
	}while(1);

	return 0;
}