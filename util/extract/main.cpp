#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/macros.hh>

#include <ProgramOptions.hh>
#include <util.hh>

using alpinocorpus::CorpusReader;
using alpinocorpus::Either;


void usage(std::string const &programName) {
    std::cerr << "Usage: " << programName << " treebank directory" << std::endl;
}

int main(int argc, char *argv[]) {
    std::unique_ptr<ProgramOptions> opts;
    try {
        opts.reset(new ProgramOptions(argc, const_cast<char const **>(argv),
                "q"));
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (opts->arguments().size() < 2) {
        usage(opts->programName());
        return 1;
    }

    std::shared_ptr<CorpusReader> reader;
    try {
        reader = openCorpus(opts->arguments().at(0), true);
    } catch (std::runtime_error &e) {
        std::cerr << "Could not open corpus: " << e.what() << std::endl;
        return 1;
    }

    std::filesystem::path targetPath = opts->arguments().at(1);

    CorpusReader::EntryIterator iter = reader->entries();
    while (iter.hasNext()) {
        alpinocorpus::Entry entry = iter.next(*reader);

        std::filesystem::path entryPath(entry.name);
        std::filesystem::path filePath = targetPath / entryPath;

        if (!std::filesystem::exists(filePath.parent_path())) {
            std::filesystem::create_directories(filePath.parent_path());
        }

        std::string content = reader->read(entry.name,
                std::list<CorpusReader::MarkerQuery>());

        std::ofstream out(filePath.c_str());
        if (!out) {
            std::cerr << "Could not write to " << entryPath << std::endl;
            return 1;
        }

        out.write(content.c_str(), content.size());
    }
}
