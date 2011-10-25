#include <ruby.h>

#include <AlpinoCorpus/capi.h>

typedef struct {
    VALUE reader;
    VALUE query;
} Query;

static VALUE Query_new(VALUE self, VALUE reader, VALUE query);
static void Query_mark(Query *query);
static void Query_free(Query *query);
