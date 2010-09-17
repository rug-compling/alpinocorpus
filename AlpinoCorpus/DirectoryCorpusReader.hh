#ifndef DIRECTORYCORPUSREADER_HH
#define DIRECTORYCORPUSREADER_HH

#include <QString>
#include <QVector>

#include "CorpusReader.hh"

namespace alpinocorpus {

class DirectoryCorpusReader : public CorpusReader
{
public:
    DirectoryCorpusReader(QString const &directory);
    QVector<QString> entries() const;
    bool open();
    QString read(const QString &entry);
private:
    QString d_directory;
    QVector<QString> d_entries;
};

}

#endif // DIRECTORYCORPUSREADER_HH
