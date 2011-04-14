#include <QDateTime>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QtDebug>

#include <iterator>
#include <stdexcept>
#include <typeinfo>

#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/util/textfile.hh>

namespace alpinocorpus {

DirectoryCorpusReader::DirectoryCorpusReader(QString const &directory,
                                             bool wantCache)
 : d_directory(directory)
{
    QDir dir(directory, "*.xml");
    if (!dir.exists() || !dir.isReadable())
        throw OpenError(directory, "non-existent or not readable");

    if (!wantCache || !readCache()) {
        QDirIterator entryIter(dir, QDirIterator::Subdirectories
                                  | QDirIterator::FollowSymlinks);
        while (entryIter.hasNext()) {
            QString entry = entryIter.next();
            entry.remove(0, directory.length());
            if (entry[0] == QDir::separator())
                entry.remove(0, 1);
            d_entries.push_back(entry);
        }
    }

    if (wantCache)
        writeCache();

    setName(directory);
}

CorpusReader::EntryIterator DirectoryCorpusReader::getBegin() const
{
    return EntryIterator(new DirIter(d_entries.begin()));
}

CorpusReader::EntryIterator DirectoryCorpusReader::getEnd() const
{
    return EntryIterator(new DirIter(d_entries.end()));
}

QString DirectoryCorpusReader::DirIter::current() const
{
    return QDir::fromNativeSeparators(*iter);
}

bool DirectoryCorpusReader::DirIter::equals(IterImpl const &other) const
{
    try {
        DirIter const &that = dynamic_cast<DirIter const &>(other);
        return iter == that.iter;
    } catch (std::bad_cast const &) {
        return false;
    }
}

void DirectoryCorpusReader::DirIter::next()
{
    ++iter;
}

QString DirectoryCorpusReader::readEntry(QString const &entry) const
{
    return util::readFile(d_directory.filePath(entry));
}

QString DirectoryCorpusReader::cachePath() const
{
    // XXX: putting the index outside the directory
    // is a fundamental design flaw. --Lars
    return QString("%1/../%2.dir_index")
            .arg(d_directory.path())
            .arg(d_directory.dirName());
}


/**
 * Read directory cache file. Returns true on success.
 */
bool DirectoryCorpusReader::readCache()
{
    QFile cache(cachePath());

    if (!cache.exists()
     || QFileInfo(d_directory.path()).lastModified()
         > QFileInfo(cache).lastModified()
     || !cache.open(QFile::ReadOnly))
        return false;

    QTextStream cacheStream(&cache);
    while (true) {
        QString line(cacheStream.readLine());
        if (line.isNull())
            break;

        d_entries.push_back(line);
    }

    if (cacheStream.atEnd())
        return true;
    else {      // I/O error occurred
        d_entries.clear();
        return false;
    }
}

void DirectoryCorpusReader::writeCache()
{
    QFile cache(cachePath());
    if (!cache.open(QFile::WriteOnly))
        return;

    QTextStream cacheStream(&cache);
    for (StrVector::const_iterator i(d_entries.begin()), end(d_entries.end());
         i != end; ++i)
    cacheStream << *i << "\n";
}

}   // namespace alpinocorpus
