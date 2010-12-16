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

    if (args.length() != 3) {
        std::cerr << "usage: " << argv[0] << " from to.dbxml" << std::endl;
        return 1;
    }

    try {
        QScopedPointer<CorpusReader> reader(CorpusReader::open(args[1]));
        DbCorpusWriter(args[2], false).write(*reader);
    } catch (alpinocorpus::Error const &e) {
        std::cerr << argv[0] << ": error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
