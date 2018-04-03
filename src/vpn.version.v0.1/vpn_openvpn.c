#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "log.h"
#include "strip.h"
#include "cmdcall.h"
#include "ucicmd.h"
#include "getiponline.h"
#include "f_operation.h"

#define SIZE 1024
#define IP_SIZE  16

void loop_openvpn();
int exec_set_gateway(char *buffer, int len);
int get_vpn_gateway(const char *dev, char *vpn_gateway, int len);
void add_gateway_route();


int main(int argc, char const *argv[])
{
	exec_set_gateway(NULL, 0);
	cmdcall_no_output("/etc/init.d/openvpn restart");
	loop_openvpn();
	return 0;
}


void loop_openvpn()
{
	int res;
	int errnum = 0;
	char cmd[SIZE];
	char buf[SIZE];

	char vpn_gateway[IP_SIZE];
	const char *dev = "tun1";  //client->tun1 , server->tun0

	while (1) {
		add_gateway_route();

		uci_get("router.vpn.changed", buf, SIZE);

		if (atoi(buf) > 0) {
	      cmdcall_no_output("/etc/init.d/openvpn restart");
	      uci_add("router.vpn.changed=0");
		}

		res = get_vpn_gateway(dev, vpn_gateway, IP_SIZE);

		sprintf(cmd, "ping -w 5 -c 2 %s", vpn_gateway);

		if (res || strstr("*", vpn_gateway) || cmdcall_no_output(cmd)) {
			errnum++;
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
		return WIFEXITED(status);  //if fail exit will return zero
	}

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

void add_gateway_route()
{
	char buf[SIZE], cmd[SIZE], routebuf[SIZE*2];
	char gateway[IP_SIZE];
	char remoteip[IP_SIZE];
	int res;
	const char *file = "/tmp/tmp_gateway";

	filecontent(file, gateway, IP_SIZE);

	uci_get("openvpn.client.remote", buf, SIZE);
	strtok(buf, " ");
	strcpy(remoteip, buf);

	getCmdResult("route -n", routebuf, SIZE*2);
	if (!strstr(routebuf, remoteip)) {
		sprintf(cmd, "route add -net %s netmask 255.255.255.255 gw %s", remoteip, gateway);
		cmdcall_no_output(cmd);
	}

	res = uci_get_no_exit("router.vpn.defaultroute", buf, SIZE);

	/*when defaultroute == 1, all network data throuth the vpn gateway*/
	if (res == 1 && atoi(buf) == 1) {
		sprintf(cmd, "route|grep -v 'grep'|grep 'default'|grep -q %s && route del default gateway %s", gateway, gateway);
		cmdcall_no_output(cmd);
		cmdcall_no_output("route |grep -q tun0 && (route|grep 'default'|grep -q tun0 || route add default dev tun0)");
	}

	return;
}
