#include <algorithm>
#include <regex>
#include <string>
#include <vector>

std::vector<std::string> split_string(std::string const &s, std::regex const &re) {
  std::vector<std::string> parts;

  std::copy(std::sregex_token_iterator(s.begin(), s.end(), re, -1),
	    std::sregex_token_iterator(),
	    std::back_inserter(parts));

  return parts;
}
