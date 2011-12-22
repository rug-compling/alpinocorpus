#ifndef ALPINO_CORPUSREADER_FACTORY_HH
#define ALPINO_CORPUSREADER_FACTORY_HH

#include <list>
#include <string>

namespace alpinocorpus {
    class CorpusReader;

    class ALPINO_CORPUS_EXPORT CorpusReaderFactory
    {
    public:
        /**
         * Reader types
         */
        enum ReaderType {
            DIRECTORY_CORPUS_READER = 0,
            COMPACT_CORPUS_READER,
            DBXML_CORPUS_READER
        };

        /**
         * Reader information
         */
        struct ReaderInfo {
          ReaderInfo(ReaderType newType, std::string newDescription,
              std::list<std::string> newExtensions) :
            readerType(newType),
            description(newDescription),
            extensions(newExtensions) {}

          ReaderType readerType;
          std::string description;
          std::list<std::string> extensions;
        };

        /**
         * Factory method: open a corpus, determining its type automatically.
         * The caller is responsible for deleting the object returned.
         */
        static CorpusReader *open(std::string const &corpusPath);

        /**
         * Factory method: open a directory with multiple corpora recursively.
         * The caller is responsible for deleting the object returned.
         */
        static CorpusReader *openRecursive(std::string const &path);

        /**
         * Check whether a particular reader type is available.
         */
         static bool readerAvailable(ReaderType readerType);

        /**
         * Retrieve a list of available corpus readers.
         */
         static std::list<ReaderInfo> readersAvailable();
    private:
        CorpusReaderFactory();
        CorpusReaderFactory(CorpusReaderFactory const &);
        CorpusReaderFactory &operator=(CorpusReaderFactory const &);
    };

}

#endif // ALPINO_CORPUSREADER_FACTORY_HH
