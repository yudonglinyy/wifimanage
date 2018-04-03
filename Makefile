CC = gcc
LIBS = -Llib $(patsubst lib%.a,-l%,$(filter lib%.a, $^))

ifeq ($(PKG_NAME),)
CFLAGS+= -D DEBUG -g3 
else
CFLAGS+= -O3
endif

LDFLAGS?=-fPIC
CFLAGS+= -Iinclude -Itest -Wall
LIBFUNC = liblist.a libdial_func.a libwifiedit_func.a libwifiscan_func.a liblogbase.a libtraversalfile.a libcurl.a libcurlhttp.a libconfig_editor.a libgetstatus.a
ALL =dial wifimon wifiscan wifiedit hidemon led wifichecker getphpconfig checkgateway setgateway update_version wifilog vpn_openvpn config_editor getstatus vpn_update_new userauth
TESTALL=vpn_update_new_test

define BUILD_LIBRARY 
@-rm -f lib/$@
$(if $(wildcard $@),@$(RM) $@) 
$(if $(wildcard ar.mac),@$(RM) ar.mac)
$(if $(filter %.a, $^), 
@echo CREATE lib/$@ > ar.mac 
@echo SAVE >> ar.mac 
@echo END >> ar.mac 
@$(AR) -M < ar.mac
) 
$(if $(filter %.o,$^),@$(AR) -q lib/$@ $(filter %.o, $^)) 
$(if $(filter %.a, $^), 
@echo OPEN lib/$@ > ar.mac 
$(foreach LIB, $(filter %.a, $^), 
@echo ADDLIB lib/$(LIB) >> ar.mac 
) 
@echo SAVE >> ar.mac 
@echo END >> ar.mac 
@$(AR) -M < ar.mac 
@$(RM) ar.mac
) 
endef


all: $(ALL)
dial: dial.o libdial_func.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
wifimon: wifimon.o getwlanname.o getiponline.o libdial_func.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS)  -luci
wifiscan: wifiscan.o libwifiscan_func.a 
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS)
wifiedit: wifiedit.o libwifiedit_func.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
hidemon: hidemon.o getwlanname.o getiponline.o ucicmd.o liblogbase.a libtraversalfile.a 
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
led: led.o getwlanname.o ucicmd.o liblogbase.a libtraversalfile.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
wifichecker: wifichecker.o ucicmd.o liblogbase.a 
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
getphpconfig: getphpconfig.o liblogbase.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS)  
checkgateway: checkgateway.o ucicmd.o getgateway.o liblogbase.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
setgateway: setgateway.o getgateway.o ucicmd.o liblogbase.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
update_version: update_version.o curlhttp.o ucicmd.o liblogbase.a f_operation.o
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -lcurl -luci
config_editor: config_editor.o libconfig_editor.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
wifilog: wifilog.o f_operation.o ucicmd.o liblogbase.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
getstatus: getstatus.o libgetstatus.a
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -luci
vpn_update_new: curlhttp.o base64.o liblogbase.a vpn_update_new.o f_operation.o ucicmd.o nettest.o
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -lcurl -luci
userauth: userauth.o liblogbase.a curlhttp.o
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -lcurl
vpn_openvpn: vpn_openvpn.o vpn_auth.o liblogbase.a curlhttp.o ucicmd.o getiponline.o des_aes.o base64.o getgateway.o http_encrypt_base64.o
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -lcurl -luci -lcrypto
test/vpn_update_new_test: test/vpn_update_new_test.o curlhttp.o base64.o liblogbase.a ucicmd.o nettest.o
	$(CC) $(MOCK_FLAG) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -lcurl -luci -Ltest/lib -lcmockery

liblist.a:
	$(MAKE) -C list M=.
	cp ./list/build/liblist.a ./lib/liblist.a
liblogbase.a: cJSON.o strip.o cmdcall.o jsonvalue.o
	$(BUILD_LIBRARY)
libtraversalfile.a: traversalfile.o liblist.a
	$(BUILD_LIBRARY)
libconfig_editor.a: config_editor_main.o liblogbase.a ucicmd.o libwifiedit_func.a
	$(BUILD_LIBRARY)
libgetstatus.a: getstatus_main.o liblogbase.a ucicmd.o getiponline.o getgateway.o f_operation.o libconfig_editor.a
	$(BUILD_LIBRARY)



libwifiscan_func.a: wifiscan_func.o liblogbase.a
	$(BUILD_LIBRARY)
libdial_func.a: dial_func.o  getwlanname.o  getiponline.o f_operation.o ucicmd.o libwifiscan_func.a liblogbase.a libtraversalfile.a
	$(BUILD_LIBRARY)
	# $(AR) $(ARFLAGS) lib/$@ $^ 
libwifiedit_func.a: wifiedit_func.o ucicmd.o liblogbase.a libdial_func.a
	$(BUILD_LIBRARY)

%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<
test/%.o: test/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

mytest: mytest.o liblogbase.a ucicmd.o getiponline.o des_aes.o base64.o
	$(CC) $(LDFLAGS) -o $@ $(filter %.o, $^) $(LIBS) -lcurl -luci -lcrypto

clean:
	-rm -f *.o
	-rm -f $(ALL) test/$(TESTALL) mytest core
	-rm -f $(patsubst %,lib/%,$(LIBFUNC))
	-rm -f test/*.o
	-rm -f test/$(TESTALL) core
	$(MAKE) -C list clean
cleanlog:
	-rm -f *.log

cleantag:
		-rm -f cscope.* tags

.PHONY: all clean cleanlog cleantag
