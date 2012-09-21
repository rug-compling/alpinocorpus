#ifndef ALPINOCORPUS_C_API
#define ALPINOCORPUS_C_API

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alpinocorpus_entry_t *alpinocorpus_entry;
typedef struct alpinocorpus_reader_t *alpinocorpus_reader;
typedef struct alpinocorpus_writer_t *alpinocorpus_writer;
typedef struct alpinocorpus_iter_t *alpinocorpus_iter;

typedef struct {
  char const *query;
  char const *attr;
  char const *value;
} marker_query_t;

/**
 * Initialize the library.
 */
void alpinocorpus_initialize();

/**
 * Cleanup the library.
 */
void alpinocorpus_cleanup();

/*** CORPUS READER ***/

/**
 * Open an Alpino treebank. Returns NULL if the corpus could not be opened.
 */
alpinocorpus_reader alpinocorpus_open(char const *path);

/**
 * Open an Alpino treebank. Returns NULL if the corpus could not be opened.
 */
alpinocorpus_reader alpinocorpus_open_recursive(char const *path);

/**
 * Close an Alpino treebank.
 */
void alpinocorpus_close(alpinocorpus_reader corpus);

/**
 * Check whether the given query is valid. Returns 1 if it is, 0 otherwise.
 */
int alpinocorpus_is_valid_query(alpinocorpus_reader corpus, char const *query);

/**
 * Get an iterator over the entries in a corpus.
 */
alpinocorpus_iter alpinocorpus_entry_iter(alpinocorpus_reader corpus);


/**
 * Get an iterator over the entries in a corpus, where the contents
 * are transformed using the given stylesheet.
 */
alpinocorpus_iter alpinocorpus_query_stylesheet_iter(alpinocorpus_reader corpus,
    char const *query, char const *stylesheet, marker_query_t *queries,
    size_t n_queries);

/**
 * Get an iterator over the entries in a corpus, where the contents
 * are transformed using the given stylesheet.
 */
alpinocorpus_iter alpinocorpus_query_stylesheet_marker_iter(alpinocorpus_reader corpus,
							    char const *query,
							    char const *stylesheet,
							    char const *markerQuery,
							    char const *markerAttr,
							    char const *markerValue);

/**
 * Get the contents of an entry. The content string is deallocated when
 * the entry is deallocated using <i>alpinocorpus_entry_free</i>.
 */
char const * alpinocorpus_entry_contents(alpinocorpus_entry entry);

/**
 * Deallocate an entry.
 */
void alpinocorpus_entry_free(alpinocorpus_entry entry);

/**
 * Get the name of an entry. The name string is deallocated when
 * the entry is deallocated using <i>alpinocorpus_entry_free</i>.
 */
char const * alpinocorpus_entry_name(alpinocorpus_entry entry);

/**
 * Get an iterator over the entries in a corpus that match a query.
 */
alpinocorpus_iter alpinocorpus_query_iter(alpinocorpus_reader reader, char const *query);

/**
 * Destroy an iterator. This is only necessary if not all entries are
 * iterated over.
 */
void alpinocorpus_iter_destroy(alpinocorpus_iter iter);

/**
 * Check whether the iterator has more entries.
 */
int alpinocorpus_iter_has_next(alpinocorpus_reader corpus,
  alpinocorpus_iter iter);

/**
 * Retrieve the next entry from the corpus. alpinocorpus_iter_has_next()
 * should be called before this function, to ensure that another entry is
 * available.
 */
alpinocorpus_entry alpinocorpus_iter_next(alpinocorpus_reader corpus,
  alpinocorpus_iter iter);

/**
 * Read an entry from the corpus.
 */
char *alpinocorpus_read(alpinocorpus_reader corpus, char const *entry);

/**
 * Read an entry, marking nodes matching a given query.
 */
char *alpinocorpus_read_mark_queries(alpinocorpus_reader reader,
    char const *entry, marker_query_t *queries, size_t n_queries);

/**
 * Read an entry, marking nodes matching a given query.
 */
char *alpinocorpus_read_mark_query(alpinocorpus_reader reader,
				   char const *entry,
				   char const *markerQuery,
				   char const *markerAttr,
				   char const *markerValue);

/**
 * Return the canonical name of the corpus.
 */
char *alpinocorpus_name(alpinocorpus_reader corpus);

/**
 * Return the number of entries in the corpus.
 */
size_t alpinocorpus_size(alpinocorpus_reader corpus);

/*** CORPUS WRITER ***/

/*
 * Open an Alpino treebank of the given type for writing. Returns NULL if the corpus could not be opened.
 * Currently, the only writertype supported is DBXML_CORPUS_WRITER.
 */
alpinocorpus_writer alpinocorpus_writer_open(char const *path, int overwrite, char const *writertype);

/*
 * Open an Alpino treebank that was opened for writing.
 */
void alpinocorpus_writer_close(alpinocorpus_writer);

/*
 * Check whether a particular writer type is available.
 */
int alpinocorpus_writer_available(char const *writertype);

/*
 * Write a single entry to the corpus. Returns NULL on succes, error message on failure.
 */
char const * alpinocorpus_write(alpinocorpus_writer, char const *name, char const *content);

/*
 * Write all entries from another corpus to corpus. Returns NULL on succes, error message on failure.
 */
char const * alpinocorpus_write_corpus(alpinocorpus_writer, alpinocorpus_reader, int failsafe);


#ifdef __cplusplus
}
#endif

#endif // ALPINOCORPUS_C_API
