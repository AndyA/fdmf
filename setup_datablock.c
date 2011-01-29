#include "vector_pairs.h"

int setup_datablock(char **datablock_ptr, int num_vecs) {	
	*datablock_ptr =  (char *) malloc(MULTI_VEC_LEN * num_vecs);
	if (*datablock_ptr == NULL) {
		perror("malloc for esblock:");
		exit(1);
	}
	return (0);
}
