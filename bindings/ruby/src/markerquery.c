#include <stdlib.h>
#include <string.h>
#include <ruby.h>

#include <AlpinoCorpus/capi.h>

#include "alpinocorpus.h"

typedef struct {
    VALUE query;
    VALUE attr;
    VALUE value;
} MarkerQuery;

static void MarkerQuery_mark(MarkerQuery *markerQuery);
static void MarkerQuery_free(MarkerQuery *markerQuery);

VALUE cMarkerQuery;

marker_query_t *markers_to_c_markers(VALUE markers, long *len)
{
    if (TYPE(markers) != T_ARRAY)
        rb_raise(rb_eRuntimeError, "need an array");
    
    *len = RARRAY_LEN(markers);

    marker_query_t *cQueries = malloc(sizeof(marker_query_t) * (*len));

    long i;
    for (i = 0; i < *len; ++i) {
        VALUE q = rb_ary_entry(markers, i);

        /* Check element class */
        if (CLASS_OF(rb_ary_entry(markers, i)) != cMarkerQuery) {
            free(cQueries);
            rb_raise(rb_eRuntimeError, "expecting elements of class MarkerQuery");
        }

        MarkerQuery *mq;
        Data_Get_Struct(q, MarkerQuery, mq);

        /* No strdup is needed, since markers will still be around when
           reading the entry. */
        cQueries[i].query = StringValueCStr(mq->query);
        cQueries[i].attr = StringValueCStr(mq->attr);
        cQueries[i].value = StringValueCStr(mq->value);
    }

    return cQueries;
}

/*
 * call-seq: MarkerQuery.new(query, attr, value)
 *
 * Creates a new marker query, using _query_ as its XPath expression,
 * and _attr_ and _value_ as the attribute-value pair to mark nodes
 * matching the query.
 */
static VALUE MarkerQuery_new(VALUE self, VALUE query, VALUE attr,
    VALUE value)
{
    query = StringValue(query);
    attr = StringValue(attr);
    value = StringValue(value);

    MarkerQuery *mq = (MarkerQuery *) malloc(sizeof(MarkerQuery));
    mq->query = query;
    mq->attr = attr;
    mq->value = value;

    VALUE tdata = Data_Wrap_Struct(self, MarkerQuery_mark, MarkerQuery_free,
        mq);
    
    VALUE argv[3] = {query, attr, value};
    rb_obj_call_init(tdata, 3, argv);
    return tdata;
}

static void MarkerQuery_mark(MarkerQuery *markerQuery)
{
    rb_gc_mark(markerQuery->query);
    rb_gc_mark(markerQuery->attr);
    rb_gc_mark(markerQuery->value);
}

static void MarkerQuery_free(MarkerQuery *markerQuery)
{
    free(markerQuery);
}

static VALUE MarkerQuery_init(VALUE self, VALUE query, VALUE attr,
    VALUE value)
{
    rb_iv_set(self, "@query", query);
    rb_iv_set(self, "@attr", attr);
    rb_iv_set(self, "@value", value);

    return self;
}

/* Queries for marking tree nodes. */

void initializeMarkerQuery()
{
    cMarkerQuery = rb_define_class_under(mAlpinoCorpus, "MarkerQuery",
        rb_cObject);
    rb_define_singleton_method(cMarkerQuery, "new",
        MarkerQuery_new, 3);
    rb_define_method(cMarkerQuery, "initialize",
        MarkerQuery_init, 3);
}
