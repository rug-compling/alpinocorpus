#ifndef DIRECTORYCORPUSREADER_HH
#define DIRECTORYCORPUSREADER_HH

#include <QString>
#include <QVector>

#include "CorpusReader.hh"

namespace indexedcorpus {

class DirectoryCorpusReader : public CorpusReader
{
public:
    DirectoryCorpusReader(QString const &directory);
    QVector<QString> entries() const;
    QString read(const QString &entry);
private:
    QString d_directory;
    QVector<QString> d_entries;
};

}

#endif // DIRECTORYCORPUSREADER_HH
