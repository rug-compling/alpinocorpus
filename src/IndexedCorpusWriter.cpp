#include <string>

#include <QMutexLocker>

#include <IndexedCorpus/util/base64.hh>
#include <IndexedCorpus/IndexedCorpusWriter.hh>

using namespace std;

namespace alpinocorpus {

void IndexedCorpusWriter::copy(IndexedCorpusWriter const &other)
{
	d_dataStream = other.d_dataStream;
	d_indexStream = other.d_indexStream;
	d_offset = other.d_offset;
}

void IndexedCorpusWriter::write(std::string const &name, std::string const &data)
{
	QMutexLocker locker(&d_mutex);

	*d_dataStream << data;
	*d_indexStream << name << "\t" << b64_encode(d_offset) << "\t" <<
		b64_encode(data.size()) << endl;
	d_offset += data.size();
}

void IndexedCorpusWriter::write(std::string const &name, char const *buf, size_t len)
{
	QMutexLocker locker(&d_mutex);

	d_dataStream->write(buf, len);
	*d_indexStream << name << "\t" << b64_encode(d_offset) << "\t" <<
		b64_encode(len) << endl;
	d_offset += len;
}

}   // namespace alpinocorpus
