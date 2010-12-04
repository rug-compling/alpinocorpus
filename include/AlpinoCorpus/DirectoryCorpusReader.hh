#ifndef ALPINO_DIRECTORYCORPUSREADER_HH
#define ALPINO_DIRECTORYCORPUSREADER_HH

#include <QString>
#include <QVector>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

/**
 * Reader for Alpino treebanks represented as a directory of XML files.
 */
class DirectoryCorpusReader : public CorpusReader
{
public:
    DirectoryCorpusReader(QString const &directory, bool cache = true);
    QVector<QString> entries() const;
    bool open();
    QString read(const QString &entry);
private:
    bool readCache();
    bool useCache();
    void writeCache();

    QString d_directory;
    QVector<QString> d_entries;
    bool d_cache;
};

}

#endif  // ALPINO_DIRECTORYCORPUSREADER_HH
