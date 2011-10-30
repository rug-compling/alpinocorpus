#include <ruby.h>

#include <AlpinoCorpus/capi.h>

typedef struct {
  alpinocorpus_reader reader;
} Reader;

void get_reader(VALUE obj, Reader **reader);

