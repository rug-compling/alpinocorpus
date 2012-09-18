#ifndef MACROS_PRIVATE_HH
#define MACROS_PRIVATE_HH

#include <string>
#include <vector>

#include <tr1/memory>

enum ChunkType { STRING_CHUNK, CALL_CHUNK, VARIABLE_CHUNK };

class MacroChunk;

typedef std::vector<std::tr1::shared_ptr<MacroChunk> > Chunks;

class Macro
{
public:
	Macro(std::string const &name, std::vector<std::string> const &args,
			Chunks chunks) :
		d_name(name), d_args(args), d_chunks(chunks) {}

	std::vector<std::string> const &args() {return d_args;}
	Chunks *chunks() { return &d_chunks; }
	std::string name() {return d_name;}


private:
	std::string d_name;
	std::vector<std::string> d_args;
	Chunks d_chunks;
};

class MacroChunk
{
public:

	virtual MacroChunk *copy() = 0;
	virtual ChunkType type() = 0;
	virtual ~MacroChunk() {}
};

class StringChunk : public MacroChunk
{
public:
	StringChunk(std::string str) : d_string(str) {}
	virtual ~StringChunk() {}

	virtual MacroChunk *copy();
	std::string const &string();
	ChunkType type();
private:
	std::string d_string;
};

inline std::string const &StringChunk::string()
{
	return d_string;
}

inline ChunkType StringChunk::type()
{
	return STRING_CHUNK;
}

inline MacroChunk *StringChunk::copy()
{
	return new StringChunk(d_string);
}

class CallChunk : public MacroChunk
{
public:
	CallChunk(std::string const &name, Chunks const &chunks)
		: d_name(name), d_chunks(chunks) {}
	virtual ~CallChunk() {}

	std::string const &name() { return d_name; }
	Chunks *chunks() { return &d_chunks; }
private:
	std::string d_name;
	Chunks d_chunks;
};

class VariableChunk : public MacroChunk
{

public:
	VariableChunk(std::string const &name) : d_name(name) {}
	virtual ~VariableChunk() {}

	std::string const &name();
	ChunkType type();
private:
	std::string d_name;
};

inline std::string const &VariableChunk::name()
{
	return d_name;
}

inline ChunkType VariableChunk::type()
{
	return VARIABLE_CHUNK;
}


#endif // MACROS_PRIVATE_HH
