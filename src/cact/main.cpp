#include <AlpinoCorpus/CorpusReader.hh>
#include <AlpinoCorpus/DbCorpusWriter.hh>
#include <AlpinoCorpus/Error.hh>

#include <QCoreApplication>
#include <QFileInfo>
#include <QRegExp>
#include <QScopedPointer>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <iostream>
#include <stdexcept>

#include "ProgramOptions.hh"

using alpinocorpus::CorpusReader;
using alpinocorpus::DbCorpusWriter;

void listCorpus(QString const &treebank, QString const &query)
{
  QScopedPointer<CorpusReader> rd(CorpusReader::open(treebank));
  CorpusReader::EntryIterator i, end(rd->end());
  
  if (query.isNull())
    i = rd->begin();
  else
    i = rd->query(CorpusReader::XPATH, query);

  QTextStream outStream(stdout);
  for (; i != end; ++i)
    outStream << *i << "\n";
  
}

void usage(QString const &programName)
{
    std::cerr << "Usage: " << programName.toUtf8().constData() << " [OPTION] treebank" <<
      std::endl << std::endl <<
      "  -c filename\tCreate a Dact dbxml archive" << std::endl <<
      "  -q query\tFilter the treebank using the given query" << std::endl << std::endl;  
}

void writeDactCorpus(QString const &treebank, QString const &treebankOut,
  QString const &query)
{
  if (QFileInfo(treebankOut).absoluteFilePath() ==
      QFileInfo(treebank).absoluteFilePath())
    throw std::runtime_error("Attempting to write to the source treebank.");
  
  QScopedPointer<CorpusReader> rd(CorpusReader::open(treebank));
    
  DbCorpusWriter wr(treebankOut, true);
  CorpusReader::EntryIterator i, end(rd->end());
  if (query.isNull())
    i = rd->begin();
  else
    i = rd->query(CorpusReader::XPATH, query);
  
  QSet<QString> seen;
  for (; i != end; ++i)
    if (!seen.contains(*i)) {
      wr.write(*i, rd->read(*i));
      seen.insert(*i);
    }
}

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  
  ProgramOptions opts(argc, const_cast<char const **>(argv), "c:lq:");
  
  if (opts.arguments().size() != 1)
  {
    usage(opts.programName());
    return 1;
  }
  
  if (opts.option('c') && opts.option('l')) {
    std::cerr << opts.programName().toUtf8().constData() <<
      ": the '-c' and '-l' options cannot be used simultaneously." <<
      std::endl;
    return 1;
  }
  
  if (!opts.option('c') && !opts.option('l')) {
    std::cerr << opts.programName().toUtf8().constData() <<
    ": either the '-c' or '-l' option should be used." <<
    std::endl;
    return 1;
  }
  
  QString query;
  if (opts.option('q'))
    query = opts.optionValue('q');  
  
  if (opts.option('c')) {
    try {
      QString treebankOut = opts.optionValue('c');
      writeDactCorpus(opts.arguments().at(0), treebankOut, query);
    } catch (std::runtime_error const &e) {
      std::cerr << opts.programName().toUtf8().constData() <<
        ": error creating Dact treebank: " << e.what() << std::endl;
      return 1;
    }
  }
  
  if (opts.option('l')) {
    try {
      listCorpus(opts.arguments().at(0), query);
    } catch (std::runtime_error const &e) {
      std::cerr << opts.programName().toUtf8().constData() <<
      ": error listing treebank: " << e.what() << std::endl;
      return 1;
    }    
  }
  
  return 0;
}
