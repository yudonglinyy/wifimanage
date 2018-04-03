#ifndef _F_OPERATION_H
#define _F_OPERATION_H

#include <sys/types.h>

int copyfile(const char *src, const char *des);
int filecontent(const char *path, char *buf, int size);
u_long file_wc(const char *file);

#endif