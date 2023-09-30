#include "dup_file.h"

#include <libgen.h>

extern "C"
{
#include "cal_md5.h"
}

#ifdef __MINGW32__
char *strndup(const char *s, size_t n)
{
    size_t len = strlen(s);
    size_t new_len = (n < len) ? n : len;
    char *new_s = (char*)malloc(new_len + 1);
    if (new_s == NULL)
    {
        perror("malloc");
        return NULL;
    }
    memcpy(new_s, s, new_len);
    new_s[new_len] = '\0';
    return new_s;
}
#endif

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

int same_file_system(const char *src, const char *dst)
{
    struct stat src_stat, dst_stat;
    if (stat(src, &src_stat) == -1 || stat(dst, &dst_stat) == -1)
    {
        perror("stat");
        return -1;
    }

    if (src_stat.st_dev == dst_stat.st_dev)
    {
        return 1;
    }

    return 0;
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

    if (mark->size <= 16 || !strict)
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
    if (remove(file.c_str()) < 0)
    {
        std::cout << "delete duplicate file failure " << file << " 0 "  << std::endl;
    }
}

void dup_file::move(const std::string &file)
{
    size_t nlen = strlen(file.c_str()) + 1;
    char tmp_str[nlen];
    memcpy(tmp_str, file.c_str(), nlen - 1);
    tmp_str[nlen - 1] = 0;
    char* filename = basename(tmp_str);
    if (!filename)
    {
        std::cerr << "move fatal get file name" << std::endl;
        exit(0);
        return;
    }

    std::string targetname = del_dir;
    targetname += std::string(filename);

    int result = same_file_system(file.c_str(), targetname.c_str());
    if (result == -1)
    {
        std::cerr << "move fatal get if same file system" << std::endl;
        exit(0);
        return;
    }

    targetname = unique_filename(targetname);

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
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0)
    {
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
    char *new_path = strdup(path.c_str());
    if (new_path == NULL)
    {
        perror("strdup");
        return NULL;
    }

    int result = access(new_path, F_OK);

    if (result == -1)
        return new_path;

    int i = 1;
    char *dot = strrchr(new_path, '.');
    char *base = strndup(new_path, dot - new_path);
    char *ext = strdup(dot);
    free(new_path);

    do
    {
        //base-i.ext
        new_path = (char*)malloc(strlen(base) + strlen(ext) + 12);
        if (new_path == NULL)
        {
            perror("malloc");
            free(base);
            free(ext);
            return NULL;
        }
        sprintf(new_path, "%s-%d%s", base, i, ext);
        result = access(new_path, F_OK);
        if (result == 0)
        {
            free(new_path);
            i++;
        }
    }
    while (result == 0);

    free(base);
    free(ext);

    std::string res(new_path);
    free(new_path);
    return res;
}


















