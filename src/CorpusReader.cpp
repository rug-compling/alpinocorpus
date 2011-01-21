#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IndexedCorpusReader.hh>

#include <QString>
#include <typeinfo>

namespace alpinocorpus {
    CorpusReader *CorpusReader::open(QString const &corpusPath)
    {
        try {
            return new DirectoryCorpusReader(corpusPath);
        } catch (OpenError const &e) {
        }

        try {
            return new IndexedCorpusReader(corpusPath);
        } catch (OpenError const &e) {
        }

        return new DbCorpusReader(corpusPath);
    }

    CorpusReader::EntryIterator CorpusReader::runQuery(QString const &) const
    {
        throw NotImplemented(typeid(*this).name(), "XQuery functionality");
    }
}
