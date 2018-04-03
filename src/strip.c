#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "strip.h"
#include "log.h"

// #define LOGPATH "strip.log"

void strip(const char *strIn, char *strOut, int len)				//strip '' and '\n' and '\r' and '\t'
{
    assert(strIn && strOut);

    int i, j ;

    i = 0;

    j = strlen(strIn) - 1;

    while(isspace(strIn[i]))
        ++i;

    while(isspace(strIn[j]) && i<j)
        --j;

	if(i > j)  
		strOut[0] = '\0';
	else
	{
		int stroutlen = j - i + 1;
		if (len < stroutlen + 1) {
			LOG("strip overflower");
		}

		// strncpy(strOut, strIn + i , stroutlen);
		int k;
		for (k = 0; k < stroutlen; k++)
		   strOut[k] = strIn[i+k];

		strOut[stroutlen] = '\0';
	}
}


char *safe_trip(const char *src)
{
	if (src == NULL)
	{
		LOG("Point is NULL");
	}

	char buffer[1024];
	char buffer2[1024];
	strip(src, buffer, sizeof(buffer));
	if (*buffer == '\0')
	{
		LOG("key is null");
	}

	char *p;
	int len = 0;
	for (p=buffer; *p != '\0'; p++) {

		if (isspace(*p) || *p == '\'' || *p == '\\') {
			LOG("key is invalid");

		} else if ( *p == '\"') {
			buffer2[len++] = '\\';
			buffer2[len++] = *p;

		} else {
			buffer2[len++] = *p;
		}
	}
	buffer2[len] = '\0';
	char *dest = (char*)malloc(strlen(buffer2)+1);
	strcpy(dest, buffer2);
	return dest;
}

char* StringJoin(const char* first,const char* last)
{
	 if (first == NULL  || last == NULL)
	 {
	 	LOG("point is null");
	 }
	 char* result;
	 int len = strlen(first)+strlen(last)+1;
	 result = (char*)malloc(len*sizeof(char));
	 memset(result,0,len*sizeof(char));
	 strcpy(result,first);
	 strcat(result,last);
	 return result;
}

