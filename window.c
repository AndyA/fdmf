#include "sonic_reducer.h"

/*
This routing operates destructively on the 
data structure passed by reference as buf.
The routine implements a Hann window
function for preparing data for the FFT.
*/

void window (f_c *buf, double *win_tbl, int len) {
	int i;
	for (i = 0; i < len; i++) {
		buf[i][0] *= win_tbl[i];
	}
}
		
	
