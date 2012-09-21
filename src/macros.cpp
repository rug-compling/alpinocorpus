#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

#include <boost/algorithm/string/replace.hpp>

#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/macros.hh>

#include "parseMacros.hh"

namespace {
	std::string macroMarker("%");
}

namespace alpinocorpus {

Macros loadMacros(std::string const &filename) {
	std::ifstream macroStream(filename.c_str());
	if (!macroStream)
		throw Error(std::string("Could not read macro file: ") +
			filename);

	std::ostringstream macroDataStream;

	macroStream >> std::noskipws;
	std::copy(std::istream_iterator<char>(macroStream),
		std::istream_iterator<char>(),
		std::ostream_iterator<char>(macroDataStream));

	std::string macroData = macroDataStream.str();

	return parseMacros(macroData.c_str());
}

std::string expandMacros(Macros const &macros, std::string query)
{
	for (Macros::const_iterator iter = macros.begin();
			iter != macros.end(); ++iter)
	{
		boost::replace_all(query, macroMarker + iter->first + macroMarker,
			iter->second);
	}

	return query;
}

}
