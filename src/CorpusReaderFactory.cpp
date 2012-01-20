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

#include <config.hh>

namespace alpinocorpus {

    CorpusReader *CorpusReaderFactory::open(std::string const &corpusPath)
    {
        try {
            return new DirectoryCorpusReader(corpusPath);
        } catch (OpenError const &) {}

        try {
            return new CompactCorpusReader(corpusPath);
        } catch (OpenError const &) {}

#if defined(USE_DBXML)
        try {
            return new DbCorpusReader(corpusPath);
        } catch (OpenError const &) {}
#endif
        
        throw OpenError(corpusPath);
    }

    CorpusReader *CorpusReaderFactory::openRecursive(std::string const &path)
    {
      return new RecursiveCorpusReader(path);
    }

    bool CorpusReaderFactory::readerAvailable(ReaderType readerType)
    {
#ifndef USE_DBXML
        if (readerType == DBXML_CORPUS_READER)
            return false;
#endif // USE_DBXML

        return true;
    }

    std::list<CorpusReaderFactory::ReaderInfo> CorpusReaderFactory::readersAvailable()
    {
        std::list<ReaderInfo> readers;

        // XXX - How to present directory corpus readers?

        readers.push_back(ReaderInfo(DIRECTORY_CORPUS_READER,
            "Directory reader", std::list<std::string>()));

        #ifdef USE_DBXML
        readers.push_back(ReaderInfo(DBXML_CORPUS_READER,
            "Dact (DBXML) corpus reader", std::list<std::string>(1, "dact")));
        #endif

        readers.push_back(ReaderInfo(COMPACT_CORPUS_READER,
            "Compact corpus reader", std::list<std::string>(1, "data.dz")));
        
        return readers;
    }


}
