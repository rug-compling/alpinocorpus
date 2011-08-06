#ifndef ALPINOCORPUS_C_API
#define ALPINOCORPUS_C_API

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alpinocorpus_reader_t *alpinocorpus_reader;
typedef struct alpinocorpus_iter_t *alpinocorpus_iter;

/**
 * Open an Alpino treebank. Returns NULL if the corpus could not be opened.
 */
alpinocorpus_reader alpinocorpus_open(char const *path);
    
/**
 * Close an Alpino treebank.
 */
void alpinocorpus_close(alpinocorpus_reader corpus);

/**
 * Get an iterator over the entries in a corpus.
 */
alpinocorpus_iter alpinocorpus_entry_iter(alpinocorpus_reader corpus);

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
 * Move the iterator forward. The pointer to the iterator will become 0 when
 * there are no more entries.
 */
alpinocorpus_iter alpinocorpus_iter_next(alpinocorpus_reader corpus,
  alpinocorpus_iter iter);

/**
 * Get the name of the current entry.
 */
char *alpinocorpus_iter_value(alpinocorpus_iter iter);

/**
 * Read an entry from the corpus.
 */
char *alpinocorpus_read(alpinocorpus_reader corpus, char const *entry);

#ifdef __cplusplus
}
#endif
    
#endif // ALPINOCORPUS_C_API
