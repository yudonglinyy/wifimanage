#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "cmdcall.h"
#include "strip.h"
// #include "curl/curl.h"
#include "curlhttp.h"
#include "ucicmd.h"
#include "f_operation.h"

#define SIZE 1024
#define MINSIZE 256
#define MAXSIZE 4096*2

int main(int argc, char const *argv[])
{

	char cmd[SIZE];
	char mac[18];
	char postdata[SIZE];
	char buf[MAXSIZE];
	char api[MINSIZE];
	char version_head[MINSIZE], version[MINSIZE], version_end[MINSIZE];
	char tarfilename[MINSIZE];
	char dirname[MINSIZE];
	char url[SIZE];
	cJSON *json_root, *json;

	getCmdResult("ubus call network.device status", buf, MAXSIZE);
	
	if(!(json_root = cJSON_Parse(buf)))
		LOG("Error before:[%s]",cJSON_GetErrorPtr());

	if (!(json = cJSON_GetObjectItem(json_root,"eth0"))) {
		LOG("json parse eth error");
	}
	if (!(json = cJSON_GetObjectItem(json,"macaddr"))) {
		LOG("json parse macaddr error");
	}
	strncpy(mac,json->valuestring,17);
	mac[17] = '\0';
	cJSON_Delete(json_root);
	json_root = json = NULL;

	filecontent("/tmp/tmp_api", api, SIZE);
	strip(api, api, sizeof(api));

	uci_get("router.system.system", version_head, sizeof(version_head));
	uci_get("router.system.version", version, sizeof(version));
	uci_get("router.system.subversion", version_end, sizeof(version_end));

	// strcpy(mac, "00:00:00");   //test
	// strcpy(version_head, "sparrow");
	// strcpy(version, "1.0.2");
	// strcpy(version_end, "pro");
	// strcpy(api, "192.168.3.105:5000");

	sprintf(postdata, "control=Identify&data={\"mac\":\"%s\",\"version_head\":\"%s\",\"version\":\"%s\",\"version_end\":\"%s\"}", 
																					mac, version_head, version, version_end);
	// sprintf(cmd, "wget http://%s/Download/VersionUpdater.php -T 10 -O response.json -q --post-data='%s'", api, postdata);
	// if (cmdcall_no_output(cmd)) {
	// 	LOGNUM(2,"Fail:%s", cmd);
	// }


	sprintf(url, "http://%s/Download/VersionUpdater.php", api);
	curlhttp(url, postdata, "post", "response.json");

	memset(buf, 0, MAXSIZE);
	filecontent("response.json", buf, MAXSIZE);

	if (!(json_root = cJSON_Parse(buf))) {
		LOG("Error before:[%s]",cJSON_GetErrorPtr());
	}

	if (!(json = cJSON_GetObjectItem(json_root,"VervificationCode"))) {
		LOG("json parse VervificationCode error");
	}
	char *VervificationCode = strdup(json->valuestring);  //test
	
	if (!(json = cJSON_GetObjectItem(json_root,"LastestVersion"))) {
		LOG("json parse LastestVersion error");
	}
	char *LastestVersion = strdup(json->valuestring);

	cJSON_Delete(json_root);

	sprintf(dirname, "%s_%s%s", version_head, LastestVersion, version_end);
	sprintf(tarfilename, "%s.tar.gz", dirname);
	// sprintf(cmd, "wget -T 10 -q -O %s http://%s/Download/Temp/%s/%s", tarfilename, api, VervificationCode, tarfilename);
	// if (cmdcall_no_output(cmd)) {
	// 	LOGNUM(3,"Fail:%s", cmd);
	// }

	sprintf(url, "http://%s/Download/Temp/%s/%s", api, VervificationCode, tarfilename);
	curlhttp(url, NULL, "get", tarfilename);

	if (access(tarfilename, F_OK)) {
		LOG("%s not found, wget fail", tarfilename);
	}
	sprintf(cmd, "tar -zxvf %s > /dev/null 2>1", tarfilename);
	if (system(cmd)) {
		LOG("tar operation fail");
	}

	if (chdir(dirname)) {
		LOG("chdir %s fail", dirname);
	}

	FILE *fpnewversion = fopen("install.txt", "r");  //test
	if (!fpnewversion) {
		LOG("open filelist fail");
	}
	char ipkname[256];
	while (fgets(ipkname, sizeof(ipkname), fpnewversion)) {
		strip(ipkname, ipkname, sizeof(ipkname));
		printf("%s\n", ipkname);					//test
		// if (access(ipkname, F_OK)) {    
		// 	LOG("have not %s", ipkname);
		// }
		// sprintf(cmd, "opkg install %s", ipkname);
		// cmdcall_no_output(cmd);
	}	
	if (ferror(fpnewversion)) {
		LOG("fgets fail");
	}
	fclose(fpnewversion);

	sprintf(postdata, "control=Complete&data=%s", VervificationCode);
	sprintf(url, "http://%s/Download/VersionUpdater.php", api);
	// if (cmdcall_no_output(cmd)) {
	// 	LOGNUM(4,"Fail:%s", cmd);
	// }
	curlhttp(url, postdata, "post", "/dev/null");

	sprintf(cmd, "rm -rf response.json %s %s", dirname, tarfilename);
	if (chdir("..") || cmdcall_no_output(cmd)) {
		LOG("clean operation fail");
	}
	
	RETURN_MSG(0, "install successful, system will reboot");  //test
	return 0;	
}


