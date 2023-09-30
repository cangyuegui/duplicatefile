#include "dup_file.h"
#include <libgen.h>
#include <errno.h>
#include <stdio.h>

dup_file::dup_file()
{

}

dup_file::~dup_file()
{

}

void dup_file::dofile(const std::string &filepath, size_t size)
{
    if (walked.count(filepath))
    {
        return;
    }
    walked.insert(filepath);

    auto size_itr = dcompare_tree.find(size);
    if (size_itr == dcompare_tree.end())
    {
        auto ct = new compare_size;
        ct->file_path = filepath;
        dcompare_tree.insert({size, ct});
        return;
    }

    compare_size* size_ct = size_itr->second;
    if (size_ct->point128.empty())
    {
        eigenvalue e;
        if (!get_eigenvalue(size_ct->file_path, size, e, POINT128))
        {
            std::cerr << "read file mark fatal " << size_ct->file_path << std::endl;
            exit(0);
            return;
        }

        compare_point128* point128 = new compare_point128;
        point128->file_path = size_ct->file_path;
        size_ct->file_path.clear();
        size_ct->point128.insert({e, point128});
    }

    eigenvalue pint128e;
    if (!get_eigenvalue(filepath, size, pint128e, POINT128))
    {
        std::cerr << "read file mark failure, nope " << filepath << std::endl;
        return;
    }

    auto point128itr = size_ct->point128.find(pint128e);
    if (point128itr == size_ct->point128.end())
    {
        compare_point128* point128 = new compare_point128;
        point128->file_path = filepath;
        size_ct->point128.insert({pint128e, point128});
        return;
    }

    if (size <= 16 || strict <= FAST)
    {
        abandon(filepath);
        return;
    }

    compare_point128* point128_ct = point128itr->second;
    if (point128_ct->md5.empty())
    {
        eigenvalue e;
        if (!get_eigenvalue(point128_ct->file_path, size, e, MD5))
        {
            std::cerr << "read file md5 fatal " << point128_ct->file_path << std::endl;
            exit(0);
            return;
        }

        point128_ct->md5.insert({e, {point128_ct->file_path}});
        point128_ct->file_path.clear();
    }

    eigenvalue md5e;
    if (!get_eigenvalue(filepath, size, md5e, MD5))
    {
        std::cerr << "read file md5 failure, nope " << filepath << std::endl;
        return;
    }

    auto md5itr = point128_ct->md5.find(md5e);
    if (md5itr == point128_ct->md5.end())
    {
        point128_ct->md5.insert({md5e, {filepath}});
        return;
    }

    if (strict <= STRICT)
    {
        abandon(filepath);
        return;
    }

    for (auto str : md5itr->second)
    {
        if (compare_binary_files(str.c_str(), filepath.c_str()))
        {
            abandon(filepath);
            return;
        }
    }

     md5itr->second.push_back(filepath);
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
    if (remove(file.c_str()) < 0)
    {
        std::cout << "delete duplicate file failure " << file << std::endl;
    }
}

void dup_file::move(const std::string &file)
{
    int result = same_file_system(file.c_str(), del_dir.c_str());
    if (result == -1)
    {
        std::cerr << "move fatal get if same file system" << std::endl;
        exit(0);
        return;
    }

    std::string targetname = unique_filename(file);

    if (result == 1)
    {
        if (rename(file.c_str(), targetname.c_str()) < 0)
        {
            std::cerr << "move fatal when move file in same file system" << std::endl;
            exit(0);
            return;
        }
    }

    if (result == 0)
    {
        if (copy_file(file, targetname) == -1)
        {
            std::cerr << "move failure when move file in different file system" << file << " -> " << targetname << std::endl;
            return;
        }

    }

    del(file);
}

int dup_file::copy_file(const std::string& src, const std::string& dst)
{
    int src_fd, dst_fd;
#ifdef WIN32
    src_fd = open(src.c_str(), O_RDONLY | O_BINARY);
#else
    src_fd = open(src.c_str(), O_RDONLY);
#endif
    if (src_fd == -1)
    {
        perror("open src");
        return -1;
    }
#ifdef WIN32
    dst_fd = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
#else
    dst_fd = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
#endif
    if (dst_fd == -1)
    {
        perror("open dst");
        close(src_fd);
        return -1;
    }
    char buffer[4096];
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))))
    {
        if (bytes_read < 0)
        {
            break;
        }

        bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written == -1)
        {
            perror("write");
            close(src_fd);
            close(dst_fd);
            return -1;
        }
    }
    if (bytes_read == -1)
    {
        perror("read");
        close(src_fd);
        close(dst_fd);
        return -1;
    }

    close(src_fd);
    close(dst_fd);
    return 0;

}

std::string dup_file::unique_filename(const std::string& path)
{
    return get_copy_path(path, del_dir);
}






















