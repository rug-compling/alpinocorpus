#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>

#include <QCoreApplication>
#include <QScopedPointer>
#include <QStringList>

#include <iostream>

using alpinocorpus::CorpusReader;
using alpinocorpus::DbCorpusWriter;

/*
 * Convert corpus to DBXML format
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();

    if (args.length() != 3 && args.length() != 4) {
        std::cerr << "usage: " << argv[0] << " [query] from to.dbxml\n";
        return 1;
    }

    size_t from, query, to;
    if (args.length() == 3) {
        query = 0;
        from  = 1;
        to    = 2;
    } else {
        query = 1;
        from  = 2;
        to    = 3;
    }

    try {
        QScopedPointer<CorpusReader> rd(CorpusReader::open(args[from]));
        DbCorpusWriter wr(args[to], true);
        CorpusReader::EntryIterator i, end(rd->end());
        if (query)
            i = rd->query(args[query]);
        else
            i = rd->begin();
        for (; i != end; ++i)
            wr.write(*i, rd->read(*i));
    } catch (alpinocorpus::Error const &e) {
        std::cerr << argv[0] << ": " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
