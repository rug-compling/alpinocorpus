#include <stdexcept>
#include <typeinfo>

#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QRegExp>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVector>
#include <QtDebug>

#include <QDictZip/QDictZipFile.hh>

#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IndexedCorpusReader.hh>
#include <AlpinoCorpus/util/base64.hh>

namespace {
    char const * const DATA_EXT = ".data.dz";
    char const * const INDEX_EXT = ".index";
}

namespace alpinocorpus {

IndexedCorpusReader::IndexedCorpusReader(QString const &filename)
{
    QString canonical(filename);
    canonicalize(canonical);
    construct(canonical);
}

IndexedCorpusReader::IndexedCorpusReader(QString const &dataPath,
                                         QString const &indexPath)
{
    QString canonical(dataPath);
    canonicalize(canonical);
    construct(canonical, dataPath, indexPath);
}

void IndexedCorpusReader::construct(QString const &canonical)
{
    QString dataPath  = canonical + DATA_EXT;
    QString indexPath = canonical + INDEX_EXT;
    construct(canonical, dataPath, indexPath);
}

void IndexedCorpusReader::construct(QString const &canonical,
                                    QString const &dataPath,
                                    QString const &indexPath)
{
    // XXX race condition up ahead
    QFileInfo data(dataPath);
    if (!data.isFile() || !data.isReadable())
        throw OpenError(dataPath, "not readable or not a plain file");

    QFileInfo index(indexPath);
    if (!index.isFile() || !index.isReadable())
        throw OpenError(indexPath, "not readable or not a plain file");

    open(dataPath, indexPath);
    setName(canonical);
}

IndexedCorpusReader &IndexedCorpusReader::operator=(IndexedCorpusReader const &other)
{
	if (this != &other)
	{
		destroy();
		copy(other);
	}
	
	return *this;
}

CorpusReader::EntryIterator IndexedCorpusReader::getBegin() const
{
    return EntryIterator(new IndexIter(d_indices.constBegin()));
}

CorpusReader::EntryIterator IndexedCorpusReader::getEnd() const
{
    return EntryIterator(new IndexIter(d_indices.constEnd()));
}

/*
 * Canonicalize file name. To be called from constructor.
 */
void IndexedCorpusReader::canonicalize(QString &filename)
{
    if (filename.endsWith(DATA_EXT))
        filename.chop(8);
    else if (filename.endsWith(INDEX_EXT))
        filename.chop(6);
    else
        throw OpenError(filename, "not an indexed (.dz) corpus file");
}

void IndexedCorpusReader::copy(IndexedCorpusReader const &other)
{
    d_dataFile = other.d_dataFile;
	d_namedIndices = other.d_namedIndices;
	d_indices = other.d_indices;
}

void IndexedCorpusReader::destroy()
{
    d_namedIndices.clear();
    d_indices.clear();
    d_dataFile.clear();
}

QString IndexedCorpusReader::IndexIter::current() const
{
    return (*iter)->name;
}

bool IndexedCorpusReader::IndexIter::equals(IterImpl const *other) const
{
    try {
        IndexIter const &that = dynamic_cast<IndexIter const &>(*other);
        return iter == that.iter;
    } catch (std::bad_cast const &) {
        return false;
    }
}

void IndexedCorpusReader::IndexIter::next()
{
    ++iter;
}

void IndexedCorpusReader::open(QString const &dataPath,
                               QString const &indexPath)
{
    QFile indexFile(indexPath);
    if (!indexFile.open(QFile::ReadOnly))
        throw OpenError(indexPath);

    d_dataFile = QDictZipFilePtr(new QDictZipFile(dataPath));
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
            throw OpenError(indexPath,
                            QString::fromUtf8("malformed line in index file"));

        QString name(lineParts[0]);
        size_t offset = util::b64_decode(lineParts[1].toAscii());
        size_t size   = util::b64_decode(lineParts[2].toAscii());

        IndexItemPtr item(new IndexItem(name, offset, size));
        d_indices.push_back(item);
        d_namedIndices[name] = item;
    }
}

QString IndexedCorpusReader::readEntry(QString const &filename) const
{
    QHash<QString, IndexItemPtr>::const_iterator iter = d_namedIndices.find(filename);
    if (iter == d_namedIndices.end())
        throw std::runtime_error("IndexedCorpusReader::read: requesting unknown data!");

    QMutexLocker locker(const_cast<QMutex *>(&d_mutex));

    d_dataFile->seek(iter.value()->offset);

    return d_dataFile->read(iter.value()->size);
}

}   // namespace alpinocorpus
