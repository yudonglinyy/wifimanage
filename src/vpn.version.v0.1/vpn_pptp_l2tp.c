#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "log.h"
#include "cmdcall.h"
#include "ucicmd.h"
#include "wifitype.h"
#include "vpn_update.h"
#include "f_operation.h"
#include "getiponline.h"
#include "config_editor_main.h"
#include "vpn_pptp_l2tp.h"

#include <time.h>

#define SIZE 1024
#define MAX_LEVEL 20
#define IP_SIZE 16

static int g_fail_times = 0;


int main(int argc, char const *argv[])
{
    char model[100];

    if (exec_set_gateway(NULL, 0)) {
        LOG("exec setgateway fail");
    }

    while (1) {
        uci_get("router.vpn.model", model, sizeof(model));

        reseting(model);
        loop_mon(model);

        sleep(5);
    }

    return 0;
}


/*reset router.vpn.change, g_fail_times, default gateway*/
void reseting(char *model)
{
    char gateway[16];
    int vpn_level;
    char cmd[SIZE], buf[SIZE];

    /*reset router.vpn.change*/
    uci_get("router.vpn.changed", buf, SIZE);
    if (!strcmp(buf, "1")) {
        cmdcall_no_output("killall pppd");
        uci_add("router.vpn.changed=0");
    }

    /*if auto mode, will update and reset the error num of time*/
    if (!strcmp(model, "auto")) {
        vpn_update();
        g_fail_times = 0;
    }

    uci_get("router.vpn.level", buf, SIZE);

    /*when use vpn, level > 0, so del the default gateway */
    if ((vpn_level = atoi(buf)) > 0) {
        cmdcall_no_output("route del default");
    }


    if (filecontent("/tmp/tmp_gateway", gateway, SIZE)) {
        LOG("filecontent fail");
    }

    /*reset the default gateway*/
    sprintf(cmd, "route | grep 'default' | grep -q %s", gateway);
    if (cmdcall_no_output(cmd)) {
        sprintf(cmd, "route add default gateway %s", gateway);
        cmdcall_no_output(cmd);
    }
}


int exec_set_gateway(char *buffer, int len)
{
    int res;
    int pipefd[2];
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

        res = execlp("setgateway", "setgateway", NULL);
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


bool conn_remote_server()
{
    char vpn1_server[SIZE];
    char gateway[IP_SIZE];
    char cmd[SIZE];

    uci_get("network.vpn1.server", vpn1_server, SIZE);
    sprintf(cmd, "route | grep -q %s && route del -net %s netmask 255.255.255.255", vpn1_server, vpn1_server);
    cmdcall_no_output(cmd);
    if (filecontent("/tmp/tmp_gateway", gateway, IP_SIZE)) {
        LOG("/tmp/tmp_gateway filecontent fail");
    }

    sprintf(cmd, "route add -net %s netmask 255.255.255.255 gateway %s", vpn1_server, gateway);
    cmdcall_no_output(cmd);

    /*'-u' will try to conn udp port*/
    sprintf(cmd, "nc -w 5 -zu %s 1701", vpn1_server);

    /*The max fail times for connecting the remote server is 1, timeout is 5, success will true*/
    if (conn_tcpport(vpn1_server, 1723, 5) || !cmdcall_no_output(cmd)) {
        return true;
    }
    return false;
}


bool conn_tcpport(const char *serverIP, int serverport, int timeout)
{
    assert(serverIP != NULL && *serverIP != '\0');
    int sockfd;
    struct hostent *h;
    struct sockaddr_in server_addr;

    if (((h=gethostbyname(serverIP)) == NULL)) {
        LOG("gethostbyname %s fail", serverIP);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(serverport);
    server_addr.sin_addr = *((struct in_addr *)h->h_addr);


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG("client socket failure");
    }

    bool res = true;
    int opt = 1;
    //set non-blocking
    if (ioctl(sockfd, FIONBIO, &opt) < 0) {
        close(sockfd);
        LOG("ioctl");
    }

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
        if (errno == EINPROGRESS) {
            int error;
            int len = sizeof(int);
            struct timeval tv_timeout;
            fd_set writefds;

            tv_timeout.tv_sec  = timeout;
            tv_timeout.tv_usec = 0;

            FD_ZERO(&writefds);
            FD_SET(sockfd, &writefds);

            if(select(sockfd + 1, NULL, &writefds, NULL, &tv_timeout) > 0) {
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
                if(error != 0) {
                    res = false;
                }
            } else { //timeout or select error
                res = false;
            }
        } else {
            res = false;
        }
    }

    close(sockfd);
    return res;
}


/*times > 3 , will update vpn, if update is success, return 0*/
int add_fail_times()
{
    g_fail_times++;

    if (g_fail_times >= 3) {
        return vpn_update();
    }

    return 0;
}


int loop_mon(char *model)
{
    int enable, total_level;
    char buf[SIZE];

    while(1) {
        uci_get("router.vpn.enable", buf, SIZE);
        enable = atoi(buf);
        if (enable == 0) {
            sleep(3);
            continue;
        }

        uci_get("router.vpn.level", buf, SIZE);
        if ((total_level = atoi(buf)) <= 0 ) {
            return 1;
        }


        if ( !conn_remote_server() && strcmp(model, "auto") == 0) {
            if (add_fail_times()) {
                return 1;
            } else {
                sleep(3);
                continue;
            }
        }

        if (ck_all_vpn(total_level)) {
            return 1;
        }

        sleep(6);

    }

    return 0;
}


int ck_all_vpn(int total_level)
{
    int level_num = 1, min_changed_level = 0, res;
    char buf[SIZE], cmd[SIZE];
    char protoArry [MAX_LEVEL][100];

    /* check vpn setting, if changed, will kill the vpn level form min_changed_level to end */
    if (ck_vpn_set_change()) {

        uci_get("router.vpn.min_changed_level", buf, SIZE);
        min_changed_level = atoi(buf);

        kill_all_vpn_pid(protoArry, min_changed_level, total_level);

        uci_add("router.vpn.changed=0");
    }


    /*get the proto of all levels, save them at protoArry[] */
    for (int i=1; i<=total_level; i++) {
        sprintf(cmd, "network.vpn%d.proto", i);
        uci_get(cmd, protoArry[level_num], 100);
    }


    /*check all level vpn online*/
    for (level_num=1; level_num<=total_level; level_num++) {

        res = ck_cur_vpn_online(protoArry[level_num], level_num, total_level);

        /*when current vpn is down,res is 1. and then try to dial. if the dial result is fail, will kill all levels vpn.*/
        if (res) {
            kill_all_vpn_pid(protoArry, level_num, total_level);

            // check peer vpn is online
            res = ck_peer_vpn_online(protoArry, level_num, total_level);
            if (res) {
                return 1;
            }

            if (dial_vpn(level_num)) {
                kill_all_vpn_pid(protoArry, 1, level_num);
                return 1;
            }
        }

        sleep(1);
    }

    return 0;
}


/*no change will return 0, if not, return 1*/
int ck_vpn_set_change()
{
    char buf[SIZE];

    uci_get("router.vpn.changed", buf, SIZE);

    return atoi(buf);
}


int ck_peer_vpn_online(char (*protoArry)[100], int total_level, int level_num)
{
    int flag = 0;
    int peer_level;
    char device[100];

    if (level_num < 2) {
        return 0;
    }

    for (peer_level=1; peer_level<level_num; peer_level++) {
        sprintf(device, "%s-vpn%d", protoArry[peer_level], peer_level);

        /*if dev isn't online, flag is 1 and will break */
        if (!isDevHasIp(device)) {
            flag = 1;
            break;
        }
    }

    if (flag) {
        kill_all_vpn_pid(protoArry, peer_level, total_level);
        return 1;
    } else {
        return 0;
    }
}


int ck_cur_vpn_online(char *proto, int level_num, int total_level)
{
    char device[20];
    sprintf(device, "%s-vpn%d", proto, level_num);

    if (!isDevHasIp(device)) {
        return 1;
    }

    /* when the level is the lastest layer, add the device as the default gateway. */
    /* if not, add the next vpn server as the gateway */
    if (level_num == total_level) {
        ck_default_gateway(proto, level_num);
    } else {
        ck_vpn_gateway(proto, level_num);
    }

    return 0;
}


int ck_vpn_gateway(char *proto, int level_num)
{
    char cmd[SIZE];
    char device[100];
    char vpn_gateway[16], next_vpn[16];

    uci_get("network.vpn%d.server", next_vpn, SIZE, level_num+1);

    sprintf(device, "%s-vpn%d", proto, level_num);
    sprintf(cmd, "route | grep %s | grep UH | cut -d" " -f1", device);

    /*if result is null, will add the next_vpn as the gateway */
    if (getCmdResult_r(cmd, vpn_gateway, SIZE) == NULL) {
        strcpy(vpn_gateway, next_vpn);
        sprintf(cmd, "route add %s dev %s", next_vpn, device);
        cmdcall_no_output(cmd);
    }

    /*when vpn_gatewat isn't nextvpn, will del the old and add the next_vpn as the gateway*/
    if (strcmp(vpn_gateway, next_vpn)) {
        sprintf(cmd, "route del %s dev %s;"
                     "route add %s dev %s", vpn_gateway, device, next_vpn, device);
        cmdcall_no_output(cmd);
    }

    return 0;
}


int ck_default_gateway(char *proto, int level_num)
{
    char cmd[SIZE];
    char device[20];
    char vpn_gateway[20];

    sprintf(device, "%s-vpn%d", proto, level_num);

    sprintf(cmd, "route | grep %s | grep -q default", device);


    if (getCmdResult_r(cmd, vpn_gateway, SIZE) != NULL) {
        if (!strcmp(vpn_gateway, device)) {
            return 0;
        } else {
            sprintf(cmd, "route del default");
            cmdcall_no_output(cmd);
        }
    }

    sprintf(cmd, "route add default dev %s", device);
    cmdcall_no_output(cmd);
    sprintf(cmd, "iptables -t filter -I FORWARD -o %s -j ACCEPT  &&"
                "iptables -S -t nat|grep -q '-A POSTROUTING -o %s -j MASQUERADE' || iptables -t nat -A POSTROUTING -o %s -j MASQUERADE",
                    device, device, device);
    cmdcall_no_output(cmd);

    return 0;
}


int dial_vpn(int level_num)
{
    char model[100];
    char cmd[SIZE];

    uci_get("router.vpn.model", model, sizeof(model));
    if (!strcmp(model, "auto") && add_fail_times()) {
        return 1;
    }

    sprintf(cmd, "uci show network.vpn%d", level_num);
    if (cmdcall_no_output(cmd)) {
        return 1;
    }


    VPN_INFO vinfo;

    vinfo.level = level_num;
    vinfo.mtu = 1380 - (50 * level_num);
    sprintf(cmd, "network.vpn%d.proto", level_num);
    uci_get(cmd, vinfo.proto, sizeof(vinfo.proto));
    sprintf(cmd, "network.vpn%d.server", level_num);
    uci_get(cmd, vinfo.server, sizeof(vinfo.server));
    sprintf(cmd, "network.vpn%d.username", level_num);
    uci_get(cmd, vinfo.username, sizeof(vinfo.username));
    sprintf(cmd, "network.vpn%d.password", level_num);
    uci_get(cmd, vinfo.password, sizeof(vinfo.password));


    if (conn_remote_server()) {
        vpn_start(&vinfo);
    } else {
        return 1;
    }

    return 0;
}


void kill_all_vpn_pid(char (*protoArry)[100], int begin, int end)
{
    char cmd[SIZE];

    for (; begin<=end; begin++) {
        sprintf(cmd, "ifconfig %s-vpn%d", protoArry[begin], begin);     //test
        if (cmdcall_no_output(cmd) == 0) {
            kill_vpn_pid(protoArry[begin], begin);
        }
    }
    return;
}


int vpn_start(VPN_INFO *pinfo)
{
    if (!strcmp(pinfo->proto, "pptp")) {
        vpn_start_pptp(pinfo);
    } else if (!strcmp(pinfo->proto, "l2tp")) {
        vpn_start_l2tp(pinfo);
    } else {
        LOG("%s is unknow", pinfo->proto);
    }

    return 0;
}

int vpn_start_pptp(VPN_INFO *pinfo)
{
    char cmd[SIZE];

    sprintf(cmd, "/usr/sbin/pppd nodetach ipparam vpn%d ifname pptp-vpn%d +ipv6 set AUTOIPV6=1 nodefaultroute usepeerdns maxfail 1 user %s password %s ip-up-script /lib/netifd/ppp-up ipv6-up-script /lib/netifd/ppp-up ip-down-script /lib/netifd/ppp-down ipv6-down-script /lib/netifd/ppp-down mtu %d mru %d plugin pptp.so pptp_server %s file /etc/ppp/options.pptp debug &",
        pinfo->level, pinfo->level, pinfo->username, pinfo->password, pinfo->mtu, pinfo->mtu, pinfo->server);
    if (cmdcall_no_output(cmd)) {
        RETURN_MSG(1, "exec pppd fail");
        return 1;
    }

    return 0;
}

int vpn_start_l2tp(VPN_INFO *pinfo)
{
    char cmd[SIZE], buf[SIZE];
    int fd;

    const char *filepath = "/tmp/l2tp";

    if (access(filepath, F_OK)) {
        mkdir(filepath, 0777);
    }

    char outfile[200];
    sprintf(outfile, "/tmp/l2tp/options.vpn%d", pinfo->level);

    sprintf(buf, "usepeerdns\n"
                "nodefaultroute\n"
                "ipparam \"vpn%d\"\n"
                "ifname \"l2tp-vpn%d\"\n"
                "ip-up-script /lib/netifd/ppp-up\n"
                "ipv6-up-script /lib/netifd/ppp-up\n"
                "ip-down-script /lib/netifd/ppp-down\n"
                "ipv6-down-script /lib/netifd/ppp-down\n"
                "# Don't wait for LCP term responses; exit immediately when killed.\n"
                "lcp-max-terminate 0\n"
                "user \"%s\" password \"%s\"\n"
                "mtu %d mru %d\n", pinfo->level, pinfo->level, pinfo->username, pinfo->password, pinfo->mtu, pinfo->mtu);

    fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd == -1) {
        LOG("open %s error", outfile);
    }

    if (write(fd, buf, strlen(buf)) != strlen(buf)) {
        LOG("couldn't write whole buffer to %s", outfile);
    }

    if (close(fd) == -1) {
        LOG("close fd fail");
    }

    sprintf(cmd, "xl2tpd-control add l2tp-vpn%d pppoptfile=%s lns=%s &&"
                "xl2tpd-control connect l2tp-vpn%d", pinfo->level, outfile, pinfo->server, pinfo->level);
    if (cmdcall_no_output(cmd)) {
        // LOG("[%s] exec fail", cmd);
        RETURN_MSG(1,"[%s] exec fail", cmd);
        return 1;
    }

    return 0;
}



