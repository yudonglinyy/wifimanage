// /***********************************
// author:arvik
// email:1216601195@qq.com
// csdn:http://blog.csdn.net/u012819339
// ************************************/
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include "/router/lede/staging_dir/target-mips_24kc_musl/usr/include/uci.h"
// #include "cmdcall.h"
// #include "unistd.h"
// #include "log.h"


// u_long file_wc(const char *file)
// {
// 	u_long lines;
// 	FILE *fp;
// 	char buf[4096];

// 	if (!file) {
// 		LOG("point is null");
// 	}

// 	if (access(file, F_OK)) {
// 		LOG("%s isn't access");
// 	}


// 	if ((fp = fopen(file, "r")) < 0) {
// 		LOG("open %s file fail");
// 	}

// 	while((fgets(buf, 4096, fp)) != NULL) {
// 		if(buf[strlen(buf)-1]=='\n') {
// 			lines++;
// 		}
// 	}

// 	fclose(fp);

// 	return lines;
// }


#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <net/route.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#if defined(__GLIBC__) && __GLIBC__ >=2 && __GLIBC_MINOR__ >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <sys/types.h>
#include <netinet/if_ether.h>
#endif



static int set_addr(unsigned char ip[16], int flag);
static int get_addr(unsigned char ip[16], int flag);

// int get_ip(unsigned char ip[16])
// {
//     return get_addr(ip, SIOCGIFADDR);
// }

// int get_ip_netmask(unsigned char ip[16])
// {
//     return get_addr(ip, SIOCGIFNETMASK);
// }

// int get_mac(unsigned char addr[6])
// {
//     return get_addr(addr, SIOCGIFHWADDR);
// }

// static int get_addr(unsigned char *addr, int flag)
// {
//     int sockfd = 0;
//     struct sockaddr_in *sin;
//     struct ifreq ifr;

//     if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
//     {
//         perror("socket error!\n");
//         return false;
//     }

//     memset(&ifr, 0, sizeof(ifr));
//     snprintf(ifr.ifr_name, (sizeof(ifr.ifr_name) - 1), "%s", "tun0");

//     if(ioctl(sockfd, flag, &ifr) < 0 )
//     {
//         perror("ioctl error!\n");
//         close(sockfd);
//         return false;
//     }
//     close(sockfd);

//     if (SIOCGIFHWADDR == flag){
//         memcpy((void *)addr, (const void *)&ifr.ifr_ifru.ifru_hwaddr.sa_data, 6);
//         /*printf("mac address: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);*/
//     }else{
//         sin = (struct sockaddr_in *)&ifr.ifr_addr;
//         snprintf((char *)addr, 16, "%s", inet_ntoa(sin->sin_addr));
//     }

//     return true;
// }

int is_valid_ip(unsigned char ipaddr[16])
{
    int ret = 0;
    struct in_addr inp;
    ret = inet_aton(ipaddr, &inp);
    if (0 == ret)
    {
        return false;
    }
    else
    {
        printf("inet_aton:ip=%u\n",ntohl(inp.s_addr));
    }

    return true;
}

/*
 * 先验证是否为合法IP，然后将掩码转化成32无符号整型，取反为000...00111...1，
 * 然后再加1为00...01000...0，此时为2^n，如果满足就为合法掩码
 *
 * */
int is_valid_netmask(unsigned char netmask[16])
{
    if(is_valid_ip(netmask) > 0)
    {
        unsigned int b = 0, i, n[4];
        sscanf(netmask, "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
        for(i = 0; i < 4; ++i) //将子网掩码存入32位无符号整型
            b += n[i] << (i * 8);
        b = ~b + 1;
        if((b & (b - 1)) == 0) //判断是否为2^n
            return true;
    }

    return false;
}


int set_ip_netmask(unsigned char ip[16])
{
    return set_addr(ip, SIOCSIFNETMASK);
}

int set_ip(unsigned char ip[16])
{
    return set_addr(ip, SIOCSIFADDR);
}

static int set_addr(unsigned char ip[16], int flag)
{
    struct ifreq ifr;
    struct sockaddr_in sin;
    int sockfd;

    if (is_valid_ip(ip) < 0)
    {
        printf("ip was invalid!\n");
        return false;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1){
        fprintf(stderr, "Could not get socket.\n");
        perror("eth0\n");
        return false;
    }

    snprintf(ifr.ifr_name, (sizeof(ifr.ifr_name) - 1), "%s", "ens33");

    /* Read interface flags */
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
        fprintf(stderr, "ifdown: shutdown ");
        perror(ifr.ifr_name);
        return false;
    }

    memset(&sin, 0, sizeof(struct sockaddr));
    sin.sin_family = AF_INET;
    inet_aton(ip, &sin.sin_addr.s_addr);
    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    if (ioctl(sockfd, flag, &ifr) < 0){
        fprintf(stderr, "Cannot set IP address. ");
        perror(ifr.ifr_name);
        return false;
    }

    return true;
}


// int set_gateway(unsigned char ip[16])
// {
//     int sockFd;
//     struct sockaddr_in sockaddr;
//     struct rtentry rt;

//     if (is_valid_ip(ip) < 0)
//     {
//         printf("gateway was invalid!\n");
//         return false;
//     }

//     sockFd = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sockFd < 0)
//     {
//         perror("Socket create error.\n");
//         return false;
//     }

//     memset(&rt, 0, sizeof(struct rtentry));
//     memset(&sockaddr, 0, sizeof(struct sockaddr_in));
//     sockaddr.sin_family = AF_INET;
//     sockaddr.sin_port = 0;
//     if(inet_aton(ip, &sockaddr.sin_addr)<0)
//     {
//         perror("inet_aton error\n" );
//         close(sockFd);
//         return false;
//     }

//     memcpy ( &rt.rt_gateway, &sockaddr, sizeof(struct sockaddr_in));
//     ((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
//     ((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
//     rt.rt_flags = RTF_GATEWAY;
//     if (ioctl(sockFd, SIOCADDRT, &rt)<0)
//     {
//         perror("ioctl(SIOCADDRT) error in set_default_route\n");
//         close(sockFd);
//         return false;
//     }

//     return true;
// }

#include <assert.h>
#include <error.h>

#include "cmdcall.h"
#include "f_operation.h"
#include "ucicmd.h"
#include "log.h"



// int conn_udp_port_test(const char *serverIP, int serverport, int timeout)
// {

//     assert(serverIP != NULL && *serverIP != '\0');
//     int sockfd;
//     struct hostent *h;
//     struct sockaddr_in server_addr;
//     int len;
//     int val = 1;
//      char revc_buf[1024];

//     if (((h=gethostbyname(serverIP)) == NULL))
//     {
//         LOG("gethostbyname %s fail", serverIP);
//     }

//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(serverport);
//     server_addr.sin_addr = *((struct in_addr *)h->h_addr);
//     bzero(&(server_addr.sin_zero), 8);

//     sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sockfd < 0)
//     {
//         LOG("client socket failure");

//     }

//     // setsockopt(sockfd, IPPROTO_IP, IP_RECVERR , &val,sizeof(int));

//     // if(sendto(sockfd,"nihao", strlen("nihao"), 0, (const struct sockaddr *)&(server_addr),sizeof(struct sockaddr_in))<0)
//     // {
//     //     perror("sendto fail ");
//     //     return-1;
//     // }
//     // printf("sendto sucess\n");
//     // int recv_len = recvfrom(sockfd, revc_buf, sizeof(revc_buf), 0, (struct sockaddr *)&server_addr, (socklen_t *)&len);
//     // printf("recv_len:%d sucess\n");



//     // const char *sendData = "hehe";
//     // ret = sendto(sockfd, sendData, strlen(sendData), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
//     // printf("%d\n", ret);


//     // int retnum  = recvfrom(sockfd, revc_buf, sizeof(revc_buf), 0, (struct sockaddr *)&server_addr, (socklen_t *)&len);
//     // if(retnum < 0)
//     // {
//     //     printf("read fail\n");
//     //     return-1;
//     // }

//     fd_set rdfds;  先申明一个 fd_set 集合来保存我们要检测的 socket句柄 

//     struct timeval tv; /* 申明一个时间变量来保存时间 */

//     int ret; /* 保存返回值 */

//     FD_ZERO(&rdfds); /* 用select函数之前先把集合清零 */

//     FD_SET(sockfd, &rdfds); /* 把要检测的句柄socket加入到集合里 */

//     tv.tv_sec = 5;
//     tv.tv_usec = 500; /* 设置select等待的最大时间为1秒加500毫秒 */

//     ret = select(sockfd + 1, &rdfds, NULL, NULL, &tv); /* 检测我们上面设置到集合rdfds里的句柄是否有可读信息 */

//     int retnum2 = connect(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr));
//     if(retnum2 < 0)
//     {
//         printf("connect fail\n");
//         return-1;
//     }

//     if(ret < 0) perror("select");/* 这说明select函数出错 */

//     else if(ret == 0) printf("超时\n"); /* 说明在我们设定的时间值1秒加500毫秒的时间内，socket的状态没有发生变化 */

//     else { /* 说明等待时间还未到1秒加500毫秒，socket的状态发生了变化 */
//         printf("ret=%s\n", ret); /* ret这个返回值记录了发生状态变化的句柄的数目，由于我们只监视了socket这一个句柄，所以这里一定ret=1，如果同时有多个句柄发生变化返回的就是句柄的总和了 */
//         /* 这里我们就应该从socket这个句柄里读取数据了，因为select函数已经告诉我们这个句柄里有数据可读 */

//         if(FD_ISSET(sockfd, &rdfds)) { /* 先判断一下socket这外被监视的句柄是否真的变成可读的了 */
//             /* 读取socket句柄里的数据 */
//             printf("dududud\n");
//         }
//     }

//     // bool res = true;
//     // int opt = 1;
//     // socklen_t addr_len;


//     // if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) < 0) {
//     //     printf("connect false\n");
//     //     return false;
//     // }


//     // char recvData[255];
//     // const char *mess = "hehe";
//     // int ret = write(sockfd, mess, strlen(mess));
//     // ret =  read(sockfd, recvData, 255);
//     // if(ret > 0)    {
//     //     recvData[ret] = 0x00;
//     //     printf(recvData);
//     // }


//     close(sockfd);
//     return ret;
// }

#include "ucicmd.h"


#include <string.h>

// int main1111(int argc, char const *argv[])
// {
//      unsigned char outbuf[1024];
//      int outlen, tmplen;


//      EVP_CIPHER_CTX *ctx;
//      ctx = EVP_CIPHER_CTX_new();

//      unsigned char key[] = "22222";
//      unsigned char intext[] = "haha1234";

//      EVP_EncryptInit_ex(ctx, EVP_des_ecb(), NULL, key, NULL);
//      OPENSSL_assert(EVP_CIPHER_CTX_key_length(ctx) == 8);
//      OPENSSL_assert(EVP_CIPHER_CTX_iv_length(ctx) == 0);

     

//      if (!EVP_EncryptUpdate(ctx, outbuf, &outlen, intext, strlen(intext))) {
//          /* Error */
//          EVP_CIPHER_CTX_free(ctx);
//          return 0;
//      }

//      if (!EVP_EncryptFinal_ex(ctx, outbuf + outlen, &tmplen)) {
//          /* Error */
//          EVP_CIPHER_CTX_free(ctx);
//          return 0;
//      }
    
//     outlen += tmplen;
//     EVP_CIPHER_CTX_free(ctx);

//     FILE *out = fopen("1.txt", "wb");

//     if (out == NULL) {
//          /* Error */
//          printf("cuowu\n");
//      }

//     fwrite(outbuf, 1, outlen, out);
//     fclose(out);
//     // char buf[1024];


//     // memcpy(buf, outbuf, outlen);
//     // printf("%s", buf);

//     return 0;
// }

#include "des_aes.h"

// void main1111122(int argc, char const *argv[])
// {
//     char buf[1024];
//     char buf2[1024];

//     unsigned char key[] = "12341234";
//     unsigned char intext[] = "haha123";
//     unsigned char iv[] = "1234567812345678";

//     http_data_encrypt(intext, buf, "des", "cfb", key, iv);

//     printf("%s\n", buf);

//     FILE *fp;
//     fp = fopen("1.txt", "wb");
//     fwrite(buf, 1, strlen(buf), fp);  
//     fclose(fp);

//     memset(buf2, 0, 1024);
//     http_data_decrypt(buf, buf2, "des", "cfb", key, iv);

//     printf("%s\n", buf2);
//     return;
// }


void main()
{
    char postdata[1024] =  "haha123";
    char enpostdata[1024];
    char key[] = "12345678";
    key[24] = '\0';
    key[25] = '4';
    http_data_encrypt(postdata, enpostdata, "des", "ecb", (u_char *)key, NULL);
    printf("%s\n", enpostdata);
}