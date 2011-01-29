#include "vector_pairs.h"

int setup_bitcount_tbl(int *tbl) {
	/* initalize lookup table */
	/* tbl[x] will hold the number of ones in the byte x. */
	int i, j;
	for (i = 0; i < 256; i++) {
		for ( tbl[i] = j = 0; j < 8; j++) {
			tbl[i] += i & (0x01 << j) ? 1 : 0;
		}
	}
	return 0;
}

