#include <stdio.h>
#include "log.h"
#include "strip.h"
#include "wifitype.h"
#include "jsonvalue.h"

/* {"ssid": "hi_man", "device": "radio0", "bssid": "30:fc:68:59:42:78", "encryption": "psk", "key": "hi_mandemimashiduoshao?", "action": "conn"} */
int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		LOG("parameter error");
	}

	cJSON *json;
	int count;
	WIFIEDIT_T argv_t;
	memset(&argv_t, 0, sizeof(argv_t));


	if(!(json = cJSON_Parse(argv[1])))
		LOG("Error before:[%s]",cJSON_GetErrorPtr());

	count = cJSON_GetArraySize(json);

	if(count != 6 && count != 2 )
	{
		RETURN_MSG(1, "Error.\nUsage:\n $1  action\n $2  device\n $3  ssid\n $4  bssid\n $5  encryption\n $6  key\n");
		return 255;
	}


	switch(count)
	{
		case 6:
			argv_t.ssid = safe_trip(cJSON_value(json, "ssid", string));
			argv_t.device = safe_trip(cJSON_value(json, "device", string));
			argv_t.encryption = safe_trip(cJSON_value(json, "encryption", string));
			argv_t.key = cJSON_value(json, "key", string);
		case 2:
			argv_t.action = safe_trip(cJSON_value(json, "action", string));
			argv_t.bssid = safe_trip(cJSON_value(json, "bssid", string));
	}

	wifiedit_main(count, &argv_t);

	free(argv_t.ssid);
	free(argv_t.device);
	free(argv_t.action);
	free(argv_t.bssid);
	free(argv_t.encryption);

	cJSON_Delete(json);

	return 0;
}
