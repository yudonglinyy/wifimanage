#include <stdio.h>
#include "log.h"
#include "strip.h"
#include "wifitype.h"
#include "jsonvalue.h"

/* {\"bssid\": \"30:fc:68:59:42:78\"} */
int main(int argc, char const *argv[])
{
	int count;
	cJSON *json=NULL;
	DIAL_T argv_t;
	memset(&argv_t, 0, sizeof(argv_t));

	/*dial all bssid*/
	if (argc == 1) {
		dial_main(0, NULL);	
	}
	/* dial one bssid*/
	else {
		if(!(json = cJSON_Parse(argv[1])))
			LOG("Error before:[%s]",cJSON_GetErrorPtr());

	    count = cJSON_GetArraySize(json);

		switch(count) {
		case 1:
			argv_t.bssid = safe_trip(cJSON_value(json, "bssid", string));
			break;
		}

		

		dial_main(count, &argv_t);

		free(argv_t.bssid);
		cJSON_Delete(json);
	}

	return 0;
}