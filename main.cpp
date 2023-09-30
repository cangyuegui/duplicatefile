#include "dup_file.h"
#include <algorithm>
#include <locale>
#include <stdlib.h>
#include <functional>
#include <io.h>
#include "help_en.h"

void adddir(std::list<std::string>& dirs, std::function<void(const std::string&, size_t)> doFile)
{
    while (!dirs.empty())
    {
        std::string name = dirs.front();
        dirs.pop_front();

        DIR *dp = opendir(name.c_str());
        if (!dp)
        {
            perror("Couldn't open the directory ");
            std::cout << name << std::endl;
            closedir(dp);
            continue;
        }

        struct dirent *ep;
        while((ep = readdir(dp)))
        {
            if(ep->d_name[0] != '.')
            {
                std::string path(ep->d_name, ep->d_namlen);
                std::string fullpath = get_absolute_path(name + "/" + path);
                std::cout << fullpath << std::endl;

                struct stat buffer;
                if (stat (fullpath.c_str(), &buffer) != 0)
                {
                    continue;
                }

                if (is_file(buffer))
                {
                     if (doFile)
                     {
                         doFile(fullpath, buffer.st_size);
                     }
                }

                if (is_dir(buffer))
                {
                     dirs.push_back(fullpath);
                }

            }
        }
        closedir(dp);
    }
}

std::string to_lower(const std::string& src)
{
    std::string dst;
    std::transform (src.begin (), src.end (), back_inserter (dst), [&] (char c)->char
    {
        const std::locale loc ("");
        return std::tolower(c, loc);
    }
    );

    return dst;
}

struct param_set
{
    bool help;
    uint8_t strict;
    std::string del_dir;
    std::list<std::string> src_dirs;
    std::string error;
};

void parse_arg(int argc, char *argv[], param_set& params)
{
    if (argc <= 1)
    {
        params.help = 1;
        return;
    }

    uint8_t state = 0; //1 del dir
    bool ever_set_backup = false;
    for (int i = 1; i < argc; ++i)
    {
        std::string targ(argv[i]);

        if (state)
        {
            std::string ttarg = get_absolute_path(targ);
            if (ttarg.empty() || access(ttarg.c_str(), F_OK) != 0 || !is_dir(ttarg))
            {
                params.error = "backup dir is error ";
                params.error += targ;
                return;
            }

            params.del_dir = ttarg;
            state = 0;
            continue;
        }

        std::string ltarg = to_lower(targ);
        if (ltarg == "-quick")
        {
            params.strict = 0;
            continue;
        }

        if (ltarg == "-strict")
        {
            params.strict = 1;
            continue;
        }

        if (ltarg == "-extremely-strict")
        {
            params.strict = 2;
            continue;
        }

        if (ltarg == "-backup")
        {
            state = 1;
            ever_set_backup = true;
            continue;
        }

        if (ltarg == "-h" || ltarg == "--help" || ltarg == "/?")
        {
            params.help = true;
            continue;
        }

        std::string ttarg = get_absolute_path(targ);
        if (ttarg.empty() || access(ttarg.c_str(), F_OK) != 0 || !is_dir(ttarg))
        {
            continue;
        }

        if (std::find(params.src_dirs.begin(), params.src_dirs.end(), targ) == params.src_dirs.end())
        {
            params.src_dirs.push_back(targ);
            continue;
        }
    }

    if (ever_set_backup && params.del_dir.empty())
    {
        params.error = "use -backup but not set dir";
    }
}

int main(int argc, char *argv[])
{
    param_set ps;
    ps.help = false;
    ps.strict = 0;
    parse_arg(argc, argv, ps);

    if (ps.help)
    {
        std::cout << help;
        return 0;
    }

    if (!ps.error.empty())
    {
        std::cerr << ps.error << std::endl;
        return 0;
    }

    if (ps.src_dirs.empty())
    {
        std::cerr << "no dir need to be resolved" << std::endl;
        return 0;
    }

    dup_file df;
    df.strict = ps.strict;
    df.del_dir = ps.del_dir;

    std::function<void(const std::string&, size_t)> f = std::bind(&dup_file::dofile, &df, std::placeholders::_1, std::placeholders::_2);
    adddir(ps.src_dirs, f);

    return 0;
}










