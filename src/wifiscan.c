#include <stdio.h>
#include "log.h"
#include "wifitype.h"
#include "strip.h"
#include "jsonvalue.h"

/* {"device": "wlan0"} */
int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		LOG("parameter error");
	}

	cJSON *json=NULL;
	int count;
	WIFISCAN_T argv_t;
	memset(&argv_t, 0, sizeof(argv_t));

	if(!(json = cJSON_Parse(argv[1])))
		LOG("Error before:[%s]",cJSON_GetErrorPtr());
	
	count = cJSON_GetArraySize(json);

	if(count != 1)
	{
		RETURN_MSG(1, "Error.\nUsage: $1 <dev>\n");
		exit(-1);
	}

	argv_t.device = safe_trip(cJSON_value(json, "device", string));

	wifiscan_main(count, &argv_t);

	free(argv_t.device);
	cJSON_Delete(json);

	return 0;
}
