#include <stdio.h>
#include "log.h"
#include "config_editor_main.h"

int main(int argc, char const *argv[])
{
	if (argc != 2) {
		LOG("parameter error");
	}

	return config_editor_main(argv[1]);
}