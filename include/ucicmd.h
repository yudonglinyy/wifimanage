#ifndef _UCICMD_H
#define _UCICMD_H

#include "uci.h"

typedef int (*UCI_OPERFUNC)(struct uci_context *, struct uci_ptr *);

int uci_operation(char *str, UCI_OPERFUNC oper_func, const char *dir);
int uci_get(const char *cmd, char *str, int len, ...);
int uci_get_no_exit(const char *cmd, char *result, int len, ...);
int uci_add(const char *cmd, ...);
int uci_add_confdir(const char *dir, const char *cmd, ...);
int uci_del(const char *cmd, ...);
int uci_del_confdir(const char *dir, const char *cmd, ...);
int uci_sim_add_list(const char *cmd, ...);
int uci_sim_del_list(const char *cmd, ...);
int uci_sim_rename(const char *cmd, ...);

#endif