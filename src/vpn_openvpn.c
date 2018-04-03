#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "log.h"
#include "strip.h"
#include "cmdcall.h"
#include "vpn_auth.h"
#include "ucicmd.h"
#include "getiponline.h"
#include "getgateway.h"
#include "curlhttp.h"

#define SIZE 1024
#define IP_SIZE  16

typedef unsigned long ulong;

void loop_openvpn();
int get_vpn_gateway(const char *dev, char *vpn_gateway, int len);


void display_usage()
{
    fprintf(stderr, "usage: userauth [option]\n"
                    "option:\n"
                    "\t -u, --url: request cert file api\n"
                    "\t -h, --help\n");
    exit(EXIT_FAILURE);
}


int loop_conn_internet(const char *gateway, const char *token_api, const char *upfile)
{
	char cmd[SIZE];
	const char *testip = "google.com";
	
	while(1) {
		/*if conn gateway fail, will return 1*/
		sprintf(cmd, "ping -W 5 -c 2 %s", gateway);
		if (cmdcall_no_output(cmd)) {
			return 1;
		}

		/*will receive a compressed package containing 4 certificate files*/
		request_vpn_cert(token_api);

		/*update all auth info about openvpn, and then write them to the upfile*/
		get_vpndail_data(upfile);

		/*loop ping google*/
		sprintf(cmd, "ping -W 5 -c 2 %s", testip);
		while (!system(cmd)) {
			sleep(5);
		}
	}
}


void loop_openvpn(const char *token_api, const char *upfile)
{
	int res;
	int errnum = 0;

	char vpn_gateway[IP_SIZE];
	const char *dev = "tun1";  //client->tun1 , server->tun0

	while (1) {
		res = get_vpn_gateway(dev, vpn_gateway, IP_SIZE);

		if (res || strstr("*", vpn_gateway)) {
			errnum++;
		} else {
			/*If the function returns, the connection failed, errnum+1*/
			loop_conn_internet(vpn_gateway, token_api, upfile);
		}

		if (errnum == 5) {
			system("/etc/init.d/openvpn restart");
			errnum = 0;
			sleep(5);
		}

		sleep(3);
	}

	return;
}


int get_vpn_gateway(const char *dev, char *vpn_gateway, int len)
{
	char buf[SIZE], cmd[SIZE];
	char testip[IP_SIZE];
	struct in_addr attr1;
	ulong l1, l2;

	if (!get_ip(dev, testip)) {
		return 1;
	}

	l1 = inet_addr(testip);
	l2 = inet_addr("0.0.0.1");

	if (l1 % 256 == 254 || l1 % 256 == 255) {
		l1-=l2;
	} else {
		l1+=l2;
	}

	memcpy(&attr1, &l1, 4);
	strcpy(testip, inet_ntoa(attr1));

	sprintf(cmd, "traceroute -m 1 %s |grep '^ '| awk '{print $2}'", testip);

	getCmdResult(cmd, buf, SIZE);

	strip(buf, buf, len);

 	strcpy(vpn_gateway, buf);

	return 0;
}


void remove_gateway_route()
{
	char cmd[SIZE], ifname[100];
	uci_get("network.wan.ifname", ifname, sizeof(ifname));
	snprintf(cmd, SIZE, "route del default dev tun1; route add default dev %s", ifname);
	cmdcall_no_output(cmd);
	return;
}

void add_gateway_route()
{
	char buf[SIZE], cmd[SIZE], routebuf[SIZE*2];
	char *gateway;
	char remoteip[IP_SIZE];
	gateway = getgateway();

	uci_get("openvpn.client.remote", buf, SIZE);
	strtok(buf, " ");
	strcpy(remoteip, buf);

	getCmdResult("route -n", routebuf, SIZE*2);
	if (!strstr(routebuf, remoteip)) {
		sprintf(cmd, "route add -host %s gw %s && route add default dev tun1", remoteip, gateway);
		cmdcall_no_output(cmd);
	}

	free(gateway);
	return;
}


void set_realip()
{
	char ip[20];
	const char *ip_url1 = "http://www.3322.org/dyndns/getip";
	const char *ip_url2 = "http://ipv4.icanhazip.com";
	const char *ip_url3 = "http://ipinfo.io/ip";

	if (curlhttp_string_timeout(ip_url1, NULL, "GET", ip, sizeof(ip), 2)
	 		&& curlhttp_string_timeout(ip_url2, NULL, "GET", ip, sizeof(ip), 2)
	 		&& curlhttp_string_timeout(ip_url3, NULL, "GET", ip, sizeof(ip), 5) ) {
        strncpy(ip, "0.0.0.0", sizeof(ip)-1);
        ip[8] = '\0';
    }

    strip(ip, ip, sizeof(ip));

    uci_add("router.auth.realip=%s", ip);

	return;	 
}

int main(int argc, char* const *argv)
{
	int ch;
    int option_index = 0;
	char token_api[SIZE] = "http://ip/auth/api/v1/token";
	const char *upfile = "/etc/openvpn/up";

	if (argc != 3 && argc != 2) {
        display_usage();
    }

	static struct option long_options[] =
    {
        {"url", required_argument, NULL,'u'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0},
    }; 

    while ((ch = getopt_long(argc, argv, "u:h:", long_options, &option_index )) != -1)
    {
        switch (ch) {
        case 'u':
            strcpy(token_api, optarg);
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
	
	remove_gateway_route();
	set_realip();
	add_gateway_route();
	system("/etc/init.d/openvpn restart");
	loop_openvpn(token_api, upfile);

	return 0;
}

