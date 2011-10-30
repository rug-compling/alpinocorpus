#ifndef ALPINOCORPUS_COMPACT_CORPUS_WRITER
#define ALPINOCORPUS_COMPACT_CORPUS_WRITER

#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>

namespace alpinocorpus {

class CompactCorpusWriterPrivate;

class CompactCorpusWriter : public CorpusWriter
{
public:
    CompactCorpusWriter(std::string const &basename);
    virtual ~CompactCorpusWriter();

protected:
    void writeEntry(std::string const &name, std::string const &content);
    void writeEntry(CorpusReader const &corpus, bool failsafe = false);

private:
    CompactCorpusWriterPrivate *d_private;
};

}

#endif // ALPINOCORPUS_COMPACT_CORPUS_WRITER
