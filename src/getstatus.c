#include <stdio.h>
#include "log.h"
#include "getstatus_main.h"

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		LOG("parameter error");
	}

	return getstatus_main(argv[1]);
}