#include "IndexNamePair.ih"

IndexNamePair::IndexNamePair(QString const &newName) : name(newName)
{
	if (name.size() == 0)
		return;

    QString buf; // XXX - Bad for concatenation, but QStringBuilder is not visible...
	
    bool prevIsDigit = name[0].isDigit();
    for (QString::const_iterator iter = name.constBegin(); iter != name.constEnd();
		++iter)
	{
        bool curIsDigit = iter->isDigit();

		if (curIsDigit && prevIsDigit)
            buf += *iter;
		else
		{
            index.push_back(buf);
            buf.clear();
            buf += *iter;
		}
		
		prevIsDigit = curIsDigit;
	}
	
	// Leftover
    index.push_back(buf);
}

bool IndexNamePairCompare::operator()(IndexNamePair const &p1, IndexNamePair const &p2)
{
    QVector<QString> const &i1 = p1.index;
    QVector<QString> const &i2 = p2.index;
	
	for (size_t i = 0; i < i1.size() && i < i2.size(); ++i)
	{
		// Both digits? Sort on digits!
        if (i1[i][0].isDigit() && i2[i][0].isDigit())
		{
            // XXX - use unbounded numbers?
            qulonglong d1 = i1[i].toULongLong();
            qulonglong d2 = i2[i].toULongLong();
			
			if (d1 != d2)
				return d1 < d2;
		}
		else
			if (i1[i] != i2[i])
				return i1[i] < i2[i];
	}
	
	// The pairs are equal...
	return false;
}
