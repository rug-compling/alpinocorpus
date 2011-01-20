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
    canonicalize(filename);
    construct();
}

IndexedCorpusReader::IndexedCorpusReader(QString const &dataFilename,
        QString const &indexFilename)
    : d_dataFilename(dataFilename), d_indexFilename(indexFilename)
{
    canonicalize(dataFilename);
    construct2();
}

/*
 * Construct from canonical name only
 */
void IndexedCorpusReader::construct()
{
    d_dataFilename  = d_canonical + DATA_EXT;
    d_indexFilename = d_canonical + INDEX_EXT;
    construct2();
}

/*
 * Construct from canonical name and two filenames
 */
void IndexedCorpusReader::construct2()
{
    // XXX race condition up ahead
    QFileInfo data(d_dataFilename);
    if (!data.isFile() || !data.isReadable())
        throw OpenError(d_dataFilename, "not readable or not a plain file");

    QFileInfo index(d_indexFilename);
    if (!index.isFile() || !index.isReadable())
        throw OpenError(d_indexFilename, "not readable or not a plain file");

    open();
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

CorpusReader::EntryIterator IndexedCorpusReader::begin() const
{
    return EntryIterator(new IndexIter(d_indices.constBegin()));
}

CorpusReader::EntryIterator IndexedCorpusReader::end() const
{
    return EntryIterator(new IndexIter(d_indices.constEnd()));
}

/*
 * Canonicalize file name. To be called from constructor.
 */
void IndexedCorpusReader::canonicalize(QString const &filename)
{
    d_canonical = filename;
    if (filename.endsWith(DATA_EXT))
        d_canonical.chop(8);
    else if (filename.endsWith(INDEX_EXT))
        d_canonical.chop(6);
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

void IndexedCorpusReader::open()
{
    QFile indexFile(d_indexFilename);
    if (!indexFile.open(QFile::ReadOnly))
        throw OpenError(d_indexFilename);

    d_dataFile = QDictZipFilePtr(new QDictZipFile(d_dataFilename));
    if (!d_dataFile->open(QDictZipFile::ReadOnly))
        throw OpenError(d_indexFilename);

    QTextStream indexStream(&indexFile);

    QString line;
    while (true)
    {
        line = indexStream.readLine();
        if (line.isNull())
            break;

        QStringList lineParts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

        if (lineParts.size() != 3)
            throw OpenError(d_indexFilename,
                            QString::fromUtf8("malformed line in index file"));

        QString name(lineParts[0]);
        size_t offset = util::b64_decode(lineParts[1].toAscii());
        size_t size   = util::b64_decode(lineParts[2].toAscii());

        IndexItemPtr item(new IndexItem(name, offset, size));
        d_indices.push_back(item);
        d_namedIndices[name] = item;
    }
}

QString IndexedCorpusReader::read(QString const &filename) const
{
    QHash<QString, IndexItemPtr>::const_iterator iter = d_namedIndices.find(filename);
    if (iter == d_namedIndices.end())
        throw std::runtime_error("IndexedCorpusReader::read: requesting unknown data!");

    QMutexLocker locker(const_cast<QMutex *>(&d_mutex));

    d_dataFile->seek(iter.value()->offset);

    return d_dataFile->read(iter.value()->size);
}

}   // namespace alpinocorpus
