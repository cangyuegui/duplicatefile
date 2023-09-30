#include "dup_file.h"
#include <algorithm>
#include <locale>
#include "help_en.h"

void adddir(std::list<std::string>& dirs, std::function<void(file_mark*)> doFile)
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
                std::string fullpath = name + "/" + path;
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
                         file_mark* fm = new file_mark(fullpath);
                         memset(fm->mark.byte, 0, sizeof(fm->mark.byte));
                         memset(fm->md5.byte, 0, sizeof(fm->md5.byte));
                         fm->size = buffer.st_size;
                         doFile(fm);
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
    bool strict;
    std::string del_dir;
    std::list<std::string> src_dirs;
};

void parse_arg(int argc, char *argv[], param_set& params)
{
    if (argc <= 1)
    {
        params.help = 1;
        return;
    }

    uint8_t state = 0; //1 del dir
    for (int i = 1; i < argc; ++i)
    {
        std::string targ(argv[i]);

        if (state)
        {
            params.del_dir = targ;
            state = 0;
            continue;
        }

        std::string ltarg = to_lower(targ);
        if (ltarg == "-strict")
        {
            params.strict = true;
            continue;
        }

        if (ltarg == "-backup")
        {
            state = 1;
            continue;
        }

        if (ltarg == "-h" || ltarg == "--help" || ltarg == "/?")
        {
            params.help = true;
            continue;
        }

        if (std::find(params.src_dirs.begin(), params.src_dirs.end(), targ) == params.src_dirs.end())
        {
            params.src_dirs.push_back(targ);
            continue;
        }
    }
}

int main(int argc, char *argv[])
{
    param_set ps;
    ps.help = false;
    ps.strict = false;
    parse_arg(argc, argv, ps);

    if (ps.help)
    {
        std::cout << help;
        return 0;
    }

    if (ps.src_dirs.empty())
    {
        std::cerr << "no dir need to be resolved";
        return 0;
    }

    std::vector<file_mark> files;
    adddir(ps.src_dirs, nullptr);

    return 0;
}










