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

    CorpusReader::EntryIterator CorpusReader::query(CorpusReader::Dialect d,
                                                    QString const &q) const
    {
        switch (d) {
          case XPATH:  return runXPath(q);
          case XQUERY: return runXQuery(q);
          default:     throw NotImplemented("unknown query language");
        }
    }

    CorpusReader::EntryIterator CorpusReader::runXPath(QString const &) const
    {
        throw NotImplemented(typeid(*this).name(), "XQuery functionality");
    }

    CorpusReader::EntryIterator CorpusReader::runXQuery(QString const &) const
    {
        throw NotImplemented(typeid(*this).name(), "XQuery functionality");
    }
}
