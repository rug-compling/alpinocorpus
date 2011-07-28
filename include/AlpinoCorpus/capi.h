#ifndef ALPINOCORPUS_C_API
#define ALPINOCORPUS_C_API

#ifdef __cplusplus
extern "C" {
#endif

typedef struct alpinocorpus_reader_t *alpinocorpus_reader_p;
typedef struct alpinocorpus_iter_t *alpinocorpus_iter_p;

/**
 * Open an Alpino treebank. Returns NULL if the corpus could not be opened.
 */
alpinocorpus_reader_p alpinocorpus_open(char const *path);
    
/**
 * Close an Alpino treebank.
 */
void alpinocorpus_close(alpinocorpus_reader_p corpus);

/**
 *
 */

alpinocorpus_iter_p alpinocorpus_entry_iter(alpinocorpus_reader_p corpus);

void alpinocorpus_iter_next(alpinocorpus_reader_p corpus, alpinocorpus_iter_p *iter);
    
char *alpinocorpus_iter_value(alpinocorpus_iter_p iter);

/**
 * Read an entry from the corpus.
 */
char *alpinocorpus_read(alpinocorpus_reader_p corpus, char const *entry);

#ifdef __cplusplus
}
#endif
    
#endif // ALPINOCORPUS_C_API