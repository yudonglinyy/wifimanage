#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"
#include "strip.h"
#include "cmdcall.h"
#include "getgateway.h"

// #define LOGPATH "checkgateway.log"
#define SIZE 1024


int main(void)
{
    char *gateway = getgateway();

    if (gateway) {
        int fd = open("/tmp/tmp_gateway", O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd == -1) {
            LOG("open tmp_gateway error");
        }
        if (write(fd, gateway, strlen(gateway)) != strlen(gateway)) {
            LOG("couldn't write whole buffer");
        }
        if (close(fd) == -1) {
            LOG("close fd fail");
        }
    }

    free(gateway);
    return 0;
}

