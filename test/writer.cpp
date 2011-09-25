#include <cassert>
#include <cstdio>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>

#include "writer.hh"

namespace ac = alpinocorpus;

static char const db_path[] = "test_suite.dact";

void canOpenDbForWriting()
{
    ac::DbCorpusWriter *writer = new ac::DbCorpusWriter(db_path, true);
    assert(writer != 0);
    delete writer;
    std::remove(db_path);
}

void canWriteEntireReader()
{
    // Open the directory-based corpus as a reference.
    ac::CorpusReader *dir_rdr = ac::CorpusReader::open("test_suite");
    assert(dir_rdr != 0);
    
    // Write DBXML corpus.
    ac::DbCorpusWriter *writer = new ac::DbCorpusWriter(db_path, true);
    assert(writer != 0);
    writer->write(*dir_rdr);
    delete writer;

    // Check corpus correctness
    ac::CorpusReader *wrtr_rdr = ac::CorpusReader::open(db_path);
    assert(dir_rdr->size() == wrtr_rdr->size());
    
    // Cleanup
    delete wrtr_rdr;
    delete dir_rdr;
    std::remove(db_path);
}

int main(int argc, char *argv[])
{
    canOpenDbForWriting();
    canWriteEntireReader();
}