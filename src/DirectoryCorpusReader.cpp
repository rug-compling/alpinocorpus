#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QString>
#include <QtDebug>

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <typeinfo>

#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IndexNamePair.hh>
#include <AlpinoCorpus/util/textfile.hh>

namespace alpinocorpus {

DirectoryCorpusReader::DirectoryCorpusReader(QString const &directory,
    bool cache)
    : d_directory(directory), d_cache(cache)
{
    open();
}

CorpusReader::EntryIterator DirectoryCorpusReader::begin() const
{
    return EntryIterator(new DirIter(d_entries.constBegin()));
}

CorpusReader::EntryIterator DirectoryCorpusReader::end() const
{
    return EntryIterator(new DirIter(d_entries.constEnd()));
}

QString const &DirectoryCorpusReader::DirIter::current() const
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

QVector<QString> DirectoryCorpusReader::entries() const
{
    return d_entries;
}

void DirectoryCorpusReader::open()
{
    QDir dir(d_directory, "*.xml");
    if (!dir.exists() || !dir.isReadable())
        throw OpenError(d_directory, "non-existent or not readable");

    if (useCache() && readCache())
      return;

    // Retrieve and sort directory entries.
    QDirIterator entryIter(dir, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
    QVector<IndexNamePair> indexedEntries;
    while (entryIter.hasNext()) {
        QString entry = entryIter.next();
        entry = entry.remove(0, d_directory.length());
        if (entry[0] == '/')
            entry.remove(0, 1);
        indexedEntries.push_back(entry);
        d_entries.push_back(entry); // Ugly hack to inform readers.
    }

    std::sort(indexedEntries.begin(), indexedEntries.end());

    d_entries.clear();
    for (QVector<IndexNamePair>::const_iterator iter = indexedEntries.constBegin();
            iter != indexedEntries.constEnd(); ++iter)
        d_entries.push_back(iter->name);

    writeCache();
}

QString DirectoryCorpusReader::read(QString const &entry)
{
    QString filename(QString("%1/%2").arg(d_directory).arg(entry));
    return util::readFile(filename);
}

bool DirectoryCorpusReader::readCache()
{
  QFile cacheFile(QString("%1.dir_index").arg(d_directory));
  if (!cacheFile.exists() || !cacheFile.open(QFile::ReadOnly)) {
    qWarning() << "DirectoryCorpusReader::readCache: Directory cache exists, but could not be opened!";
    return false;
  }

  QTextStream cacheStream(&cacheFile);
  while (true)
  {
    QString line(cacheStream.readLine());
    if (line.isNull())
      break;

    d_entries.push_back(line);
  }

  return true;
}

bool DirectoryCorpusReader::useCache()
{
  if (d_cache == false)
    return false;

  QString cachePath(QString("%1.dir_index").arg(d_directory));
  QFileInfo cacheInfo(cachePath);
  if (!cacheInfo.exists())
    return false;

  QFileInfo dirInfo(d_directory);
  if (dirInfo.lastModified() > cacheInfo.lastModified())
    return false;

  return true;
}

void DirectoryCorpusReader::writeCache()
{
   if (d_cache == false)
     return;

  QString cachePath(QString("%1.dir_index").arg(d_directory));
  QFile cacheFile(cachePath);
  if (!cacheFile.open(QFile::WriteOnly))
    return;

  QTextStream cacheStream(&cacheFile);
  // QTextStreamIterator???
  for (QVector<QString>::iterator iter = d_entries.begin();
      iter != d_entries.end(); ++iter)
    cacheStream << *iter << "\n";
}

}   // namespace alpinocorpus
