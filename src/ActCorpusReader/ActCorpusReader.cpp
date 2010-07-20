#include "ActCorpusReader.ih"

QString ActCorpusReader::stripCorpusExt(QString const &name) const
{
    QString noextName = name;
    if (name.endsWith(ACT_DATA_EXT))
        noextName.chop(8);
    else if (name.endsWith(ACT_INDEX_EXT))
        noextName.chop(6);

    return noextName;
}

bool ActCorpusReader::dzCorpusExists(QFileInfo const &name) const
{
    QString noextName = stripCorpusExt(name.filePath());

    QFileInfo dataPath(noextName + ACT_DATA_EXT);
    QFileInfo indexPath(noextName + ACT_INDEX_EXT);

    return dataPath.isFile() && indexPath.isFile();
}

QString ActCorpusReader::findEntry(QVector<QString> const &entries, QString const &entry,
                                  int offset) const
{
    QVector<QString>::const_iterator iter = find(entries.constBegin(), entries.constEnd(),
                                               entry);
    if (iter == entries.constEnd())
        throw runtime_error("ActCorpusReader::findEntry: File name unknown!");

    iter += offset;

    if (iter < entries.constBegin() || iter >= entries.constEnd())
        throw runtime_error("ActCorpusReader::findEntry: Offset points out of bounds!");

    return *iter;
}

QString ActCorpusReader::getData(QString const &pathName)
{
    QFileInfo corpusPath(pathName);
    QFileInfo filePart(corpusPath.fileName());
    QFileInfo dirPart(corpusPath.path());

    QMutexLocker locker(&d_mutex);

    vector<unsigned char> data;

    if (d_lastCorpusPath == dirPart.filePath())
        return d_lastCorpusReader.read(filePart.filePath());
    else if (corpusPath.path().isEmpty())
        return readFile(pathName);
    else if (dzCorpusExists(dirPart))
        return readFromCorpus(dirPart, filePart);

    return readFile(pathName);
}

QVector<QString> ActCorpusReader::entries(QString const &pathName)
{
    QFileInfo corpusPath(pathName);

    QMutexLocker locker(&d_mutex);

    if (d_lastCorpusPath == corpusPath.filePath())
        return d_lastCorpusReader.entries();

    if (dzCorpusExists(corpusPath))
        return entriesCorpus(corpusPath);
    else
        return entriesDirectory(corpusPath);
}

QVector<QString> ActCorpusReader::entriesCorpus(QFileInfo const &corpusPath)
{
    QString noextName = stripCorpusExt(corpusPath.filePath());

    QString dataFilename = noextName + ACT_DATA_EXT;
    QFileInfo dataPath(dataFilename);
    if (!dataPath.isFile() || !dataPath.isReadable())
        throw runtime_error("ActCorpusReader::pathNameCorpus: data is not a regular readable file!");


    QString indexFilename = noextName + ACT_INDEX_EXT;
    QFileInfo indexPath(indexFilename);
    if (!indexPath.isFile() || !indexPath.isReadable())
        throw runtime_error("ActCorpusReader::pathNameCorpus: index is not a regular readable file!");

    IndexedCorpusReader corpusReader(dataFilename, indexFilename);

    return corpusReader.entries();
}

QVector<QString> ActCorpusReader::entriesDirectory(QFileInfo const &corpusPath)
{
    if (corpusPath.filePath() != d_lastDir)
    {
        d_lastDirEntries.clear();

        QDir dir(corpusPath.filePath(), "*.xml");
        if (!dir.exists() || !dir.isReadable())
            throw runtime_error("ActCorpusReader::pathNameDirectory: Could not read directory entries!");

        // Retrieve and sort directory entries.
        QStringList entries(dir.entryList());
        copy(entries.constBegin(), entries.constEnd(), back_inserter(d_lastDirEntries));
        sort(d_lastDirEntries.begin(), d_lastDirEntries.end(), IndexNamePairCompare());

        d_lastDir = corpusPath.filePath();
    }

    QVector<QString> entries;

    for (QVector<IndexNamePair>::const_iterator iter = d_lastDirEntries.constBegin();
    iter != d_lastDirEntries.constEnd(); ++iter)
        entries.push_back(iter->name);

    return entries;
}

QString ActCorpusReader::pathName(QString const &pathName, int offset)
{
    QFileInfo corpusPath(pathName);
    QFileInfo filePart(corpusPath.fileName());
    QFileInfo dirPart(corpusPath.path());

    QByteArray dirPartData(dirPart.filePath().toUtf8());
    QByteArray filePartData(filePart.filePath().toUtf8());

    QMutexLocker locker(&d_mutex);

    if (d_lastCorpusPath == dirPart.filePath())
    {
        return QString(dirPartData.constData()) + "/" +
                findEntry(d_lastCorpusReader.entries(), filePartData.constData(), offset);
    }

    if (corpusPath.path().isEmpty())
        return pathNameDirectory(QFileInfo("."), QFileInfo("path"), offset);

    string newPath;
    if (dzCorpusExists(dirPart))
        return pathNameCorpus(dirPart, filePart, offset);

    return pathNameDirectory(dirPart, filePart, offset);
}

QString ActCorpusReader::pathNameCorpus(QFileInfo const &corpus,
                                       QFileInfo const &filename, int offset)
{
    QString noextName = stripCorpusExt(corpus.filePath());

    QString dataFilename = noextName + ACT_DATA_EXT;
    QFileInfo dataPath(dataFilename);
    if (!dataPath.isFile() || !dataPath.isReadable())
        throw runtime_error("ActCorpusReader::pathNameCorpus: data is not a regular readable file!");

    QString indexFilename = noextName + ACT_INDEX_EXT;
    QFileInfo indexPath(indexFilename);
    if (!indexPath.isFile() | !indexPath.isReadable())
        throw runtime_error("ActCorpusReader::pathNameCorpus: index is not a regular readable file!");

    IndexedCorpusReader corpusReader(dataFilename, indexFilename);

    QVector<QString> const &entries = corpusReader.entries();

    QByteArray filenameData(filename.filePath().toUtf8());
    QString found(findEntry(entries, filenameData.constData(), offset));

    d_lastCorpusPath = corpus.filePath();
    d_lastCorpusReader = corpusReader;

    return corpus.filePath() + "/" + found;
}

QString ActCorpusReader::pathNameDirectory(QFileInfo const &directory,
                                          QFileInfo const &filename, int offset)
{
    if (directory.filePath() != d_lastDir)
    {
        d_lastDirEntries.clear();

        QDir dir(directory.filePath(), "*.xml");
        if (!dir.exists() || !dir.isReadable())
            throw runtime_error("ActCorpusReader::pathNameDirectory: Could not read directory entries!");

        // Retrieve and sort directory entries.
        QStringList entries(dir.entryList());
        for (QStringList::const_iterator iter = entries.begin();
            iter != entries.end(); ++iter)
        {
            QByteArray entryData(iter->toUtf8());
            d_lastDirEntries.push_back(IndexNamePair(entryData.constData()));
        }
        sort(d_lastDirEntries.begin(), d_lastDirEntries.end(), IndexNamePairCompare());

        d_lastDir = directory.filePath();
    }

    QString path = directory.filePath() + "/" + filename.filePath();

    pair<QVector<IndexNamePair>::const_iterator, QVector<IndexNamePair>::const_iterator> found =
            equal_range(d_lastDirEntries.begin(), d_lastDirEntries.end(), path, IndexNamePairCompare());

    if (found.first == found.second)
        throw runtime_error("ActCorpusReader::pathNameDirectory: Unknown path!");

    QVector<IndexNamePair>::const_iterator iter = found.first;
    iter += offset;
    if (iter < d_lastDirEntries.begin() || iter >= d_lastDirEntries.end())
        throw runtime_error("ActCorpusReader::pathNameDirectory: Offset points out of bounds!");

    return iter->name;
}

QString ActCorpusReader::readFromCorpus(QFileInfo const &corpus,
                                                      QFileInfo const &file)
{
    QString noextName = stripCorpusExt(corpus.filePath());

    QString dataFilename = noextName + ACT_DATA_EXT;
    QFileInfo dataPath(dataFilename);
    if (!dataPath.isFile() | !dataPath.isReadable())
        throw runtime_error("ActCorpusReader::pathNameCorpus: data is not a regular readable file!");

    QString indexFilename = noextName + ACT_INDEX_EXT;
    QFileInfo indexPath(indexFilename);
    if (!indexPath.isFile() || !indexPath.isReadable())
        throw runtime_error("ActCorpusReader::pathNameCorpus: index is not a regular readable file!");

    IndexedCorpusReader corpusReader(dataFilename, indexFilename);

    d_lastCorpusPath = corpus.filePath();
    d_lastCorpusReader = corpusReader;

    return corpusReader.read(file.filePath());
}

