#include <QDateTime>
#include <QDir>
#include <QDirIterator>
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
    QDir dir(d_directory, "*.xml");
    if (!dir.exists() || !dir.isReadable())
        throw OpenError(d_directory, "non-existent or not readable");

    if (!wantCache || !readCache()) {
        QDirIterator entryIter(dir, QDirIterator::Subdirectories
                                  | QDirIterator::FollowSymlinks);
        while (entryIter.hasNext()) {
            QString entry = entryIter.next();
            entry.remove(0, d_directory.length());
            if (entry[0] == QDir::separator())
                entry.remove(0, 1);
            d_entries.push_back(entry);
        }
    }

    if (wantCache)
        writeCache();
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
    (void)++iter;
}

void DirectoryCorpusReader::DirIter::prev()
{
    (void)--iter;
}

QVector<QString> DirectoryCorpusReader::entries() const
{
    return d_entries;
}

QString DirectoryCorpusReader::read(QString const &entry)
{
    QString filename(QString("%1/%2").arg(d_directory).arg(entry));
    return util::readFile(filename);
}

/**
 * Read directory cache file. Returns true on success.
 */
bool DirectoryCorpusReader::readCache()
{
    QFile cache(QString("%1.dir_index").arg(d_directory));

    if (!cache.exists()
     || QFileInfo(d_directory).lastModified() > QFileInfo(cache).lastModified()
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
