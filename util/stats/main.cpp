#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include <tr1/memory>
#include <tr1/unordered_map>

#include <boost/scoped_ptr.hpp>

#include <AlpinoCorpus/CorpusReader.hh>

#include <ProgramOptions.hh>
#include <util.hh>

using alpinocorpus::CorpusReader;

namespace tr1 = std::tr1;

typedef tr1::unordered_map<std::string, size_t> ValueCounts;

ValueCounts countQuery(tr1::shared_ptr<CorpusReader> reader,
    std::string const &query)
{
    CorpusReader::EntryIterator i, end(reader->end());
  
    ValueCounts counts;
    for (CorpusReader::EntryIterator iter = reader->query(CorpusReader::XPATH, query);
            iter != reader->end(); ++iter)
        ++counts[iter.contents(*reader)];
    
    return counts;
}

void printFrequencies(tr1::shared_ptr<CorpusReader> reader,
    ValueCounts const &counts, bool relative)
{
    if (relative)
    {
        size_t count = 0;
        for (ValueCounts::const_iterator iter = counts.begin();
                iter != counts.end(); ++iter)
            count += iter->second;

        for (ValueCounts::const_iterator iter = counts.begin();
                iter != counts.end(); ++iter)
            std::cout << iter->first << " " <<
                (static_cast<double>(iter->second) / count) << std::endl;

    }
    else
        for (ValueCounts::const_iterator iter = counts.begin();
                iter != counts.end(); ++iter)
            std::cout << iter->first << " " << iter->second << std::endl;
}


void usage(std::string const &programName)
{
    std::cerr << "Usage: " << programName << " [OPTION] query treebank" <<
        std::endl << std::endl <<
        "  -p\tRelative item frequencies" << std::endl <<
        "  -r\tProcess a directory of corpora recursively" << std::endl << std::endl;

}

int main(int argc, char *argv[])
{
    boost::scoped_ptr<ProgramOptions> opts;
    try {
        opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
            "pr"));
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (opts->arguments().size() != 2)
    {
        usage(opts->programName());
        return 1;
    }

    tr1::shared_ptr<CorpusReader> reader;
    try {
        reader.reset(openCorpus(opts->arguments().at(1), opts->option('r')));
    } catch (std::runtime_error &e) {
        std::cerr << "Could not open corpus: " << e.what() << std::endl;
        return 1;
    }

    std::string query;
    query = opts->arguments().at(0);
    if (!reader->isValidQuery(CorpusReader::XPATH, false, query)) {
        std::cerr << "Invalid (or unwanted) query: " << query << std::endl;
        return 1;
    }

    ValueCounts counts(countQuery(reader, query));
    printFrequencies(reader, counts, opts->option('p'));
}
