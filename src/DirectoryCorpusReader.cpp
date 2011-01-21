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
    return EntryIterator(new DirIter(d_entries.constBegin()));
}

CorpusReader::EntryIterator DirectoryCorpusReader::getEnd() const
{
    return EntryIterator(new DirIter(d_entries.constEnd()));
}

QString DirectoryCorpusReader::DirIter::current() const
{
    return *iter;
}

bool DirectoryCorpusReader::DirIter::equals(IterImpl const *other) const
{
    try {
        DirIter const &that = dynamic_cast<DirIter const &>(*other);
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

void DirectoryCorpusReader::cacheFile(QFile &) const
{
    // XXX: putting the index outside the directory
    // is a fundamental design flaw. --Lars
    QFile cache(QString("%1/../%2.dir_index")
                .arg(d_directory.path())
                .arg(d_directory.dirName()));
}


/**
 * Read directory cache file. Returns true on success.
 */
bool DirectoryCorpusReader::readCache()
{
    QFile cache;
    cacheFile(cache);

    if (!cache.exists()
     || QFileInfo(d_directory.path()).lastModified()
         > QFileInfo(cache).lastModified()
     || !cache.open(QFile::ReadOnly))
        return false;

    // XXX no error handling below
    QTextStream cacheStream(&cache);
    while (true) {
        QString line(cacheStream.readLine());
        if (line.isNull())
            break;

        d_entries.push_back(line);
    }

    return true;
}

void DirectoryCorpusReader::writeCache()
{
  QFile cache;
  cacheFile(cache);
  if (!cache.open(QFile::WriteOnly))
    return;

  QTextStream cacheStream(&cache);
  // QTextStreamIterator???
  for (QVector<QString>::iterator iter = d_entries.begin();
      iter != d_entries.end(); ++iter)
    cacheStream << *iter << "\n";
}

}   // namespace alpinocorpus
