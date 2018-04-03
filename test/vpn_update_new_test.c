#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"


#define conn_tcpport conn_tcpport_mock
#include "nettest.h"
#undef conn_tcpport


bool conn_tcpport_mock(const char *serverIP, int serverport, int timeout)
{
	printf("%s\n", "testssss");
	return (bool)mock();
}

bool conn_udpport_mock(const char *serverIP, int serverport, int timeout)
{
	return (bool)mock();
}


// Test case that fails as leak_memory() leaks a dynamically allocated block.
void conn_tcpport_test(void **state) {
    will_return(conn_tcpport_mock, 1);
    will_return(conn_udpport_mock, 1);
    assert_true(test_conn_vpn_server("127.0.0.1") == true);

}

int main(int argc, char* argv[]) {
    const UnitTest tests[] = {
        unit_test(conn_tcpport_test),
    };
    return run_tests(tests);
}