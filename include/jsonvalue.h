#ifndef _JSONVALUE_H
#define _JSONVALUE_H

#include "cJSON.h"

#define cJSON_value(obj, key, type) cJSON_##type##_value( cJSON_GetObjectItem((obj),(key)) )

int cJSON_int_value(cJSON *p);
char *cJSON_string_value(cJSON *p);
int cJSON_bool_value(cJSON *p);
cJSON * cJSON_array_value(cJSON *p);

#endif