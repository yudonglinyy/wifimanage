#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#include "log.h"
#include "cmdcall.h"
#include "strip.h"
#include "jsonvalue.h"
#include "wifitype.h"
#include "ucicmd.h"
#include "config_editor_main.h"

#define SIZE 1024

typedef enum network_type {wifi, dhcp, static_type, mobile, lan, ap, vpn, vpn_setting, openvpn_setting, ipsec, user, apcheck} NETWORK_T;



int killvpnmon()
{
    char cmd[SIZE];
    char sysfilename[SIZE];
    int res;

    uci_get("router.system.system", sysfilename, 256);
    sprintf(cmd, "killall %s", sysfilename);
    res = cmdcall_no_output(cmd);
    if (res) {
        return 1;
    }
    return 0;
}


int startvpnmon()
{
    char sysfilename[256];

    uci_get("router.system.system", sysfilename, 256);

    pid_t pid;
    switch (pid=fork()) {
    case -1:
        LOG("startvpnmon handle error");
    case 0:
        execlp(sysfilename, sysfilename, NULL);  //test
    default:
        return 0;
    }
}

NETWORK_T get_network_type(const char *network)
{
    NETWORK_T type;

    if (!strcmp(network, "wifi")) {
        type = wifi;
    }
    else if (!strcmp(network, "dhcp")) {
        type = dhcp;
    }
    else if (!strcmp(network, "static")) {
        type = static_type;
    }
    else if (!strcmp(network, "3g")) {
        type = mobile;
    }
    else if (!strcmp(network, "lan")) {
        type = lan;
    }
    else if (!strcmp(network, "ap")) {
        type = ap;
    }
    else if (!strcmp(network, "vpn")) {
        type = vpn;
    }
    else if (!strcmp(network, "vpn_setting")) {
        type = vpn_setting;
    }
    else if (!strcmp(network, "openvpn_setting")) {
        type = openvpn_setting;
    }
    else if (!strcmp(network, "ipsec")) {
        type = ipsec;
    }
    else if (!strcmp(network, "user")) {
        type = user;
    }
    else if (!strcmp(network, "apcheck")) {
        type = apcheck;
    }
    else {
        LOG("network argv fail");
    }

    return type;
}


int exec_wifiedit(int argc_n, WIFIEDIT_T *pargv, char *buffer, int len)
{
    int pipefd[2];
    int res;
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    switch (pid=fork()) {
    case -1:
        LOG("fork fail");
        break;
    case 0:
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);

        res = wifiedit_main(argc_n, pargv);
        exit(res);
    default:
        close(pipefd[1]);
        if (buffer) {    //if buffer is NULL, don't read the result
            read(pipefd[0], buffer, len);
        }
        int status;
        waitpid(pid, &status, 0);

        close(pipefd[0]);

        //if fail exit, WIFEXITED will return zero)
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }

        return -1;
    }
}


void wfcheck_type(int defaultroute)
{
    char type[SIZE];
    const char *tmp_gateway = "/tmp/tmp_gateway";

    killvpnmon();

    uci_get("router.system.wan_type", type, SIZE);

    /*if buf isn't wifi and defaultroute is not zero, will restart*/
    if (strcmp(type, "wifi") && defaultroute != 0 ) {
        cmdcall_no_output("/etc/init.d/network restart");
        if (!access(tmp_gateway, F_OK) && remove(tmp_gateway)) {
            LOG("rm %s file fail", tmp_gateway);
        }

        dial_main(0, NULL);
        uci_add("router.system.wan_type=wifi");
    }

    return;
}

void set_vpnconf_haschange()
{
    uci_add("router.vpn.changed=1");
    uci_add("router.vpn.min_changed_level=1");

    startvpnmon();

    cmdcall_no_output("ipsec restart");  //test
    return;
}


void kill_vpn_pid(char *proto, int level)
{
    char cmd[SIZE], buf[SIZE];
    pid_t pppdpid;

    if (!strcmp(proto, "pptp")) {
        sprintf(cmd, "ps|grep -v 'grep'|grep '/usr/sbin/pppd nodetach ipparam vpn%d'|awk '{print $1}'", level);
        if (NULL != getCmdResult_r(cmd, buf, SIZE)) {
            pppdpid = atoi(buf);

            if (kill(pppdpid, SIGKILL)) {
                LOG("kill fail");
            }
        }

    }
    else if (strcmp(proto, "l2tp")){
        sprintf(cmd, "ps|grep -v 'grep'|grep '/usr/sbin/pppd plugin pppol2tp.so pppol2tp 10 passive nodetach : file /tmp/l2tp/options.vpn%d'|awk '{print $1}'", level);
        if (NULL != getCmdResult_r(cmd, buf, SIZE)) {
            pppdpid = atoi(buf);

            if (kill(pppdpid, SIGKILL)) {
                LOG("kill fail");
            }
        }

        sprintf(cmd, "xl2tpd-control remove l2tp-vpn%d", level);
        cmdcall_no_output(cmd);
    }
    else {
        LOG("%s is unknow", proto);
    }
}


/* {"network":"wifi", "ssid" : "XX", "bssid": "XX", "device": "XX", "key" : "XX", "encryption": "XX", "disabled": "XX", "action": "XX"} */
void wifiopt_fun(int count, cJSON *opt)
{
    char buf[SIZE];
    char *actionbuf = NULL;
    int res;
    WIFIEDIT_T wifiargv;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 7) {
        LOG("count is wrong");
    }


    wifiargv.ssid = safe_trip(cJSON_value(opt, "ssid", string));
    wifiargv.encryption = safe_trip(cJSON_value(opt, "encryption", string));
    wifiargv.key = cJSON_value(opt, "key", string);
    wifiargv.device = safe_trip(cJSON_value(opt, "device", string));
    wifiargv.bssid = safe_trip(cJSON_value(opt, "bssid", string));
    actionbuf = safe_trip(cJSON_value(opt, "action", string));;
    wifiargv.action = actionbuf;

    uci_get("router.wifi.defaultroute", buf, SIZE);
    int defaultroute = atoi(buf);
    if (defaultroute == 1) {
        uci_add("network.wan.enabled=0");
    }

    uci_get("router.vpn.model", buf, SIZE);
    if (!strcmp(buf, "openvpn")) {
        res = cmdcall_no_output("/etc/init.d/openvpn stop");
        if (res) {
            LOG("openvpn stop operation fail");
        }
    }

    /*conn*/
    if (!strcmp(wifiargv.action, "conn")) {
        wfcheck_type(defaultroute);

        char connected_wifi_ssid[SIZE];
        uci_get("wireless.wificlient_chocobo.ssid", connected_wifi_ssid, SIZE);

        if (!strcmp(wifiargv.ssid, connected_wifi_ssid)) {
            RETURN_MSG(0, "%s already connect", wifiargv.ssid);
            return;
        }

        /*if exec fail,  res is -1, success will return the exit status*/
        res = exec_wifiedit(2, &wifiargv, NULL, 0);

        if (res) {
            wifiargv.action = "new";
            if (exec_wifiedit(6, &wifiargv, NULL, 0)) {
                LOG("exec wifiedit new fail");
            }

            wifiargv.action = "conn";
            if (exec_wifiedit(6, &wifiargv, NULL, 0)) {
                LOG("exec wifiedit conn fail");
            }
        }

        set_vpnconf_haschange();
    }
    /*edit*/
    else if (!strcmp(wifiargv.action, "edit")) {
        if (exec_wifiedit(6, &wifiargv, buf, SIZE)) {
            LOG("exec wifiedit edit fail");
        }

        set_vpnconf_haschange();
    }
    /*del*/
    else if (!strcmp(wifiargv.action, "del")) {
        exec_wifiedit(2, &wifiargv, NULL, 0);

        uci_get("wireless.wificlient_chocobo.ssid", buf, SIZE);

        /*if buf==ssid will restart*/
        if (!strcmp(buf, wifiargv.ssid)) {
            uci_del("wireless.wificlient_chocobo");

            cmdcall_no_output("/etc/init.d/network reload &&"
                                "ipsec restart");
        }
    }
    /*hidden*/
    else if (!strcmp(wifiargv.action, "hidden")) {
        killvpnmon();

        wfcheck_type(defaultroute);
        set_vpnconf_haschange();
        startvpnmon();
    }
    else {
        LOG("%s is unknow", wifiargv.action);
    }

    free(actionbuf);
    free(wifiargv.ssid);
    free(wifiargv.device);
    free(wifiargv.bssid);
    free(wifiargv.encryption);
    free(wifiargv.key);

    return;
}


/* {"network":"dhcp", "device": "XX"} */
void dhcpopt_fun(int count, cJSON *opt)
{
    char buf[SIZE];
    const char *tmp_gateway = "/tmp/tmp_gateway";
    char *device = NULL;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 2) {
        LOG("count is wrong");
    }

    switch(count) {
    case 2:
        device = safe_trip(cJSON_value(opt, "device", string));
        break;
    }

    uci_del("network.wan");
    uci_add("network.wan=interface");
    uci_add("network.wan.ifname=%s", device);
    uci_add("network.wan.proto=dhcp");

    cmdcall_no_output("/etc/init.d/openvpn stop");

    killvpnmon();

    uci_get("router.system.wan_type", buf, SIZE);
    if (!strcmp(buf, "wifi")) {
        uci_del("wireless.wificlient_chocobo");
        cmdcall_no_output("killall wifimon; /etc/init.d/network restart; ifup wan");
    }
    else {
        cmdcall_no_output("/etc/init.d/network");
    }

    uci_add("router.system.wan_type=dhcp");
    uci_add("router.vpn.changed=1");
    uci_add("router.vpn.min_changed_level=1");

    if (!access(tmp_gateway, F_OK) && remove(tmp_gateway)) {
        LOG("rm %s file fail", tmp_gateway);
    }

    sleep(3);
    startvpnmon();
    cmdcall_no_output("ipsec restart");
    return;
}


/* {"network":"static", "ipaddr": "XX", "netmask": "XX", "gateway": "XX"} */
void staticopt_fun(int count, cJSON *opt)
{
    char buf[SIZE];
    const char *tmp_gateway = "/tmp/tmp_gateway";
    char *ipaddr = NULL, *netmask = NULL, *gateway = NULL;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 4) {
        LOG("count is wrong");
    }

    switch(count) {
    case 4:
        ipaddr = safe_trip(cJSON_value(opt, "ipaddr", string));
        netmask = safe_trip(cJSON_value(opt, "netmask", string));
        gateway = safe_trip(cJSON_value(opt, "gateway", string));
        break;
    }

    uci_del("network.wan");
    uci_add("network.wan=interface");
    uci_add("network.wan.ifname=eth0");
    uci_add("network.wan.proto=static");
    uci_add("network.wan.ipaddr=%s", ipaddr);
    uci_add("network.wan.netmask=%s", netmask);
    uci_add("network.wan.gateway=%s", gateway);

    cmdcall_no_output("/etc/init.d/openvpn stop");

    killvpnmon();

    uci_get("router.system.wan_type", buf, SIZE);
    if (!strcmp(buf, "wifi")) {
        uci_del("wireless.wificlient_chocobo");
        cmdcall_no_output("killall wifimon; /etc/init.d/network restart; ifup wan");
    }
    else {
        cmdcall_no_output("/etc/init.d/network restart");
    }

    uci_add("router.system.wan_type=static");
    uci_add("router.vpn.changed=1");
    uci_add("router.vpn.min_changed_level=1");

    if (!access(tmp_gateway, F_OK) && remove(tmp_gateway)) {
        LOG("rm %s file fail", tmp_gateway);
    }

    sleep(3);
    startvpnmon();
    cmdcall_no_output("ipsec restart");

    free(ipaddr);
    free(netmask);
    free(gateway);

    return;
}


/* {"network":"3g", "apn" : "XX", "service": "XX", "dialnumber": "XX", "proto" : "XX", "device": "XX", "mode": "XX"} */
void mobileopt_fun(int count, cJSON *opt)
{
    char buf[SIZE];
    char *device = NULL, *apn = NULL, *service = NULL, *dialnumber = NULL, *proto = NULL, *mode = NULL;
    const char *tmp_gateway = "/tmp/tmp_gateway";

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 7 && count != 3) {
        LOG("count is wrong");
    }

    switch(count) {
    case 7:
        apn = safe_trip(cJSON_value(opt, "apn", string));
        service = safe_trip(cJSON_value(opt, "service", string));
        dialnumber = safe_trip(cJSON_value(opt, "dialnumber", string));
        proto = safe_trip(cJSON_value(opt, "proto", string));
    case 3:
        mode = safe_trip(cJSON_value(opt, "mode", string));
        device = safe_trip(cJSON_value(opt, "device", string));
        break;
    }

    uci_del("network.wan");
    uci_add("network.wan=interface");

    if (!strcmp(mode, "router")) {
        uci_add("network.wan.ifname=%s", device);
        uci_add("network.wan.proto=dhcp");
        uci_add("router.usb.mode=router");
    } else if (!strcmp(mode, "stick")) {
        uci_add("router.usb.mode=stick");
        uci_add("network.wan.ifname=eth0");
        uci_add("network.wan.device=/dev/%s", device);
        uci_add("network.wan.apn=%s", apn);
        uci_add("network.wan.service=%s", service);
        uci_add("network.wan.dialnumber=%s", dialnumber);
        uci_add("network.wan.proto=%s", proto);
    } else {
        LOG("%s is unknow", mode);
    }

    cmdcall_no_output("/etc/init.d/openvpn stop");

    killvpnmon();

    if (!strcmp(buf, "wifi")) {
        uci_del("wireless.wificlient_chocobo");
        cmdcall_no_output("killall wifimon; /etc/init.d/network restart; ifup wan");
    }
    else {
        cmdcall_no_output("/etc/init.d/network restart");
    }

    uci_add("router.system.wan_type=3g");
    uci_add("router.vpn.changed=1");
    uci_add("router.vpn.min_changed_level=1");


    if (!access(tmp_gateway, F_OK) && remove(tmp_gateway)) {
        LOG("rm %s file fail", tmp_gateway);
    }

    sleep(3);
    startvpnmon();
    cmdcall_no_output("ipsec restart");

    free(device);
    free(apn);
    free(service);
    free(dialnumber);
    free(proto);
    free(mode);

    return;
}


/* {"network":"lan", "ipaddr": "XX", "netmask": "XX"} */
void lanopt_fun(int count, cJSON *opt)
{
    char cmd[SIZE];
    char *ipaddr = NULL, *netmask = NULL;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 3) {
        LOG("count is wrong");
    }

    switch (count) {
    case 3:
        ipaddr = safe_trip(cJSON_value(opt, "ipaddr", string));
        netmask = safe_trip(cJSON_value(opt, "netmask", string));
        break;
    }

    uci_add("network.lan.ipaddr=%s", ipaddr);
    uci_add("network.lan.netmask=%s", netmask);
    uci_add("firewall.router_22.dest_ip=%s", ipaddr);
    uci_add("firewall.router_80.dest_ip=%s", ipaddr);

    cmdcall_no_output("/etc/init.d/firewall reload;"
                        "/etc/init.d/network reload");

    cmdcall_no_output(cmd);

    free(ipaddr);
    free(netmask);

    return;
}


/* {"network":"ap", "device" : "XX", "label": "XX", "ssid": "XX", "key" : "XX", "encryption": "XX", "disabled": "XX", "action": "XX"} */
void apopt_fun(int count, cJSON *opt)
{
    char *device = NULL, *label = NULL, *ssid = NULL, *key = NULL, *encryption = NULL, *disabled = NULL, *action = NULL;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 8 && count != 3) {
        LOG("count is wrong");
    }

    switch(count) {
    case 8:
        encryption = safe_trip(cJSON_value(opt, "encryption", string));
        disabled = safe_trip(cJSON_value(opt, "disabled", string));
        ssid = safe_trip(cJSON_value(opt, "ssid", string));
        key = cJSON_value(opt, "key", string);  
        device = safe_trip(cJSON_value(opt, "device", string));
    case 3:
        label = safe_trip(cJSON_value(opt, "label", string));
        action = safe_trip(cJSON_value(opt, "action", string));
        break;
    }


    /*add*/
    if (!strcmp(action, "add")) {
        uci_add("wireless.%s=wifi-iface", label);
        uci_add("wireless.%s.device=%s", label, device);
        uci_add("wireless.%s.mode=ap", label);
        uci_add("wireless.%s.network=lan", label);
        uci_add("wireless.%s.encryption=%s", label, encryption);
        uci_add("wireless.%s.disabled=%s", label, disabled);
        uci_add("wireless.%s.wpa_group_rekey=0", label);
        uci_add("wireless.%s.wpa_pair_rekey=0", label);
        uci_add("wireless.%s.wpa_master_rekey=0", label);
        uci_add("wireless.%s.ssid=%s", label, ssid);

        if (strcmp(encryption, "none")) {
            uci_add("/bdcapdir","wireless.%s.key=%s", label, key);
        }

        cmdcall_no_output("/etc/init.d/network reload");
    }
    /*del*/
    else if (!strcmp(action, "del")) {
        uci_del("wireless.%s", label);
        cmdcall_no_output("/etc/init.d/network reload");
    }
    /*edit*/
    else if (!strcmp(action, "edit")) {
        uci_add("wireless.%s.ssid=%s", label, ssid);
        uci_add("wireless.%s.encryption=%s", label, encryption);
        uci_add("wireless.%s.disabled=%s", label, disabled);

        if (strcmp(encryption, "none")) {
            uci_add("/bdcapdir", "wireless.%s.key=%s", label, key);
        } else {
            uci_del("wireless.%s.key", label);
        }

        cmdcall_no_output("/etc/init.d/network reload");
    }
    
    else {
        LOG("action %s is wrong", action);
    }

    free(device);
    free(label);
    free(ssid);
    free(key);
    free(encryption);
    free(disabled);

    return;
}

/* {"network":"vpn", "proto" : "XX", "server": "XX", "username": "XX", "password" : "XX", "level": 1, "psk": "XX", "mac": "XX","action": "XX"} */
void vpnopt_fun(int count, cJSON *opt)
{
    char buf[SIZE], cmd[SIZE];
    char *action = NULL, *proto = NULL, *server = NULL, *username = NULL, *password = NULL, *psk = NULL, *mac = NULL;
    int level=-1, oldlevel;
    char oldproto[100], oldserver[100];

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 9 && count != 8) {
        LOG("count is wrong");
    }

    switch(count) {
    case 9:   /*action: add edit*/
        psk = cJSON_value(opt, "psk", string);
    case 8:   /*action: del*/
        proto = safe_trip(cJSON_value(opt, "proto", string));
        server = safe_trip(cJSON_value(opt, "server", string));
        username = safe_trip(cJSON_value(opt, "username", string));
        password = cJSON_value(opt, "password", string);
        level = cJSON_value(opt, "level", int);
        action = safe_trip(cJSON_value(opt, "action", string));
        mac = safe_trip(cJSON_value(opt, "mac", string));
        break;
    }

    uci_get("router.vpn.level", buf, SIZE);
    oldlevel = atoi(buf);

    if (!strcmp(action, "add")) {
        cmdcall_no_output("route del default");
        int newlevel = oldlevel + 1;

        uci_add("network.vpn%d=interface", newlevel);
        uci_add("network.vpn%d.proto=%s", newlevel, proto);
        uci_add("network.vpn%d.server=%s", newlevel, server);
        uci_add("network.vpn%d.username=%s", newlevel, username);
        uci_add("network.vpn%d.password=%s", newlevel, password);
        uci_add("network.vpn%d.mac=%s", newlevel, mac);
        uci_add("network.vpn%d.auto=0", newlevel);
        uci_add("network.vpn%d.delegate=0", newlevel);


        if (mac) {
            uci_add("network.vpn%d.mac=%s", level, mac);
        }

        if (!strcmp(proto, "l2tp")) {
            sprintf(cmd, "{\"network\":\"ipsec\", \"ip\": \"%s\", \"psk\": \"%s\", \"level\": %d, \"action\": \"add\"}",
                            server, psk, newlevel);
            config_editor_main(cmd);
        }

        uci_add("router.vpn.level=%d", newlevel);
        uci_add("router.vpn.changed=1");
        uci_add("router.vpn.min_changed_level=%d", newlevel-1);
        uci_sim_add_list("firewall.wan.network=vpn%d", newlevel);

    } else if (!strcasecmp(action, "del")) {
        uci_get("network.vpn%d.proto", oldproto, sizeof(oldproto), level);
        uci_get("network.vpn%d.server", oldserver, sizeof(oldserver), level);
        kill_vpn_pid(oldproto, level);

        uci_del("network.vpn%d", level);
        sprintf(cmd, "route del %s", oldserver);
        cmdcall_no_output(cmd);

        for (;level<oldlevel;level++) {
            uci_get("network.vpn%d.proto", buf, SIZE, level+1);
            if (!strcmp(buf, "l2tp")) {
                sprintf(cmd, "sed -i \"s/^conn vpn%d\\$/conn vpn%d/\" /etc/ipsec.conf", level+1, level);
                cmdcall_no_output(cmd);
            }
            uci_sim_rename("network.vpn%d=vpn%d", level+1, level);
        }

        uci_add("router.vpn.level=%d", oldlevel-1);

    } else if (!strcasecmp(action, "edit")) {
        sprintf(cmd, "uci show network.vpn%d", level);
        if (cmdcall_no_output(cmd) && !strcmp(proto, "l2tp")) {
            sprintf(cmd, "{\"network\":\"vpn\", \"proto\":\"%s\", \"server\":\"%s\", \"username\":\"%s\", \"password\":\"%s\", \"level\": %d, \"psk\": \"%s\", \"mac\": \"%s\",\"action\": \"add\"}",
                    proto, server, username, password, level, psk, mac);
            config_editor_main(cmd);
            return;
        }

        uci_get("network.vpn%d.server", oldserver, sizeof(oldserver), level);
        uci_get("network.vpn%d.proto", oldproto, sizeof(oldproto), level);

        /*when old proto is l2tp, del the old one of the ipsec file*/
        if (!strcmp(oldproto, "l2tp")) {
            sprintf(cmd, "{\"network\":\"ipsec\", \"ip\": \"%s\", \"level\": %d, \"action\": \"del\"}",
                    server, level);
            config_editor_main(cmd);

        /*when new proto is l2tp, add the new one of the ipsec file*/
        if (!strcmp(proto, "l2tp"))
            sprintf(cmd, "{\"network\":\"ipsec\", \"ip\": \"%s\", \"psk\": \"%s\", \"level\": %d, \"action\": \"add\"}",
                    server, psk, level);
            config_editor_main(cmd);
        }

        uci_add("network.vpn%d.mac=%s", level, mac);
        uci_add("network.vpn%d.proto=%s", level, proto);
        uci_add("network.vpn%d.server=%s", level, server);
        uci_add("network.vpn%d.username=%s", level, username);
        uci_add("network.vpn%d.password=%s", level, password);
        uci_add("router.vpn.changed=1");
        uci_add("router.vpn.min_changed_level=%d", level);

    } else {
        LOG("%s is unknow", action);
    }

    free(proto);
    free(server);
    free(username);
    free(password);
    free(action);
    free(psk);
    free(mac);

    return;
}

/* {"network":"vpn_setting", "timeout": "XX","action": "save"} */
void vpn_settingopt_fun(int count, cJSON *opt)
{
    char *timeout = NULL, *action = NULL;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 3) {
        LOG("count is wrong");
    }

    switch(count) {
    case 3:
        timeout = safe_trip(cJSON_value(opt, "timeout", string));
        action = safe_trip(cJSON_value(opt, "action", string));
        break;
    }

    if (!strcmp(action, "save")) {
        uci_add("router.vpn.timeout=%s", timeout);
    } else {
        LOG("%s is unknow", action);
    }

    free(timeout);
    free(action);
}


/* {"network":"openvpn_setting", "netmask" : "XX", "client_to_client": "XX", "duplicate_cn": "XX", "topology" : "XX", "defaultroute": "XX", "target": "XX", "enabled": "XX", "ip": "XX", "port": "XX", "proto": "XX", "cipher": "XX"} */
void openvpn_settingopt_fun(int count, cJSON *opt)
{
    char buf[SIZE];
    char *target=NULL, *enabled = NULL, *ip = NULL, *port = NULL, *proto = NULL, *cipher = NULL, *netmask = NULL, *client_to_client = NULL, *duplicate_cn = NULL, *topology = NULL, *defaultroute = NULL;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 7 && count != 12) {
        LOG("count is wrong");
    }

    switch(count) {
    case 12:
        netmask = safe_trip(cJSON_value(opt, "netmask", string));
        client_to_client = safe_trip(cJSON_value(opt, "client_to_client", string));
        duplicate_cn = safe_trip(cJSON_value(opt, "duplicate_cn", string));
        topology = safe_trip(cJSON_value(opt, "topology", string));
        defaultroute = safe_trip(cJSON_value(opt, "defaultroute", string));
    case 7:
        target = safe_trip(cJSON_value(opt, "target", string));
        enabled = safe_trip(cJSON_value(opt, "enabled", string));
        ip = safe_trip(cJSON_value(opt, "ip", string));
        port = safe_trip(cJSON_value(opt, "port", string));
        proto = safe_trip(cJSON_value(opt, "proto", string));
        cipher = safe_trip(cJSON_value(opt, "cipher", string));
        break;
    }

    uci_add("openvpn.%s.enabled=%s", target, enabled);
    uci_add("openvpn.%s.proto=%s", target, proto);
    uci_add("openvpn.%s.cipher=%s", target, cipher);

    if (!strcmp(target, "client")) {
        uci_add("openvpn.%s.remote=%s %s", target, ip, port);
    } else if (!strcasecmp(target, "server")) {
        uci_add("openvpn.server.port=%s", port);
        uci_add("openvpn.server.server=%s %s", ip, netmask);
        uci_add("openvpn.server.client_to_client=%s", client_to_client);
        uci_add("openvpn.server.duplicate_cn=%s", duplicate_cn);
        uci_add("openvpn.server.topology=%s", topology);

        int flag = 0;
        if (!uci_get_no_exit("openvpn.server.push", buf, SIZE)) {  //uci get success
            if (!strstr(buf, "redirect-gateway")) {
                flag = 1;  //has the value
            }
        }

        if (!strcasecmp(defaultroute, "YES") && 0 == flag) {
            uci_sim_add_list("openvpn.server.push=%s %s", "redirect-gateway", "def1");
        } else if (1 == flag){
            uci_sim_del_list("openvpn.server.push=%s %s", "redirect-gateway", "def1");
        }
    } else {
         LOG("%s is unknow", target);
    }

    uci_add("router.vpn.changed=1");
    RETURN_MSG(0, "config update success!");

    free(netmask);
    free(client_to_client);
    free(duplicate_cn);
    free(topology);
    free(defaultroute);
    free(target);
    free(enabled);
    free(ip);
    free(port);
    free(proto);
    free(cipher);
}


/* {"network":"ipsec", "ip" : "XX", "psk": "XX", "level": 1, "action": "XX"} */
void ipsecopt_fun(int count, cJSON *opt)
{
    char cmd[SIZE];
    char *ip = NULL, *psk = NULL, *action = NULL;
    int level = -1;
    FILE *fd;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 5 && count != 4 && count != 3) {
        LOG("count is wrong");
    }

    switch(count) {
    case 5:
        psk = cJSON_value(opt, "psk", string);
    case 4:
        level = cJSON_value(opt, "level", int);
    case 3:
        ip = safe_trip(cJSON_value(opt, "ip", string));
        action = safe_trip(cJSON_value(opt, "action", string));
        break;
    }

    /*if psk == "NULL", will write '\0' to the file*/
    if (psk && !strcasecmp(psk, "NULL")) {
        free(psk);
        psk = strdup("");
    }

    if (!strcmp(action, "get")) {
        sprintf(cmd, "sed -n \"s/%s.*\\\"\\(.*\\)\\\"/\\1/p\" /etc/ipsec.secrets", ip);
        system(cmd);

    } else if (!strcmp(action, "edit")) {
        sprintf(cmd, "sed -i \"s/\\(%s.*\\\"\\).*\\\"/\\1%s\\\"/\" /etc/ipsec.secrets", ip, psk);
        cmdcall_no_output(cmd);

        cmdcall_no_output("ipsec rereadsecrets &&"
                            "ipsec reload");

    } else if (!strcmp(action, "add")) {
        if ((fd = fopen("/etc/ipsec.secrets", "a")) == NULL) {
            LOG("fopen /etc/ipsec.secrets fail");
        }
        fprintf(fd, "%s : PSK \"%s\"\n", ip, psk);
        fclose(fd);

        if ((fd = fopen("/etc/ipsec.conf", "a")) == NULL) {
            LOG("fopen /etc/ipsec.conf fail");
        }

        fprintf(fd, "conn vpn%d\n", level);
        fprintf(fd, "\tauthby=psk\n");
        fprintf(fd, "\trekey=yes\n");
        fprintf(fd, "\ttype=transport\n");
        fprintf(fd, "\tkeyexchange=ikev1\n");
        fprintf(fd, "\tkeyingtries=3\n");
        fprintf(fd, "\tleft=%%defaultroute\n");
        fprintf(fd, "\tleftprotoport=17/1701\n");
        fprintf(fd, "\tright=%s\n", ip);
        fprintf(fd, "\trightprotoport=17/1701\n");
        fprintf(fd, "\tesp=aes256-sha1,3des-sha1\n");
        fprintf(fd, "\tike=aes256-sha1-modp1024,3des-sha1-modp1024\n");
        fprintf(fd, "\tauto=add\n");
        fclose(fd);

        cmdcall_no_output("ipsec rereadsecrets &&"
                        "ipsec reload");

    } else if (!strcmp(action, "del")) {
        sprintf(cmd, "ipsec down vpn%d;"
                    "sed -i \"/%s/d\" /etc/ipsec.secrets;"
                    "sed -i \"/^conn vpn%d\\$/,+13d\" /etc/ipsec.conf", level, ip, level);
        cmdcall_no_output(cmd);

        cmdcall_no_output("ipsec rereadsecrets &&"
                        "ipsec reload");

    } else {
        LOG("%s is unknow", action);
    }

    free(ip);
    free(psk);
    free(action);
}


/* {"network":"user", "password": "XX","action": "XX"} */
void useropt_fun(int count, cJSON *opt)
{
    char buf[SIZE];
    char *password = NULL, *action = NULL;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 3 && count != 2) {
        LOG("count is wrong");
    }

    switch(count) {
    case 3:
        password = cJSON_value(opt, "password", string);
    case 2:
        action = safe_trip(cJSON_value(opt, "action", string));
        break;
    }

    if (!strcmp(action, "set")) {
        uci_add("router.system.password=%s", password);
    } else if (!strcmp(action, "get")) {
        uci_get("router.system.password", buf, SIZE);
        RETURN_MSG(0, "password=%s",  buf);
    } else {
        LOG("%s is unknow", action);
    }

    free(password);
    free(action);

    return;
}


/* {"network":"apcheck", "label": "XX", "device": "XX","action": "XX"} */
void apcheckopt_fun(int count, cJSON *opt)
{
    int res;
    char cmd[SIZE], buf[SIZE];
    char *device = NULL, *label = NULL, *action = NULL;

    if (opt == NULL) {
        LOG("the point opt is NULL");
    }
    if (count != 4 && count != 3) {
        LOG("count is wrong");
    }

    switch(count) {
    case 4:
        label = safe_trip(cJSON_value(opt, "label", string));
    case 3:
        device = safe_trip(cJSON_value(opt, "device", string));
        action = safe_trip(cJSON_value(opt, "action", string));
        break;
    }
   
    int status = 0;

    /*add*/
    if (!strcmp(action, "add")) {
        sprintf(cmd, "uci get wireless.%s", device);
        res = cmdcall_no_output(cmd);
        if (res) {
            status = 1;
            RETURN_MSG(1, "device not found!");
        } else {
            res = uci_get_no_exit("wireless.%s.disabled", buf, SIZE, device);
            if (res == 0 && atoi(buf) != 0) {
                status = 2;
                RETURN_MSG(2, "device has disconnected!");
            }
        }
    } 
    /*del*/
    else if (!strcmp(action, "del")) {
        sprintf(cmd, "uci show wireless.%s", label);
        res = cmdcall_no_output(cmd);
        if (res) {
            status = 3;
            RETURN_MSG(3, "config not found!");
        }
    }
    else {
        sprintf(cmd, "grep -c \"option device '%s'\" /etc/config/wireless", device);
        getCmdResult(cmd, buf, SIZE);
        if (atoi(buf) > 8) {
            status = 5;
            RETURN_MSG(5, "ap&sta limit 8 per device!");

        } else if ( ! (res = uci_get_no_exit("wireless.%s", NULL, 0, label))) {
            status = 4;
            RETURN_MSG(4, "config name has existed!");
        }
    }

    if (status == 0) {
        RETURN_MSG(0, "success");
    }

    free(action);
    free(device);
    free(label);

    return;
}


int exec_config_editor(char *cmd, char *buffer, int len)
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
        
        int res = config_editor_main(cmd);

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


int config_editor_main(char const *argvstr)
{
    assert(argvstr != NULL);

    cJSON *json;
    char *network = NULL;

    if(!(json = cJSON_Parse(argvstr)))
        LOG("Error before:[%s]",cJSON_GetErrorPtr());

    int count = cJSON_GetArraySize(json);
    
    network = safe_trip(cJSON_value(json, "network", string));

    switch (get_network_type(network)) {
    case wifi:
        wifiopt_fun(count, json);
        break;
    case dhcp:
        dhcpopt_fun(count, json);
        break;
    case static_type:
        staticopt_fun(count, json);
        break;
    case mobile:
        mobileopt_fun(count, json);
        break;
    case lan:
        lanopt_fun(count, json);
        break;
    case ap:
        apopt_fun(count, json);
        break;
    case vpn:
        vpnopt_fun(count, json);
        break;
    case vpn_setting:
        vpn_settingopt_fun(count, json);
        break;
    case openvpn_setting:
        openvpn_settingopt_fun(count, json);
        break;
    case ipsec:
        ipsecopt_fun(count, json);
        break;
    case user:
        useropt_fun(count, json);
        break;
    case apcheck:
        apcheckopt_fun(count, json);
        break;
    default:
        LOG("json network argv is wrong");
    }

    cJSON_Delete(json);
    free(network);
    
    return 0;
}


