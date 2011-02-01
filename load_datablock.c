#include "fdmf_vector_pairs.h"

int load_datablock(GDBM_FILE kdb, char *dblk, char **filenames, int *bhashes) {
/* Load datablock with vectors */
/* and load basehashes with hashes of basenames. */
/* prescan_db already checked for empty db */
	datum mykey, myvalue;
	int dbidx=0;
	char *loadptr = dblk;
	mykey = gdbm_firstkey(kdb); 
	do {
		myvalue = gdbm_fetch(kdb, mykey);
		setup_filename_element(filenames, dbidx, mykey);
		bcopy(mykey.dptr, filenames[dbidx], mykey.dsize);
		filenames[dbidx][mykey.dsize] = '\0'; /* null terminate */
		bcopy(myvalue.dptr, loadptr, MULTI_VEC_LEN);
		bhashes[dbidx] = *(int *)(myvalue.dptr + MULTI_VEC_LEN);
		dbidx++;
		loadptr += MULTI_VEC_LEN;
		mykey = gdbm_nextkey(kdb, mykey);
	} while (mykey.dptr != NULL);
	gdbm_close(kdb);
	return (0);
}
