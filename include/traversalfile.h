#ifndef _TRAVERSAL_FILE_H
#define _TRAVERSAL_FILE_H

#include <dirent.h>
#include "list.h"

list_t *traversal_file(const char *path);
int filter(const struct dirent *file);

#endif