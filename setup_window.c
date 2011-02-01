#include "fdmf_sonic_reducer.h"

/*
This routine allocates the memory for the 
FFT window function lookup table, and it
initializes the table with calculated values
of the standard Hann window.
*/
	
double *setup_window(int len) {	
	int i;
	double *win_tbl = (double *) malloc(len * sizeof(double));
	if (win_tbl == NULL) {
		perror("malloc for win_tbl");
		exit(1);
	}

	for (i = 0; i < len; i++) {
		win_tbl[i] = 0.5 - 0.5 * cos(6.283 * ((double)i / len));
	}
	return (win_tbl);
}
