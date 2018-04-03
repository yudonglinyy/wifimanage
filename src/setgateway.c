#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "log.h"
#include "cmdcall.h"
#include "getgateway.h"
#include "ucicmd.h"

// #define LOGPATH "setgateway.log"
#define SIZE 1024


void write_file_to_tmp(const char *path, const char *content, int len);
int setgateway_main(void);


int main(int argc, char const *argv[])
{
	return setgateway_main();
}


int setgateway_main(void)
{
	char *gateway = NULL;
	char buffer[SIZE];
	char cmd[SIZE];
	char model[SIZE];
	char ip[SIZE];

	uci_get("router.vpn.model", model, SIZE);
	if (strcmp(model, "auto")) {
        RETURN_MSG(1, "vpn.model isn't auto");
		return 1;
	}

	while ((gateway = getgateway()) == NULL) {
		sleep(2);
	}

	if ( ! cmdcall_no_output("resolveip -t 2 api.pengztech.com") ) {
		uci_get("router.system.domain", buffer, SIZE);
		sprintf(cmd, "resolveip -t 2 %s", buffer);
		getCmdResult(cmd, ip, SIZE);
		sprintf(cmd, "route add -net %s netmask 255.255.255.255 gw %s", ip, gateway);
		cmdcall_no_output(cmd);

		write_file_to_tmp("/tmp/tmp_gateway", gateway, strlen(gateway));
		write_file_to_tmp("/tmp/tmp_api", ip, strlen(ip));
    } else {
        RETURN_MSG(2, "can't connect api.pengztech.com");
        free(gateway);
        return 2;
    }

	free(gateway);

	return 0;
}


void write_file_to_tmp(const char *path, const char *content, int len)
{
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd == -1) {
		LOG("open %s error", path);
	}
	if (write(fd, content, len) != len) {
		LOG("couldn't write whole buffer to %s", path);
	}
	if (close(fd) == -1) {
		LOG("close fd fail");
	}
}

