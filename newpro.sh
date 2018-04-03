#!/bin/bash

cd /router/lede
mkdir feeds/packages/utils/mylib/Makefile
cp feeds/packages/utils/wifimanage/Makefile  feeds/packages/utils/mylib/Makefile
vim feeds/packages/utils/mylib/Makefile
scripts/feeds update -i
scripts/feeds install mylib
make menuconfig
make package/XXXXXX/compile -j8 V=s

#MMB   /router/lede/staging_dir/toolchain-mips_24kc_gcc-5.4.0_musl/bin/mips-openwrt-linux-musl-gcc
#YOUBING  /router/lede/staging_dir/target-mips_24kc_musl/usr/lib/liblist.a