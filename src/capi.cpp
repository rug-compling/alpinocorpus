#include <cstdlib>
#include <cstring>

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

alpinocorpus_reader_p alpinocorpus_open(char const *path)
{
    alpinocorpus::CorpusReader *reader = alpinocorpus::CorpusReader::open(path);
    
    return new alpinocorpus_reader_t(reader);
}
    
void alpinocorpus_close(alpinocorpus_reader_p reader)
{
    delete reader->corpusReader;
    delete reader;
}

alpinocorpus_iter_p alpinocorpus_entry_iter(alpinocorpus_reader_p corpus)
{
    if (corpus->corpusReader->begin() == corpus->corpusReader->end())
        return NULL;
    
    alpinocorpus_iter_p i = new alpinocorpus_iter_t(corpus->corpusReader->begin());
    
    return i;        
}

alpinocorpus_iter_p alpinocorpus_query_iter(alpinocorpus_reader_p reader, char const *query)
{
    alpinocorpus::CorpusReader::EntryIterator iter =
        reader->corpusReader->query(alpinocorpus::CorpusReader::XPATH, query);
    
    if (iter == reader->corpusReader->end())
        return NULL;
    
    alpinocorpus_iter_p i = new alpinocorpus_iter_t(iter);
    
    return i;    
}

void alpinocorpus_iter_next(alpinocorpus_reader_p reader, alpinocorpus_iter_p *iter)
{
    if (++(*iter)->entryIter == reader->corpusReader->end()) {
        free(*iter);
        *iter = NULL;
    }
}
    
char *alpinocorpus_iter_value(alpinocorpus_iter_p iter)
{
    QString entry = *(iter->entryIter);
    QByteArray data = entry.toUtf8();
    size_t len = strlen(data.constData()) + 1;
    char *cstr = reinterpret_cast<char *>(malloc(sizeof(char) * len));
    strcpy(cstr, data.constData());
    return cstr;    
}

char *alpinocorpus_read(alpinocorpus_reader_p reader, char const *entry)
{
    QString str = reader->corpusReader->read(entry);
    QByteArray data = str.toUtf8();
    size_t len = strlen(data.constData()) + 1;
    char *cstr = reinterpret_cast<char *>(malloc(sizeof(char) * len));
    strcpy(cstr, data.constData());
    return cstr;
}

}