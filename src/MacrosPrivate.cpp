#include "MacrosPrivate.hh"

#include <map>
#include <stdexcept>

typedef std::map<std::string, std::string> ArgMap;
typedef std::map<std::string, Macro> Macros;

std::vector<std::string> strings(Chunks const &chunks)
{
	std::vector<std::string> result;

	for (Chunks::const_iterator iter = chunks.begin();
		iter != chunks.end(); ++iter)
	{
		if ((*iter)->type() == STRING_CHUNK)
		{
			StringChunk *stringChunk = reinterpret_cast<StringChunk *>(iter->get());
			result.push_back(stringChunk->string());
		}

	}

	return result;
}

void substArgs(ArgMap const &args, Chunks *chunks);
void substCalls(Macros const &macros, Chunks *chunks);

void applyMacro(Macros const &macros, Macro *macro, std::vector<std::string> const &args)
{
	if (macro->args().size() != args.size())
		throw std::runtime_error(std::string("Insufficient number of arguments for: " + macro->name()));

	ArgMap argMap;
	for (int i = 0; i < args.size(); ++i)
		argMap[macro->args()[i]] = args[i];

	substArgs(argMap, macro->chunks());
	substCalls(macros, macro->chunks());
}

void substArgs(ArgMap const &args, Chunks *chunks)
{
	for (Chunks::iterator iter = chunks->begin();
		iter != chunks->end(); ++iter)
	{
		// Variable chunk: replace variable by the given argument.
		if ((*iter)->type() == VARIABLE_CHUNK)
		{
			VariableChunk *varChunk = reinterpret_cast<VariableChunk *>(iter->get());
			ArgMap::const_iterator found = args.find(varChunk->name());
			if (found == args.end())
				throw std::runtime_error(std::string("Unknown argument variable: ") + varChunk->name());

			iter->reset(new StringChunk(found->second));
		}
		// Call chunk: replacements could be necessary in arguments.
		else if ((*iter)->type() == CALL_CHUNK)
		{
			CallChunk *callChunk = reinterpret_cast<CallChunk *>(iter->get());
			substArgs(args, callChunk->chunks());
		}
	}
}

void substCalls(Macros const &macros, Chunks *chunks)
{
	Chunks result;

	for (Chunks::iterator iter = chunks->begin();
		iter != chunks->end(); ++iter)
	{
		if ((*iter)->type() == CALL_CHUNK)
		{
			CallChunk *callChunk = reinterpret_cast<CallChunk *>(iter->get());
			Macros::const_iterator found = macros.find(callChunk->name());
			if (found == macros.end())
				throw std::runtime_error(std::string("Unknown macro: ") + callChunk->name());

			std::vector<std::string> args = strings(*callChunk->chunks());

			Macro macro = found->second;

			// XXX - copy macro's AST!!!
			applyMacro(macros, &macro, args);

			result.insert(result.end(), macro.chunks()->begin(), macro.chunks()->end());
		}
		else {
			result.push_back(*iter);
		}
	}
}

int main() {}