#include <QDateTime>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QtDebug>

#include <iterator>
#include <stdexcept>
#include <typeinfo>

#include <AlpinoCorpus/Error.hh>
#include <util/textfile.hh>

#include "DirectoryCorpusReaderPrivate.hh"

namespace alpinocorpus {

DirectoryCorpusReaderPrivate::DirectoryCorpusReaderPrivate(
    std::string const &directory, bool wantCache)
    : d_directory(QString::fromUtf8(directory.c_str()))
{
    QDir dir(QString::fromUtf8(directory.c_str()), "*.xml");
    if (!dir.exists() || !dir.isReadable())
        throw OpenError(directory, "non-existent or not readable");

    if (!wantCache || !readCache()) {
        QDirIterator entryIter(dir, QDirIterator::Subdirectories
                                  | QDirIterator::FollowSymlinks);
        while (entryIter.hasNext()) {
			QString entry = QDir::toNativeSeparators(entryIter.next());
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

DirectoryCorpusReaderPrivate::~DirectoryCorpusReaderPrivate()
{}

CorpusReader::EntryIterator DirectoryCorpusReaderPrivate::getBegin() const
{
    return EntryIterator(new DirIter(d_entries.begin()));
}

CorpusReader::EntryIterator DirectoryCorpusReaderPrivate::getEnd() const
{
    return EntryIterator(new DirIter(d_entries.end()));
}

std::string DirectoryCorpusReaderPrivate::DirIter::current() const
{
    return QDir::fromNativeSeparators(*iter).toUtf8().constData();
}

bool DirectoryCorpusReaderPrivate::DirIter::equals(IterImpl const &other) const
{
    try {
        DirIter const &that = dynamic_cast<DirIter const &>(other);
        return iter == that.iter;
    } catch (std::bad_cast const &) {
        return false;
    }
}

void DirectoryCorpusReaderPrivate::DirIter::next()
{
    ++iter;
}

std::string DirectoryCorpusReaderPrivate::readEntry(std::string const &entry) const
{
    return util::readFile(d_directory.filePath(QString::fromUtf8(entry.c_str()))).toUtf8().constData();
}

QString DirectoryCorpusReaderPrivate::cachePath() const
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
bool DirectoryCorpusReaderPrivate::readCache()
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

void DirectoryCorpusReaderPrivate::writeCache()
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
