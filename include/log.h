#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#define LOGPATH "/tmp/wifimanage.log"

#define RETURN_MSG(msg_num, msg, ...)  do {										\
	char msg_1[1024];															\
	cJSON *log_msg_json = cJSON_CreateObject();									\
	cJSON_AddItemToObject(log_msg_json,"status",cJSON_CreateNumber(msg_num));	\
	sprintf(msg_1, msg, ## __VA_ARGS__);										\
	cJSON_AddItemToObject(log_msg_json, "msg", cJSON_CreateString(msg_1));		\
	char *log_msg_str = cJSON_PrintUnformatted(log_msg_json);					\
	printf("%s", log_msg_str);													\
	cJSON_Delete(log_msg_json);													\
	free(log_msg_str);															\
	log_msg_json = NULL;														\
	log_msg_str = NULL;															\
} while (0)


#define RETURN_JSON(msg_num, log_msg_subobj)  do {														\
	cJSON *log_msg_json = cJSON_CreateObject();															\
	cJSON_AddItemToObject(log_msg_json,"status",cJSON_CreateNumber(msg_num));							\
	cJSON_AddItemToObject(log_msg_json, "msg", log_msg_subobj);											\
	char *log_msg_str = cJSON_PrintUnformatted(log_msg_json);											\
	printf("%s", log_msg_str);																			\
	cJSON_Delete(log_msg_json);																			\
	free(log_msg_str);																					\
	log_msg_json = NULL;																				\
	log_msg_str = NULL;																					\
} while (0)


#define LOGTOFILE(plogtimep, err, ...)	do{																\
	fprintf(fplog, "[%s: %d]%s\t", __FILE__, __LINE__, __func__);										\
	fprintf(fplog, err, ## __VA_ARGS__);																\
	fprintf(fplog, "\t(%s)\t%s", strerror(errno), ctime(plogtimep));									\
	fclose(fplog);																						\
}while (0)


#ifdef	DEBUG
#define LOGTOSTDIN(plogtimep, err, ...) do {															\
	printf("\n[%s: %d]%s\t", __FILE__, __LINE__, __func__); 											\
	printf(err, ## __VA_ARGS__);																		\
	printf("\t(%s)\t%s", strerror(errno), ctime(plogtimep));											\
}while (0)
#else
#define LOGTOSTDIN(plogtimep, err, ...)
#endif


#define LOG(err, ...) do{																				\
	FILE *fplog = fopen(LOGPATH,"a");																	\
	if (!fplog) perror(LOGPATH" isn't exist");															\
	time_t logtimep = time(NULL);																		\
	RETURN_MSG(1, err, ## __VA_ARGS__);																	\
	LOGTOFILE(&logtimep, err, ## __VA_ARGS__);															\
	LOGTOSTDIN(&logtimep, err, ## __VA_ARGS__);															\
	exit(1);																							\
}while (0)


#define LOGNUM(msg_num, err, ...) do{																	\
	FILE *fplog = fopen(LOGPATH,"a");																	\
	if (!fplog) perror(LOGPATH" isn't exist");															\
	time_t logtimep = time(NULL);																		\
	RETURN_MSG(msg_num, err, ## __VA_ARGS__);															\
	LOGTOFILE(&logtimep, err, ## __VA_ARGS__);															\
	LOGTOSTDIN(&logtimep, err, ## __VA_ARGS__);															\
	exit(1);																							\
}while (0)

#endif
