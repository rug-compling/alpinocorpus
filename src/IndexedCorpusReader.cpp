#include <stdexcept>

#include <QFile>
#include <QMutexLocker>
#include <QRegExp>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVector>
#include <QtDebug>

#include <QDictZip/QDictZipFile.hh>

#include <AlpinoCorpus/IndexedCorpusReader.hh>
#include <AlpinoCorpus/util/base64.hh>

namespace alpinocorpus {

IndexedCorpusReader::IndexedCorpusReader(QString const &dataFilename,
        QString const &indexFilename)
    : d_dataFilename(dataFilename), d_indexFilename(indexFilename)
{
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

QVector<QString> IndexedCorpusReader::entries() const
{
    QVector<QString> entries;
	
    for (QVector<IndexItemPtr>::const_iterator iter = d_indices.constBegin();
            iter != d_indices.constEnd(); ++iter)
		entries.push_back((*iter)->name);
	
	return entries;
}

bool IndexedCorpusReader::open()
{
    QFile indexFile(d_indexFilename);
    if (!indexFile.open(QFile::ReadOnly)) {
        qCritical() << "Could not open index file for reading!";
        return false;
    }

    d_dataFile = QDictZipFilePtr(new QDictZipFile(d_dataFilename));
    if (!d_dataFile->open(QDictZipFile::ReadOnly)) {
        qCritical() << "Could not open data file for reading!";
        return false;
    }

    QTextStream indexStream(&indexFile);

    QString line;
    while (true)
    {
        line = indexStream.readLine();
        if (line.isNull())
            break;

        QStringList lineParts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

        if (lineParts.size() != 3) {
            qCritical() << "Malformed line in index file!";
            return false;
        }

        QString name(lineParts[0]);
        size_t offset = util::b64_decode(lineParts[1].toAscii());
        size_t size   = util::b64_decode(lineParts[2].toAscii());

        IndexItemPtr item(new IndexItem(name, offset, size));
        d_indices.push_back(item);
        d_namedIndices[name] = item;
    }

    return true;
}

QString IndexedCorpusReader::read(QString const &filename)
{
    QHash<QString, IndexItemPtr>::const_iterator iter = d_namedIndices.find(filename);
    if (iter == d_namedIndices.end())
        throw std::runtime_error("IndexedCorpusReader::read: requesting unknown data!");

    QMutexLocker locker(&d_mutex);

    d_dataFile->seek(iter.value()->offset);

    return d_dataFile->read(iter.value()->size);
}

}   // namespace alpinocorpus
