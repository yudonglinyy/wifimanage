#API文档

##目录

* [config_editor](#config_editor)
* [dial](#dial)
* [hidemon](hidemon)
* [led](#led)
* [update_version](#update_version)
* [vpn_openvpn](#vpn_openvpn)
* [vpn_pptp_l2tp](#vpn_pptp_l2tp)
* [wificheck](#wificheck)
* [wifiedit](#wifiedit)
* [wifilog](#wifilog)
* [wifimon](#wifimon)
* [wifiscan](#wifiscan)
* [getstatus](#getstatus)
* [vpn_update_new](#vpn_update_new)


## config_editor
### para: 
wifi:
> {"network":"wifi", "ssid" : "XX", "bssid": "XX", "device": "XX", "key" : "XX", "encryption": "XX", "disabled": "XX", "action": "XX"} 

>| key        | type   | value                      | comment |
>| ---------- | ------ | -------------------------- | ------- |
>| network    | string | wifi                       |         |
>| action     | string | conn / edit / del / hidden |         |
>| ssid       | string | XX                         |         |
>| bssid      | string | XX                         |         |
>| device     | string | XX                         |         |
>| key        | string | XX                         |         |
>| encryption | string | XX                         |         |
>| disabled   | string | XX                         |         |
>
>

dhcp:
> {"network":"dhcp", "device": "XX"}

> | key     | type   | value | comment |
> | ------- | ------ | ----- | ------- |
> | network | string | dhcp  |         |
> | device  | string | XX    |         |
>
> 

static:
>  {"network":"static", "ipaddr": "XX", "netmask": "XX", "gateway": "XX"}

>| key     | type   | value  | comment |
>| ------- | ------ | ------ | ------- |
>| network | string | static |         |
>| ipaddr  | string | XX     |         |
>| netmask | string | XX     |         |
>| gateway | string | XX     |         |
>
>

3g:
> {"network":"3g", "apn" : "XX", "service": "XX", "dialnumber": "XX", "proto" : "XX", "device": "XX", "mode": "XX"}

>| key          | type   | value          | comment                      |
>| ------------ | ------ | -------------- | ---------------------------- |
>| network      | string | 3g             |                              |
>| device       | string | XX             |                              |
>| mode         | string | router / stick |                              |
>| apn          | string | XX / null      | when mode is  router, ignore |
>| service      | string | XX / null      | when mode is  router, ignore |
>| dialnumber   | string | XX / null      | when mode is  router, ignore |
>| proto / null | string | XX / null      | when mode is  router, ignore |

lan:
> {"network":"lan", "ipaddr": "XX", "netmask": "XX"}

>| key     | type   | value | comment |
>| ------- | ------ | ----- | ------- |
>| network | string | lan   |         |
>| ipaddr  | string | XX    |         |
>
>


ap:
>{"network":"ap", "device" : "XX", "label": "XX", "ssid": "XX", "key" : "XX", "encryption": "XX", "disabled": "XX", "action": "XX"}
>
>|    key     |  type  |      value       | comment                   |
>| :--------: | :----: | :--------------: | ------------------------- |
>|  network   | string |        ap        |                           |
>|   action   | string | add / del / edit |                           |
>|   label    | string |        XX        |                           |
>|   device   | string |        XX        | when action = del, ignore |
>|    ssid    | string |        XX        | when action = del, ignore |
>|    key     | string |        XX        | when action = del, ignore |
>| encryption | string |        XX        | when action = del, ignore |
>|  disabled  | string |        XX        | when action = del, ignore |
>
>

vpn:
>{"network":"vpn", "proto" : "XX", "server": "XX", "username": "XX", "password" : "XX", "level": 1, "psk": "XX", "mac": "XX","action": "XX"}

>| key      | type   | value           | comment                    |
>| -------- | ------ | --------------- | -------------------------- |
>| network  | string | vpn             |                            |
>| action   | string | add / del /edit |                            |
>| username | string | XX              |                            |
>| password | string | XX              |                            |
>| level    | int    | XX              |                            |
>| mac      | string | XX              |                            |
>| proto    | string | XX              |                            |
>| server   | string | XX              |                            |
>| psk      | string | XX              | when action = del , ignore |


vpn_setting:
>{"network":"vpn_setting", "timeout": "XX","action": "save"}
>
>| key     | type   | value       | comment |
>| ------- | ------ | ----------- | ------- |
>| network | string | vpn_setting |         |
>| action  | string | save        |         |
>| timeout | string | XX          |         |
>
>

openvpn_setting:
>{"network":"openvpn_setting", "netmask" : "XX", "client_to_client": "XX", "duplicate_cn": "XX", "topology" : "XX", "defaultroute": "XX", "target": "XX", "enabled": "XX", "ip": "XX", "port": "XX", "proto": "XX", "cipher": "XX"}
>
>| key              | type   | value           | comment                       |
>| ---------------- | ------ | --------------- | ----------------------------- |
>| network          | string | openvpn_setting |                               |
>| target           | string | client / server |                               |
>| enabled          | string | XX              |                               |
>| ip               | string | XX              |                               |
>| port             | string | XX              |                               |
>| proto            | string | XX              |                               |
>| cipher           | string | XX              |                               |
>| client_to_client | string | XX              | when target = client , ignore |
>| duplicate_cn     | string | XX              | when target = client , ignore |
>| topology         | string | XX              | when target = client , ignore |
>| defaultroute     | string | yes / no        | when target = client , ignore |
>| netmask          | string | XX              | when target = client , ignore |
>
>

ipsec:
>{"network":"ipsec", "ip" : "XX", "psk": "XX", "level": 1, "action": "XX"}
>
>| key     | type   | value                  | comment                          |
>| ------- | ------ | ---------------------- | -------------------------------- |
>| network | string | ipsec                  |                                  |
>| action  | string | get / edit / add / del |                                  |
>| ip      | string | XX                     |                                  |
>| level   | int    | XX                     | when action = get, ignore        |
>| psk     | string | XX / "NULL"            | when action = get / del , ignore |
>
>

user:
> {"network":"user", "password": "XX","action": "XX"}
>
> | key      | type   | value     | comment |
> | -------- | ------ | --------- | ------- |
> | network  | string | user      |         |
> | action   | string | set / get |         |
> | password | string | XX        |         |
>

apcheck:

>{"network":"apcheck", "label": "XX", "device": "XX","action": "XX"}
>
>| key     | type   | value              | comment                       |
>| ------- | ------ | ------------------ | ----------------------------- |
>| network | string | apcheck            |                               |
>| action  | string | add/ del / default |                               |
>| device  | string | XX                 |                               |
>| label   | string | XX                 | when action = add, no require |
>
>




 ### retrun: 
 >0		succ
 >
 >1/2/3/4/5	fail		{"status":XX,"msg":"XXX"}


## dial
### para:
1.全部尝试拨号:
>{"argc": 0}

2.指定id拨号:
>{"argc": 1, "argv": {"bssid": "XX"}}

### return:
>0		succ
>
>1		fail		{"status":XX,"msg":"XXX"}



## hidemon

### para:

>{"argc": 3, "argv": {"ssid": "XX", "encryption": "XX", "key": "XXX"}}
>
>| key        | type   | value | comment |
>| ---------- | ------ | ----- | ------- |
>| ssid       | string | XX    |         |
>| encryption | string | XX    |         |
>| key        | string | XX    |         |
>
>

### return:

> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}

## led
### para
>none

### return

> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}


## update_version
### para

> none

### return
> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}


## vpn_openvpn
### para

> none

### return
> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}


## vpn_pptp_l2tp
### para

>none

### return
> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}


## wificheck
### para

> none

###return
> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}


## wifiedit
### para

> {"argc": 6, "argv": {"ssid": "XX", "device": "XX", "bssid": "XX", "encryption": "XX", "key": "XX", "action": "XX"}}
>
> | key        | type   | value                   | comment                           |
> | ---------- | ------ | ----------------------- | --------------------------------- |
> | action     | string | new / edit / conn / del |                                   |
> | bssid      | string | XX                      |                                   |
> | ssid       | string | XX                      | when aciton = conn / del , ignore |
> | device     | string | XX                      | when aciton = conn / del , ignore |
> | encryption | string | XX                      | when aciton = conn / del , ignore |
> | key        | string | XX                      | when aciton = conn / del , ignore |
>
> 

### return
> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}


## wifilog
### para

> [wifi | honey]

### return
> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}


## wifimon
### para

> none

### return
> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}


## wifiscan
### para

> {"argc": 1, "argv": {"device": "wlanXX"}}
>
> | key    | type   | value  | comment |
> | ------ | ------ | ------ | ------- |
> | device | string | wlanXX |         |
>
> 

### return
> 0		succ
>
> 1		fail		{"status":XX,"msg":"XXX"}





## getstatus
### wan
#### para
>  {"network": "wan"}
#### return
> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" :	 {"info": XX, "ip" : XX, "mask" : XX, "gateway" : XX, "ssid" : XX, "pingmsg" : XX}

### lan

#### para

> {"network": "lan"}

#### return

> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" :	 {ip" : XX, "mask" : XX}

### ap

#### para

> {"network": "ap", "action": "XX", "device": "XX"}

#### return

> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" :	 {"wlan" : XX}

### client

#### para

> {"network": "client"}

#### return

> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" :	 {"dev1" : [ {"dhcpinfo" :   [{"ip" : xx, "host" : xx, "mac" : xx}, {...}, {...}  } ], "dev2": [ ] }  ] }

### settings_3g

#### para

> {"network": "settings_3g"}

#### return

> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" :	 {"device" : XX, "proto" : XX, "apn" : XX, "service" : XX, "dialnumber" : XX}

### vpn

#### para

> {"network": "vpn", "level":0, "logread":bool}
>
> | key     | type   | value      | comment                                  |
> | ------- | ------ | ---------- | ---------------------------------------- |
> | network | string | vpn        |                                          |
> | level   | int    | xx         | when router.vpn.model == openvpn, no require |
> | logread | bool   | true/flase | when router.vpn.model == openvpn, no require, if true, will read log from /usr/bin/logread |
>
> 

#### return

> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" : 	 {"username" : XX, "password" : XX, "server" : XX, "proto" : XX, "vpn_left_count" : XX, "vpn_psk" : XX}	 

### vpn_setting

#### para

> {"network": "vpn_setting"}

#### return

> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" :	{“timeout” : XX}

### systeminfo

#### para

> {"network": "systeminfo"}

#### return

> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" :	 {"system" : XX, "version" : XX, "subversion" : XX, "mac" : XX}	 

### wifilog

#### para

> {"network":"wifilog"}

#### return

> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" :	{“result” : [  {"time" : XX, "attacker" : XX, "protocol" : XX }, {...}, {...}  ]}

### honeypot_log

#### para

> {"network": "honeypot_log"}

#### return

> "status" :	 0	succ
>
> ​		1	fail 
>
>  "msg" :	[ {"time" : xx, "ip" : xx, "port" : xx }, {...}, {...} ]


### vpn_update_new

#### para
> usage: update_vpn [option]
> option:
> ​	-H, --hour: 
> ​	-n, --num: 
> ​	-u, --url: 
> ​	-a, --action: 1:UsingVpnList 2:AllVpnList 3:DdnsList
> ​	-h, --help

#### return
>1 ： fail



### userauth

#### pare 
/auth/api/v1/userlist

{
    
    "token": "123456789", 
    "api_key": "123456789", 
    "serverid": "1", 
    "time": "2017-01-02 11:00:00", 
    "version": "1"
}

token:
{
    "mac": "ff:ff:ff:ff:ff:ff", 
    "ip": "192.168.1.1", 
    "token": "123456789", 
    "api_key": "123456789", 
    "clientid": "1", 
    "serverid": "1", 
    "time": "2017-01-02 11:00:00", 
    "version": "1"
}

#### return
{
    "statuscode": "1", 
    "X-RateLimit-Limit": "5000"
	"X-RateLimit-Remaining": "4999"
}



