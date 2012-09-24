// #define CAPI_DEBUG

#ifdef CAPI_DEBUG
#include <iostream>
#endif
#include <cstdlib>
#include <cstring>
#include <exception>
#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusWriter.hh>
#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/capi.h>

extern "C" {

#include <libxslt/xslt.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libexslt/exslt.h>

void alpinocorpus_initialize()
{
  xmlInitMemory();
  xmlInitParser();
  xmlXPathInit();
  exsltRegisterAll();
}

void alpinocorpus_cleanup()
{
  xsltCleanupGlobals();
  xmlCleanupParser();
}

struct alpinocorpus_entry_t {
  char const *name;
  char const *contents;
};

struct alpinocorpus_reader_t {
    alpinocorpus_reader_t(alpinocorpus::CorpusReader *reader) : corpusReader(reader) {}

    alpinocorpus::CorpusReader *corpusReader;
};

struct alpinocorpus_iter_t {
    alpinocorpus_iter_t(alpinocorpus::CorpusReader::EntryIterator iter) : entryIter(iter) {}

    alpinocorpus::CorpusReader::EntryIterator entryIter;
};

alpinocorpus_reader alpinocorpus_open(char const *path)
{
    alpinocorpus::CorpusReader *reader;

    try {
        reader = alpinocorpus::CorpusReaderFactory::open(path);
    } catch (std::exception const &) {
        return NULL;
    }

    return new alpinocorpus_reader_t(reader);
}

alpinocorpus_reader alpinocorpus_open_recursive(char const *path)
{
    alpinocorpus::CorpusReader *reader;

    try {
        reader = alpinocorpus::CorpusReaderFactory::openRecursive(path);
    } catch (std::exception const &) {
        return NULL;
    }

    return new alpinocorpus_reader_t(reader);
}

void alpinocorpus_close(alpinocorpus_reader reader)
{
    delete reader->corpusReader;
    delete reader;
}

int alpinocorpus_is_valid_query(alpinocorpus_reader reader, char const *query)
{
  return int(reader->corpusReader->isValidQuery(
      alpinocorpus::CorpusReader::XPATH, false, query));
}

alpinocorpus_iter alpinocorpus_entry_iter(alpinocorpus_reader corpus)
{
    alpinocorpus_iter i = new alpinocorpus_iter_t(corpus->corpusReader->entries());

    return i;
}

char const * alpinocorpus_entry_contents(alpinocorpus_entry entry)
{
    return entry->contents;
}

void alpinocorpus_entry_free(alpinocorpus_entry entry)
{
    std::free(const_cast<char *>(entry->name));
    std::free(const_cast<char *>(entry->contents));
    std::free(entry);
}

char const * alpinocorpus_entry_name(alpinocorpus_entry entry)
{
    return entry->name;
}

alpinocorpus_iter alpinocorpus_query_stylesheet_iter(alpinocorpus_reader corpus,
    char const *query, char const *stylesheet, marker_query_t *queries,
    size_t n_queries)
{
    std::list<alpinocorpus::CorpusReader::MarkerQuery> markerQueries;

    for (size_t i = 0; i < n_queries; ++i) {
        alpinocorpus::CorpusReader::MarkerQuery query(
          queries[i].query,
          queries[i].attr,
          queries[i].value
        );

        markerQueries.push_back(query);
    }

    alpinocorpus::CorpusReader::EntryIterator iter;
    try {
        iter = corpus->corpusReader->queryWithStylesheet(
            alpinocorpus::CorpusReader::XPATH, query, stylesheet,
            markerQueries);
    } catch (std::exception const &) {
        return NULL;
    }

    return new alpinocorpus_iter_t(iter);
}

alpinocorpus_iter alpinocorpus_query_stylesheet_marker_iter(alpinocorpus_reader corpus,
							    char const *query,
							    char const *stylesheet,
							    char const *markerQuery,
							    char const *markerAttr,
							    char const *markerValue)
{
    marker_query_t m [1];
    m[0].query = markerQuery;
    m[0].attr = markerAttr;
    m[0].value = markerValue;
    return alpinocorpus_query_stylesheet_iter(corpus, query, stylesheet, m, 1);
}

alpinocorpus_iter alpinocorpus_query_iter(alpinocorpus_reader reader, char const *query)
{
    alpinocorpus::CorpusReader::EntryIterator iter;

    try {
        iter = reader->corpusReader->query(alpinocorpus::CorpusReader::XPATH, query);
    } catch (std::exception const &) {
        return NULL;
    }

    alpinocorpus_iter i = new alpinocorpus_iter_t(iter);

    return i;
}

void alpinocorpus_iter_destroy(alpinocorpus_iter iter)
{
    delete iter;
}

int alpinocorpus_iter_has_next(alpinocorpus_reader reader, alpinocorpus_iter iter)
{
    try {
        return iter->entryIter.hasNext();
    } catch (std::exception const &) {
        return 0;
    }
}

alpinocorpus_entry alpinocorpus_iter_next(alpinocorpus_reader reader,
    alpinocorpus_iter iter)
{
    alpinocorpus::Entry e;
    try {
        e = iter->entryIter.next(*reader->corpusReader);
    } catch (std::exception const &) {
        return NULL;
    }

    alpinocorpus_entry ce = reinterpret_cast<alpinocorpus_entry>(
        std::malloc(sizeof(alpinocorpus_entry_t)));

	ce->name = strdup(e.name.c_str());
    ce->contents = strdup(e.contents.c_str());

    return ce;
}

char *alpinocorpus_read(alpinocorpus_reader reader, char const *entry)
{
    std::string str;
    try{
        str = reader->corpusReader->read(entry);
    } catch (std::exception const &) {
        return NULL;
    }

    size_t len = str.size() + 1;
    char *cstr = reinterpret_cast<char *>(std::malloc(len));
    if (cstr)
        std::memcpy(cstr, str.c_str(), len);
    return cstr;
}

char *alpinocorpus_read_mark_queries(alpinocorpus_reader reader,
    char const *entry, marker_query_t *queries, size_t n_queries)
{
    std::list<alpinocorpus::CorpusReader::MarkerQuery> markerQueries;

    for (size_t i = 0; i < n_queries; ++i) {
        alpinocorpus::CorpusReader::MarkerQuery query(
          queries[i].query,
          queries[i].attr,
          queries[i].value
        );

        markerQueries.push_back(query);
    }

    std::string str;
    try{
        str = reader->corpusReader->read(entry, markerQueries);
    } catch (std::exception const &) {
        return NULL;
    }

    size_t len = str.size() + 1;
    char *cstr = reinterpret_cast<char *>(std::malloc(len));
    if (cstr)
        std::memcpy(cstr, str.c_str(), len);
    return cstr;
}

char *alpinocorpus_read_mark_query(alpinocorpus_reader reader,
				   char const *entry,
				   char const *markerQuery,
				   char const *markerAttr,
				   char const *markerValue)
{
    marker_query_t m [1];
    m[0].query = markerQuery;
    m[0].attr = markerAttr;
    m[0].value = markerValue;
    return alpinocorpus_read_mark_queries(reader, entry, m, 1);
}

char *alpinocorpus_name(alpinocorpus_reader reader)
{
    std::string str(reader->corpusReader->name());
    size_t len = str.size() + 1;
    char *cstr = reinterpret_cast<char *>(std::malloc(len));
    if (cstr)
        std::memcpy(cstr, str.c_str(), len);
    return cstr;
}

size_t alpinocorpus_size(alpinocorpus_reader reader)
{
    return reader->corpusReader->size();
}

struct alpinocorpus_writer_t {
    alpinocorpus_writer_t(alpinocorpus::CorpusWriter *writer) : corpusWriter(writer) {}

    alpinocorpus::CorpusWriter *corpusWriter;
};


alpinocorpus_writer alpinocorpus_writer_open(char const *path, int overwrite, char const *writertype)
{
    alpinocorpus::CorpusWriter *writer;
    alpinocorpus::CorpusWriter::WriterType wt;
    std::string WriterType(writertype);

    if (WriterType == "DBXML_CORPUS_WRITER")
        wt = alpinocorpus::CorpusWriter::DBXML_CORPUS_WRITER;
    else if (WriterType == "COMPACT_CORPUS_WRITER")
        wt = alpinocorpus::CorpusWriter::COMPACT_CORPUS_WRITER;
    else {
#ifdef CAPI_DEBUG
        std::cerr << "Invalid writertype " << writertype << std::endl;
#endif
        return NULL;
    }

    try {
        writer = alpinocorpus::CorpusWriter::open(path, overwrite ? true : false,  wt);
    } catch (std::exception const &e) {
#ifdef CAPI_DEBUG
        std::cerr << e.what() << std::endl;
#endif
        return NULL;
    }

    return new alpinocorpus_writer_t(writer);
}

void alpinocorpus_writer_close(alpinocorpus_writer writer)
{
    delete writer->corpusWriter;
    delete writer;
}

int alpinocorpus_writer_available(char const *writertype)
{
    alpinocorpus::CorpusWriter::WriterType wt;
    std::string WriterType(writertype);

    if (WriterType == "DBXML_CORPUS_WRITER")
        wt = alpinocorpus::CorpusWriter::DBXML_CORPUS_WRITER;
    else if (WriterType == "COMPACT_CORPUS_WRITER")
        wt = alpinocorpus::CorpusWriter::COMPACT_CORPUS_WRITER;
    else
        return 0;

    return alpinocorpus::CorpusWriter::writerAvailable(wt) ? 1 : 0;
}

char const * alpinocorpus_write(alpinocorpus_writer writer, char const *name, char const *content)
{
    try {
        writer->corpusWriter->write(name, content);
    } catch (std::exception const &e) {
        return e.what();
    }
    return NULL;
}

char const * alpinocorpus_write_corpus(alpinocorpus_writer writer, alpinocorpus_reader reader, int failsafe)
{
    try {
        writer->corpusWriter->write(*(reader->corpusReader), failsafe ? true : false);
    } catch (std::exception const &e) {
        return e.what();
    }
    return NULL;
}

}
