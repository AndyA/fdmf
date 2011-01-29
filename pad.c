#include "common.h"

void pad(int bufbytes, char *buf, int byte_count) {
	int i;
	for (i = byte_count; i < bufbytes; i++) {
		buf[i] = 0;
	}
}

