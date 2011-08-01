#include <stdexcept>
#include <string>
#include <typeinfo>

#include <tr1/unordered_map>

#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QtDebug>

#include <AlpinoCorpus/QDictZipFile.hh>

#include <AlpinoCorpus/Error.hh>
#include <util/base64.hh>

#include "IndexedCorpusReaderPrivate.hh"

namespace {
    char const * const DATA_EXT = ".data.dz";
    char const * const INDEX_EXT = ".index";
}

namespace alpinocorpus {

IndexedCorpusReaderPrivate::IndexedCorpusReaderPrivate(std::string const &filename)
{
    std::string canonical(filename);
    canonicalize(canonical);
    construct(canonical);
}

    IndexedCorpusReaderPrivate::IndexedCorpusReaderPrivate(std::string const &dataPath,
    std::string const &indexPath)
{
    std::string canonical(dataPath);
    canonicalize(canonical);
    construct(canonical, dataPath, indexPath);
}

void IndexedCorpusReaderPrivate::construct(std::string const &canonical)
{
    std::string dataPath  = canonical + DATA_EXT;
    std::string indexPath = canonical + INDEX_EXT;
    construct(canonical, dataPath, indexPath);
}

void IndexedCorpusReaderPrivate::construct(std::string const &canonical,
    std::string const &dataPath,
    std::string const &indexPath)
{
    // XXX race condition up ahead
    QFileInfo data(QString::fromUtf8(dataPath.c_str()));
    if (!data.isFile() || !data.isReadable())
        throw OpenError(dataPath, "not readable or not a plain file");

    QFileInfo index(QString::fromUtf8(indexPath.c_str()));
    if (!index.isFile() || !index.isReadable())
        throw OpenError(indexPath, "not readable or not a plain file");

    open(dataPath, indexPath);
    setName(canonical);
}

CorpusReader::EntryIterator IndexedCorpusReaderPrivate::getBegin() const
{
    ItemVector::const_iterator begin(d_indices.begin());
    return EntryIterator(new IndexIter(begin));
}

CorpusReader::EntryIterator IndexedCorpusReaderPrivate::getEnd() const
{
    ItemVector::const_iterator end(d_indices.end());
    return EntryIterator(new IndexIter(end));
}

size_t IndexedCorpusReaderPrivate::getSize() const
{
  return d_indices.size();
}

bool endsWith(std::string const &str, std::string const &end)
{
    size_t pos = str.rfind(end);
    
    if (pos == std::string::npos)
        return false;
        
    return (pos + end.size() == str.size());
}
    
/*
 * Canonicalize file name. To be called from constructor.
 */
void IndexedCorpusReaderPrivate::canonicalize(std::string &filename)
{
    if (endsWith(filename, DATA_EXT))
        filename = filename.substr(0, filename.size() - 8);
    else if (endsWith(filename, INDEX_EXT))
        filename = filename.substr(0, filename.size() - 6);
    else
        throw OpenError(filename, "not an indexed (.dz) corpus file");
}

std::string IndexedCorpusReaderPrivate::IndexIter::current() const
{
    return (*iter)->name;
}

bool IndexedCorpusReaderPrivate::IndexIter::equals(IterImpl const &other) const
{
    try {
        IndexIter const &that = dynamic_cast<IndexIter const &>(other);
        return iter == that.iter;
    } catch (std::bad_cast const &) {
        return false;
    }
}

void IndexedCorpusReaderPrivate::IndexIter::next()
{
    ++iter;
}

void IndexedCorpusReaderPrivate::open(std::string const &dataPath,
    std::string const &indexPath)
{
    QFile indexFile(QString::fromUtf8(indexPath.c_str()));
    if (!indexFile.open(QFile::ReadOnly))
        throw OpenError(indexPath);

    d_dataFile = QDictZipFilePtr(new QDictZipFile(QString::fromUtf8(dataPath.c_str())));
    if (!d_dataFile->open(QDictZipFile::ReadOnly))
        throw OpenError(indexPath);

    QTextStream indexStream(&indexFile);

    QString line;
    while (true)
    {
        line = indexStream.readLine();
        if (line.isNull())
            break;

        QStringList lineParts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

        if (lineParts.size() != 3)
            throw OpenError(indexPath, "malformed line in index file");

        std::string name(lineParts[0].toUtf8().constData());
        size_t offset = util::b64_decode(lineParts[1].toAscii().constData());
        size_t size   = util::b64_decode(lineParts[2].toAscii().constData());

        IndexItemPtr item(new IndexItem(name, offset, size));
        d_indices.push_back(item);
        d_namedIndices[name] = item;
    }
}

std::string IndexedCorpusReaderPrivate::readEntry(std::string const &filename) const
{
    IndexMap::const_iterator iter = d_namedIndices.find(filename);
    if (iter == d_namedIndices.end())
        throw Error("IndexedCorpusReaderPrivate::read: requesting unknown data!");

    QMutexLocker locker(const_cast<QMutex *>(&d_mutex));

    if (!d_dataFile->seek(iter->second->offset))
        throw Error("Seek on compressed data failed.");

    return d_dataFile->read(iter->second->size).constData();
}

}   // namespace alpinocorpus
