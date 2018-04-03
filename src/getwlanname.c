#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "log.h"
#include "strip.h"
#include "getwlanname.h"
#include "cmdcall.h"
#include "traversalfile.h"
#include "ucicmd.h"

// #define LOGPATH "getwlanname.log"

#define SIZE 1024

char* getwlanname()    //get wlan0
{
	char buffer[SIZE];
	char wlannamepath[SIZE];
	char device[100];
	char *wlanstr = NULL;

	
	if (getwifidevice(device,sizeof(device)))
	{
		LOG("get router fail");
	}

	uci_get("wireless.%s.path",buffer, SIZE, device);
	

	sprintf(wlannamepath, "/sys/devices/%s/net/", buffer);
	if (access(wlannamepath, F_OK))
	{
		LOG("%s", wlannamepath);
	}
	else
	{
		// DIR *pDir;
		// struct dirent *dp;

		// pDir = opendir(wlannamepath);
		// if (pDir == NULL)
		// 	LOG("opendir failed");

		// while ((dp=readdir(pDir)) != NULL)
		// {
		// 	errno = 0;
		// 	if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
		// 		continue;

		// 	// wlanstr = (char*)malloc(strlen(dp->d_name)+1);
		// 	// strcpy(wlanstr, dp->d_name);
		// 	wlanstr = strdup(dp->d_name);
		// 	break;
		// }

		// if (errno != 0)
		// 	LOG("readdir");
	
		// if (closedir(pDir) == -1)
		// 	LOG("closedir");

		list_t *pfile_list = traversal_file(wlannamepath);
		wlanstr = strdup((char *)pfile_list->head->val);   //the first node file in the list
		list_destroy(pfile_list);
	}

	// strcpy(jsonString, "{\"wlan\":\"wlan0\",\"habit\":\"lol\",\"myhabit\":{\"sport\":\"baseball\",\"study\":\"computer\"}}");   //test
	// printf("json: %s\n", jsonString);
	// cJSON *json;
	// if(!(json = cJSON_Parse(jsonString)))
	// 	printf("Error before:[%s]",cJSON_GetErrorPtr());
	// char *wlanstr = cJSON_Print(cJSON_GetObjectItem(json,"wlan"));
	// printf("data:%s\n",wlanstr);
	// // free(wlanstr);  //test
	// cJSON_Delete(json);

	return wlanstr;
}

int getwifidevice(char *device, int len)   //get radio0
{
	char buffer[SIZE];
	int res;

	res = uci_get_no_exit("router.wifi.sta", buffer, SIZE);
	strip(buffer, device, len);
	
	return res;
}


