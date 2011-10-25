#include <stdlib.h>
#include <string.h>
#include <ruby.h>

#include <AlpinoCorpus/capi.h>

#include "alpinocorpus.h"

static void Reader_free(alpinocorpus_reader reader);

VALUE cReader;

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
static VALUE Reader_new(VALUE self, VALUE path)
{
    VALUE argv[1];
    
    alpinocorpus_reader reader = alpinocorpus_open(StringValueCStr(path));
    if (reader == NULL)
        rb_raise(rb_eRuntimeError, "can't open corpus");
    
    VALUE tdata = Data_Wrap_Struct(self, 0, Reader_free, reader);
    argv[0] = path;
    rb_obj_call_init(tdata, 1, argv);
    return tdata;
}

static void Reader_free(alpinocorpus_reader reader) {
    alpinocorpus_close(reader);
}

static VALUE Reader_init(VALUE self, VALUE path)
{
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

    alpinocorpus_reader reader;
    Data_Get_Struct_Ptr(self, alpinocorpus_reader, reader);

    alpinocorpus_iter iter;
    if ((iter = alpinocorpus_entry_iter(reader)) == NULL)
        rb_raise(rb_eRuntimeError, "could not iterate over corpus");
    
    entries_iterator(reader, iter);

    return self;
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

    alpinocorpus_reader reader;
    Data_Get_Struct_Ptr(self, alpinocorpus_reader, reader);

    char *data;
    if (markers == Qnil)
        data = alpinocorpus_read(reader, cEntry);
    else
        data = read_markers(reader, cEntry, markers);

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
    alpinocorpus_reader reader;
    Data_Get_Struct_Ptr(self, alpinocorpus_reader, reader);
    
    if (alpinocorpus_is_valid_query(reader, StringValueCStr(query)))
        return Qtrue;
    else
        return Qfalse;
}

/* Reader for Alpino treebanks. */

void initializeReader()
{
    cReader = rb_define_class_under(mAlpinoCorpus,
        "Reader", rb_cObject);

    rb_define_singleton_method(cReader, "new",
        Reader_new, 1);
    rb_define_method(cReader, "initialize",
        Reader_init, 1); 
    rb_define_method(cReader, "each",
        Reader_each, 0);
    rb_define_method(cReader, "query",
        Reader_query, 1);
    rb_define_method(cReader, "read",
        Reader_read, -1);
    rb_define_method(cReader, "validQuery?",
        Reader_valid_query, 1);

    rb_include_module(cReader, rb_mEnumerable);    
}

