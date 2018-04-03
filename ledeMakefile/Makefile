include $(TOPDIR)/rules.mk

PKG_NAME:=wifimanage
PKG_VERSION:=1.0.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=libcurl

export PKG_NAME

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=$(PKG_NAME)
	#DEPENDS:=+libc
	DEPENDS:=+libcurl +libuci +libc
	MAINTAINER:=Mazilo <myEmail.add>
endef

#define Package/$(PKG_NAME)/description
#	This is a hello world package that contains a program to only print "hello world".
#endef

CONFIGURE_VARS+= \
	CC="$(TOOLCHAIN_DIR)/bin/$(TARGET_CC)"

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	#$(CP) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/dial $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/wifimon $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/wifiedit $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/hidemon $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/wifiscan $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/led $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/wifichecker $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/getphpconfig $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/checkgateway $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/setgateway $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/update_version $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/vpn_openvpn $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/vpn_pptp_l2tp $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/wifilog $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/config_editor $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/getstatus $(1)/bin
	#$(CP) $(PKG_BUILD_DIR)/mytest $(1)/bin
	$(CP) /lib32/libc.so.6 $(1)/bin
	$(CP) $(TOPDIR)/staging_dir/target-mips_24kc_musl/usr/lib/libcrypto.so.1.0.0 $(1)/bin
	#$(CP) $(TOPDIR)/staging_dir/target-mips_24kc_musl/usr/lib/libuci.so  $(1)/bin
	#$(CP) $(TOPDIR)/staging_dir/hostpkg/lib/libubox.so $(1)/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME),+libcurl,libuci,+libc,+libraries))
