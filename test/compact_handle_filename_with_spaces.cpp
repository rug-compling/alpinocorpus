#include <string>

#include <AlpinoCorpus/CompactCorpusReader.hh>

namespace ac = alpinocorpus;

static std::string const corpus_path = "test_corpora/filename_with_spaces.index";

int main(int argc, char *argv[])
{
  ac::CompactCorpusReader reader(corpus_path);
  ac::CompactCorpusReader::EntryIterator iter = reader.entries();
  ac::Entry entry = iter.next(reader);
  if (entry.name == "filename with spaces.xml") {
    return 0;
  }

  return 1;
}
