#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <stdexcept>
#include <string>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/macros.hh>

#include <ProgramOptions.hh>
#include <util.hh>
#include <boost/filesystem/operations.hpp>

using alpinocorpus::CorpusReader;
using alpinocorpus::Either;

void usage(std::string const &programName) {
    std::cerr << "Usage: " << programName << " treebank entry" <<
      std::endl << std::endl <<
      "  -a attribute\tAttribute for marking (default: active)" << std::endl <<
      "  -m filename\tLoad macro file" << std::endl <<
      "  -q query\tMark nodes using a query" << std::endl <<
      "  -v value\tValue for marking (default: 1)" << std::endl << std::endl;
}

int main(int argc, char *argv[]) {
    boost::scoped_ptr<ProgramOptions> opts;
    try {
        opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
                "a:m:q:v:"));
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (opts->arguments().size() < 2) {
        usage(opts->programName());
        return 1;
    }

    boost::scoped_ptr<CorpusReader> reader;
    try {
        reader.reset(openCorpus(opts->arguments().at(0), true));
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

    std::list<CorpusReader::MarkerQuery> markerQueries;
    if (opts->option('q')) {
        std::string query = alpinocorpus::expandMacros(macros, opts->optionValue('q'));

        Either<std::string, alpinocorpus::Empty> valid =
          reader->isValidQuery(CorpusReader::XPATH, false, query);
        if (valid.isLeft()) {
            std::cerr << "Invalid (or unwanted) query: " << query << std::endl << std::endl;
            std::cerr << valid.left() << std::endl;
            return 1;
        }

        std::string attribute = "active";
        if (opts->option('a'))
            attribute = opts->optionValue('a');

        std::string value = "1";
        if (opts->option('v'))
            value = opts->optionValue('v');

        markerQueries.push_back(CorpusReader::MarkerQuery(query, attribute, value));
    }

    std::cout << reader->read(opts->arguments().at(1), markerQueries);

}
