#include "dup_file.h"

extern "C"
{
#include "cal_md5.h"
}

bool get_eigenvalue(const std::string &path, size_t size, eigenvalue &data, MARK_TYPE type)
{
    memset(&data, 0, sizeof(data));

    int fd;
    if (size <= 16)
    {
#ifdef WIN32
        fd = open(path.c_str(), O_RDONLY | O_BINARY);
#else
        fd = open(file_path, O_RDONLY);
#endif
        if (-1 == fd)
        {
            std::cerr << "open file failure " << path << std::endl;
            return false;
        }

        int ret = read(fd, data.byte, sizeof(data.byte));
        if (-1 == ret)
        {
            std::cerr << "read file failure " << path << std::endl;
            close(fd);
            return false;
        }

        return true;
    }

    if (type == MD5)
    {
        if (compute_file_md5(path.c_str(), data.byte) == 0)
        {
            return true;
        }

        return false;
    }

    size_t step = size / 16;
    if (step < 1)
    {
        step = 1;
    }
#ifdef WIN32
    fd = open(path.c_str(), O_RDONLY | O_BINARY);
#else
    fd = open(file_path, O_RDONLY);
#endif
    if (-1 == fd)
    {
        std::cerr << "open file failure " << path << std::endl;
        return false;
    }

    for (size_t i = 0; i < 16; ++i)
    {
        int ret = read(fd, &(data.byte[i]), 1);
        if (-1 == ret)
        {
            std::cerr << "read file failure " << path << std::endl;
            close(fd);
            return false;
        }
        ret = lseek(fd, step - 1, SEEK_CUR);
        if (-1 == ret)
        {
            std::cerr << "read file failure " << path << std::endl;
            close(fd);
            return false;
        }
    }

    close(fd);
    return true;
}

bool less_eigenvalue(const eigenvalue &e1, const eigenvalue &e2)
{
    if (e1.data[0] == e2.data[0])
    {
        return e1.data[1] < e2.data[1];
    }

    return e1.data[0] < e2.data[0];
}

std::string replace_all(std::string str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

bool equal_eigenvalue(const eigenvalue &e1, const eigenvalue &e2)
{
    return e1.data[0] == e2.data[0] &&  e1.data[1] == e2.data[1];
}

bool empty_eigenvalue(const eigenvalue& e1)
{
    return e1.data[0] == 0 && e1.data[1] == 0;
}

bool is_file(const struct stat &buffer)
{
    return S_ISREG(buffer.st_mode);
}

bool is_dir(const struct stat &buffer)
{
    return S_ISDIR(buffer.st_mode);
}

dup_file::dup_file()
{

}

void dup_file::dofile(file_mark *mark)
{
    if (!get_eigenvalue(mark->name, mark->size, mark->mark, POINT128))
    {
        std::cerr << "read file failure " << mark->name << std::endl;
        delete mark;
        return;
    }

    auto oitr = mark_files.find(mark->mark);
    if (oitr == mark_files.end())
    {
        mark_files.insert({mark->mark, mark});
        return;
    }

    if (mark->size <= 16)
    {
        std::cout << "duplicate file " << mark->name << " 0 "  << oitr->second->name << std::endl;
        abandon(mark->name);
        delete mark;
        return;
    }

    if (empty_eigenvalue(oitr->second->md5))
    {
        if (!get_eigenvalue(oitr->second->name, oitr->second->size, oitr->second->md5, MD5))
        {
            std::cerr << "read file failure " << oitr->second->name << std::endl;
            delete mark;
            std::cerr << "fatal error";
            exit(0);
            return;
        }

        md5_files.insert({oitr->second->md5, oitr->second});
    }

    if (!get_eigenvalue(mark->name, mark->size, mark->md5, MD5))
    {
        std::cerr << "read file failure " << mark->name << std::endl;
        delete mark;
        return;
    }

    auto md5itr = md5_files.find(mark->md5);
    if (md5itr != md5_files.end())
    {
        std::cout << "duplicate file " << mark->name << " 0 "  << md5itr->second->name << std::endl;
        abandon(mark->name);
        delete mark;
        return;
    }

     md5_files.insert({mark->md5, mark});
}

void dup_file::abandon(const std::string &file)
{
    if (del_dir.empty())
    {
        del(file);
        return;
    }

    move(file);
}

void dup_file::del(const std::string &file)
{

}

void dup_file::move(const std::string &file)
{

}
















