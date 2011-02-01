#include "fdmf_vector_pairs.h"

GDBM_FILE setup_db(int *num_vecs) {
	GDBM_FILE db;
	char * dbfilename = strcat(getpwuid(getuid())->pw_dir, "/.fdmf");
	db = gdbm_open(dbfilename, 512, GDBM_READER, 0660, 0);
	if (db == NULL) {
		perror(NULL);
		exit(1);
	}
	*num_vecs = prescan_db(db);

	if (*num_vecs < 0) {
		fprintf(stderr, "Prescan of the database failed.\n");
		exit(1);
	}
	return (db);
}
