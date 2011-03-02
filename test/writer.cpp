#include <gtest/gtest.h>

#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <cstdio>
#include <QString>

namespace ac = alpinocorpus;

static char const db_path[] = "test_suite.dact";

class DbCorpusWriterTest : public ::testing::Test
{
  protected:
    ac::CorpusReader *reader;
    ac::DbCorpusWriter *writer;

    virtual void SetUp()
    {
        reader = ac::CorpusReader::open("test_suite");
    }

    virtual void TearDown()
    {
        (void)std::remove(db_path);
        delete reader;
    }
};

TEST_F(DbCorpusWriterTest, CanOpenDbForWriting)
{
    writer = new ac::DbCorpusWriter(QString(db_path), true);
    ASSERT_TRUE(writer != NULL);
}

TEST_F(DbCorpusWriterTest, CanWriteEntireReader)
{
    writer->write(*reader);
    ac::CorpusReader *wrtr_rdr = ac::CorpusReader::open(QString(db_path));
    ASSERT_EQ(reader->size(), wrtr_rdr->size());
    delete wrtr_rdr;
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
