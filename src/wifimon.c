#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "log.h"
#include "getwlanname.h"
#include "wifitype.h"
#include "getiponline.h"


// #define LOGPATH "wifimon.log"


int main(int argc, char const *argv[])
{
	fclose(stdout);

	char* wlanstr = getwlanname();

	while(isDevHasIp(wlanstr))
	{
		printf("is online\n");
		sleep(15);
	}
	free(wlanstr);

	DIAL_T dial_argv;
	dial_argv.bssid = "new";
	int res = dial_main(1, &dial_argv);

	return res;
}



