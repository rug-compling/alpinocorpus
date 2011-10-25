#include <ruby.h>

#include <AlpinoCorpus/capi.h>

static void CorpusReader_free(alpinocorpus_reader reader);
static VALUE CorpusReader_valid_query(VALUE self, VALUE query);

typedef struct {
    VALUE reader;
    VALUE query;
} Query;

static VALUE Query_new(VALUE self, VALUE reader, VALUE query);
static void Query_mark(Query *query);
static void Query_free(Query *query);

typedef struct {
    VALUE query;
    VALUE attr;
    VALUE value;
} MarkerQuery;

static void MarkerQuery_mark(MarkerQuery *markerQuery);
static void MarkerQuery_free(MarkerQuery *markerQuery);
