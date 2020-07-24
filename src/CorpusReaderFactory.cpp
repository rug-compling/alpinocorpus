#include <list>
#include <string>

#include <AlpinoCorpus/CompactCorpusReader.hh>
#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusReader.hh>
#include <AlpinoCorpus/DirectoryCorpusReader.hh>
#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/RecursiveCorpusReader.hh>

namespace alpinocorpus {

    CorpusReader *CorpusReaderFactory::open(std::string const &corpusPath)
    {
        try {
            return new DirectoryCorpusReader(corpusPath);
        } catch (OpenError const &) {}

        try {
            return new CompactCorpusReader(corpusPath);
        } catch (OpenError const &) {}

        try {
            return new DbCorpusReader(corpusPath);
        } catch (OpenError const &) {}

        throw OpenError(corpusPath);
    }

    CorpusReader *CorpusReaderFactory::openRecursive(std::string const &path,
        bool dactOnly)
    {
      return new RecursiveCorpusReader(path, dactOnly);
    }

    bool CorpusReaderFactory::readerAvailable(ReaderType readerType)
    {
        return true;
    }

    std::list<CorpusReaderFactory::ReaderInfo> CorpusReaderFactory::readersAvailable()
    {
        std::list<ReaderInfo> readers;

        // XXX - How to present directory corpus readers?

        readers.push_back(ReaderInfo(DIRECTORY_CORPUS_READER,
            "Directory reader", std::list<std::string>()));

        readers.push_back(ReaderInfo(DBXML_CORPUS_READER,
            "Dact (DBXML) corpus reader", std::list<std::string>(1, "dact")));

        readers.push_back(ReaderInfo(COMPACT_CORPUS_READER,
            "Compact corpus reader", std::list<std::string>(1, "data.dz")));

        return readers;
    }


}
