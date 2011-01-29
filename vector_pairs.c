/* Kurt Rosenfeld 2004 */
/* GPL */

#include "vector_pairs.h"

int main(int argc, char **argv) {
	int num_vecs, *basehashes, threshold[] = {DFL_THRESH};
	char *datablock, **filenames, delim;	
	GDBM_FILE db;
	
	process_args(argc, argv, threshold, &delim);
	db = setup_db(&num_vecs);
	fprintf(stderr, "vector pairs processing %d vectors.\n", num_vecs);
	setup_datablock(&datablock, num_vecs);
	setup_filenames_array(&filenames, num_vecs);	
	setup_basehashes(&basehashes, num_vecs);
	load_datablock(db, datablock, filenames, basehashes);
	run_tests(datablock, basehashes, filenames, num_vecs, threshold, delim); 
	return 0;
}
			

