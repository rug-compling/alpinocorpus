#include <QString>
#include <QFileInfo>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/IndexedCorpusReader.hh>

namespace alpinocorpus {

char const * const ACT_INDEX_EXT = ".index";
char const * const ACT_DATA_EXT = ".data.dz";

QString canonicalizePath(QString const &name)
{
    QString noextName = name;
    if (name.endsWith(ACT_DATA_EXT))
        noextName.chop(8);
    else if (name.endsWith(ACT_INDEX_EXT))
        noextName.chop(6);

    return noextName;
}

bool dzCorpusExists(QString const &corpus)
{
    QFileInfo dataPath(corpus + ACT_DATA_EXT);
    QFileInfo indexPath(corpus + ACT_INDEX_EXT);

    return dataPath.isFile() && dataPath.isReadable() &&
        indexPath.isFile() && indexPath.isReadable();
}

CorpusReader *CorpusReader::newCorpusReader(QString const &corpusPath)
{
    QString canonicalPath(canonicalizePath(corpusPath));

    if (dzCorpusExists(canonicalPath))
        return new IndexedCorpusReader(canonicalPath + ACT_DATA_EXT,
            canonicalPath + ACT_INDEX_EXT);
    else
        return new DirectoryCorpusReader(corpusPath);
}

}   // namespace alpinocorpus