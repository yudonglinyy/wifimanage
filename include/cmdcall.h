#ifndef _CMDCALL_H
#define _CMDCALL_H 


char* getCmdResult(const char *cmd, char *resultStr, int len);
int cmdcall_no_output(const char *str);
char* getCmdResult_r(const char *cmd, char *resultStr, int len);
// char* json_argv(cJSON *json, const char *key, const char* valuetype);

#endif