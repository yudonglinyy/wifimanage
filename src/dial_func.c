#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "log.h"
#include "cJSON.h"
#include <stdbool.h>
#include "strip.h"
#include "getwlanname.h"
#include "cmdcall.h"
#include "list.h"
#include "traversalfile.h"
#include "wifitype.h"
#include "getiponline.h"
#include "f_operation.h"
#include "ucicmd.h"


// #define LOGPATH "./dial.log"


bool dialafile(const char *filename, const char *wlanstr);
int list_match(void *src, void *des);
list_t *get_wifiscan();
int exec_wifiscan(char *buffer, int len);


int dial_main(int argc_n, DIAL_T *pArgv)
{
	const char *wifiPath = "/etc/config/conf/wifi/";
	const char *wfPath = "/tmp/wf/";
	const char *startwifimon = "wifimon";
	char *wlanstr = getwlanname();
	bool dialsuccessflag = false;
	
	/* change dirpath is /tmp/wf */
	if (chdir(wfPath)) {
		LOG("chidr %s fail", wfPath);
	}     

	/*dial a file*/
	if (argc_n == 1)
	{
		char *wfBssidFile = StringJoin(wfPath, pArgv->bssid);
		char *wifiBssidFile = StringJoin(wifiPath, pArgv->bssid);
		char *newBssidFile = StringJoin(wfPath, "new");
		char *filename = NULL;
		
		if (!access(wfBssidFile, F_OK)) {
			filename = wfBssidFile;
		} else if (!access(wifiBssidFile, F_OK)) {
			filename = wifiBssidFile;
		}
		else {
			LOG("both %s and %s are not exist", wfBssidFile, wifiBssidFile);
		}

		if (dialafile(filename, wlanstr))    //dial the new bssid file
		{
			if (strcmp(pArgv->bssid, "new"))
			{
				if (copyfile(wfBssidFile, wifiBssidFile))
					LOG("cp ssid-file error");

				if (copyfile(wfBssidFile, newBssidFile))
					LOG("cp new-file error");
			}
			
			dialsuccessflag = true;
		}

		free(wfBssidFile);
		free(wifiBssidFile);
		free(newBssidFile);
	}
	/*dial all files*/
	else if (argc_n == 0)
	{
		list_t *pfile_list = traversal_file(wifiPath);
		list_t *pjson_list = get_wifiscan();
		list_node_t *file_node;

		list_iterator_t *it = list_iterator_new(pfile_list, LIST_HEAD);
		while ((file_node = list_iterator_next(it)) &&  list_find(pjson_list, file_node->val )) {
			/*try to dial a file*/
			if (dialafile(file_node->val, wlanstr)) {
				dialsuccessflag = true;
				break;
			}
		}
		list_iterator_destroy(it);
		list_destroy(pfile_list);	
	}
	else
	{	
		LOG("json error\n");
	}


	free(wlanstr);

	if (!dialsuccessflag)
	{
		uci_del("wireless.wificlient_chocobo");
		cmdcall_no_output("/etc/init.d/network reload");
		RETURN_MSG(255, "Dial Error\n");	
		return 255;
	} 
	else
	{
		pid_t pid;
		int res = 0;
		switch (pid = fork()) {
		case -1:
			LOG("handle error");
		case 0:
			res = execlp(startwifimon, startwifimon, (char *)NULL);
		default:
			if (res)
			{
				RETURN_MSG(res, "exec wifimon fail");
				return res;
			} else {
				RETURN_MSG(0, "dial success and exec wifimon");
				return 0;
			}
		}
	}
	
}

bool dialafile(const char *filename, const char *wlanstr)
{
	if (!wlanstr) {
		LOG("wlanstr is NULL");
	}
	
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	int n = 3;
	while(n--)
	{
		uci_del("wireless.wificlient_chocobo");
		FILE *pFilepath = fopen(filename, "r");
		if (pFilepath == NULL)
		{
			LOG("%s", filename);
		}
		memset(buffer, 0, sizeof(buffer));
		if (fread(buffer, 1,sizeof(buffer), pFilepath) == 0) {
			LOG("fread %s fail", filename);
		}
		FILE *pConfigpath = fopen("/etc/config/wireless", "a+");
		fwrite(buffer, 1, strlen(buffer), pConfigpath);
		fclose(pFilepath);
		fclose(pConfigpath);
		
		cmdcall_no_output("/etc/init.d/network reload");
		sleep(16);       

		if(isDevHasIp(wlanstr))
		{
			return true;
		}
	}
	return false;
}


int list_match(void *src, void *des)
{
	return !strcmp(src, des);
}

list_t *get_wifiscan()
{
	cJSON *json = NULL;
	char *pMac = NULL;
	char *macstr = NULL;
	char buffer[4096];
	
	list_t *plist = list_new();
	plist->free=free;
	plist->match = list_match;

	if ( ! exec_wifiscan(buffer, sizeof(buffer)) ) {
		LOG("wifiscan exit error");
	}
	
	if(!(json = cJSON_Parse(buffer))) {
		LOG("Error before:[%s]",cJSON_GetErrorPtr());
	}

	cJSON *json_status = cJSON_GetObjectItem(json,"status");

	//when json_status is NULL or valueint isn't 0, will log and exit;
	if ( ! json_status || json_status->valueint) {			
		LOG("get wifiscan json is wrong");
	}

	cJSON *pjson_list;
	if (!(pjson_list = cJSON_GetObjectItem(json,"result"))) {
		LOG("pjson_list is NULL");
	}

	for (pjson_list=pjson_list->child; pjson_list != NULL; pjson_list = pjson_list->next)
	{
		pMac = cJSON_GetObjectItem(pjson_list,"mac")->valuestring;
		macstr = (char *)malloc(strlen(pMac)+1);
		strcpy(macstr, pMac);
		list_rpush(plist, list_node_new( macstr));
	}

	cJSON_Delete(json);

	return plist;
}

int exec_wifiscan(char *buffer, int len)
{
	int pipefd[2];
	pid_t pid;

	if (pipe(pipefd) == -1) {
    	perror("pipe");
		exit(EXIT_FAILURE);
	}

	switch (pid=fork()) {
	case -1:
		LOG("fork fail");
		break;
	case 0:
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		char* wlanstr = getwlanname();
		
		WIFISCAN_T wifiscan_argv;
		wifiscan_argv.device = wlanstr;
		int res = wifiscan_main(1, &wifiscan_argv);

		free(wlanstr);
		exit(res);
	default:
		close(pipefd[1]);
		if (buffer) {    //if buffer is NULL, don't read the result
			read(pipefd[0], buffer, len);
		}
		int status;
		waitpid(pid, &status, 0);
		close(pipefd[0]);
		return WIFEXITED(status);  //if fail exit will return zero
	}
	
}
