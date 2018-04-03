#include <stdio.h>
#include <stdarg.h>
#include "log.h"
#include "ucicmd.h"

#define SIZE 1024


int uci_get(const char *cmd, char *result, int len, ...)
{
	if (NULL == cmd) {
		LOG("point is null");
	}

	char str[SIZE], str2[SIZE];
	va_list ap;

    va_start(ap, len);
	vsnprintf (str, SIZE, cmd, ap);
    va_end(ap);

    strcpy(str2, str);
	struct uci_context * ctx = NULL;
	struct uci_ptr ptr;

	ctx = uci_alloc_context();

	if(UCI_OK != uci_lookup_ptr(ctx, &ptr, str, true))
	{
		LOG("[%s] no found", cmd);
		return -1;
	}

	if ( !(ptr.flags & UCI_LOOKUP_COMPLETE) ) {
		LOG("uci_get no found [%s]", str2);
	}

	if (!ptr.o) {
		LOG("[%s] no value", cmd);
	}

	char *buf = ptr.o->v.string;
	if (!buf && strlen(buf)) {
		LOG("result is null");
	}

	/*if don't get the result, result is null*/
	if (result) {
		memset(result, 0 , len);
		strncpy(result, buf, strlen(buf)<len? strlen(buf): len-1);
	}

	uci_free_context(ctx);

	return 0;
}

int uci_get_no_exit(const char *cmd, char *result, int len, ...)
{
	if (NULL == cmd) {
		LOG("point is null");
	}

	char str[SIZE];
	va_list ap;

    va_start(ap, len);
	vsnprintf (str, SIZE, cmd, ap);
    va_end(ap);

	struct uci_context * ctx = NULL;
	struct uci_ptr ptr;

	ctx = uci_alloc_context();

	if(UCI_OK != uci_lookup_ptr(ctx, &ptr, str, true))
	{
		return -1;
	}

	if ( !(ptr.flags & UCI_LOOKUP_COMPLETE) ) {
		return -1;
	}

	if (!ptr.o) {
		return -1;
	}

	char *buf = ptr.o->v.string;
	if (!buf && strlen(buf)) {
		return -1;
	}

	/*if don't get the result, result is null*/
	if (result) {
		memset(result, 0 , len);
		strncpy(result, buf, strlen(buf)<len? strlen(buf): len-1);
	}

	uci_free_context(ctx);

	return 0;
}


int uci_operation(char *str, UCI_OPERFUNC oper_func, const char *dir)
{
	struct uci_context * ctx = NULL;
	struct uci_ptr ptr;

	if (dir) {
		uci_set_confdir(ctx, dir);
	}
	
	ctx = uci_alloc_context();

	if(UCI_OK != uci_lookup_ptr(ctx, &ptr, str, true))
	{
		LOG("[%s] error", str);
	}

	oper_func(ctx, &ptr);
	uci_commit(ctx, &ptr.p, false);
	uci_free_context(ctx);

	return 0;
}


int uci_add(const char *cmd, ...)
{
	if (NULL == cmd) {
		LOG("point is null");
	}

	char str[SIZE];
	va_list ap;

    va_start(ap, cmd);
	vsnprintf (str, SIZE, cmd, ap);
    va_end(ap);

	return uci_operation(str, uci_set, NULL);
}


int uci_add_confdir(const char *dir, const char *cmd, ...)
{
	if (NULL == cmd) {
		LOG("point is null");
	}

	char str[SIZE];
	va_list ap;

    va_start(ap, cmd);
	vsnprintf (str, SIZE, cmd, ap);
    va_end(ap);

	return uci_operation(str, uci_set, dir);
}


int uci_del(const char *cmd, ...)
{
	if (NULL == cmd) {
		LOG("point is null");
	}

	char str[SIZE];
	va_list ap;

    va_start(ap, cmd);
	vsnprintf (str, SIZE, cmd, ap);
    va_end(ap);

	return uci_operation(str, uci_delete, NULL);
}


int uci_del_confdir(const char *dir, const char *cmd, ...)
{
	if (NULL == cmd) {
		LOG("point is null");
	}

	char str[SIZE];
	va_list ap;

    va_start(ap, cmd);
	vsnprintf (str, SIZE, cmd, ap);
    va_end(ap);

	return uci_operation(str, uci_delete, dir);
}


int uci_sim_add_list(const char *cmd, ...)
{
	if (NULL == cmd) {
		LOG("point is null");
	}

	char str[SIZE];
	va_list ap;

    va_start(ap, cmd);
	vsnprintf (str, SIZE, cmd, ap);
    va_end(ap);

	return uci_operation(str, uci_add_list, NULL);
}


int uci_sim_del_list(const char *cmd, ...)
{
	if (NULL == cmd) {
		LOG("point is null");
	}

	char str[SIZE];
	va_list ap;

    va_start(ap, cmd);
	vsnprintf (str, SIZE, cmd, ap);
    va_end(ap);

	return uci_operation(str, uci_del_list, NULL);
}


int uci_sim_rename(const char *cmd, ...)
{
	if (NULL == cmd) {
		LOG("point is null");
	}

	char str[SIZE];
	va_list ap;

    va_start(ap, cmd);
	vsnprintf (str, SIZE, cmd, ap);
    va_end(ap);

	return uci_operation(str, uci_rename, NULL);
}
