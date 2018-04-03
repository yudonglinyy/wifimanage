#ifndef _CONFIG_EDITOR_MAIN_H
#define _CONFIG_EDITOR_MAIN_H

int config_editor_main(char const *argvstr);
void kill_vpn_pid(char *proto, int level);
int exec_config_editor(char *cmd, char *buffer, int len);

#endif

