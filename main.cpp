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
//#include <QtCore>

typedef unsigned __int128 uint128_t;

static bool is_file(const std::string& filename) {
    struct stat   buffer;
    return (stat (filename.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

static bool is_dir(const std::string& filefodler) {
    struct stat   buffer;
    return (stat (filefodler.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

enum MARK_TYPE
{
    MD5,
    POINT128
};

/*
struct FileCheck
{
    QFileInfo info;
    __int128 speedMark;
};
*/

struct ent{
    std::string name;
    uint128_t mark;
    ent(std::string name): name(name){}
    bool operator<(const ent &e) const {
        return name < e.name;
    }
};

void adddir(std::vector<ent>& files, std::list<std::string>& dirs)
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
                if (is_file(fullpath))
                {
                     files.push_back(ent(fullpath));

                }

                if (is_dir(fullpath))
                {
                     dirs.push_back(fullpath);
                }

            }
        }
        closedir(dp);
    }
}

/*
QString convertToSytemEncode(const QByteArray &srcString)
{
    const char* systemCode =
        #ifdef Q_OS_WIN
            "GB2312"
        #elif defined Q_OS_LINUX
            "UTF-8"
        #endif
            ;
    QTextCodec* pTextCodec = QTextCodec::codecForName(systemCode);
    if (NULL == pTextCodec)
    {
        return "";
    }
    return pTextCodec->toUnicode(srcString);
}
*/
std::map<std::string, std::string> parseArg(int argc, char **argv, std::list<std::string>& absVl)
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

/*
void traversalFile(const QFileInfo& fi, std::function<void(const QFileInfo&)> doFunc = nullptr)
{
    if (!fi.exists())
    {
        return;
    }

    if (fi.isFile())
    {
        if (doFunc)
        {
            doFunc(fi);
        }

        return;
    }

    if (!fi.isDir())
    {
        return;
    }

    QDir dir(fi.absoluteFilePath());
    for (auto cfi : dir.entryInfoList())
    {
        traversalFile(cfi, doFunc);
    }
}

QByteArray fileChecksum(const QString &fileName,
                        QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f)) {
            f.close();
            return hash.result();
        }
    }
    return QByteArray("error md5");
}

QByteArray GetRapidNumber(const QFileInfo& info, int level)
{
    if (info.size() < 1024 * 1024 || level > 10)
    {
        return fileChecksum(info.absoluteFilePath(), QCryptographicHash::Md5);
    }

    QByteArray array64;
    QFile file(info.absoluteFilePath());
    if (!file.open(QFile::ReadOnly))
    {
        array64 = "error 64";
        return array64;
    }

    qint64 rapidStep = 0;
    switch (level)
    {
    case 0:
        rapidStep = 8;
        break;
    case 1:
        rapidStep = 32;
        break;
    case 2:
        rapidStep = 128;
        break;
    default:
        rapidStep = 256;
        break;
    }

    qint64 step = (info.size() - 100) / rapidStep;
    for (qint64 i = 0; i < rapidStep; ++i)
    {
        qint64 start = step * i;
        file.seek(start);
        array64 += file.read(8);
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(array64);
    array64 = hash.result();
    QString pNumber = QString::number(info.size(), 16);
    QString stringData("****************");
    for (int i = 0; i < pNumber.size(); ++i)
    {
        stringData[i] = pNumber[i];
    }

    array64.prepend(stringData.toUtf8());
    return array64;
}

QByteArray Get64Info(const QString &fileName)
{
    QByteArray array64;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        array64 = "error 64";
    }

    if (file.size() <= 64)
    {
        array64 = file.readAll();
    }
    else
    {
        array64 = file.read(64);
    }

    file.close();
    return array64;
}
*/

int main(int argc, char *argv[])
{
    std::list<std::string> absVl;
    std::map<std::string, std::string> vars = parseArg(argc, argv, absVl);
    for (auto i = vars.begin(); i != vars.end(); ++i)
    {
        std::cout << i->first << i->second << std::endl;
    }

    for (auto str : absVl)
    {
        std::cout << str << std::endl;
    }

    std::vector<ent> files;
    std::list<std::string> dirs = {"."};
    adddir(files, dirs);


    return 0;
}










