#include <stdlib.h>
#include <string.h>
#include <ruby.h>

#include <AlpinoCorpus/capi.h>

#include "alpinocorpus.h"
#include "corpusreader.h"

typedef struct {
    VALUE reader;
    VALUE query;
} Query;

static void Query_mark(Query *query);
static void Query_free(Query *query);

VALUE cQuery;

VALUE Query_new(VALUE self, VALUE reader, VALUE query) {
    /* The query should at the very least be convertable to String. */
    query = StringValue(query);

    /* Let's bail out early if the query is not valid. */
    if (Reader_valid_query(reader, query) == Qfalse)
        rb_raise(rb_eRuntimeError, "invalid query");


    Query *q = (Query *) malloc(sizeof(Query));
    q->reader = reader;
    q->query = query;

    VALUE tdata = Data_Wrap_Struct(self, Query_mark, Query_free, q);

    VALUE argv[2];
    argv[0] = reader;
    argv[0] = query;
    rb_obj_call_init(tdata, 2, argv);
    return tdata;
}

static void Query_mark(Query *query)
{
    rb_gc_mark(query->reader);
    rb_gc_mark(query->query);
}

static void Query_free(Query *query)
{
    free(query);
}

static VALUE Query_init(VALUE self, VALUE reader, VALUE path)
{
  rb_iv_set(self, "@path", path);
  return self;
}

/*
 * call-seq:
 *   query.each {|entry| block} -> query
 *
 * Execute a code block for each corpus entry name (matching the query). 
 */
static VALUE Query_each(VALUE self) {
    if (!rb_block_given_p())
        rb_raise(rb_eArgError, "a block is required");

    Query *query;
    Data_Get_Struct(self, Query, query);

    Reader *reader;
    Data_Get_Struct(query->reader, Reader, reader);

    char *cQuery = StringValueCStr(query->query);

    alpinocorpus_iter iter;
    if ((iter = alpinocorpus_query_iter(reader->reader, cQuery)) == NULL)
        rb_raise(rb_eRuntimeError, "could not execute query");

    entries_iterator(reader->reader, iter);

    return self;
}

/* Queries over Reader instances. */

void initializeQuery()
{
    cQuery = rb_define_class_under(mAlpinoCorpus, "Query",
        rb_cObject);
    rb_define_method(cQuery, "initialize",
        Query_init, 2);
    rb_define_method(cQuery, "each",
        Query_each, 0);

    rb_include_module(cQuery, rb_mEnumerable);
}
