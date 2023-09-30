#ifndef CAL_MD5_H
#define CAL_MD5_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// function declare

#ifdef __cplusplus
extern "C" {
#endif

int compute_string_md5(unsigned char *dest_str, unsigned int dest_len, char *md5_str);
int compute_file_md5(const char *file_path, unsigned char* odata);

#ifdef __cplusplus
}
#endif


#endif

