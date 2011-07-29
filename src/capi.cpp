#include <cstdlib>
#include <cstring>
#include <exception>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/capi.h>

#include <QtDebug>


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
        reader = alpinocorpus::CorpusReader::open(path);
    } catch (std::exception const &e) {
        return NULL;
    }
    
    return new alpinocorpus_reader_t(reader);
}
    
void alpinocorpus_close(alpinocorpus_reader reader)
{
    delete reader->corpusReader;
    delete reader;
}

alpinocorpus_iter alpinocorpus_entry_iter(alpinocorpus_reader corpus)
{
    if (corpus->corpusReader->begin() == corpus->corpusReader->end())
        return NULL;
    
    alpinocorpus_iter i = new alpinocorpus_iter_t(corpus->corpusReader->begin());
    
    return i;        
}

alpinocorpus_iter alpinocorpus_query_iter(alpinocorpus_reader reader, char const *query)
{
    alpinocorpus::CorpusReader::EntryIterator iter;
    
    try {
        iter = reader->corpusReader->query(alpinocorpus::CorpusReader::XPATH, query);
    } catch (std::exception const &e) {
        return NULL;
    }
        
    if (iter == reader->corpusReader->end())
        return NULL;
    
    alpinocorpus_iter i = new alpinocorpus_iter_t(iter);
    
    return i;    
}

void alpinocorpus_iter_next(alpinocorpus_reader reader, alpinocorpus_iter *iter)
{
    if (++(*iter)->entryIter == reader->corpusReader->end()) {
        free(*iter);
        *iter = NULL;
    }
}
    
char *alpinocorpus_iter_value(alpinocorpus_iter iter)
{
    QString entry;
    try {
        entry = *(iter->entryIter);
    } catch (std::exception const &e) {
        return NULL;
    }
    
    QByteArray data = entry.toUtf8();
    size_t len = strlen(data.constData()) + 1;
    char *cstr = reinterpret_cast<char *>(malloc(sizeof(char) * len));
    strlcpy(cstr, data.constData(), len);
    return cstr;    
}

char *alpinocorpus_read(alpinocorpus_reader reader, char const *entry)
{
    QString str;
    try{ 
        str = reader->corpusReader->read(entry);
    } catch (std::exception const &e) {
        return NULL;
    }
    
    QByteArray data = str.toUtf8();
    size_t len = strlen(data.constData()) + 1;
    char *cstr = reinterpret_cast<char *>(malloc(sizeof(char) * len));
    strlcpy(cstr, data.constData(), len);
    return cstr;
}

}