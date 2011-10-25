#include <stdlib.h>
#include <string.h>
#include <ruby.h>

#include <AlpinoCorpus/capi.h>

#include "alpinocorpus.h"

/* Classes*/
VALUE cCorpusReader;
VALUE cQuery;

/* CorpusReader */

#define Data_Get_Struct_Ptr(obj,type,sval) do {\
    Check_Type(obj, T_DATA); \
    sval = (type)DATA_PTR(obj);\
} while (0)


VALUE static entries_iterator(alpinocorpus_reader reader, alpinocorpus_iter iter)
{
    do {
        char *val;
        if ((val = alpinocorpus_iter_value(iter)) == NULL)
            rb_raise(rb_eRuntimeError, "coul not retrieve iterator value");
        
        VALUE rString = rb_str_new2(val);
        free(val);

        int status;
        rb_protect(rb_yield, rString, &status);
        
        if (status) {
            alpinocorpus_iter_destroy(iter);
            rb_jump_tag(status);
        }

    } while (alpinocorpus_iter_next(reader, iter));

    alpinocorpus_iter_destroy(iter);

    return Qnil;
}


static void CorpusReader_free(alpinocorpus_reader reader) {
    alpinocorpus_close(reader);
}

static VALUE CorpusReader_init(VALUE self, VALUE path)
{
  rb_iv_set(self, "@path", path);
  return self;
}

static VALUE CorpusReader_each(VALUE self)
{
    if (!rb_block_given_p())
        rb_raise(rb_eArgError, "a block is required");

    alpinocorpus_reader reader;
    Data_Get_Struct_Ptr(self, alpinocorpus_reader, reader);

    alpinocorpus_iter iter;
    if ((iter = alpinocorpus_entry_iter(reader)) == NULL)
        rb_raise(rb_eRuntimeError, "could not iterate over corpus");
    
    return entries_iterator(reader, iter);
}

static VALUE CorpusReader_query(VALUE self, VALUE query)
{
    return Query_new(cQuery, self, query);
}

static VALUE CorpusReader_read(VALUE self, VALUE entry)
{
    char *cEntry = StringValueCStr(entry);

    alpinocorpus_reader reader;
    Data_Get_Struct_Ptr(self, alpinocorpus_reader, reader);

    char *data = alpinocorpus_read(reader, cEntry);
    if (data == NULL)
        rb_raise(rb_eRuntimeError, "can't read entry");

    VALUE rData = rb_str_new2(data);
    free(data);

    return rData;
}

static VALUE CorpusReader_valid_query(VALUE self, VALUE query)
{
    alpinocorpus_reader reader;
    Data_Get_Struct_Ptr(self, alpinocorpus_reader, reader);
    
    if (alpinocorpus_is_valid_query(reader, StringValueCStr(query)))
        return Qtrue;
    else
        return Qfalse;
}

static VALUE CorpusReader_new(VALUE self, VALUE path)
{
    VALUE argv[1];
    
    alpinocorpus_reader reader = alpinocorpus_open(StringValueCStr(path));
    if (reader == NULL)
        rb_raise(rb_eRuntimeError, "can't open corpus");
    
    VALUE tdata = Data_Wrap_Struct(self, 0, CorpusReader_free, reader);
    argv[0] = path;
    rb_obj_call_init(tdata, 1, argv);
    return tdata;
}

/* Query */

static VALUE Query_new(VALUE self, VALUE reader, VALUE query) {
    /* The query should at the very least be convertable to String. */
    query = StringValue(query);

    /* Let's bail out early if the query is not valid. */
    if (CorpusReader_valid_query(reader, query) == Qfalse)
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

static VALUE Query_each(VALUE self) {
    if (!rb_block_given_p())
        rb_raise(rb_eArgError, "a block is required");

    Query *query;
    Data_Get_Struct(self, Query, query);

    alpinocorpus_reader reader;
    Data_Get_Struct_Ptr(query->reader, alpinocorpus_reader, reader);

    char *cQuery = StringValueCStr(query->query);

    alpinocorpus_iter iter;
    if ((iter = alpinocorpus_query_iter(reader, cQuery)) == NULL)
        rb_raise(rb_eRuntimeError, "could not execute query");

    return entries_iterator(reader, iter);
}

/* Module initialization */

void initializeCorpusReader()
{
    cCorpusReader = rb_define_class("CorpusReader", rb_cObject);

    rb_define_singleton_method(cCorpusReader, "new",
        CorpusReader_new, 1);
    rb_define_method(cCorpusReader, "initialize",
        CorpusReader_init, 1); 
    rb_define_method(cCorpusReader, "each",
        CorpusReader_each, 0);
    rb_define_method(cCorpusReader, "query",
        CorpusReader_query, 1);
    rb_define_method(cCorpusReader, "read",
        CorpusReader_read, 1);
    rb_define_method(cCorpusReader, "validQuery?",
        CorpusReader_valid_query, 1);

    rb_include_module(cCorpusReader, rb_mEnumerable);    
}

void initializeQuery()
{
    cQuery = rb_define_class("Query", rb_cObject);
    rb_define_method(cQuery, "initialize",
        Query_init, 2);
    rb_define_method(cQuery, "each",
        Query_each, 0);

    rb_include_module(cQuery, rb_mEnumerable);
}

void Init_alpinocorpus()
{
    initializeCorpusReader();
    initializeQuery();
}
