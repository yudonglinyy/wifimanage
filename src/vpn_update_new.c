#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <getopt.h>
#include "log.h"
#include "base64.h"
#include "curl/curl.h"
#include "ucicmd.h"
#include "curlhttp.h"
#include "cmdcall.h"
#include "f_operation.h"
#include "strip.h"
#include "jsonvalue.h"
#include "vpn_update_new.h"
#include "nettest.h"

#define MAXSIZE 4096
#define SIZE 1024
#define MINSIZE 256

int hour;
int num;
int type;
char action[MINSIZE];
char action_get_vpnlist[MINSIZE];
char url[MINSIZE]="http://ip/UpdateAPI/UpdateAPI.php";
const char *action_updata_ip = "UpdateDdnsIP";
const char *filename = "/tmp/response_data";


void display_usage()
{
    fprintf(stderr, "usage: update_vpn [option]\n"
                    "option:\n"    
                    "\t -H, --hour: \n"
                    "\t -n, --num: \n"
                    "\t -u, --url: \n"
                    "\t -a, --action: 1:UsingVpnList 2:AllVpnList 3:DdnsList\n"
                    "\t -h, --help\n");
    exit(EXIT_FAILURE);
}


int main(int argc, char* const *argv)
{
    int ch;
    int option_index = 0;
    int hour = 1;

    if (argc <= 7 && argc != 1) {
        display_usage();
    }

    static struct option long_options[] =
    {  
        {"hour", required_argument, NULL, 'H'},
        {"num", required_argument, NULL, 'n'},
        {"action", required_argument, NULL, 'a'},
        {"url", required_argument, NULL,'u'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0},
    }; 

    while ((ch = getopt_long(argc, argv, "H:a:n:h:u:", long_options, &option_index )) != -1)
    {
        switch (ch) {
        case 'H':
            hour = atoi(optarg);
            break;
        case 'n':
            num = atoi(optarg);
            break;
        case 'a':
            type = atoi(optarg);
            break;
        case 'u':
            strcpy(url, optarg);
            break;
        case '?':
            printf("无效的选项字符 \' %c \'\n", (char)ch);
            exit(EXIT_FAILURE);
        case ':':
            printf("缺少选项参数！\n");
            exit(EXIT_FAILURE);
        case 'h':
        default:
            display_usage();
        }
    }
    
    while(1) {     
        vpn_update();
        sleep(hour * 60);  
    }
    
    return 0;
}

int vpn_update()
{
    memset(action, 0, MINSIZE);

    if (type == UsingVpnList) {      
        strcpy(action_get_vpnlist, "UsingVpnList"); 
        strcpy(action, "UpdateVpn");    
    } else if (type == AllVpnList) {
        strcpy(action_get_vpnlist, "AllVpnList"); 
        strcpy(action, "UpdateVpn");
    } else if (type == DdnsList){
        strcpy(action_get_vpnlist, "DdnsList");
        strcpy(action, "UpdateDdnsStatus"); 
    } else {
        LOG("request type is wrong");
    }
    
    update_from_type(type);

    return 0;
}


int update_from_type(int type)
{
    int count;
    int updateipflag;
    char buf[SIZE], response[SIZE];
    char item_post_data[SIZE];

     /*{Hour,Num}*/
    sprintf(item_post_data, "{\"Hour\": \"%d\",\"Num\": \"%d\"}", hour, num);
    action_base64_curlhttp(url, action_get_vpnlist, item_post_data);
    
    /*read respone from a file */
    filecontent(filename, response, SIZE);
    if (strlen(response) == 0) {
        LOG("response file is none, curl %s {hour, num} fail", url);
    }

    base64_decode(response, (u_char *)buf);
    strip(buf, buf, SIZE);


    if (type == 1 || type == 2) {
        VPNLIST *p = parse_vpn_data(buf, &count);
        
        //ping or do something
        for(int i=0; i < count; i++, p++) {
            ping_vpn(p);
            set_post_data_vpn(p, item_post_data, SIZE);
            action_base64_curlhttp(url, action, item_post_data);
        }

        free_vpnlist(p, count);
        p = NULL;
    } else if (type == 3){
        DDNSLIST * p = parse_ddns_data(buf, &count);

        //ping or do something
        for(int i=0; i < count; i++, p++) {
            updateipflag = ping_ddns(p);
            if (updateipflag) {
                action_base64_curlhttp(url, action_updata_ip, item_post_data);
            }
            
            set_post_data_ddns(p, item_post_data, SIZE);
            action_base64_curlhttp(url, action, item_post_data);
        }

        free_ddnslist(p, count);
        p = NULL;
    } else {
        LOG("request type is wrong");
    }

    return 0;
}


void free_vpnlist(VPNLIST *plist, int count)
{
    VPNLIST *p = plist;
    for (int i=0; i<count; i++,p++) {
        free(p->mac);
        free(p->ip);
        free(p->username);
        free(p->password);
        free(p->dialmethod);
        free(p->speed);
        free(p->updatetimes);
        p->mac = NULL;
        p->ip = NULL;
        p->username = NULL;
        p->password = NULL;
        p->dialmethod = NULL;
        p->speed = NULL;
        p->updatetimes = NULL;
    }
    free(plist);
    return;
}


void free_ddnslist(DDNSLIST *plist, int count)
{
    DDNSLIST *p = plist;
    for (int i=0; i<count; i++,p++) {
        free(p->mac);
        free(p->ip);
        free(p->domain);
        free(p->status);
        p->mac = NULL;
        p->ip = NULL;
        p->domain = NULL;
        p->status = NULL;
    }
    free(plist);
    return;
}


VPNLIST * parse_vpn_data(char *buf, int *pcount)
{
    cJSON *json=NULL, *item=NULL;
    VPNLIST *p = NULL;

    if(!(json = cJSON_Parse(buf)))
        LOG("Error before:[%s]",cJSON_GetErrorPtr());

    *pcount = cJSON_GetArraySize(json);

    if ((p = (VPNLIST *)malloc(*pcount * sizeof(VPNLIST))) == NULL) {
        LOG("malloc fail");
    }
    for (int i=0; i< *pcount; i++) {
        item = item? item->next : json->child;
        p[i].mac = strdup(cJSON_value(item, "Mac", string));
        p[i].ip = strdup(cJSON_value(item, "IP", string));
        p[i].username = strdup(cJSON_value(item, "Username", string));
        p[i].password = strdup(cJSON_value(item, "Password", string));
        p[i].dialmethod = strdup(cJSON_value(item, "DialMethod", string));
        p[i].updatetimes = strdup(cJSON_value(item, "UpdateTimes", string));
    }

    cJSON_Delete(json);
    return p;
}


DDNSLIST * parse_ddns_data(char *buf, int *pcount)
{
    /*  
    [{"Mac":"00:d0:41:d0:b9:db","IP":"111.255.33.49","Domain":"tw107080.dynamic-dns.net","Status":"Normal"},{"Mac":"00:1E:6E:01:33:B9","IP":"14.161.44.179","Domain":"thientricamera.dyndns.org","Status":"Normal"}]
    */
    cJSON *json=NULL, *item=NULL;
    DDNSLIST * p = NULL;

    if(!(json = cJSON_Parse(buf)))
        LOG("Error before:[%s]",cJSON_GetErrorPtr());

    *pcount = cJSON_GetArraySize(json);

    if ((p = (DDNSLIST *)malloc(*pcount * sizeof(DDNSLIST))) == NULL) {
        LOG("malloc fail");
    }
    for (int i=0; i<*pcount; i++) {
        item = (item==NULL ? json->child : item->next);
        p[i].mac = strdup(cJSON_value(item, "Mac", string));
        p[i].ip = strdup(cJSON_value(item, "IP", string));
        p[i].domain = strdup(cJSON_value(item, "Domain", string));
        p[i].status = strdup(cJSON_value(item, "Status", string));
    }

    cJSON_Delete(json);
    return p;
}


int ping_vpn(VPNLIST *p)
{
    char cmd[SIZE];
    char buf[SIZE];

    char speed[MINSIZE], updatetimes[MINSIZE];
    int updatetimesnum = atoi(p->updatetimes);
    memset(buf, 0, sizeof(buf));

    sprintf(cmd, "ping -c 4  -w 15 %s 2>&1 | tail -n 1  | awk '{print $4}' | awk -F '/' '{print $2}'", p->ip);
    getCmdResult(cmd, buf, SIZE);
    if (strlen(buf) == 0) {
        strcpy(speed, "Disconnect");
        updatetimesnum++;
        sprintf(updatetimes, "%d", updatetimesnum);
    } else if (strlen(buf) < MINSIZE){
        sprintf(speed, "%sms", buf);
        updatetimesnum = 0;
        sprintf(updatetimes, "%d", updatetimesnum);     
    } else {
        LOG("speed[] len is too long");
    }

    free(p->speed);
    free(p->updatetimes);
    p->speed = strdup(speed);
    p->updatetimes = strdup(updatetimes);

    return 0;
}


int ping_ddns(DDNSLIST *p)
{
    char buf[SIZE], cmd[SIZE];
    char domain[MINSIZE];
    char status[MINSIZE];
    char ip[MINSIZE];
    
    int i;
    char *delim = " ";
    char *pbuf, *temp;
    char *olddomain = p->domain;

    sprintf(cmd, "ping -c 4  -w 15 %s 2>&1 | head -n 1", olddomain);
    getCmdResult(cmd, buf, SIZE);
    if (strstr( buf, "unknow")) {
        strcpy(status, "Obsolete");
    } else if (strstr(buf, "bytes of data")) {
        /* spaced as split, the 2 part is domain, the 3 part is ip */
        for (i=0,temp=buf; i<2; i++,temp=NULL) {  
           pbuf = strtok(temp, delim); 
        }
        strcpy(domain, pbuf);
    } else {
        LOG("ping ddns error");
    }

    /*new ip*/
    pbuf = strtok(NULL, delim);  

    /* domain != olddomain*/
    if (strcmp(domain, olddomain) || !strchr(pbuf, '.')) {
        strcpy(status, "Abnormal");
    }


    strncpy(ip, pbuf+1, strlen(pbuf)-2);
    strcpy(status, "Normal");

    int updateipflag = test_conn_vpn_server(domain);
        
    free(p->ip);
    free(p->status);
    free(p->domain);
    p->ip = strdup(ip);
    p->status = strdup(status);
    p->domain = strdup(domain);
    return updateipflag;
}


int action_base64_curlhttp(const char *url, const char *action, const char *post_data)
{
    char action_encrt[MINSIZE], post_data_encrt[SIZE], all_post_data[SIZE];

    base64_encode(action_encrt, (u_char *)action, strlen(action));
    base64_encode(post_data_encrt, (u_char *)post_data, strlen(post_data));
    sprintf(all_post_data, "Action=%s&Data=%s", action_encrt, post_data_encrt);
    if (curlhttp(url, all_post_data, "POST", filename)) {
        exit(1);
    }
    return 0;
}


void set_post_data_vpn(VPNLIST *p, char *item_post_data, int len)
{
    /*{Mac,Username,Password,Speed,UpdateTimes}*/
    memset(item_post_data, 0, len);
    sprintf(item_post_data, "{\"Mac\":\"%s\",\"Username\":\"%s\",\"Password\":\"%s\",\"Speed\":\"%s\",\"UpdateTimes\":\"%s\"}", p->mac, p->username, p->password, p->speed, p->updatetimes);
}


void set_post_data_ddns(DDNSLIST *p, char *item_post_data, int len)
{
    /*{Domain,Status}*/
    memset(item_post_data, 0, len);
    sprintf(item_post_data, "{\"Domain\":\"%s\",\"Status\":\"%s\"}", p->domain, p->status);

}


void set_post_data_ddns_ip(DDNSLIST *p, char *item_post_data, int len)
{
    /*{Mac,Domain,IP}*/
    memset(item_post_data, 0, len);
    sprintf(item_post_data, "{\"Mac\":\"%s\",\"Domain\":\"%s\",\"IP\":\"%s\"}", p->mac, p->domain, p->ip);
}

