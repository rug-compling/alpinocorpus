#include <cstdlib>
#include <cstring>
#include <exception>
#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/CorpusReaderFactory.hh>
#include <AlpinoCorpus/capi.h>

extern "C" {

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
    alpinocorpus_iter i = new alpinocorpus_iter_t(corpus->corpusReader->begin());
    
    return i;
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

int alpinocorpus_iter_end(alpinocorpus_reader reader, alpinocorpus_iter iter)
{
  return iter->entryIter == reader->corpusReader->end();
}

void alpinocorpus_iter_next(alpinocorpus_reader reader,
    alpinocorpus_iter iter)
{
  ++(iter->entryIter);
}
    
char *alpinocorpus_iter_value(alpinocorpus_iter iter)
{
    std::string entry;
    try {
        entry = *(iter->entryIter);
    } catch (std::exception const &) {
        return NULL;
    }
    
    size_t len = entry.size() + 1;
    char *cstr = reinterpret_cast<char *>(std::malloc(len));
    if (cstr != NULL)
        std::memcpy(cstr, entry.c_str(), len);
    return cstr;
}

char *alpinocorpus_iter_contents(alpinocorpus_reader reader,
    alpinocorpus_iter iter)
{
    if (reader->corpusReader == NULL)
        return NULL;

    std::string contents;
    try {
        contents = iter->entryIter.contents(*reader->corpusReader);
    } catch (std::exception const &) {
        return NULL;
    }

    size_t len = contents.size() + 1;
    char *cstr = reinterpret_cast<char *>(std::malloc(len));
    if (cstr != NULL)
        std::memcpy(cstr, contents.c_str(), len);
    
    return cstr;
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

char *alpinocorpus_name(alpinocorpus_reader reader)
{
    std::string str(reader->corpusReader->name());
    size_t len = str.size() + 1;
    char *cstr = reinterpret_cast<char *>(std::malloc(len));
    if (cstr)
        std::memcpy(cstr, str.c_str(), len);
    return cstr;
}

unsigned long long alpinocorpus_size(alpinocorpus_reader reader)
{
    return (unsigned long long) reader->corpusReader->size();
}

}
