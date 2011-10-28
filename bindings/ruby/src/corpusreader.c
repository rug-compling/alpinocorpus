#include <stdlib.h>
#include <string.h>
#include <ruby.h>

#include <AlpinoCorpus/capi.h>

#include "alpinocorpus.h"
#include "corpusreader.h"

VALUE cReader;

static void Reader_free(Reader *reader);

void check_reader_open(Reader *reader)
{
    /* Raise an exception if the reader was closed. */
    if (reader->reader == NULL)
        rb_raise(rb_eIOError, "closed reader");
}

int validate_c_markers(alpinocorpus_reader reader, marker_query_t *markers,
    long len)
{
    long i;
    for (i = 0; i < len; ++i)
        if (!alpinocorpus_is_valid_query(reader, markers[i].query))
            return 0;
    
    return 1;
}

char *read_markers(alpinocorpus_reader reader, char *entry, VALUE markers)
{
    long len;
    marker_query_t *cQueries = markers_to_c_markers(markers, &len);

    if (!validate_c_markers(reader, cQueries, len)) {
        free(cQueries);
        rb_raise(rb_eRuntimeError, "invalid query");
    }

    char *data = alpinocorpus_read_mark_queries(reader, entry, cQueries,
        len);
    
    free(cQueries);

    return data;
}

static VALUE Reader_s_alloc(VALUE self)
{
  return Data_Wrap_Struct(self, 0, Reader_free, 0);
}

static void Reader_free(Reader *reader) {
    if (reader->reader != NULL)
        alpinocorpus_close(reader->reader);

    free(reader);
}

static VALUE Reader_close(VALUE self)
{
    Reader *reader;
    Data_Get_Struct(self, Reader, reader);

    if (reader->reader != NULL) {
        alpinocorpus_close(reader->reader);
        reader->reader = NULL;
    }

    return Qnil;
}

/*
 * call-seq: Reader.new(path)
 *
 * Constructs a corpus reader, opening the corpus at _path_. The
 * corpus can be one of the following types:
 *
 * * Dact (DBXML)
 * * Compact corpus
 * * Directory with XML files
 *
 */
static VALUE Reader_initialize(VALUE self, VALUE path)
{
    alpinocorpus_reader reader = alpinocorpus_open(StringValueCStr(path));
    if (reader == NULL)
        rb_raise(rb_eRuntimeError, "can't open corpus");

    Reader *r = ALLOC(Reader);
    r->reader = reader;
    DATA_PTR(self) = r;

    rb_iv_set(self, "@path", path);

    return self;
}

/*
 * call-seq:
 *   reader.each {|entry| block} -> reader
 *
 * Execute a code block for each corpus entry name. 
 */
static VALUE Reader_each(VALUE self)
{
    if (!rb_block_given_p())
        rb_raise(rb_eArgError, "a block is required");

    Reader *reader;
    Data_Get_Struct(self, Reader, reader);

    check_reader_open(reader);

    alpinocorpus_iter iter;
    if ((iter = alpinocorpus_entry_iter(reader->reader)) == NULL)
        rb_raise(rb_eRuntimeError, "could not iterate over corpus");
    
    entries_iterator(reader->reader, iter);

    return self;
}

static VALUE Reader_s_open(VALUE self, VALUE path)
{
    VALUE argv[1];
    argv[0] = path;
    VALUE reader = rb_class_new_instance(1, argv, self);

    if (rb_block_given_p())
        return rb_ensure(rb_yield, reader, Reader_close, reader);

    return reader;
}

/*
 * call-seq:
 *   reader.query(q) -> query
 *
 * Returns a Query instance for the given query _q_.
 */
static VALUE Reader_query(VALUE self, VALUE query)
{
    return Query_new(cQuery, self, query);
}

/*
 * call-seq:
 *   reader.read(entry[, markers]) -> data
 *
 * Reads an entry from the corpus. Nodes matching a query can be marked
 * by providing a list of MarkerQuery.
 */
static VALUE Reader_read(int argc, VALUE *argv, VALUE self)
{
    VALUE entry, markers;
    rb_scan_args(argc, argv, "11", &entry, &markers);

    char *cEntry = StringValueCStr(entry);

    Reader *reader;
    Data_Get_Struct(self, Reader, reader);

    check_reader_open(reader);

    char *data;
    if (markers == Qnil)
        data = alpinocorpus_read(reader->reader, cEntry);
    else
        data = read_markers(reader->reader, cEntry, markers);

    if (data == NULL)
        rb_raise(rb_eRuntimeError, "can't read entry");

    VALUE rData = rb_str_new2(data);
    free(data);

    return rData;
}

/*
 * call-seq:
 *   reader.validQuery?(query) -> bool
 *
 * Validate an XPath query using _reader_.
 */
VALUE Reader_valid_query(VALUE self, VALUE query)
{
    Reader *reader;
    Data_Get_Struct(self, Reader, reader);

    check_reader_open(reader);
    
    if (alpinocorpus_is_valid_query(reader->reader, StringValueCStr(query)))
        return Qtrue;
    else
        return Qfalse;
}

/* Reader for Alpino treebanks. */

void initializeReader()
{
    cReader = rb_define_class_under(mAlpinoCorpus,
        "Reader", rb_cObject);

    rb_define_alloc_func(cReader, Reader_s_alloc); 

    rb_define_singleton_method(cReader, "open", Reader_s_open, 1);

    rb_define_method(cReader, "initialize",
        Reader_initialize, 1); 
    rb_define_method(cReader, "close",
        Reader_close, 0);
    rb_define_method(cReader, "each",
        Reader_each, 0);
    rb_define_method(cReader, "query",
        Reader_query, 1);
    rb_define_method(cReader, "read",
        Reader_read, -1);
    rb_define_method(cReader, "valid_query?",
        Reader_valid_query, 1);

    rb_include_module(cReader, rb_mEnumerable);    
}

