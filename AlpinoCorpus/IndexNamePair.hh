#ifndef INDEX_NAME_PAIR_HH
#define INDEX_NAME_PAIR_HH

#include <QString>
#include <QVector>

namespace alpinocorpus {

struct IndexNamePair
{
    IndexNamePair() {}
    IndexNamePair(QString const &newName);
    IndexNamePair(QVector<QString> const &newIndex, QString const &newName) :
		index(newIndex), name(newName) {}
    QVector<QString> index;
    QString name;
};

struct IndexNamePairCompare
{
	bool operator()(IndexNamePair const &p1, IndexNamePair const &p2);
};

}

#endif // INDEX_NAME_PAIR
