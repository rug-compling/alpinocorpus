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

string ActCorpusReader::findEntry(vector<string> const &entries, string const &entry,
                                  int offset) const
{
    vector<string>::const_iterator iter = find(entries.begin(), entries.end(),
                                               entry);
    if (iter == entries.end())
        throw runtime_error("ActCorpusReader::findEntry: File name unknown!");

    iter += offset;

    if (iter < entries.begin() || iter >= entries.end())
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
    {
        QByteArray filePartData = filePart.filePath().toUtf8();
        data = d_lastCorpusReader.read(filePartData.constData());
    }
    else if (corpusPath.path().isEmpty())
        return readFile(pathName);
    else if (dzCorpusExists(dirPart))
        data = readFromCorpus(dirPart, filePart);
    else
        return readFile(pathName);

    return QString::fromUtf8(reinterpret_cast<char const *>(&data[0]), data.size());
}

vector<string> ActCorpusReader::entries(string const &pathName)
{
    QFileInfo corpusPath(pathName.c_str());

    QMutexLocker locker(&d_mutex);

    if (d_lastCorpusPath == corpusPath.filePath())
        return d_lastCorpusReader.entries();

    if (dzCorpusExists(corpusPath))
        return entriesCorpus(corpusPath);
    else
        return entriesDirectory(corpusPath);
}

vector<string> ActCorpusReader::entriesCorpus(QFileInfo const &corpusPath)
{
    QString noextName = stripCorpusExt(corpusPath.filePath());

    QString dataFilename = noextName + ACT_DATA_EXT;
    QFileInfo dataPath(dataFilename);
    if (!dataPath.isFile())
        throw runtime_error("ActCorpusReader::pathNameCorpus: data is not a regular file!");

    QByteArray dataFilenameData(dataFilename.toUtf8());
    QSharedPointer<istream> dataStream(new DzIstream(dataFilenameData.constData()));
    if (!*dataStream)
        throw runtime_error("ActCorpusReader::pathNameCorpus: Could not open corpus data file for reading!");

    QString indexFilename = noextName + ACT_INDEX_EXT;
    QFileInfo indexPath(indexFilename);
    if (!indexPath.isFile())
        throw runtime_error("ActCorpusReader::pathNameCorpus: index is not a regular file!");

    QByteArray indexFilenameData(indexFilename.toUtf8());
    QSharedPointer<ifstream> indexStream(new ifstream(indexFilenameData.constData()));
    if (!*indexStream)
        throw runtime_error("ActCorpusReader::pathNameCorpus: Could not open corpus index file for reading!");

    IndexedCorpusReader corpusReader(dataStream, indexStream);

    return corpusReader.entries();
}

vector<string> ActCorpusReader::entriesDirectory(QFileInfo const &corpusPath)
{
    if (corpusPath.filePath() != d_lastDir)
    {
        d_lastDirEntries.clear();

        QDir dir(corpusPath.filePath(), "*.xml");
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

        d_lastDir = corpusPath.filePath();
    }

    vector<string> entries;

    for (vector<IndexNamePair>::const_iterator iter = d_lastDirEntries.begin();
    iter != d_lastDirEntries.end(); ++iter)
        entries.push_back(iter->name);

    return entries;
}

string ActCorpusReader::pathName(string const &pathName, int offset)
{
    QFileInfo corpusPath(pathName.c_str());
    QFileInfo filePart(corpusPath.fileName());
    QFileInfo dirPart(corpusPath.path());

    QByteArray dirPartData(dirPart.filePath().toUtf8());
    QByteArray filePartData(filePart.filePath().toUtf8());

    QMutexLocker locker(&d_mutex);

    if (d_lastCorpusPath == dirPart.filePath())
    {
        return string(dirPartData.constData()) + "/" +
                findEntry(d_lastCorpusReader.entries(), filePartData.constData(), offset);
    }

    if (corpusPath.path().isEmpty())
        return pathNameDirectory(QFileInfo("."), QFileInfo("path"), offset);

    string newPath;
    if (dzCorpusExists(dirPart))
        newPath = pathNameCorpus(dirPart, filePart, offset);
    else
        newPath = pathNameDirectory(dirPart, filePart, offset);

    return newPath;
}

string ActCorpusReader::pathNameCorpus(QFileInfo const &corpus,
                                       QFileInfo const &filename, int offset)
{
    QString noextName = stripCorpusExt(corpus.filePath());

    QString dataFilename = noextName + ACT_DATA_EXT;
    QFileInfo dataPath(dataFilename);
    if (!dataPath.isFile())
        throw runtime_error("ActCorpusReader::pathNameCorpus: data is not a regular file!");

    QByteArray dataFilenameData(dataFilename.toUtf8());
    QSharedPointer<istream> dataStream(new DzIstream(dataFilenameData.constData()));
    if (!*dataStream)
        throw runtime_error("ActCorpusReader::pathNameCorpus: Could not open corpus data file for reading!");

    QString indexFilename = noextName + ACT_INDEX_EXT;
    QFileInfo indexPath(indexFilename);
    if (!indexPath.isFile())
        throw runtime_error("ActCorpusReader::pathNameCorpus: index is not a regular file!");

    QByteArray indexFilenameData(indexFilename.toUtf8());
    QSharedPointer<ifstream> indexStream(new ifstream(indexFilenameData.constData()));
    if (!*indexStream)
        throw runtime_error("ActCorpusReader::pathNameCorpus: Could not open corpus index file for reading!");

    IndexedCorpusReader corpusReader(dataStream, indexStream);

    vector<string> const &entries = corpusReader.entries();

    QByteArray filenameData(filename.filePath().toUtf8());
    string found(findEntry(entries, filenameData.constData(), offset));

    d_lastCorpusPath = corpus.filePath();
    d_lastCorpusReader = corpusReader;

    QByteArray result((corpus.filePath() + "/" + found.c_str()).toUtf8());
    return result.constData();
}

string ActCorpusReader::pathNameDirectory(QFileInfo const &directory,
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
    QByteArray pathData(path.toUtf8());

    pair<vector<IndexNamePair>::const_iterator, vector<IndexNamePair>::const_iterator> found =
            equal_range(d_lastDirEntries.begin(), d_lastDirEntries.end(), string(pathData.constData()), IndexNamePairCompare());

    if (found.first == found.second)
        throw runtime_error("ActCorpusReader::pathNameDirectory: Unknown path!");

    vector<IndexNamePair>::const_iterator iter = found.first;
    iter += offset;
    if (iter < d_lastDirEntries.begin() || iter >= d_lastDirEntries.end())
        throw runtime_error("ActCorpusReader::pathNameDirectory: Offset points out of bounds!");

    return iter->name;
}

vector<unsigned char> ActCorpusReader::readFromCorpus(QFileInfo const &corpus,
                                                      QFileInfo const &file)
{
    QString noextName = stripCorpusExt(corpus.filePath());

    QString dataFilename = noextName + ACT_DATA_EXT;
    QFileInfo dataPath(dataFilename);
    if (!dataPath.isFile())
        throw runtime_error("ActCorpusReader::pathNameCorpus: data is not a regular file!");

    QByteArray dataFilenameData(dataFilename.toUtf8());
    QSharedPointer<istream> dataStream(new DzIstream(dataFilenameData.constData()));
    if (!*dataStream)
        throw runtime_error("ActCorpusReader::readFromCorpus: Could not open corpus data file for reading!");

    QString indexFilename = noextName + ACT_INDEX_EXT;
    QFileInfo indexPath(indexFilename);
    if (!indexPath.isFile())
        throw runtime_error("ActCorpusReader::pathNameCorpus: index is not a regular file!");

    QByteArray indexFilenameData(indexFilename.toUtf8());
    QSharedPointer<ifstream> indexStream(new ifstream(indexFilenameData.constData()));
    if (!*indexStream)
        throw runtime_error("ActCorpusReader::readFromCorpus: Could not open corpus index file for reading!");

    IndexedCorpusReader corpusReader(dataStream, indexStream);

    d_lastCorpusPath = corpus.filePath();
    d_lastCorpusReader = corpusReader;

    QByteArray filenameData(file.filePath().toUtf8());
    return corpusReader.read(filenameData.constData());
}

