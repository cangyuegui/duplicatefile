#ifndef FILE_TOOL_H
#define FILE_TOOL_H

#include <iostream>
#include <map>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <dirent.h>
#include <cstdio>
#include <sys/stat.h>
#include <functional>
#include <fcntl.h>
#include <set>

#ifdef _MSC_VER
#ifdef STRICT
#undef STRICT
#endif
#endif


enum MARK_TYPE
{
    MD5,
    POINT128
};

union eigenvalue
{
    uint32_t mark[4];
    uint8_t byte[16];
    uint64_t data[2];
};

#ifdef __MINGW32__
char *strndup(const char *s, size_t n);
#endif
bool get_eigenvalue(const std::string& path, size_t size, eigenvalue& data, MARK_TYPE type);
bool less_eigenvalue(const eigenvalue& e1, const eigenvalue& e2);
bool equal_eigenvalue(const eigenvalue& e1, const eigenvalue& e2);
bool empty_eigenvalue(const eigenvalue& e1);

struct eigenvalue_compare
{
   bool operator() (const eigenvalue& lhs, const eigenvalue& rhs) const
   {
       return less_eigenvalue(lhs, rhs);
   }
};

std::string replace_all(std::string str, const std::string& from, const std::string& to);
bool is_file(const struct stat& buffer);
bool is_dir(const struct stat& buffer);
bool is_dir(const std::string& dir);
int same_file_system(const char *src, const char *dst);
std::string get_absolute_path(const std::string &path);
bool compare_binary_files(const char *file1, const char *file2);
std::string get_copy_path(const std::string &src, const std::string &dst_dir);


#endif // FILE_TOOL_H
