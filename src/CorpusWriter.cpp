#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>

namespace alpinocorpus {
    void CorpusWriter::write(std::string const &name, std::string const &content)
    {
        writeEntry(name, content);
    }
    
    void CorpusWriter::write(CorpusReader const &corpus, bool failsafe)
    {
        writeEntry(corpus, failsafe);
    }
}
