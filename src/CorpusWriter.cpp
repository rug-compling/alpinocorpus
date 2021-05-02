#include <string>

#include <AlpinoCorpus/CompactCorpusWriter.hh>
#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>

namespace alpinocorpus {
    CorpusWriter *CorpusWriter::open(std::string const &filename,
        bool overwrite, WriterType writerType)
    {
      switch (writerType) {
        case DBXML_CORPUS_WRITER:
          return new DbCorpusWriter(filename, overwrite);
        case COMPACT_CORPUS_WRITER:
          return new CompactCorpusWriter(filename);
        default:
          throw Error("Trying to write to a corpus of an unknown type type");
      }
    }

    bool CorpusWriter::writerAvailable(WriterType writerType)
    {
      return true;
    }

    void CorpusWriter::write(std::string const &name, std::string const &content)
    {
        writeEntry(name, content);
    }
    
    void CorpusWriter::write(CorpusReader const &corpus, bool failsafe)
    {
        writeEntry(corpus, failsafe);
    }
}
