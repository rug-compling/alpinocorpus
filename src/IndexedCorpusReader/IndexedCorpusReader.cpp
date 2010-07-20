#include "IndexedCorpusReader.ih"

IndexedCorpusReader::IndexedCorpusReader(QString const &dataFilename,
        QString const &indexFilename)
{
    QFile indexFile(indexFilename);
    if (!indexFile.open(QFile::ReadOnly))
        throw "Could not open index file for reading!";

    d_dataFile = DictZipFilePtr(new DictZipFile(dataFilename));
    if (!d_dataFile->open(DictZipFile::ReadOnly))
        throw "Could not open data file for reading!";

    QTextStream indexStream(&indexFile);

    QString line;
    while (true)
    {
        line = indexStream.readLine();
        if (line.isNull())
            break;

        QStringList lineParts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

        if (lineParts.size() != 3)
            throw "Malformed line in index file!";

        QString name(lineParts[0]);
        size_t offset = b64_decode(lineParts[1].toAscii());
        size_t size = b64_decode(lineParts[2].toAscii());

        IndexItemPtr item(new IndexItem(name, offset, size));
        d_indices.push_back(item);
        d_namedIndices[name] = item;
    }


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

QString IndexedCorpusReader::read(QString const &filename)
{
    QHash<QString, IndexItemPtr>::const_iterator iter = d_namedIndices.find(filename);
	if (iter == d_namedIndices.end())
		throw runtime_error("IndexedCorpusReader::read: requesting unknown data!");

    QMutexLocker locker(&d_mutex);

    d_dataFile->seek(iter.value()->offset);

    return d_dataFile->read(iter.value()->size);
}
