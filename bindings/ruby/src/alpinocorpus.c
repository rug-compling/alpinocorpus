#include <stdlib.h>
#include <ruby.h>

#include <AlpinoCorpus/capi.h>

#include "alpinocorpus.h"

VALUE mAlpinoCorpus;

void entries_iterator(alpinocorpus_reader reader, alpinocorpus_iter iter)
{
    do {
        char *val;
        if ((val = alpinocorpus_iter_value(iter)) == NULL)
            rb_raise(rb_eRuntimeError, "couldn't retrieve iterator value");
        
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
}

void Init_alpinocorpus()
{
    mAlpinoCorpus = rb_define_module("AlpinoCorpus");

    initializeCorpusReader();
    initializeQuery();
    initializeMarkerQuery();
}
