#include "DirectoryCorpusReader.ih"

DirectoryCorpusReader::DirectoryCorpusReader(QString const &directory) :
        d_directory(directory)
{
    QDir dir(directory, "*.xml");
    if (!dir.exists() || !dir.isReadable())
        throw runtime_error("DirectoryCorpusReader::DirectoryCorpusReader: Could not read directory entries!");

    // Retrieve and sort directory entries.
    QStringList entries(dir.entryList());
    QVector<IndexNamePair> indexedEntries;
    copy(entries.constBegin(), entries.constEnd(), back_inserter(indexedEntries));
    sort(indexedEntries.begin(), indexedEntries.end(), IndexNamePairCompare());

    for (QVector<IndexNamePair>::const_iterator iter = indexedEntries.constBegin();
    iter != indexedEntries.constEnd(); ++iter)
        d_entries.push_back(iter->name);
}

QVector<QString> DirectoryCorpusReader::entries() const
{
    return d_entries;
}

QString DirectoryCorpusReader::read(QString const &entry)
{
    QString filename(QString("%1/%2").arg(d_directory).arg(entry));
    return readFile(filename);
}
