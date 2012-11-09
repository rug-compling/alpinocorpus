#ifndef ALPINO_REMOTECORPUSREADER_HH
#define ALPINO_REMOTECORPUSREADER_HH

#include <list>
#include <string>

#include <AlpinoCorpus/CorpusReader.hh>

namespace alpinocorpus {

    class RemoteCorpusReaderPrivate;

    /*! \class RemoteCorpusReader RemoteCorpusReader.hh "AlpinoCorpus/RemoteCorpusReader.hh"
        \brief RemoteCorpusReader opens a corpus for reading on a remote web server.

        All requests to the server should return plain text, unless stated otherwise.
        If the result is plain text, it should be encoded in UTF-8, and use these escapes within individual values:

         - \\  ->  \\\\
         - formfeed  ->  \\f
         - newline  ->  \\n
         - return  ->  \\r
         - tab  ->  \\t

        The following HTTP or HTTPS calls are used to communicate with the server:

        <b> GET [url-prefix]/corpora </b>

        Return a list of corpora, with on each line four tab-separated values:

         - corpus name
         - number of entries
         - short corpus description, without newlines
         - long corpus description, possibly with escaped newlines

        <b> GET [url-prefix]/[corpusname]/entry/[filename]?[options] </b>

        Return a single entry as XML.

        With the following three options, all items matching query [query] gain an attribute [attr] with the value [value]:

        \verbatim markerQuery=[query]&markerAttr=[attr]&markerValue=[value] \endverbatim

        <b> GET [url-prefix]/[corpusname]/entries?[options] </b> \n
        <b> POST [url-prefix]/[corpusname]/entries?[options] </b> + XSL stylesheet\n

        Without options, return a list of all entry names in the corpus.

         - The list \b may start with a line containing Ctrl-B + newline. This will be ignored.
         - The list \b must end with a line containing Ctrl-D + newline.
         - The server \b may return a line starting with Ctrl-[ indicating an error. The remainder of the line
           is used as the description of the error.

        The server should return its message headers as soon as possible. If the server doesn't deliver its headers
        before it has some content avaible, the line with Ctrl-B can be sent to get things going.

        To limit the output to names of entries that match a query, use the following. \n
        If an entry matches the query multiple times, it is returned multiple times.

        \verbatim query=[query] \endverbatim

        The following should return on each line two tab-separated values, the entry name, and the
        words for the part of the entry that match the query.

        \verbatim query=[query]&contents=1 \endverbatim

        With the POST method, a XSL stylesheet is sent to the server in the body of the request, not coded in any way.
        The server should apply the stylesheet on all entries (if no query was provided) or on all entries matching the query.
        To mark the matching elements in each entry before applying the stylesheet, add these options:

        \verbatim markerQuery=[query]&markerAttr=[attr]&markerValue=[value] \endverbatim

        To skip some lines at the beginning (for resuming an interrupted download), use:

        \verbatim start=[n] \endverbatim

        <b> GET [url-prefix]/[corpusname]/validQuery?query=[query] </b>

        Validate the query on the server. Should return `true`, `yes` or `1` if the query is valid.

        This is only used if compiled without support for DbXml.

    */
    class RemoteCorpusReader : public CorpusReader
    {
    public:
        //! Constructor with a url in the form: [url-prefix]/[corpusname]
        RemoteCorpusReader(std::string const &);
        virtual ~RemoteCorpusReader();

    private:
        Either<std::string, Empty> validQuery(QueryDialect d, bool variables,
            std::string const &query) const;
        EntryIterator getEntries() const;
        std::string getName() const;
        std::string readEntry(std::string const &) const;
        std::string readEntryMarkQueries(std::string const &entry,
            std::list<MarkerQuery> const &queries) const;
        EntryIterator runQueryWithStylesheet(QueryDialect d, std::string const &q,
            std::string const &stylesheet,
            std::list<MarkerQuery> const &markerQueries) const;
        EntryIterator beginWithStylesheet(std::string const &stylesheet,
            std::list<MarkerQuery> const &markerQueries) const;
        EntryIterator runXPath(std::string const &) const;
        EntryIterator runXQuery(std::string const &) const;
        size_t getSize() const;

        RemoteCorpusReaderPrivate *d_private;
    };

}   // namespace alpinocorpus

#endif
