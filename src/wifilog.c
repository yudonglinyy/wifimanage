#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "cmdcall.h"
#include "ucicmd.h"
#include "f_operation.h"
#include "wifilog.h"

#define SIZE 1024


int main(int argc, char const *argv[])
{
	if (argc != 2) {
		RETURN_MSG(1, "Usage: %s [wifi|honey]", argv[0]);
		return 1;
	}

	const char *filelogpath = "/root/log";
	if (access(filelogpath, F_OK)) {
		LOG("%s isn't exist", filelogpath);
	}

	switch (argv[1][0]) {
	case 'w':
			wifilog();
			break;
	case 'h':
			honeylog();
			break;
	default:
		RETURN_MSG(1, "Usage: %s [wifi|honey]", argv[0]);
		return 1;
	}

	return 0;
}


void wifilog()
{
	char buf[SIZE];
	char cmd[SIZE];
	int enable;
	u_long wifi_log_lines, maxlines, dellines, count;
	const char *wifi_log_path = "/root/log/wifi.log";

	uci_get("router.wifilog.enable", buf, SIZE);

	if ((enable = atoi(buf)) == 0) {
		RETURN_MSG(0, "wifilog is disable");
		return;
	}

	uci_get("router.wifilog.maxlines", buf, SIZE);
	maxlines = strtoul(buf, NULL, 10);

	uci_get("router.wifilog.dellines", buf, SIZE);
	dellines = strtoul(buf, NULL, 10);

	if (access(wifi_log_path, F_OK)) {
		sprintf(cmd, "logread -f -e 'deauthenticated due to local deauth request' -F %s &", wifi_log_path);
		cmdcall_no_output(cmd);
		wifi_log_lines = 0;
	} else {
		wifi_log_lines = file_wc(wifi_log_path);
	}

	if (wifi_log_lines >= maxlines) {
		count = wifi_log_lines - dellines;
		cmdcall_no_output("killall logread");

		sprintf(cmd, "sed -i '1,'%lu'd' %s", count, wifi_log_path);
		cmdcall_no_output(cmd);

		sprintf(cmd, "logread -f -e 'deauthenticated due to local deauth request' -F %s &", wifi_log_path);
		cmdcall_no_output(cmd);
	}

}

void honeylog()
{
	char buf[SIZE];
	char cmd[SIZE];
	int enable;
	u_long honey_log_lines, maxlines, dellines, count;
	const char *honey_log_path = "/root/log/honey.log";

	uci_get("router.honeypot.enable", buf, SIZE);

	if ((enable = atoi(buf)) == 0) {
		RETURN_MSG(0, "honey is disable");
		return;
	}

    uci_get("router.honeypot.maxlines", buf, SIZE);
	maxlines = strtoul(buf, NULL, 10);

	uci_get("router.honeypot.dellines", buf, SIZE);
	dellines = strtoul(buf, NULL, 10);

	if (access(honey_log_path, F_OK)) {
		honey_log_lines = 0;
	} else {
		honey_log_lines = file_wc(honey_log_path);
	}

	if (honey_log_lines >= maxlines) {
		count = honey_log_lines - dellines;
		sprintf(cmd, "sed -i '1,'%lu'd' %s", count, honey_log_path);
		cmdcall_no_output(cmd);
	}
}

