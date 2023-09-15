#include <QtCore>

enum MARK_TYPE
{
    MD5,
    POINT24,
    POINT64,
    POINT128
};

struct FileCheck
{
    QFileInfo info;
    __int128 speedMark;
};

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

QMap<QString, QVariant> ParseArg(int argc, char **argv)
{
    QMap<QString, QVariant> res;
    QVariantList absVl;

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
    QString sc;
    for (int i = 1; i < argc; ++i)
    {
        QByteArray barg(argv[i]);
        QString arg = convertToSytemEncode(barg);
        if (arg.startsWith("-"))
        {
            if (arg.size() > 1)
            {
                ds = SECTION_ARG;
                if (!sc.isEmpty())
                {
                    res.insert(sc, QVariant());
                }

                sc = arg.mid(1, arg.size() - 1);
            }
        }
        else
        {
            if (ds == SECTION_ARG)
            {
                res.insert(sc, arg);
                sc.clear();
            }
            else
            {
                absVl.append(arg);
            }

            ds = ABS_ARG;
        }
    }

    if (!sc.isEmpty())
    {
        res.insert(sc, QVariant());
        sc.clear();
    }

    res.insert("-abs_all_params", absVl);
    return res;
}

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


int main(int argc, char *argv[])
{
    QMap<QString, QVariant> vars = ParseArg(argc, argv);
    for (auto i = vars.begin(); i != vars.end(); ++i)
    {
        qDebug() << i.key() << i.value();
    }


    return 0;
}










