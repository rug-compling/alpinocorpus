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

#include <QFileInfo>
#include <QMutex>
#include <QString>
#include <QVector>

#include "DLLDefines.hh"
#include "IndexNamePair.hh"
#include "IndexedCorpusReader.hh"

namespace indexedcorpus {

class INDEXED_CORPUS_EXPORT ActCorpusReader
{
public:
	ActCorpusReader();
	~ActCorpusReader();
    QVector<QString> entries(QString const &path);
	QString getData(QString const &path);
    QString pathName(QString const &path, int offset);
private:
	bool dzCorpusExists(QFileInfo const &name) const;
    QVector<QString> entriesCorpus(QFileInfo const &name);
    QVector<QString> entriesDirectory(QFileInfo const &name);
    QString findEntry(QVector<QString> const &entries,
        QString const &entry, int offset) const;
    QString pathNameCorpus(QFileInfo const &corpus,
		QFileInfo const &filename, int offset);
    QString pathNameDirectory(QFileInfo const &directory,
		QFileInfo const &filename, int offset);
    QString readFromCorpus(
		QFileInfo const &corpus,
		QFileInfo const &file);
	QString stripCorpusExt(QString const &name) const;

	QString d_lastDir;
    QVector<IndexNamePair> d_lastDirEntries;
	QString d_lastCorpusPath;
	IndexedCorpusReader d_lastCorpusReader;

	QMutex d_mutex;
};

}

#endif // ACT_CORPUS_READER

