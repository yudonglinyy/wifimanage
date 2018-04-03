#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "log.h"
#include "ucicmd.h"
#include "strip.h"
#include "cmdcall.h"
#include "getiponline.h"
#include "curlhttp.h"
#include "jsonvalue.h"
#include "vpn_auth.h"
#include "http_encrypt_base64.h"

#define SIZE 4096
#define MAXSIZE 4096


const char *openvpn_confdir = "/etc/openvpn";
const char *cert[4] = {"client.crt", "client.key", "ta.key", "ca.crt"};


int get_vpn_dail_ip(char *ip, int len)
{
	memset(ip, 0, len);
	uci_get("router.auth.realip", ip, len);
	return 0;	 
}

int get_vpn_dail_mac(const char *dev, char *mac, int len)
{
	if (len < 18) {
		LOG("mac overflower");
	}
	get_mac(dev, mac);
	return 0;
}


int get_vpn_dail_hash(char *hash, int len)
{
	uci_get("router.auth.hash", hash, len);
	return 0;
}


int get_vpn_dail_key(char *key, int len)
{
	uci_get("router.auth.key", key, len);
	return 0;
}


int get_vpn_dail_version(char *version, int len)
{
	uci_get("router.auth.version", version, len);
	return 0;
}


static char * getnowdate(char *date)
{  
	time_t timer=time(NULL);
	sprintf(date, "%ld", timer);
	return 0; 
}


void gen_random_key(char *key, int len)
{
	uint32_t randomnum;
	srand((unsigned int)(time(NULL)));
	randomnum = (uint32_t)rand() % 10000;
	snprintf(key, len, "%04u", randomnum);
	return;
}


int set_realkey(const char *key, char *realkey, int len)
{
	const char *staticpass = "peng";
	snprintf(realkey, len, "%s%s", key, staticpass);
	return 0;
}


int encrypt_token(char *postdata, char *key, char *enpostdata, int len)
{
printf("postdata:%s\n", postdata);

	http_data_encrypt(postdata, enpostdata, "des", "ecb", (u_char *)key, NULL);

printf("%s\n", enpostdata);
	return 0;
}


int decrypt_token(char *response_raw, char *key, char *response, int len)
{
	http_data_decrypt(response_raw, response, "des", "ecb", (u_char *)key, NULL);
	return 0;
}


int set_token_postdata(ReqTokendata *pitem, char *key, char *enpostdata, int len)
{
	char postdata_data[SIZE];
	char enpostdata_data[SIZE];
	char realkey[SIZE];

	set_realkey(key, realkey, SIZE);

    sprintf(postdata_data, "{\"ip\":\"%s\",\"mac\":\"%s\",\"hash\":\"%s\",\"version\":\"%s\",\"reqtime\":\"%s\"}", pitem->ip, pitem->mac, pitem->hash, pitem->version, pitem->reqtime);

    encrypt_token(postdata_data, realkey, enpostdata_data, SIZE);

    snprintf(enpostdata, len, "data=%s&key=%s", enpostdata_data, key);
printf("enpostdata and key:%s\n", enpostdata);

    return 0;   
}


int parse_token_response(const char *response, RespTokendata *pitem)
{
	assert(pitem != NULL);

	cJSON *json;
	char url[SIZE], realkey[SIZE];
	char *encrtpy_url = NULL;

    if(!(json = cJSON_Parse(response)))
        LOG("Error before:[%s]",cJSON_GetErrorPtr());

    pitem->status = atoi(cJSON_value(json, "status", string));
    pitem->msg = strdup(cJSON_value(json, "msg", string));

    if (pitem->status) {
    	pitem->key = pitem->url = NULL;
    	return pitem->status;
    }
    
    pitem->key = strdup(cJSON_value(json, "key", string));
    set_realkey(pitem->key, realkey, SIZE);

    encrtpy_url = cJSON_value(json, "url", string);
    decrypt_token(encrtpy_url, realkey, url, SIZE);
    pitem->url = strdup(url);

    cJSON_Delete(json);

    return pitem->status;
}


void update_cert_hash(const char *tarfilename)
{
	char cmd[SIZE];
	char buf[SIZE];
	char dirname[SIZE];
	char hash[SIZE];

	strcpy(dirname, tarfilename);
	if (strtok(dirname, ".") == NULL) {
		LOG("%s is not a tar file", tarfilename);
	}
 
	memset(hash, 0, SIZE);
	for(int i=0; i<4; i++) {
		sprintf(cmd, "/usr/bin/md5sum %s/%s | awk '{print $1}'", dirname, cert[i]);
		getCmdResult(cmd, buf, SIZE);
		strncat(hash, buf, 31);
		strcat(hash, "|");
	}
	if (strlen(hash) != 128) {
		LOG("get the cert hash buf is wrong(%s)", hash);
	}
	hash[127] = '\0';
	uci_add("router.auth.hash=%s", hash);

	sprintf(cmd, "cp -f %s/* %s && rm -rf %s", dirname, openvpn_confdir, dirname);
	if (cmdcall_no_output(cmd))
	{
		LOG("%s", cmd);
	}

	return;
}


void free_response_token_data(RespTokendata *pitem)
{
	free(pitem->msg);
	free(pitem->key);
	free(pitem->url);
	return;
}


void decomp_tarfile(const char *tarfilename, const char *password)
{
	char dirname[SIZE];
	char cmd[SIZE];

	strcpy(dirname, tarfilename);
	if (strtok(dirname, ".") == NULL) {
		LOG("%s is not a tar file", tarfilename);
	}

	if (access(tarfilename, F_OK)) {
		LOG("%s not found, curl fail", tarfilename);
	}
	// openssl des3 -d -k password -salt -in /path/to/file.tar.gz | tar xzf -

	sprintf(cmd, "openssl des3 -d -k %s -salt -in %s | tar xzf -", password, tarfilename);
	printf("cmd:%s\n", cmd);
	if (system(cmd)) {
		LOG("tar operation fail");
	}

	if( remove(tarfilename) != 0 ) {
        LOG("removed %s fail", tarfilename);
	}

	return;
}


/*the first api request, /api/v1/token*/
void request_vpn_cert(const char *url)
{
	const char *device = "eth1";
	int status;
	char postdata[SIZE];
	char response_raw[SIZE];
	char mac[SIZE], ip[SIZE], hash[SIZE], version[SIZE], reqtime[SIZE], key[SIZE], realkey[SIZE];

	ReqTokendata request_data;
	RespTokendata response_data;

	request_data.mac = mac;
	request_data.ip = ip;
	request_data.hash = hash;
	request_data.version = version;
	request_data.reqtime = reqtime;

	get_vpn_dail_mac(device, mac, SIZE);
	get_vpn_dail_ip(ip, SIZE);
	get_vpn_dail_hash(hash, SIZE);
	get_vpn_dail_version(version, SIZE);
	getnowdate(reqtime);
	gen_random_key(key, SIZE);

	set_token_postdata(&request_data, key, postdata, SIZE);

	if (curlhttp_string(url, postdata, "POST", response_raw, SIZE)) {
        exit(1);
    }

    if (strlen(response_raw) == 0) {
        LOG("response_raw is none, curl %s {hour, num} fail", url);
    }

    strip(response_raw, response_raw, SIZE);

    status = parse_token_response(response_raw, &response_data);

    if (status) {
    	LOG("auth fail,response code:%d(%s)", response_data.status, response_data.msg);
    }

    if (response_data.url != strstr(response_data.url, "http") 
    	&& response_data.url != strstr(response_data.url, "https")) {
    	LOG("the response url is invalid");
    }

    char *tarfilename = strrchr(response_data.url, '/') + 1;

    if (curlhttp(response_data.url, NULL, "get", tarfilename)){
    	exit(1);
    }

    uci_add("router.auth.key=%s", response_data.key);
    set_realkey(response_data.key, realkey, SIZE);

    decomp_tarfile(tarfilename, realkey);
    update_cert_hash(tarfilename);

    free_response_token_data(&response_data);
    return;
}




int encrypt_info(char *info, char *key, char *eninfo, int len)
{
	http_data_encrypt(info, eninfo, "des", "ecb", (u_char *)key, NULL);
	return 0;
}


/*the second api request, need write to the upfile*/
void get_vpndail_data(const char *upfile)
{
	FILE *fp;
	char ip[SIZE];
	char mac[SIZE];
	char version[SIZE];
	char reqtime[SIZE];
	char hash[SIZE];
	char key[SIZE];
	char realkey[SIZE];
	

	char info[SIZE], eninfo[SIZE];
	
	if((fp = fopen(upfile,"w"))==NULL)
	{
		LOG("cannot open %s", upfile);
	}

	get_vpn_dail_mac("ens33", mac, SIZE);
	get_vpn_dail_ip(ip, SIZE);
	get_vpn_dail_version(version, SIZE);
	getnowdate(reqtime);

	get_vpn_dail_hash(hash, SIZE);
	get_vpn_dail_key(key, SIZE);
	strcpy(realkey, key);
	
	sprintf(info, "%s|%s|%s|%s", mac, ip, version, reqtime);
	set_realkey(key, realkey, SIZE);
	encrypt_info(info, realkey, eninfo, SIZE);

	fprintf(fp, "%s|%s\n%s", eninfo, key, hash);
	
	fclose(fp);

	return;
}


