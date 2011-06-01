#ifndef CACT_PROGRAMOPTIONS_HH
#define CACT_PROGRAMOPTIONS_HH

#include <map>
#include <set>

#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>

#include <unistd.h>

class ProgramOptions
{
public:
	ProgramOptions(int argc, char const *argv[], char const *optString);
	QVector<QString> const &arguments() const;
	QString const &programName() const;
	bool option(char option) const;
	QString const &optionValue(char option) const;
private:
	QString d_programName;
	QMap<char, QString> d_optionValues;
	QSet<char> d_options;
	QVector<QString> d_arguments;
};

inline QVector<QString> const &ProgramOptions::arguments() const
{
	return d_arguments;
}

inline QString const &ProgramOptions::programName() const
{
	return d_programName;
}

inline bool ProgramOptions::option(char option) const
{
	return d_options.find(option) != d_options.end();
}

#endif // CACT_PROGRAMOPTIONS_HH
