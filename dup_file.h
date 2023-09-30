#ifndef DUP_FILE_H
#define DUP_FILE_H

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

struct file_mark
{
    std::string name;
    eigenvalue mark;
    eigenvalue md5;
    size_t size;
    file_mark(const std::string& cname): name(cname){}
    bool operator<(const file_mark &e) const
    {
        return name < e.name;
    }
};


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
int same_file_system(const char *src, const char *dst);

class dup_file
{
public:
    dup_file();

    void dofile(file_mark* mark);
    void abandon(const std::string& file);
    void del(const std::string& file);
    void move(const std::string& file);
    int copy_file(const std::string& src, const std::string& dst);
    std::string unique_filename(const std::string& path);

    std::string src_dir;
    std::string del_dir;
    bool strict;

private:
    std::map<eigenvalue, file_mark*, eigenvalue_compare> md5_files;
    std::map<eigenvalue, file_mark*, eigenvalue_compare> mark_files;
};

#endif // DUP_FILE_H
