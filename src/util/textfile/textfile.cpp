#include "textfile.ih"

vector<unsigned char> indexedcorpus::readFile(string const &filename)
{
	path p(filename);
	if (!is_regular_file(p))
          throw runtime_error(string("readFile: '") + filename + "' is not a regular file!");

	vector<unsigned char> data;

	ifstream dataStream(filename.c_str());
	if (!dataStream)
          throw runtime_error(string("readFile: '") + filename + "' could be read!");

	dataStream >> noskipws;

	copy(istream_iterator<char>(dataStream), istream_iterator<char>(),
		back_inserter(data));
	
	return data;
}
