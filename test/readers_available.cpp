#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusReaderFactory.hh>

namespace ac = alpinocorpus;

typedef std::list<ac::CorpusReaderFactory::ReaderInfo> ReaderList;
typedef std::list<std::string> ExtList;

int main(int argc, char *argv[])
{
  ReaderList readers = ac::CorpusReaderFactory::readersAvailable();

  for (ReaderList::const_iterator iter = readers.begin();
    iter != readers.end(); ++iter)
  {
    std::cout << iter->description << " ( ";
    std::copy(iter->extensions.begin(), iter->extensions.end(),
        std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << "): ";

    // Corresponds with CorpusReader::readerAvailable?
    if (ac::CorpusReaderFactory::readerAvailable(iter->readerType))
      std::cout << "OK";
    else
      std::cout << "Error in readerAvailable!";

    std::cout << std::endl;
  }


  return 0;
}
