#include <ruby.h>

#include <AlpinoCorpus/capi.h>

#define Data_Get_Struct_Ptr(obj,type,sval) do {\
    Check_Type(obj, T_DATA); \
    sval = (type)DATA_PTR(obj);\
} while (0)

void entries_iterator(alpinocorpus_reader reader, alpinocorpus_iter iter);
marker_query_t *markers_to_c_markers(VALUE markers, long *len);

VALUE CorpusReader_valid_query(VALUE self, VALUE query);
VALUE Query_new(VALUE self, VALUE reader, VALUE query);

void initializeCorpusReader();
void initializeMarkerQuery();
void initializeQuery();

extern VALUE mAlpinoCorpus;
extern VALUE cMarkerQuery;
extern VALUE cQuery;
