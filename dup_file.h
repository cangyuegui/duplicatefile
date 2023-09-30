#ifndef DUP_FILE_H
#define DUP_FILE_H

#include "file_tool.h"

struct compare_point128
{
    std::string file_path;
    std::map<eigenvalue, std::list<std::string>, eigenvalue_compare> md5;
};

struct compare_size
{
    std::string file_path;
    std::map<eigenvalue, compare_point128*, eigenvalue_compare> point128;
};

enum dup_file_level
{
    FAST = 0,
    STRICT = 1,
    EXTREMELY_STRICT = 2
};

class dup_file
{
public:
    dup_file();
    virtual ~dup_file();

    void dofile(const std::string& filepath, size_t size);
    void abandon(const std::string& file);
    void del(const std::string& file);
    void move(const std::string& file);
    int copy_file(const std::string& src, const std::string& dst);
    std::string unique_filename(const std::string& path);

    std::string del_dir;
    uint8_t strict; //0 fast 1 strict 2 Extremely strict

private:
    std::set<std::string> walked;
    std::map<size_t, compare_size*> dcompare_tree;
};

#endif // DUP_FILE_H
