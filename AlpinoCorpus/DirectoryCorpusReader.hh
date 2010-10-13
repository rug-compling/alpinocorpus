#ifndef DIRECTORYCORPUSREADER_HH
#define DIRECTORYCORPUSREADER_HH

#include <QString>
#include <QVector>

#include "CorpusReader.hh"

namespace alpinocorpus {

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

#endif // DIRECTORYCORPUSREADER_HH
