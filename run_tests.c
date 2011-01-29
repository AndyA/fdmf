#include "vector_pairs.h"

int run_tests(char *dblk, int *hashes, char **names, 
				int vecs, int *thres, char delim) {
	/* compare all pairs of vectors for possible matches */
	int i, j, k, bitcount[256], type1_err=0, type2_err=0;
	char terminator;
	struct stat stat_results;
	if (delim == '\0') {
		terminator = '\0';
	}
	else {
		terminator = '\n';
	}
	setup_bitcount_tbl(bitcount);
	for (i = 0; i < vecs; i++) {
		for (j = i + 1; j < vecs; j++) {
			int distance[NUM_TESTS], score, test;
			char *ptr_a = dblk + i * MULTI_VEC_LEN;
			char *ptr_b = dblk + j * MULTI_VEC_LEN;
			int basenames_match, contents_similar;	
			/* calculate distance between two vectors for each test*/
			for (score = test = 0; test < NUM_TESTS; test++) {
				for (distance[test] = k = 0; k < VECTOR_BYTES; k++) {
					unsigned char c = *(ptr_a + k) ^ *(ptr_b + k);
					distance[test] += bitcount[c];
				}
				ptr_a += VECTOR_BYTES;
				ptr_b += VECTOR_BYTES;
				if (distance[test] < thres[test]) score++;
			} /* post: score contains the number of tests that passed */
			contents_similar = (score >= SCORE_THRESHOLD) ? 1 : 0;
			basenames_match = (hashes[i] == hashes[j]) ? 2 : 0;
#ifdef IGNORE_GHOST_FILES
			/* If either file of a pair does not exist, ignore the pair */
			if (contents_similar + basenames_match) {
				if (stat(names[i], &stat_results)) continue;
				if (stat(names[j], &stat_results)) continue;
			}
# endif
			switch (contents_similar + basenames_match) {
				case 0: /* different contents, different basenames */
					/* do nothing */
					break;
				case 1: /* similar contents, different basenames */
			  		printf("%s%c%s%c", names[i], delim, names[j], terminator);
					type1_err++;	
					break;
				case 2: /* different contents, same basenames */
			  	/*	printf("type 2: %s%c%s%c", names[i], delim, 
								names[j], terminator); */
					type2_err++;
					break;
				case 3: /* similar contents, same basenames */
			  		printf("%s%c%s%c", names[i], delim, names[j], terminator);
					break;
			}

		}
	}
	fprintf(stderr, "%d\t%d\t%d\t", thres[0], thres[1], thres[2]);
	fprintf(stderr, "%d\t%d\n", type1_err, type2_err); 
	return(0);
}
