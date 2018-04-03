#ifndef _NETTEST_H
#define _NETTEST_H

#include <stdbool.h>

bool conn_tcpport(const char *serverIP, int serverport, int timeout);
bool conn_udpport(const char *serverIP, int serverport, int timeout);
bool test_conn_vpn_server(char *ip);

#endif