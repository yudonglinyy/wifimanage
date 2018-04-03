#include <stdio.h>
#include <regex.h>
#include <unistd.h>
#include <stdbool.h>
#include "log.h"
#include "cmdcall.h"
#include "ucicmd.h"

// #define LOGPATH "wifichecker.log"
#define SIZE  1024

int get_device_total_num();
bool match_bool(const char *src, regex_t *preg);

int main(int argc, char const *argv[])
{
	char buffer[SIZE];
	char wlannamepath[SIZE];
	int disabled, num, res;
	bool config_changed;

	while (1) {
		config_changed = false;
		int device_total_num = get_device_total_num();
		for(num=0; num<device_total_num; num++) {
			res = uci_get_no_exit("wireless.radio%d.disabled", buffer, SIZE, num);
			if (res) {
				disabled = 1;
				config_changed = true;
				// memset(cmd, 0, SIZE);
				// sprintf(cmd, "uci set wireless.radio%d.disabled=0", num);

			} else {
				disabled = atoi(buffer);
			}
			uci_get_no_exit("wireless.radio%d.path", buffer, SIZE, num);
			sprintf(wlannamepath, "/sys/devices/%s", buffer);
			if (access(wlannamepath, F_OK)) {		//path is not exist, if disabled is 0,will change 1
				if (!disabled) 					
					config_changed = true;

				disabled = 1;
			} else {								//path is exist, if disabled is 1,will change 0
				if (disabled)               
					config_changed = true;

				disabled = 0;
			}

			uci_add("wireless.radio%d.disabled=%d", num, disabled);
		}
		
		
		if (config_changed) {
			cmdcall_no_output("/etc/init.d/network reload;");
			// wait();
		}

		sleep(3);
	}
	return 0;
}

int get_device_total_num() 
{
	char buffer[SIZE];
	int num = 0;

	FILE *fp = fopen("/etc/config/wireless", "r");
	if (fp == NULL) {
		LOG("open error");
	}

	const char *radio_pattern = "^config \\S+ \'radio[0-9]+\'\n$";
	int cflags = REG_EXTENDED ;
	regex_t reg;
	regcomp(&reg,radio_pattern,cflags);					//compile

	while (NULL != fgets(buffer, SIZE, fp)) {
		if (match_bool(buffer, &reg))
			num++;
	}

	regfree (&reg);         //release
	fclose(fp);
	return num;
}


bool match_bool(const char *src, regex_t *preg)   //match regular
{
	int status;
	regmatch_t pmatch[10];
	const size_t nmatch = 10;

	status = regexec(preg,src,nmatch,pmatch,0);		//match
	if(status == REG_NOMATCH)
		return false;
   	else if(status == 0)
		return true;
    else
    {
    	LOG("match");
		return false;
    }
}