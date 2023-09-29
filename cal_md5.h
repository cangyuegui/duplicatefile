#ifndef CAL_MD5_H
#define CAL_MD5_H
#include "md5.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define READ_DATA_SIZE	1024 * 1024
#define MD5_SIZE		16
#define MD5_STR_LEN		(MD5_SIZE * 2)

// function declare
int compute_string_md5(unsigned char *dest_str, unsigned int dest_len, char *md5_str);
int compute_file_md5(const char *file_path, uint8_t* odata);

#endif

