#include "dup_file.h"

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

std::map<std::string, std::string> parse_arg(int argc, char **argv, std::list<std::string>& absVl)
{
    std::map<std::string, std::string> res;

    if (argc <= 1)
    {
        return res;
    }

    enum DoState
    {
        ABS_ARG,
        SECTION_ARG,
    };

    DoState ds = ABS_ARG;
    std::string sc;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg(argv[i]);

        if (arg.rfind("-", 0) == 0)
        {
            if (arg.size() > 1)
            {
                ds = SECTION_ARG;
                if (!sc.empty())
                {
                    res.insert({sc, std::string()});
                }

                sc = arg.substr(1, arg.size() - 1);
            }
        }
        else
        {
            if (ds == SECTION_ARG)
            {
                res.insert({sc, arg});
                sc.clear();
            }
            else
            {
                absVl.push_back(arg);
            }

            ds = ABS_ARG;
        }

    }

    if (!sc.empty())
    {
        res.insert({sc, std::string()});
        sc.clear();
    }

    return res;

}


int main(int argc, char *argv[])
{
    std::list<std::string> absVl;
    std::map<std::string, std::string> vars = parse_arg(argc, argv, absVl);
    for (auto i = vars.begin(); i != vars.end(); ++i)
    {
        std::cout << i->first << i->second << std::endl;
    }

    for (auto str : absVl)
    {
        std::cout << str << std::endl;
    }

    std::vector<file_mark> files;
    std::list<std::string> dirs = {"D:\\Work\\build-FilterMultiFilesExe-Desktop_Qt_5_14_1_MinGW_64_bit-Debug"};
    adddir(dirs, nullptr);

    return 0;
}










