#include "log.h"
#include "jsonvalue.h"


int cJSON_int_value(cJSON *p)
{
	if (!p) {
	    LOG("json lack of some keys");
	}

	if (!cJSON_IsNumber(p)) {
	    LOG("the value of \"%s\" isn't number", p->string);
	}

	return p->valueint;
}


char *cJSON_string_value(cJSON *p)
{
	if (!p) {
	    LOG("the key of json is wrong");
	}

	if (!cJSON_IsString(p)) {
	    LOG("the value of \"%s\" isn't string", p->string);
	}

	return p->valuestring;
}


int cJSON_bool_value(cJSON *p)
{
	if (!p) {
	    LOG("the key of json is wrong");
	}

	if (!cJSON_IsBool(p)) {
	    LOG("the value of \"%d\" isn't bool", p->valueint);
	}

	return p->valueint;
}


cJSON *cJSON_array_value(cJSON *p)
{
	if (!p) {
	    LOG("the key of json is wrong");
	}

	if (!cJSON_IsArray(p)) {
	    LOG("the value of \"%s\" isn't array", p->string);
	}

	return p->child;
}