#include <stdlib.h>
#include <ruby.h>

#include <AlpinoCorpus/capi.h>

#include "alpinocorpus.h"

VALUE mAlpinoCorpus;

void entries_iterator(alpinocorpus_reader reader, alpinocorpus_iter iter)
{
    VALUE rString;
    int status;

    for (; !alpinocorpus_iter_end(reader, iter);
        alpinocorpus_iter_next(reader, iter))
    {
        char *val;
        if ((val = alpinocorpus_iter_value(iter)) == NULL)
            rb_raise(rb_eRuntimeError, "couldn't retrieve iterator value");
        
        rString = rb_str_new2(val);
        free(val);

        rb_protect(rb_yield, rString, &status);
        
        if (status) {
            alpinocorpus_iter_destroy(iter);
            rb_jump_tag(status);
        }

    }

    alpinocorpus_iter_destroy(iter);
}

void Init_alpinocorpus_ext()
{
    mAlpinoCorpus = rb_define_module("AlpinoCorpus");

    initializeReader();
    initializeQuery();
    initializeMarkerQuery();
}
