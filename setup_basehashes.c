#include "fdmf_vector_pairs.h"

int setup_basehashes(int **basehashes_ptr, int num_vecs) {
	*basehashes_ptr = malloc(BASE_HASH_LEN * num_vecs);
	if (*basehashes_ptr == NULL) {
		perror("malloc for basehashes[]:");
		exit(1);
	}
	return(0);
}
