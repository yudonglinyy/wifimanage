#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "log.h"
#include "base64.h"
#include "curl/curl.h"
#include "getiponline.h"
#include "ucicmd.h"
#include "curlhttp.h"
#include "des_aes.h"
#include "cmdcall.h"
#include "f_operation.h"
#include "strip.h"
#include "vpn_update.h"
#include "config_editor_main.h"

#define SIZE 4096


int vpn_update()
{
    char buf[SIZE], cmd[SIZE], response[SIZE];
    char data2_row[SIZE] = {0};
    char post_data[SIZE] = {0};
    char data1[SIZE] = {0}, data2[SIZE] = {0};
    char mac[18], api[100], url[SIZE];
    struct vpn_info vpn_arry[10];
    int port;

    const char *filename = "/tmp/response_data";
    const char *dev = "br-lan";

    if (filecontent("/tmp/tmp_api", api, sizeof(api))) {
        LOG("/tmp/tmp_api isn't exist");
    }
    strip(api, api, sizeof(api));

    uci_get("router.system.updateport", buf, SIZE);
    port = atoi(buf);
    sprintf(url, "http://%s:%d/RouterBox.php", api, port);

    if (!get_mac(dev, mac)) {
        LOG("get mac fail");
    }

    char method[256];
    char mode[256];
    u_char key[256];
    char ivbuf[256];
    u_char *iv = NULL;
 
    uci_get("router.vpn.updatemethod", method, sizeof(method));
    uci_get("router.vpn.updatemode", mode, sizeof(mode));
    uci_get("router.vpn.updatekey", (char *)key, sizeof(key));
    
    uci_get("router.vpn.updateiv", ivbuf, sizeof(ivbuf));
    if (!strcasecmp(ivbuf, "NULL")) {
        iv = NULL;
    } else {
        iv = (u_char *)ivbuf;
    }

    // base64_encode(data1, (u_char *)mac, strlen(mac));
    http_data_encrypt(mac, data1, method, mode, key, iv);

    vpn_update_data(data2_row, SIZE);

    // base64_encode(data2, (u_char *)data2_row, strlen(data2_row));
    http_data_encrypt(data2_row, data2, method, mode, key, iv);
    

    sprintf(post_data, "data1=%s&data2=%s", data1, data2);
    if (curlhttp(url, post_data, "POST", filename)) {
        return 1;
    }

    filecontent(filename, buf, SIZE);
    // base64_decode(buf, (u_char *)response);
    http_data_decrypt(buf, response, method, mode, key, iv);

    int total_num = 0;
    if (parse_data(response, vpn_arry, &total_num)) {
        RETURN_MSG(1, "response check error");
    }

    for (int i=0; i<total_num; i++) {
        if (*(vpn_arry[i].proto) == 0) {
            strcpy(vpn_arry[i].proto, "pptp");  //test
        }

        if (!strcmp(vpn_arry[i].proto, "l2tp")) {
            sprintf(cmd, "{\"network\":\"vpn\", \"proto\" : \"%s\", \"server\": \"%s\", \"username\": \"%s\", \"password\":\"%s\", \"mac\": \"%s\", \"psk\":\"%s\", \"level\": %d, \"action\": \"edit\"}",
                        vpn_arry[i].proto, vpn_arry[i].server, vpn_arry[i].username, vpn_arry[i].password, vpn_arry[i].mac, vpn_arry[i].psk, i+1);
        } else {
            sprintf(cmd, "{\"network\":\"vpn\", \"proto\" : \"%s\", \"server\": \"%s\", \"username\": \"%s\", \"password\":\"%s\", \"mac\": \"%s\", \"level\": %d, \"action\": \"edit\"}",
                        vpn_arry[i].proto, vpn_arry[i].server, vpn_arry[i].username, vpn_arry[i].password, vpn_arry[i].mac, i+1);
        }

        config_editor_main(cmd);
    }

    cmdcall_no_output("/etc/init.d/firewall reload");
    sprintf(buf, "router.vpn.level=%d", total_num);
    uci_add(buf);
    uci_add("router.vpn.min_changed_level=2");
    uci_add("router.vpn.changed=1");

    return 0;
}

int vpn_update_data(char *data, int size)
{
    char buf[SIZE], cmd[SIZE];
    char mac[SIZE], server[SIZE], username[SIZE],  password[SIZE];
    int vpn_num = 0, len = 0;

    if ( (vpn_num = get_vpn_num()) == 0) {
        strcpy(data, "00:00:00:00:00:00|127.0.0.1|username|password");
        return 0;
    }

    for (int i=1; i<=vpn_num; i++) {
        sprintf(cmd, "network.vpn%d.mac", i);
        uci_get(cmd, mac, SIZE);
        sprintf(cmd, "network.vpn%d.server", i);
        uci_get(cmd, server, SIZE);
        sprintf(cmd, "network.vpn%d.username", i);
        uci_get(cmd, username, SIZE);
        sprintf(cmd, "network.vpn%d.password", i);
        uci_get(cmd, password, SIZE);

        if (1 == i) {
            sprintf(buf, "%s|%s|%s|%s", mac, server, username, password);
            strcpy(data, buf);
        } else {
            sprintf(buf, "||%s|%s|%s|%s", mac, server, username, password);
            strcat(data, buf);
        }

        len+=strlen(buf);
        if (len >= size) {
            LOG("post data is too long");
        }
    }

    return 0;
}

int get_vpn_num()
{
    int num;
    char buf[SIZE];

    uci_get("router.vpn.level", buf, SIZE);
    num = atoi(buf);

    return num;
}

int parse_data(char *data, struct vpn_info *vpn_arry, int *p_level_num)
{
    char delims[] = "||";
    char delims2 = '|';
    char *result = NULL, *result2 = NULL;
    int num = 0;

    while( (result = strstr(data, delims))  != NULL )
    {
        result[0] = '\0';

        result2 = strtok(data, &delims2);
        strcpy(vpn_arry[num].mac, result2);

        result2 = strtok(NULL, &delims2);
        strcpy(vpn_arry[num].server, result2);

        result2 = strtok(NULL, &delims2);
        strcpy(vpn_arry[num].username, result2);

        result2 = strtok(NULL, &delims2);
        strcpy(vpn_arry[num].password, result2);

        memset(vpn_arry[num].proto, 0, sizeof(vpn_arry[num].proto));
        strcpy(vpn_arry[num].psk, "NULL");

        result2 = NULL;

        data = result + strlen(delims);

        num++;
    }

    result2 = strtok(data, &delims2);
    strcpy(vpn_arry[num].mac, result2);

    result2 = strtok(NULL, &delims2);
    strcpy(vpn_arry[num].server, result2);

    result2 = strtok(NULL, &delims2);
    strcpy(vpn_arry[num].username, result2);

    result2 = strtok(NULL, &delims2);
    strcpy(vpn_arry[num].password, result2);

    memset(vpn_arry[num].proto, 0, sizeof(vpn_arry[num].proto));
    strcpy(vpn_arry[num].psk, "NULL");

    result2 = NULL;

    num++;

    *p_level_num = num;

    return 0;
}


