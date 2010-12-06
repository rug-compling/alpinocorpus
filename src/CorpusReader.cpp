#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/IndexedCorpusReader.hh>

#include <QString>

namespace alpinocorpus {
    /*
     * XXX Should return some kind of smart pointer
     */
    CorpusReader *CorpusReader::newCorpusReader(QString const &corpusPath)
    {
        try {
            return new DbCorpusReader(corpusPath);
        } catch (OpenError const &e) {
        }

        try {
            return new IndexedCorpusReader(corpusPath);
        } catch (OpenError const &e) {
        }

        return new DirectoryCorpusReader(corpusPath);
    }
}
