#include <string>

#include <AlpinoCorpus/CompactCorpusWriter.hh>
#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>

#include "CompactCorpusWriterPrivate.hh"

namespace alpinocorpus {

CompactCorpusWriter::CompactCorpusWriter(std::string const &basename) :
    d_private(new CompactCorpusWriterPrivate(basename))
{}

CompactCorpusWriter::~CompactCorpusWriter()
{
    delete d_private;
}

void CompactCorpusWriter::writeEntry(std::string const &name, std::string const &content)
{
    d_private->writeEntry(name, content);
}

void CompactCorpusWriter::writeEntry(CorpusReader const &corpus, bool failsafe)
{
    d_private->writeEntry(corpus, failsafe);
}


}
