#ifndef _VPN_PPTP_L2TP_H
#define _VPN_PPTP_L2TP_H


void reseting(char *model);
int exec_set_gateway(char *buffer, int len);
bool conn_remote_server();
bool conn_tcpport(const char *serverIP, int serverport, int timeout);
int add_fail_times();
int loop_mon(char *model);
int ck_all_vpn(int total_level);
int ck_vpn_set_change();
int ck_peer_vpn_online(char (*protoArry)[100], int total_level, int level_num);
int ck_cur_vpn_online(char *proto, int level_num, int total_level);
int ck_vpn_gateway(char *proto, int level_num);
int ck_default_gateway(char *proto, int level_num);
int dial_vpn(int level_num);
void kill_vpn_pid(char *proto, int level);
void kill_all_vpn_pid(char (*protoArry)[100], int begin, int end);
int vpn_start(VPN_INFO *pinfo);
int vpn_start_pptp(VPN_INFO *pinfo);
int vpn_start_l2tp(VPN_INFO *pinfo);


#endif