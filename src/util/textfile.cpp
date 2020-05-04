#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "textfile.hh"

namespace alpinocorpus { namespace util {

std::string readFile(std::string const &filename)
{
    std::filesystem::path p(filename);
    
    if (!std::filesystem::is_regular_file(p))
        throw std::runtime_error(std::string("readFile: '")
                                + filename
                                + "' is not a regular file");
    
    std::string data;
    
    std::ifstream dataStream(filename.c_str());
    if (!dataStream)
        throw std::runtime_error(std::string("readFile: '")
                                 + filename
                                 + "' could not be opened for reading");
    
    dataStream >> std::noskipws;
    
    std::copy(std::istream_iterator<char>(dataStream), std::istream_iterator<char>(),
        std::back_inserter(data));

    return data;
}

} } // namespace alpinocorpus::util
