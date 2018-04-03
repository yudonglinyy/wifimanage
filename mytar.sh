#!/bin/bash

make clean
make
make clean
cd ..
mv wifimanage wifimanage-1.0.1
tar -Jcf wifimanage-1.0.1.tar.xz wifimanage-1.0.1
cp wifimanage-1.0.1.tar.xz /router/lede/dl/ 
rm -f wifimanage-1.0.1.tar.xz
mv wifimanage-1.0.1 wifimanage
cd /router/lede
make package/wifimanage/clean V=99
make package/wifimanage/compile  -j9 V=99
if [[ $? -ne 0 ]]
then
	echo "fail!!!"
else
	scp /router/lede/bin/packages/mips_24kc/packages/wifimanage_1.0.1-1_mips_24kc.ipk  root@192.168.3.105:~
	echo "chengggong"
fi
