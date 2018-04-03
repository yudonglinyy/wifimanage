#include <stdio.h>
#include "log.h"
#include "cmdcall.h"
#include "wifitype.h"

#define SIZE 1024

int vpn_start(VPN_INFO *vinfo)
{
	if (strcmp(pinfo->proto, "pptp")) {
		vpn_start_pptp(vinfo);
	} else if (strcmp(pinfo->proto, "l2tp")) {
		vpn_start_l2tp(vinfo);
	} else {
		LOG("%s is unknow", %pinfo->proto);
	}

	return 0;
}

void vpn_start_pptp(VPN_INFO *vinfo)
{
	int res;
	char cmd[SIZE];

	sprintf(cmd, "/usr/sbin/pppd nodetach ipparam vpn%d ifname pptp-vpn%d +ipv6 set AUTOIPV6=1 nodefaultroute usepeerdns maxfail 1 user %s password %s ip-up-script /lib/netifd/ppp-up ipv6-up-script /lib/netifd/ppp-up ip-down-script /lib/netifd/ppp-down ipv6-down-script /lib/netifd/ppp-down mtu %d mru %d plugin pptp.so pptp_server %s file /etc/ppp/options.pptp debug &",
		pinfo->level, pinfo->level, pinfo->username, pinfo->password, pinfo->mtu, pinfo->mtu, pinfo->server);

	if (cmdcall_no_output(cmd)) {
		LOG("exec pppd fail");
	}

	return 0
}

int vpn_start_l2tp(VPN_INFO *vinfo)
{
	char buf[SIZE];
	int fd;

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
				"user \"%s\" password \"%d\"\n"
				"mtu %d mru %d\n", pinfo->level, pinfo->level, pinfo->username, pinfo->password, pinfo->mtu, pinfo->mtu);

	fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd == -1) {
		LOG("open %s error", outfile);
	}

	if (write(fd, buf, strlen(buf)) != strlen(buf)) {
		LOG("couldn't write whole buffer to %s", path);
	}

	if (close(fd) == -1) {
		LOG("close fd fail");
	}

	res = cmdcall_no_output(cmd, "xl2tpd-control add l2tp-vpn%d pppoptfile=%s lns=%s &&
  						xl2tpd-control connect l2tp-vpn%d &", pinfo->level, outfile, pinfo->server, vpn_level);

	if (res) {
		LOG("[%s] exec fail");
	}

	return 0;
}
