#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>

#include "log.h"
#include "cmdcall.h"
#include "getiponline.h"

bool getIpOnline1111111(const char *wlanstr)
{
    char cmd[100];
    int res;

    sprintf(cmd, "ifconfig %s", wlanstr);
    res = cmdcall_no_output(cmd);
    if (res) {
        return false;
    } else {
        return true;
    }
}


#define IP_SIZE 16

bool getIpOnline(const char *eth_inf)
{
    int sd;
    // struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        LOG("socket error!");
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        close(sd);
        return false;
    }

    // memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    // snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));
    // printf("test:%s\n", inet_ntoa(sin.sin_addr));

    close(sd);
    return true;
}


int get_ip(const char *dev, char *ip)
{
    return get_addr(dev, ip, SIOCGIFADDR);
}

int get_if(const char *dev, char *ip)
{
    return get_addr(dev, ip, SIOCGIFFLAGS);
}

int get_ip_netmask(const char *dev, char *ip)
{
    return get_addr(dev, ip, SIOCGIFNETMASK);
}

int get_mac(const char *dev, char *addr)
{
    return get_addr(dev, addr, SIOCGIFHWADDR);
}

bool get_addr(const char *dev, char *addr, int flag)
{
    int sockfd = 0;
    unsigned char buf[6];
    struct sockaddr_in *sin;
    struct ifreq ifr;

    if ((sockfd = socket(AF_PACKET,SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        LOG("socket error!");
    }

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, (sizeof(ifr.ifr_name)), "%s", dev);

    if(ioctl(sockfd, flag, &ifr) < 0 )
    {
        close(sockfd);
        LOG("ioctl %s flag(%d): unknown interface", ifr.ifr_name, flag);
    }
    close(sockfd);


    if (SIOCGIFHWADDR == flag){
        memcpy((void *)buf, (const void *)&ifr.ifr_ifru.ifru_hwaddr.sa_data, 6);
        sprintf(addr, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x", buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
        addr[18] = '\0';
        /*printf("mac address: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);*/
    } else {
        sin = (struct sockaddr_in *)&ifr.ifr_addr;
        snprintf(addr, 16, "%s", inet_ntoa(sin->sin_addr));
        addr[15] = '\0';
    }

    return true;
}

bool isDevUp(const char *dev)
{
    int sockfd = 0;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, (sizeof(ifr.ifr_name)), "%s", dev);

    if ((sockfd = socket(AF_PACKET,SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        LOG("socket error!");
    }

    if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0 )
    {
        return false;
    }
    close(sockfd);

    if (IFF_UP & (ifr.ifr_flags)) {
        return true;
    } else {
        return false;
    }
}


bool isDevHasIp(const char *dev)
{
    int res;
    int sockfd = 0;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, (sizeof(ifr.ifr_name)), "%s", dev);

    if ((sockfd = socket(AF_PACKET,SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        LOG("socket error!");
    }

    if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0 )
    {
        res = false;
    } else {
        res = true;
    }
    
    close(sockfd);
    return res;
}


int getdevlist()
{//想要获取当前网口网线插入状态，需要用到ifreq结构体，获取网卡的信息，然后socket结合网卡驱动的ioctl，就可以得到与网线插入状态相关的数据。
    int number;                             
    struct ifconf ifc;          //用来保存所有接口信息的   
    struct ifreq buf[16];       //这个结构定义在net/if.h，用来配置ip地址，激活接口，配置MTU等接口信息
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (void *)buf;

    int sockfd;
    if(-1 == (sockfd = socket(AF_PACKET,SOCK_RAW, htons(ETH_P_ALL)) ))     
    {
        perror("socket build !");
        return 1;
    } 
    if(-1 == ioctl(sockfd, SIOCGIFCONF, (char *)&ifc)) //SIOCGIFCONF用来获取所有配置接口的信息，将所获取的信息保存到ifc里。
    {
        perror("SIOCGIFCONF !");
        return 1;
    }
    
    number = ifc.ifc_len / sizeof(struct ifreq);
    printf("the interface number is %d \n",number);
    int tmp;
    for(tmp = number-1;tmp >= 0;tmp--)
    {
        printf("the interface name is %s\n",buf[tmp].ifr_name);

        /* 接口的状态信息获取 */
        ioctl(sockfd,SIOCGIFFLAGS,(char *)&buf[tmp]);
        
            if(IFF_UP & buf[tmp].ifr_flags)
                printf("%s UP\n", buf[tmp].ifr_name);
            else 
                printf("%s DOWN\n", buf[tmp].ifr_name);
       
    }
    return 0;
}
