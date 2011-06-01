#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string>

#include <QString>

#include "ProgramOptions.hh"

ProgramOptions::ProgramOptions(int argc, char const *argv[], char const *optString)
	: d_programName(argv[0])
{
	opterr = 0;
	
	int opt;
	while ((opt = getopt(argc, const_cast<char * const *>(argv), optString)) != -1)
	{
		if (opt == '?' || opt == '|')
			throw std::runtime_error(std::string("Unknown option or missing argument: ") +
				static_cast<char>(optopt));
		
		d_options.insert(opt);
		
		if (optarg != 0)
			d_optionValues[opt] = optarg;
	}
	
  std::copy(argv + optind, argv + argc, std::back_inserter(d_arguments));
}

QString const &ProgramOptions::optionValue(char option) const
{
	QMap<char, QString>::const_iterator iter = d_optionValues.find(option);
	if (iter == d_optionValues.end())
		throw std::runtime_error("Tried to extract the value for an option without argument.");
	return iter.value();
}
