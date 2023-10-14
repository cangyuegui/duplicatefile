#include "file_tool.h"

#ifdef __MINGW32__
#include <libgen.h>
#endif
#include <stdlib.h>

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
        fd = open(path.c_str(), O_RDONLY);
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

        close(fd);
        return true;
    }

    if (type == MD5)
    {
        unsigned char* ddata = data.byte;
        const char* cpath = path.c_str();
        if (compute_file_md5(cpath, ddata) == 0)
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
    fd = open(path.c_str(), O_RDONLY);
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

bool is_dir(const std::string &dir)
{
    struct stat src_stat;
    if (stat(dir.c_str(), &src_stat) == 0 &&  S_ISDIR(src_stat.st_mode))
    {
        return true;
    }

    return false;
}

int same_file_system(const char *src, const char *dst)
{
    struct stat src_stat, dst_stat;
    int srcr = stat(src, &src_stat);

    if (srcr == -1)
    {
        perror("stat");
        return -1;
    }

    int dstr = stat(dst, &dst_stat);
    std::cout << dst << std::endl;
    if (dstr == -1)
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

std::string get_absolute_path(const std::string &path)
{
    std::string abs_path;
    if (path.empty())
    {
        return abs_path;
    }

    #if defined(__GNUC__) && !defined(__MINGW32__)
    char *buffer = realpath(path.c_str(), NULL);
    if (buffer) {
        abs_path = buffer;
        free(buffer);
    }
    #endif

    #ifdef __MINGW32__
    char buffer[_MAX_PATH];
    if (_fullpath(buffer, path.c_str(), _MAX_PATH)) {
        abs_path = buffer;
    }
    #endif

    return abs_path;
}

bool compare_binary_files(const char *file1, const char *file2)
{
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");
    if (f1 == NULL || f2 == NULL) {
        return false;
    }
    uint64_t buffer1, buffer2;
    while (true) {
        buffer1 = 0;
        buffer2 = 0;
        size_t n1 = fread(&buffer1, 1, sizeof(uint64_t), f1);
        if (n1 == 0) {
            break;
        }
        size_t n2 = fread(&buffer2, 1, sizeof(uint64_t), f2);
        if (n2 == 0) {
            break;
        }
        if (buffer1 != buffer2) {
            return false;
        }
    }
    fclose(f1);
    fclose(f2);
    //return feof(f1) && feof(f2);
    return true;
}

std::string get_copy_path(const std::string& from, const std::string& to)
{
    if (from.empty() || to.empty())
    {
        return std::string();
    }

    const int MAX_PATH_LEN = 1024;
    char *copy_path = NULL;
    const char* src = from.c_str();
    std::cout << from << std::endl;
    const char* dst_dir = to.c_str();
        if (src == NULL || dst_dir == NULL || src[0] == '\0' || dst_dir[0] == '\0') {
            return std::string();
        }
        struct stat st;
        if (stat(src, &st) != 0 || !S_ISREG(st.st_mode)) {
            return std::string();
        }
        if (stat(dst_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
            return std::string();
        }
        char src_drive[MAX_PATH_LEN];
        char src_dir[MAX_PATH_LEN];
        char src_file[MAX_PATH_LEN];
        char src_ext[MAX_PATH_LEN];
        _splitpath(src, src_drive, src_dir, src_file, src_ext);
        int index = 0;
        do {
            copy_path = (char*)malloc(MAX_PATH_LEN);
            if (copy_path == NULL) {
                return NULL;
            }
            if (index == 0) {
                _makepath(copy_path, NULL, dst_dir, src_file, src_ext);
            }
            else {
                char temp_file[MAX_PATH_LEN];
                sprintf(temp_file, "%s(%d)", src_file, index);
                _makepath(copy_path, NULL, dst_dir, temp_file, src_ext);
            }
            index++;
        } while (_access(copy_path, 0) == 0);

        std::string copypath(copy_path);
        free(copy_path);
        return copypath;
}
