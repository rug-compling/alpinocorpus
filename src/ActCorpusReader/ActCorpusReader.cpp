#include "ActCorpusReader.ih"

ActCorpusReader::ActCorpusReader()
{
}

ActCorpusReader::~ActCorpusReader()
{
}

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
        throw runtime_error("ActCorpusReader::entriesCorpus: data is not a regular readable file!");


    QString indexFilename = noextName + ACT_INDEX_EXT;
    QFileInfo indexPath(indexFilename);
    if (!indexPath.isFile() || !indexPath.isReadable())
        throw runtime_error("ActCorpusReader::entriesCorpus: index is not a regular readable file!");

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
            throw runtime_error("ActCorpusReader::entriesDirectory: Could not read directory entries!");

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

QString ActCorpusReader::readFromCorpus(QFileInfo const &corpus,
                                                      QFileInfo const &file)
{
    QString noextName = stripCorpusExt(corpus.filePath());

    QString dataFilename = noextName + ACT_DATA_EXT;
    QFileInfo dataPath(dataFilename);
    if (!dataPath.isFile() | !dataPath.isReadable())
        throw runtime_error("ActCorpusReader::readFromCorpus: data is not a regular readable file!");

    QString indexFilename = noextName + ACT_INDEX_EXT;
    QFileInfo indexPath(indexFilename);
    if (!indexPath.isFile() || !indexPath.isReadable())
        throw runtime_error("ActCorpusReader::readFromCorpus: index is not a regular readable file!");

    IndexedCorpusReader corpusReader(dataFilename, indexFilename);

    d_lastCorpusPath = corpus.filePath();
    d_lastCorpusReader = corpusReader;

    return corpusReader.read(file.filePath());
}

