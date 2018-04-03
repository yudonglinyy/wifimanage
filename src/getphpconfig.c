#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "log.h"
#include "strip.h"
#include "cmdcall.h"

// #define LOGPATH "getphpconfigl.log"
#define SIZE 1024


int main(int argc, char const *argv[])
{
	char buffer[SIZE];
	char cmd[SIZE];
	char name[SIZE];
	char model[SIZE];
	char enabled[6];

	if (access("/web", F_OK) != 0 )
	{
		if (mkdir("/web", 0644) == -1)
			LOG("mkdir fail"); 
	}

	FILE *fpconfig;
	if ((fpconfig = fopen("/web/config.php", "w")) == NULL) {
		LOG("open php configfile fail");
	}
	fwrite("<?php\n", 1, 6, fpconfig);

	int i;
	int total_num =atoi(getCmdResult("uci get router.@system[0].module_num", buffer, SIZE));
	for (i=0; i<total_num; i++) {
		memset(cmd, 0, SIZE);
		sprintf(cmd, "uci get router.@module[%d].enable", i);
		if ((atoi(getCmdResult(cmd, buffer, SIZE))) == 1) {
			strcpy(enabled, "true");
		} else {
			strcpy(enabled, "false");
		}

		memset(cmd, 0, SIZE);
		sprintf(cmd, "uci get router.@module[%d].name", i);
		getCmdResult(cmd, name, SIZE);
		sprintf(buffer, "$%s_enabled=%s;\n", name, enabled);
		fwrite(buffer, 1, strlen(buffer), fpconfig);

		if (!strcmp(name, "vpn")) {
			memset(cmd, 0, SIZE);
			sprintf(cmd, "uci get router.@module[%d].model", i);
			getCmdResult(cmd, model, SIZE);

			memset(buffer, 0, SIZE);
			sprintf(buffer, "$vpn_model=%s;\n", model);
			fwrite(buffer, 1, strlen(buffer), fpconfig);
		}
	
	}
	fwrite("?>", 1, 2, fpconfig);
	fclose(fpconfig);

	return 0;


}