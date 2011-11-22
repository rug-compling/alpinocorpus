#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>
#include <config.hh>

namespace alpinocorpus {
    CorpusWriter *CorpusWriter::open(std::string const &filename,
        bool overwrite, WriterType writerType)
    {
      switch (writerType) {
        case DBXML_CORPUS_WRITER:
#ifdef ALPINOCORPUS_WITH_DBXML
          return new DbCorpusWriter(filename, overwrite);
#else
          throw Error("AlpinoCorpus library without DBXML support");
#endif
          break;
        default:
          throw Error("Trying to write to a corpus of an unknown type type");
      }
    }

    bool CorpusWriter::writerAvailable(WriterType writerType)
    {
#ifndef ALPINOCORPUS_WITH_DBXML
      if (writerType == DBXML_CORPUS_WRITER)
        return false;
#endif
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
