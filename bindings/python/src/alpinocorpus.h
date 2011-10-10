#include <string>

#include <Python.h>

#include <AlpinoCorpus/CorpusReader.hh>

typedef struct {
  PyObject_HEAD
  alpinocorpus::CorpusReader *reader;
} CorpusReader;

typedef struct {
  PyObject_HEAD
  CorpusReader *reader;

  // This is a bit ugly, but we have to allocate the iterator separately.
  // Python lives in C-land, and tp_dealloc will not call the iterator
  // destructor. By managing the memory ourselves, we can use the delete
  // operator.
  alpinocorpus::CorpusReader::EntryIterator *iter;
} EntryIterator;

typedef struct {
  PyObject_HEAD
  std::string *query;
  std::string *attr;
  std::string *value;
} MarkerQuery;

static PyObject *CorpusReader_new(PyTypeObject *type, PyObject *args,
  PyObject *kwds);
static void CorpusReader_dealloc(CorpusReader *self);
static PyObject *CorpusReader_entries(CorpusReader *self);
static PyObject *CorpusReader_query(CorpusReader *self, PyObject *args);
static PyObject *CorpusReader_read(CorpusReader *self, PyObject *args);
static PyObject *CorpusReader_readMarkQueries(CorpusReader *self, PyObject *args);

static void EntryIterator_dealloc(EntryIterator *self);
static PyObject *EntryIterator_iter(PyObject *self);
static PyObject *EntryIterator_iternext(PyObject *self);

static PyObject *MarkerQuery_new(PyTypeObject *type, PyObject *args,
  PyObject *kwds);
static PyObject *MarkerQuery_dealloc(MarkerQuery *self);

