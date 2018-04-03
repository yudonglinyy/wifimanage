#!/bin/bash

script_type="user-pass-verify"
common_name="haha.txt"
remote_ip="192.168.100.102"
serverid=1
url="http://127.0.0.1:5000/auth/api/v1/connect"
tmpfile="/tmp/${common_name}"

status=0

case "$script_type" in
up)
  echo "up" >> /tmp/vpn.log
  ;;
down)
  echo "down" >> /tmp/vpn.log
  ;;
client-connect)
  echo "connect" >> /tmp/vpn.log
  cid=`sed -n '1p' $tmpfile`
  vpnip=`sed -n '2p' $tmpfile`
  
  if [[ "" == $(grep "${cid}" /etc/iptables/rules.v4) ]]; then
        iptables -t mangle -A PREROUTING -s ${remote_ip} -j MARK --set-mark ${cid}
        iptables -t nat -A POSTROUTING -s ${remote_ip}/32 -j SNAT --to-source ${vpnip}
        service iptables save

  fi

  if [[ "" == $(ip rule show | grep ${cid}) ]]; then
        ip rule add from all fwmark $cid table $cid
        ip route flush cache
  fi

  if [[ "" == $(ip route list | grep ${cid}) ]]; then
        ip route add default via ${vpnip} dev eth0 table $cid
  fi

  if [ ! -d "/etc/openvpn/static/" ]; then
    mkdir /etc/openvpn/static/
  fi

  echo "ifconfig-push ${vpnip} ${ifconfig_local}" >> /etc/openvpn/static/${common_name}

  exit 0
  ;;
client-disconnect)
  echo "disconnect" >> /tmp/vpn.log

  ;;
user-pass-verify)
  echo "user-pass-verify" >> /tmp/vpn.log
  cat $1 >> /tmp/vpn.log

  buf=`sed -n '1p' $1`
  cert_hash=`sed -n '2p' $1`

  info=`echo $buf | awk -F '|' '{print $1}'`
  key=`echo $buf | awk -F'|' '{print $2}'`

  ./userauth -i $info -a $cert_hash -k $key -s $serverid -u $url -f $tmpfile
  exit $?
  ;;
*)
  echo "other" >> /tmp/vpn.log
  ;;
esac

exit status
