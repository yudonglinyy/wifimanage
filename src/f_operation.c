#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "log.h"
#include "f_operation.h"

int copyfile(const char *src, const char *des)
{
	char buffer[1024];
	int len;
	int fpsrc, fpdes;

	if (src == NULL || des == NULL) {
		return -1;
	}
	if ((fpsrc = open(src, O_RDONLY | O_CREAT, 0777)) == -1) {
		return -1;
	}

	if ((fpdes = open(des, O_WRONLY | O_CREAT | O_TRUNC, 0777)) == -1) {
		return -1;
	}

	while((len = read(fpsrc, buffer, sizeof(buffer))) > 0) {  
        if (write(fpdes, buffer ,len) != len) {
        	return -1;
        }  
    }

    if (len == -1) {
    	return -1;
    }

    if (close(fpsrc) == -1) {
    	return -1;
    }
    if (close(fpdes) == -1) {
    	return -1;
    }

    return 0;
}


int filecontent(const char *path, char *buf, int size)
{
	int len;
	int fp;

	if (path == NULL || buf == NULL) {
		return -1;
	}
	if ((fp = open(path, O_RDONLY)) == -1) {
		return -1;
	}

	memset(buf, 0, size);

	if ((len = read(fp, buf, size))< 0) { 
    	LOG("read fail");
    }

    if (len == size) {
    	buf[size-1] = '\0';
    }
    
    if (close(fp) == -1) {
		LOG("close fd fail");
	}
    
    return 0;
}

/*count lines of the file*/
u_long file_wc(const char *file)  
{ 
	u_long lines = 0;  
	FILE *fp;
	char buf[4096];

	if (!file) {
		LOG("point is null");
	}

	if (access(file, F_OK)) {
		LOG("%s isn't access", file);
	}

	if ((fp = fopen(file, "r")) < 0) {
		LOG("open %s file fail", file);
	}

	while((fgets(buf, 4096, fp)) != NULL) {
		if(buf[strlen(buf)-1]=='\n') {
			lines++;
		}
	}
		
	fclose(fp);

	return lines;
}