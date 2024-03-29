#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/macros.hh>

#include <ProgramOptions.hh>
#include <util.hh>

using alpinocorpus::CorpusReader;
using alpinocorpus::Either;


typedef std::unordered_map<std::string, size_t> ValueCounts;

ValueCounts countQuery(std::shared_ptr<CorpusReader> reader,
    std::string const &query)
{
    CorpusReader::EntryIterator i;

    ValueCounts counts;
    CorpusReader::EntryIterator iter = reader->query(CorpusReader::XPATH, query);
    while (iter.hasNext())
      ++counts[iter.next(*reader).contents];

  return counts;
}

void printFrequencies(ValueCounts const &counts, bool relative)
{
    if (relative)
    {
        size_t count = 0;
        for (auto iter = counts.begin(); iter != counts.end(); ++iter)
            count += iter->second;

        for (auto iter = counts.begin(); iter != counts.end(); ++iter)
            std::cout << iter->first << " " <<
        (static_cast<double>(iter->second) / count) << std::endl;

    }
    else
        for (auto iter = counts.begin(); iter != counts.end(); ++iter)
            std::cout << iter->first << " " << iter->second << std::endl;
}


void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] query treebanks" <<
    std::endl << std::endl <<
    "  -m filename\tLoad macro file" << std::endl <<
    "  -p\t\tRelative item frequencies" << std::endl << std::endl;

}

int main(int argc, char *argv[])
{
    std::unique_ptr<ProgramOptions> opts;
    try {
        opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
            "m:p"));
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (opts->arguments().size() < 2)
    {
        usage(opts->programName());
        return 1;
    }

    std::shared_ptr<CorpusReader> reader;
    try {
        if (opts->arguments().size() == 1)
          reader = std::shared_ptr<CorpusReader>(
            openCorpus(opts->arguments().at(0), true));
        else
          reader = openCorpora(opts->arguments().begin() + 1, 
                opts->arguments().end(), true);
    } catch (std::runtime_error &e) {
        std::cerr << "Could not open corpus: " << e.what() << std::endl;
        return 1;
    }

    alpinocorpus::Macros macros;
    if (opts->option('m')) {
        std::string macrosFn = opts->optionValue('m');
        try {
          macros = alpinocorpus::loadMacros(macrosFn);
        } catch (std::runtime_error &e) {
          std::cerr << e.what() << std::endl;
          return 1;
        }
    }

    std::string query = alpinocorpus::expandMacros(macros, opts->arguments().at(0));
    Either<std::string, alpinocorpus::Empty> valid =
      reader->isValidQuery(CorpusReader::XPATH, false, query);
    if (valid.isLeft()) {
      std::cerr << "Invalid (or unwanted) query: " << query << std::endl << std::endl;
      std::cerr << valid.left() << std::endl;
      return 1;
    }
    
    ValueCounts counts(countQuery(reader, query));
    printFrequencies(counts, opts->option('p'));
}
