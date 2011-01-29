/* COPYRIGHT 2004 Kurt Rosenfeld */

#ifndef _COMMON_H
#define _COMMON_H

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <string.h>

#define BYTES_PER_STEREO_SAMPLE 4
#define BYTES_PER_MONO_SAMP 2 

void write_to_fd(int fd, char *buf, int size);
int read_from_fd(int fd, char *buf, int size);
void pad(int bufbytes, char *buf, int byte_count); 

#endif /* !_COMMON_H */
