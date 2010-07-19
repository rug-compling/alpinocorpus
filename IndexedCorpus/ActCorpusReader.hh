/*
 * Reader for Alpino Corpus Tool-style corpora
 *
 * Alpino uses compact corpora for treebanks, which can be read by combining
 * DzStreamBuf and IndexedCorpusReader instances. However, pathnames, rather
 * than arbitrary names are used. If paths lead to existing files, reading
 * a file directly is preferred over reading the compact corpus.
 *
 * This class implements such functionality.
 */


#ifndef ACT_CORPUS_READER
#define ACT_CORPUS_READER

#include <string>
#include <vector>

#include <QFileInfo>
#include <QMutex>
#include <QString>

#include "IndexNamePair.hh"
#include "IndexedCorpusReader.hh"

namespace indexedcorpus {

class ActCorpusReader
{
public:
	ActCorpusReader() {}
	std::vector<std::string> entries(std::string const &path);
	QString getData(QString const &path);
	std::string pathName(std::string const &path, int offset);
private:
	bool dzCorpusExists(QFileInfo const &name) const;
	std::vector<std::string> entriesCorpus(QFileInfo const &name);
	std::vector<std::string> entriesDirectory(QFileInfo const &name);
	std::string findEntry(std::vector<std::string> const &entries,
		std::string const &entry, int offset) const;
	std::string pathNameCorpus(QFileInfo const &corpus,
		QFileInfo const &filename, int offset);
	std::string pathNameDirectory(QFileInfo const &directory,
		QFileInfo const &filename, int offset);
	std::vector<unsigned char> readFromCorpus(
		QFileInfo const &corpus,
		QFileInfo const &file);
	QString stripCorpusExt(QString const &name) const;

	QString d_lastDir;
	std::vector<IndexNamePair> d_lastDirEntries;
	QString d_lastCorpusPath;
	IndexedCorpusReader d_lastCorpusReader;

	QMutex d_mutex;
};

}

#endif // ACT_CORPUS_READER

