// Experimental parser for macros
//
// TODO:
//
// - Eat whitespace between opening/closing quotes and the actual query.
//
// The C++ source file is automatically generated using Ragel. If you
// make changes to the automaton, use:
//
//  ragel parseMacros.rl -o parseMacros.cpp
//

#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#include <cstring>

#include <AlpinoCorpus/Error.hh>
#include <AlpinoCorpus/macros.hh>

#include "parseMacros.hh"

namespace {

%%{
	machine macros;
	write data;
}%%


struct Substitution
{
	size_t begin;
	size_t n;
	std::string macro;
};

typedef std::vector<Substitution> Substitutions;

}

namespace alpinocorpus {

Macros parseMacros(char const *data)
{
	int cs = 0;
	char const *p = data;
	char const *pe = p + std::strlen(data);
	char const *eof = p + std::strlen(data);

	char const *strStart = p;

	char const *substStart = 0;

	Macros macros;

	Substitutions substitutions;

	std::string lastKey;

	std::ostringstream buf;
%%{
	action str_char {
		buf << fc;
	}

	action key {
		lastKey = buf.str();
		buf.str("");
	}

	action queryStart {
		strStart = p;
	}

	action queryEnd {
		// This action is executed when the query is closed ("""), we have to
		// three to avoid including the quotes.
		std::string query(strStart, p - 3);

		// Apply substitutions that were found in the macro.
		for (Substitutions::const_reverse_iterator iter = substitutions.rbegin();
			iter != substitutions.rend(); ++iter)
		{
			Macros::const_iterator mIter = macros.find(iter->macro);
			if (mIter == macros.end())
			{
				std::cerr << "Unknown macro: " << iter->macro << std::endl;
				continue;
			}

			query.replace(iter->begin, iter->n, mIter->second);
		}

		substitutions.clear();
		macros[lastKey] = query;
	}

	action substBegin {
		substStart = p;
	}

	action substEnd {
		std::string macro(substStart, p);

		// substStart is the position of the macro name. Decrement by one to
		// get the position of the percentage sign.
		size_t begin = substStart - strStart - 1;

		// Add two, to account for both percentage signs.
		size_t n = p - substStart + 2;

		Substitution subst = {begin, n, macro};
		substitutions.push_back(subst);
	}

	separator = "\"\"\"";
	whitespace = [\n\r\t ]+;
	comment = '#' [^\n]* '\n';

	key = ([A-Za-z0-9_]+) $ str_char % key;

	substitution = ('%' % substBegin) (any - '%')+ ('%' > substEnd);
	query = (substitution | (any - '%')+)* -- separator;
	queryVal = (separator % queryStart) query (separator % queryEnd);

	main := ((whitespace | comment)* key whitespace* '=' whitespace* queryVal whitespace*)*;

	write init;
	write exec;
}%%

	if (cs < macros_first_final)
	{
		if (p == pe)
			throw Error("Unexpected end of file");
		else {
			std::ostringstream err;
			size_t errPos = p - data;
			err << "Error in macro file at position " << errPos << ":" << std::endl << std::endl;

			// Extract the line where the error occured
			std::string str(data);
			size_t ctxBegin = str.find_last_of("\r\n", errPos);
			ctxBegin = ctxBegin == std::string::npos ? 0 : ++ctxBegin;
			size_t ctxEnd = str.find_first_of("\r\n", errPos);

			std::string context = str.substr(ctxBegin, ctxEnd - ctxBegin);
			err << context << std::endl;

			size_t cursor = 0;
			if (errPos > ctxBegin)
				cursor = errPos - ctxBegin;

			for (size_t i = 0; i < cursor; ++i)
				err << " ";
			err << "^";

			throw Error(err.str());
		}
	}

	return macros;
}

}
