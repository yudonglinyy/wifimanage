#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "traversalfile.h"
#include "log.h"

// #define LOGPATH "traversal_file.log"


list_t *traversal_file(const char *path)
{
	if(path? access(path, F_OK): 1)
	{
		LOG("%s is fail access", path);
	}

	list_t *plist = list_new();
	plist->free = free;
	// DIR *pDir;
	// struct dirent *dp;

	// pDir = opendir(path);
	// if (pDir == NULL)
	// 	LOG("opendir failed");

	while(1)
	{
	// 	errno = 0;
	// 	dp = readdir(pDir);
	// 	if (dp == NULL)
	// 		break;

	// 	if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
	// 		continue;
		
	// 	char *namestr = (char*)malloc(strlen(dp->d_name)+1);
	// 	strcpy(namestr, dp->d_name);
	// 	list_rpush(plist, list_node_new(namestr));
	// }

	// if (errno != 0)
	// 	LOG("readdir");

	// if (closedir(pDir) == -1)
	// 	LOG("closedir");


		struct dirent **namelist;
		int n = scandir(path, &namelist, filter, alphasort);
		if (n < 0)
			LOG("readdir error");
		else {
			for (int i=0; i<n; i++) {
				char *namestr = strdup(namelist[i]->d_name);
				list_rpush(plist, list_node_new(namestr));
				free(namelist[i]);
			}
			free(namelist);
			break;
		}
	}
	return plist;
}

int filter(const struct dirent *file)
{
	//string which startwith '.' will filter, if want to keep the file return 1
	if ( '.' == file->d_name[0])  
		return 0;
	else
		return 1;
}


