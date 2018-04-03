#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <dirent.h>

#include "log.h"
#include "cmdcall.h"
#include "strip.h"
#include "wifitype.h"
#include "ucicmd.h"
#include "config_editor_main.h"
#include "jsonvalue.h"
#include "getgateway.h"
#include "getiponline.h"
#include "f_operation.h"
#include "getstatus_main.h"

#define SIZE 1024
#define MIXSIZE 256
#define IPSIZE 16
#define MACSIZE 18

typedef enum network_type {wan, lan, ap, client, settings_3g, vpn, vpn_setting, systeminfo, wifilog, honeypot_log} NETWORK_T;


static NETWORK_T getstatus_network_type(const char *network)
{
    NETWORK_T type;

    if (!strcmp(network, "wan")) {
        type = wan;
    }
    else if (!strcmp(network, "lan")) {
        type = lan;
    }
    else if (!strcmp(network, "ap")) {
        type = ap;
    }
    else if (!strcmp(network, "client")) {
        type = client;
    }
    else if (!strcmp(network, "settings_3g")) {
        type = settings_3g;
    }
    else if (!strcmp(network, "vpn")) {
        type = vpn;
    }
    else if (!strcmp(network, "vpn_setting")) {
        type = vpn_setting;
    }
    else if (!strcmp(network, "systeminfo")) {
        type = systeminfo;
    }
    else if (!strcmp(network, "wifilog")) {
        type = wifilog;
    }
    else if (!strcmp(network, "honeypot_log")) {
        type = honeypot_log;
    }
    else {
        LOG("network argv fail");
    }

    return type;
}


static int filter(const struct dirent *file)
{
    if (strstr("ttyUSB", file->d_name) == file->d_name)
        return 1;
    else
        return 0;
}


static inline int test_conn(const char *ip)
{
    char cmd[50];

    sprintf(cmd, "ping -c 1 -W 2 %s", ip);

    return (! cmdcall_no_output(cmd));
}


static int testip_check(const char *check_ip)
{
    char buf[50], cmd[SIZE];
    char gateway[16];

    const char *gatewaypath = "/tmp/tmp_gateway";
    

    getCmdResult("route -n", buf, SIZE);
    if (!strstr(buf, check_ip)) {
        if (filecontent(gatewaypath, gateway, sizeof(gateway))) {
            LOG("%s isn't exist", gatewaypath);
        }
        sprintf(cmd, "route add -net %s netmask 255.255.255.255 gateway %s", check_ip, gateway);
        cmdcall_no_output(cmd);
    }

    return test_conn(check_ip);
}


static int myawk(char *buf, const char *delim, char **res, int arraysize)
{
    int i;
    char *str1, *token=NULL;

    for(i=0, str1=buf; i<arraysize; str1=NULL) {
        token = strtok(str1, delim);
        if (token == NULL)
           break;

        res[i++] = token;
    }

    return i;
}


/* {"network": "wan"} */
static void wanopt_fun(int count, cJSON *opt)
{
    char buf[SIZE], wanstat[SIZE], info[SIZE], dev[SIZE];
    char check_ip[16];
    char ip[16], mask[16];
    char pingmsg[256];
    char *gateway = NULL, *ssid=NULL;
    struct dirent **namelist;
    int status;

    if (count != 1) {
        LOG("the count of keys is wrong");
    }

    uci_get("router.system.check_ip", check_ip, sizeof(check_ip));
    uci_get("router.system.wan_type", wanstat, SIZE);

    /*static*/
    if (!strcmp(wanstat, "static")) {
        
        if (isDevUp("eth0")) {
            get_ip("eth0", ip);
            get_ip_netmask("eth0", mask);
            gateway = getgateway();
        } else {
            sprintf(info, "%s ----It seems you havenot connect the cable!", wanstat);
        }
    } 
    /*dhcp*/
    else if (!strcmp(wanstat, "dhcp")) {
        if (isDevUp("eth0")) {
            get_ip("eth0", ip);
            get_ip_netmask("eth0", mask);
            gateway = getgateway();
        } else {
            sprintf(info, "%s ----It seems you havenot connect the cable!", wanstat);
        }
    }
    /*3g*/
    else if (!strcmp(wanstat, "3g")) {
        uci_get("router.use.devname", dev, SIZE);
        if (strcmp(wanstat, "null") ||  0 < scandir("/dev", &namelist, filter, alphasort)) {
            sprintf(info, "It seems router cannot connect the 3G/4G Device!");  
        } else {
            if (isDevHasIp("3g-wan")) {
                get_ip("3g-wan", ip);
                get_ip_netmask("3g-wan", mask);
                gateway = getgateway();
            } else if (isDevHasIp(dev)) {
                get_ip(dev, ip);
                get_ip_netmask(dev, mask);
                gateway = getgateway();
            } else {
                sprintf(info, "%s----Connecting...", wanstat);
            }
        }
    }/*wifi*/
    else if (!strcmp(wanstat, "wifi")) {
        char path[MIXSIZE], devicepath[MIXSIZE];
        char ssid[MIXSIZE];

        uci_get("router.wifi.sta", dev, sizeof(dev));
        uci_get("wireless.%s.path", path, SIZE, dev);
        sprintf(devicepath, "/sys/devices/%s/net", path);
        if (access(devicepath, F_OK)) {
            LOG("dev %s isn't exist", devicepath);
        }
        sprintf(buf, "ls %s| head -n 1", devicepath);
        getCmdResult(buf, dev, MIXSIZE);

        if (!uci_get_no_exit("wireless.wificlient_chocobo.ssid", ssid, MIXSIZE)) {
            sprintf(info, "WiFi----%s", ssid);
            get_ip(dev, ip);
            get_ip_netmask(dev, mask);
            gateway = getgateway();
        } else {
            strcpy(info, "WiFi----not connected");
        }
    }
    else {
        LOG("%s from 'router.system.wan_type' is unknow", wanstat);
    }

    if (testip_check(check_ip)) {
        status = 0;
        sprintf(pingmsg, "ping %s successful", check_ip);
    } else {
        status = 1;
        sprintf(pingmsg, "ping %s failed", check_ip);
    }
    
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "info", info);
    cJSON_AddStringToObject(json, "ip", ip);
    cJSON_AddStringToObject(json, "mask", mask);
    cJSON_AddStringToObject(json, "gateway", gateway);
    cJSON_AddStringToObject(json, "ssid", ssid);
    cJSON_AddStringToObject(json, "pingmsg", pingmsg);

    RETURN_JSON(status, json);
    return;
}


/* {"network": "lan"} */
static void lanopt_fun(int count, cJSON *opt)
{
    char ip[16], mask[16];
    
    uci_get("network.lan.ipaddr", ip, sizeof(ip));
    uci_get("network.lan.netmask", mask, sizeof(mask));
    RETURN_MSG(0, "%s|%s", ip, mask);

    return;
}


/* {"network": "ap", "action": "XX", "device": "XX"} */
static void apopt_fun(int count, cJSON *opt)
{
    char cmd[SIZE];
    char wlan[100], path[MIXSIZE], devicepath[MIXSIZE];
    char *device = NULL, *action = NULL;

    if (count != 2 && count != 3) {
        LOG("count is wrong");
    }

    switch(count) {
    case 3: /*action = get*/
        device = safe_trip(cJSON_value(opt, "device", string));
    case 2:  /*action = status*/
        action = safe_trip(cJSON_value(opt, "action", string));
        break;
    }

    if (!strcasecmp(action, "status")) {
        system("wifi status");

    } else if (!strcasecmp(action, "get")) {
        uci_get("wireless.%s.path", path, SIZE, device);
        sprintf(devicepath, "/sys/devices/%s/net", path);
        if (access(devicepath, F_OK)) {
            LOG("dev %s isn't exist", devicepath);
        }

        sprintf(cmd, "ls %s | head -n 1", devicepath);
        getCmdResult(cmd, wlan, sizeof(wlan));

        cJSON *obj=cJSON_CreateObject();
        cJSON_AddItemToObject(obj,"wlan",cJSON_CreateString(wlan));
        RETURN_JSON(0, obj);
    } else {
        LOG("%s is unknow", action);
    }

    free(device);
    free(action);
    return;
}


static cJSON *dhcpinfo_json_res(char *mac)
{
    char buf[SIZE];
    char ip[MIXSIZE], host[MIXSIZE];
    char *res[4];
    const char *dhcpfilepath = "/tmp/dhcp.leases";
    cJSON *json =  cJSON_CreateObject();
    cJSON *obj = NULL;
    cJSON *arrayjson = cJSON_CreateArray();
    

    if (access(dhcpfilepath, F_OK)) {
        LOG("%s isn't exist", dhcpfilepath);
    }

    FILE *fp = fopen(dhcpfilepath, "r");

    while (fgets(buf, SIZE, fp)) {
        strcpy(ip, "UNKNOW");
        strcpy(host, "UNKNOW");
        if (strstr(buf, mac)) {
            if (myawk(buf, " ", res, 4) == 4) {
                strcpy(ip, res[2]);
                strcpy(host, res[3]);
            }
        }

        obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "ip", ip);
        cJSON_AddStringToObject(obj, "host", host);
        cJSON_AddStringToObject(obj, "mac", mac);
        cJSON_AddItemToArray(arrayjson, obj);
    }

    cJSON_AddItemToObject(json, "dhcpinfo", arrayjson);

    return json;
}


static cJSON* info_from_if(const char *interface, char *result, int len)
{
    char out[SIZE], cmd[100];
    FILE *pOut = NULL;
    char *res[2];
    cJSON *itemobj = NULL;
    cJSON *arrayjson = cJSON_CreateArray();

    sprintf(cmd, "iw dev %s station dump", interface);
    pOut = popen(cmd, "r");
    memset(out, 0, SIZE);
    while ((fgets(out, SIZE, pOut)) != NULL) {
        if (!strstr(out, "Station")) {
           continue;
        }
        if (myawk(out, " ", res, 2) != 2) {
           LOG("get mac from (%s) fail", cmd); 
        }

        itemobj = dhcpinfo_json_res(res[1]);
        cJSON_AddItemToArray(arrayjson,itemobj);
        memset(out, 0, SIZE);
    }

    return arrayjson;
}


/* {"network":"client"} */
static void clientopt_fun(int count, cJSON *opt)
{
    int num = 0;
    char buf[SIZE];
    FILE *pOut;
    char pif[MIXSIZE];
    char *res[2];
    cJSON *itemarray = NULL;
    cJSON *json = cJSON_CreateObject();

    pOut = popen("iw dev", "r");
    memset(buf, 0, SIZE);
    while ((fgets(buf, SIZE, pOut)) != NULL) {
        if (!strstr(buf, "Interface")) {
           continue;
        }
        if (myawk(buf, " ", res, 2) != 2) {
            LOG("get wifi interface from data fail");
        }
        
        strip(res[1], pif, SIZE);
        itemarray = info_from_if(pif, buf, SIZE);
        cJSON_AddItemToObject(json, pif, itemarray);
        num++;
        memset(buf, 0, SIZE);
    }

    if (!num) {
        cJSON_Delete(json);
        RETURN_MSG(0, "the devices is null");
    } else {
        RETURN_JSON(0, json);
    }
    
    return;
}


/* {"network": "settings_3g"} */
static void settings_3gopt_fun(int count, cJSON *opt)
{
    char buf[SIZE];
    cJSON *obj = cJSON_CreateObject();
    cJSON *router, *stick;
    char devname[MIXSIZE], mode[MIXSIZE], wan_type[MIXSIZE], device[MIXSIZE], proto[MIXSIZE], apn[MIXSIZE], service[MIXSIZE], dialnumber[MIXSIZE];

    uci_get("router.usb.devname", devname, MIXSIZE);
    uci_get("router.usb.mode", mode, MIXSIZE);
    uci_get("router.system.wan_type", wan_type, MIXSIZE);

    cJSON_AddStringToObject(obj,"mode", mode);
    cJSON_AddItemToObject(obj, "router", router=cJSON_CreateObject());
    cJSON_AddStringToObject(router, "device", devname);
    cJSON_AddItemToObject(obj, "stick", stick=cJSON_CreateObject());

    if (!strcmp(wan_type, "3g") && !strcmp(mode, "stick")) {
        uci_get("network.wan.proto", proto, MIXSIZE);
        uci_get("network.wan.apn", apn, MIXSIZE);
        uci_get("network.wan.service", service, MIXSIZE);
        uci_get("network.wan.dialnumber", dialnumber, MIXSIZE);
        uci_get("network.wan.device", buf, SIZE);
        sprintf(device, "%s##*/", buf);
    } else {
        strcpy(device, "ttyUSB3");
        strcpy(proto, "3g");
        strcpy(apn, "3gnet");
        strcpy(service, "umts");
        strcpy(dialnumber, "*99#");
    }

    cJSON_AddStringToObject(stick, "device", device);
    cJSON_AddStringToObject(stick, "proto", proto);
    cJSON_AddStringToObject(stick, "apn", apn);
    cJSON_AddStringToObject(stick, "service", service);
    cJSON_AddStringToObject(stick, "dialnumber", dialnumber);
    
    RETURN_JSON(0, obj);
    return;
}


static cJSON *openvpn_vpn_info()
{
    cJSON *json = cJSON_CreateObject();;

    if (isDevUp("eth0")) {
        json = cJSON_CreateString("online");
    } else {
        json = cJSON_CreateString("connecting...");
    }

    return json;
}


static cJSON *other_vpn_info(int level, bool logread)
{
    char cmd[SIZE], buf[SIZE];
    char proto[MIXSIZE], vpn_type[MIXSIZE], pidpath[MIXSIZE], pid[MIXSIZE], username[MIXSIZE], password[MIXSIZE];
    char server[MIXSIZE], vpn_left_count[MIXSIZE], vpn_psk[MIXSIZE];
    char vpn_lanip[MIXSIZE];
    int vpnstatus;
    cJSON *json = NULL;

    uci_get("network.vpn%d.proto", proto, MIXSIZE, level);
    sprintf(vpn_type, "%s-vpn%d", proto, level);
    if (logread) {
        sprintf(pidpath, "/var/run/%s.pid", vpn_type);

        if (access(pidpath, F_OK)) {
            LOG("%s isn't exist", pidpath);
        }

        if (filecontent(pidpath, pid, SIZE)) {
            LOG("pidpath isn't exist");  
        }

        sprintf(cmd, "logread | grep %s", pid);
        if (getCmdResult_r(cmd, buf, SIZE)) {
            json = cJSON_CreateString("connecting");
        } else {
            json = cJSON_CreateString("offline");
        }

    } else {
        json = cJSON_CreateObject();

        if (isDevUp(vpn_type)) {
            vpnstatus = 0;
            cJSON_AddStringToObject(json, "status", "connecting");
        } else {
            vpnstatus = 1;
            cJSON_AddStringToObject(json, "status", "offline");
        }

        if (!vpnstatus) {
            get_ip(vpn_type, vpn_lanip);
            cJSON_AddStringToObject(json, "ip", vpn_lanip);
        }
        uci_get("network.vpn%d.username", username, MIXSIZE, level);
        uci_get("network.vpn%d.password", password, MIXSIZE, level);
        uci_get("router.vpn.vpn_left_count", vpn_left_count, MIXSIZE);
        uci_get("network.vpn%d.server", server, MIXSIZE, level);

        sprintf(cmd, "{\"network\":\"ipsec\", \"action\":\"get\", \"ip\":\"%s\"}", server);
        if (exec_config_editor(cmd, vpn_psk, MIXSIZE)) {
            LOG("exec config_editor fail %s", cmd);
        }

        strip(vpn_psk, vpn_psk, MIXSIZE);

        cJSON_AddStringToObject(json, "username", username);
        cJSON_AddStringToObject(json, "password", password);
        cJSON_AddStringToObject(json, "server", server);
        cJSON_AddStringToObject(json, "proto", proto);
        cJSON_AddStringToObject(json, "vpn_left_count", vpn_left_count);
        cJSON_AddStringToObject(json, "vpn_psk", vpn_psk);
    }

    return json;
}


/* {"network": "vpn", "level":0, "logread":bool} */
static void vpnopt_fun(int count, cJSON *opt)
{
    char model[MIXSIZE];
    bool logread = NULL;
    int level = -1;
    cJSON *obj;

    if (count != 1 && count != 3) {
        LOG("count is wrong");
    }

    switch(count) {
    case 3:
        logread = cJSON_value(opt, "logread", bool);
        level = cJSON_value(opt, "level", int);
        break;
    }

    uci_get("router.vpn.model", model, MIXSIZE);

    if (!strcasecmp(model, "openvpn")) {
        obj = openvpn_vpn_info();
    } else {
        obj = other_vpn_info(level, logread);
    }

    RETURN_JSON(0, obj);
    return;
}


/* {"network":"vpn_setting"} */
static void vpn_settingopt_fun(int count, cJSON *opt)
{
    char timeout[MIXSIZE];
    cJSON *json = cJSON_CreateObject();

    uci_get("router.vpn.timeout", timeout, MIXSIZE);
    cJSON_AddStringToObject(json, "timeout", timeout);

    RETURN_JSON(0, json);
    return;
}


/* {"network": "systeminfo"} */
static void systeminfoopt_fun(int count, cJSON *opt)
{
    cJSON *json = cJSON_CreateObject();
    char systeminfo[MIXSIZE], version[MIXSIZE], subversion[MIXSIZE], mac[MIXSIZE];

    uci_get("router.system.system", systeminfo, MIXSIZE);
    uci_get("router.system.version", version, MIXSIZE);
    uci_get("router.system.subversion", subversion, MIXSIZE);
    uci_get("router.system.mac", mac, MIXSIZE);

    cJSON_AddStringToObject(json, "system", systeminfo);
    cJSON_AddStringToObject(json, "version", version);
    cJSON_AddStringToObject(json, "subversion", subversion);
    cJSON_AddStringToObject(json, "mac", mac);

    RETURN_JSON(0, json);
    return;
}


static cJSON *wifilog_json_res(char **res)
{
    char time[MIXSIZE], protocol[MIXSIZE];

    cJSON *obj = cJSON_CreateObject();

    sprintf(time, "%s %s %s %s", res[1], res[2], res[3], res[4]);
    sprintf(protocol, "%s %s", res[10], res[11]);
    protocol[strlen(protocol)-1] = '\0';

    cJSON_AddStringToObject(obj, "time", time);
    cJSON_AddStringToObject(obj, "attacker", res[9]);
    cJSON_AddStringToObject(obj, "protocol", protocol);

    return obj;
}


/* {"network":"wifilog"} */
static void wifilogopt_fun(int count, cJSON *opt)
{
    char buf[SIZE];
    char *groups[20];
    FILE *fp=NULL;
    cJSON *itemobj=NULL;
    cJSON * json = cJSON_CreateObject();
    cJSON *arrayjson = cJSON_CreateArray();
    const char *wifilogpath = "/root/log/wifi.log";

    char *res = getCmdResult_r("logread | grep 'deauthenticated due to local deauth request'|grep -o '[0-9a-f:]\\{17\\}'|uniq -c|head -n 1|awk '{print $1}'", buf, SIZE);
    if (res && atoi(res) > 3) {
        cJSON_AddStringToObject(json, "content", "Your wifi is under attack!!");
    } else {
        cJSON_AddStringToObject(json, "content", "It seems your WiFi is safe now");
    }

    if (access(wifilogpath, F_OK)) {
        LOG("%s isn't exist", wifilogpath);
    }
    if (!(fp = fopen(wifilogpath, "r"))) {
        LOG("fopen %s fail", wifilogpath);
    }
    memset(buf, 0, SIZE);
    while((fgets(buf, SIZE, fp)) != NULL) {
        buf[strlen(buf)-1] = '\0';
        myawk(buf, " ", groups, sizeof(groups)/sizeof(char *));
        itemobj = wifilog_json_res(groups);
        cJSON_AddItemToArray(arrayjson,itemobj);
        memset(buf, 0, SIZE);
    }

    if (fclose(fp)) {
        LOG("fclose %s fail", wifilogpath);
    }

    cJSON_AddItemToObject(json, "result", arrayjson);
    RETURN_JSON(0, json);
    return;
}


static cJSON *honeylog_json_res(char **res)
{
    char time[MIXSIZE];
    cJSON *obj = cJSON_CreateObject();

    sprintf(time, "%s %s", res[0], res[1]);

    cJSON_AddStringToObject(obj, "time", time);
    cJSON_AddStringToObject(obj, "ip", res[6]);
    cJSON_AddStringToObject(obj, "port", res[3]);

    return obj;
}


/* {"network":"honeypot_log"} */
static void honeypot_logopt_fun(int count, cJSON *opt)
{
    char buf[SIZE];
    char * res[20];
    FILE *fp;
    cJSON *itemobj;
    cJSON *arrayjson = cJSON_CreateArray();
    const char *honeylogpath = "/root/log/honey.log";

    if (cmdcall_no_output("pidof python")) {
        printf("Honeypot is not running\n");
    } else {
        printf("Honeypot is running\n");
    }

    if (access(honeylogpath, F_OK)) {
        LOG("%s isn't exist", honeylogpath);
    }
    if (!(fp = fopen(honeylogpath, "r"))) {
        LOG("fopen %s fail", honeylogpath);
    }
    while((fgets(buf, SIZE, fp)) != NULL) {
        if (strstr(buf, "127.0.0.1")) {
            continue;
        }
        myawk(buf, " ", res, sizeof(res)/sizeof(char *));
        itemobj = honeylog_json_res(res);
        cJSON_AddItemToArray(arrayjson,itemobj);
    }

    if (fclose(fp)) {
        LOG("fclose %s fail", honeylogpath);
    }

    RETURN_JSON(0, arrayjson);
    return;
}


int exec_getstatus(char *cmd, char *buffer, int len)
{
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1) {
        LOG("pipe");
    }

    switch (pid=fork()) {
    case -1:
        LOG("fork fail");
        break;
    case 0:
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        
        int res = getstatus_main(cmd);

        exit(res);
    default:
        close(pipefd[1]);
        if (buffer) {    //if buffer is NULL, don't read the result
            read(pipefd[0], buffer, len);
        }
        int status;
        waitpid(pid, &status, 0);
        close(pipefd[0]);

        //if fail exit, WIFEXITED will return zero
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
    
}


int getstatus_main(char const *argvstr)
{
    assert(argvstr != NULL);

    cJSON *json;
    char *network = NULL;

    if(!(json = cJSON_Parse(argvstr)))
        LOG("Error before:[%s]",cJSON_GetErrorPtr());

    int count = cJSON_GetArraySize(json);
    
    network = safe_trip(cJSON_value(json, "network", string));

    switch (getstatus_network_type(network)) {
    case wan:
        wanopt_fun(count, json);
        break;
    case lan:
        lanopt_fun(count, json);
        break;
    case ap:
        apopt_fun(count, json);
        break;
    case client:
        clientopt_fun(count, json);
        break;
    case settings_3g:
        settings_3gopt_fun(count, json);
        break;
    case vpn:
        vpnopt_fun(count, json);
        break;
    case vpn_setting:
        vpn_settingopt_fun(count, json);
        break;
    case systeminfo:
        systeminfoopt_fun(count, json);
        break;
    case wifilog:
        wifilogopt_fun(count, json);
        break;
    case honeypot_log:
        honeypot_logopt_fun(count, json);
        break;
    default:
        LOG("json network key is wrong");
    }

    cJSON_Delete(json);
    free(network);

    return 0;
}
