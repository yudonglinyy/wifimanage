#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "log.h"
#include "strip.h"
#include "cmdcall.h"

// #define LOGPATH "cmdcall.log"


char* getCmdResult(const char *cmd, char *resultStr, int len)
{
   if (!cmd) {
      LOG("cmd is NULL");
   }
   
   if (!resultStr) {
      LOG("resultStr is NULL");
   }

   int SIZE = 4096;
   char *buffer = NULL;
   
   if (SIZE < len)
   {
      if ( (buffer = (char*)realloc(buffer, SIZE = 2 * len)) == NULL)
         LOG("realloc fail");
   }
   else
   {
      if ( (buffer = (char*)malloc(SIZE)) == NULL)
         LOG("malloc fail");
   }
   memset(resultStr, 0, len);
   FILE *pResult = popen(cmd, "r");
   if(pResult == NULL)
   {
      LOG("pResult is null [%s]", cmd);
   }
   memset(buffer, 0, SIZE);
   if ( 0 == fread(buffer, 1, SIZE, pResult)) {
      LOG("get result is null:[%s]", cmd);
   }
   strip(buffer, resultStr, SIZE);
   if (len < (int)strlen(resultStr)+1)   //protect not to overflow
      LOG("result overflow [%s] %d < %d", cmd, len, (int)strlen(resultStr)+1);
   pclose(pResult);
   free(buffer);
   return resultStr;
}


int cmdcall_no_output(const char *str)
{
   fflush(stdout);

   /*hide stdout*/
   int bak_out_fd = dup(STDOUT_FILENO);
   int bak_err_fd = dup(STDERR_FILENO);
   close(STDOUT_FILENO);
   close(STDERR_FILENO);

   /*run cmd*/
   int cmdcall_res = system(str);

   /*resume stdout*/
   dup2(bak_out_fd, STDOUT_FILENO);
   dup2(bak_err_fd, STDERR_FILENO);

   close(bak_out_fd);
   close(bak_err_fd);
   
   return cmdcall_res;
}

/*if result is null, will return NULL*/
char* getCmdResult_r(const char *cmd, char *resultStr, int len)
{
   if (!cmd) {
      LOG("cmd is NULL");
   }

   if (!resultStr) {
      LOG("resultStr is NULL");
   }
   
   int SIZE = 4096;
   char *buffer = NULL;
   
   if (SIZE < len)
   {
      if ( (buffer = (char*)realloc(buffer, SIZE = 2 * len)) == NULL)
         LOG("realloc fail");
   }
   else
   {
      if ( (buffer = (char*)malloc(SIZE)) == NULL)
         LOG("malloc fail");
   }
   memset(resultStr, 0, len);
   FILE *pResult = popen(cmd, "r");
   if(pResult == NULL)
   {
      LOG("%s", cmd);
   }
   memset(buffer, 0, SIZE);
   if ( 0 == fread(buffer, 1, SIZE, pResult)) {
      resultStr = NULL;
   }
   else {
      strip(buffer, resultStr, SIZE);
      if (len < (int)strlen(resultStr)+1)   //protect not to overflow
         LOG("%s", cmd);
   }
   
   pclose(pResult);
   free(buffer);
   return resultStr;
}


// void* json_argv(cJSON *json, const char *key, const char* valuetype)
// {
//    assert(json != NULL && key != NULL && valuetype != NULL);

//    cJSON *obj;

//    if ((obj=cJSON_GetObjectItem(json, key)) == NULL) {
//       LOG("%s is unknow", key);
//    }

//    if (!strcmp(valuetype, "string")) {
//       return safe_trip(obj->valuestring); //malloc a value and return the poite
//    } else {
//       return obj->valueint;
//    }
// }