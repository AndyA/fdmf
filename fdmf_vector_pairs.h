#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <gdbm.h>
#include <assert.h>
#include "common.h"

#define SCORE_THRESHOLD 3
#define BASE_HASH_LEN 4
#define FILE_HASH_LEN 16
#define NUM_TESTS 3
#define VECTOR_BYTES 32
#define MINKEYSIZE 2
#define MAXKEYSIZE 1000
/* MULTI_VEC_LEN: sum of lengths of vectors for each test */
#define MULTI_VEC_LEN NUM_TESTS * VECTOR_BYTES
#define DFL_THRESH 75, 115, 85
/* IGNORE_GHOST_FILES: If this is #defined, then we will not output anything
   about matching pairs of files if either file does not exist. In other words,
   both files must exist for anything to be printed.  */
#define IGNORE_GHOST_FILES
int setup_bitcount_tbl( int *t );
int prescan_db( GDBM_FILE );
int process_args( int argc, char **argv, int *threshold_ptr,
                  char *out_delim );
int setup_datablock( char **datablock_ptr, int num_vecs );
GDBM_FILE setup_db( int *num_vecs_ptr );
int setup_filenames_array( char ***filenames_ptr, int num_vecs );
int setup_basehashes( int **basehashes_ptr, int num_vecs );
int setup_filename_element( char **filenames, int dbidx, datum mykey );
int load_datablock( GDBM_FILE db, char *dblk, char **filenames,
                    int *bhashes );
int run_tests( char *dblk, int *hashes, char **names, int vecs, int *thres,
               char delim );
void print_usage( char ** );
float name_compare( char *, char * );
