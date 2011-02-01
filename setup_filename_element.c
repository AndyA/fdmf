#include "fdmf_vector_pairs.h"

int setup_filename_element(char **filenames, int dbidx, datum mykey) {
	filenames[dbidx] = (char *)malloc(mykey.dsize + 1); 
	if (filenames[dbidx] == NULL) {
		perror("malloc for filename buffer:");
		exit(1);
	}
	return(0);
}
