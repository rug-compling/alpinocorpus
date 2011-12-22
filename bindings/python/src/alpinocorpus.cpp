#include <list>
#include <stdexcept>
#include <string>

#include <Python.h>

#include <AlpinoCorpus/CorpusReader.hh>

#include "alpinocorpus.h"

void raise_exception(char const *msg)
{
  PyObject *msgObj = Py_BuildValue("s", msg);
  PyErr_SetObject(PyExc_RuntimeError, msgObj);
}
///////////////////
// Method tables //
///////////////////

static PyMethodDef AlpinoCorpusMethods[] = {
  {NULL, NULL, 0, NULL}
};

static PyMethodDef CorpusReader_methods[] = {
  {"entries", (PyCFunction) CorpusReader_entries, METH_NOARGS, "Entries" },
  {"query", (PyCFunction) CorpusReader_query, METH_VARARGS, "Execute a query" },
  {"read", (PyCFunction) CorpusReader_read, METH_VARARGS, "Read entry" },
  {"readMarkQueries", (PyCFunction) CorpusReader_readMarkQueries, METH_VARARGS, "Read entry, marking queries" },
  {"validQuery", (PyCFunction) CorpusReader_validQuery, METH_VARARGS, "Validate a query" },
  {NULL} // Sentinel
};

static PyMethodDef EntryIterator_methods[] = {
  {NULL} // Sentinel
};

static PyMethodDef MarkerQuery_methods[] = {
  {NULL} // Sentinel
};

/////////////////////////////
// Python type definitions //
/////////////////////////////


static PyTypeObject CorpusReaderType = {
            PyObject_HEAD_INIT(NULL)
            0,                                        /* ob_size */
            "alpinocorpus.CorpusReader",              /* tp_name */
            sizeof(CorpusReader),                     /* tp_basicsize */
            0,                                        /* tp_itemsize */
            (destructor)CorpusReader_dealloc,         /* tp_dealloc */
            0,                                        /* tp_print */
            0,                                        /* tp_getattr */
            0,                                        /* tp_setattr */
            0,                                        /* tp_compare */
            0,                                        /* tp_repr */
            0,                                        /* tp_as_number */
            0,                                        /* tp_as_sequence */
            0,                                        /* tp_as_mapping */
            0,                                        /* tp_hash */
            0,                                        /* tp_call */
            0,                                        /* tp_str */
            0,                                        /* tp_getattro */
            0,                                        /* tp_setattro */
            0,                                        /* tp_as_buffer */
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
            "CorpusReader objects",                   /* tp_doc */
            0,                                        /* tp_traverse */
            0,                                        /* tp_clear */
            0,                                        /* tp_richcompare */
            0,                                        /* tp_weaklistoffset */
            0,                                        /* tp_iter */
            0,                                        /* tp_iternext */
            CorpusReader_methods,                     /* tp_methods */
            0,                                        /* tp_members */
            0,                                        /* tp_getset */
            0,                                        /* tp_base */
            0,                                        /* tp_dict */
            0,                                        /* tp_descr_get */
            0,                                        /* tp_descr_set */
            0,                                        /* tp_dictoffset */
            0,                                        /* tp_init */
            0,                                        /* tp_alloc */
            CorpusReader_new,                         /* tp_new */
        };

static PyTypeObject EntryIteratorType = {
            PyObject_HEAD_INIT(NULL)
            0,                                        /* ob_size */
            "alpinocorpus.EntryIterator",             /* tp_name */
            sizeof(EntryIterator),                    /* tp_basicsize */
            0,                                        /* tp_itemsize */
            (destructor)EntryIterator_dealloc,        /* tp_dealloc */
            0,                                        /* tp_print */
            0,                                        /* tp_getattr */
            0,                                        /* tp_setattr */
            0,                                        /* tp_compare */
            0,                                        /* tp_repr */
            0,                                        /* tp_as_number */
            0,                                        /* tp_as_sequence */
            0,                                        /* tp_as_mapping */
            0,                                        /* tp_hash */
            0,                                        /* tp_call */
            0,                                        /* tp_str */
            0,                                        /* tp_getattro */
            0,                                        /* tp_setattro */
            0,                                        /* tp_as_buffer */
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
            "EntryIterator objects",                  /* tp_doc */
            0,                                        /* tp_traverse */
            0,                                        /* tp_clear */
            0,                                        /* tp_richcompare */
            0,                                        /* tp_weaklistoffset */
            EntryIterator_iter,                       /* tp_iter */
            EntryIterator_iternext,                   /* tp_iternext */
            EntryIterator_methods,                    /* tp_methods */
            0,                                        /* tp_members */
            0,                                        /* tp_getset */
            0,                                        /* tp_base */
            0,                                        /* tp_dict */
            0,                                        /* tp_descr_get */
            0,                                        /* tp_descr_set */
            0,                                        /* tp_dictoffset */
            0,                                        /* tp_init */
            0,                                        /* tp_alloc */
            0,                                        /* tp_new */
        };

static PyTypeObject MarkerQueryType = {
            PyObject_HEAD_INIT(NULL)
            0,                                        /* ob_size */
            "alpinocorpus.MarkerQuery",               /* tp_name */
            sizeof(MarkerQuery),                      /* tp_basicsize */
            0,                                        /* tp_itemsize */
            (destructor)MarkerQuery_dealloc,          /* tp_dealloc */
            0,                                        /* tp_print */
            0,                                        /* tp_getattr */
            0,                                        /* tp_setattr */
            0,                                        /* tp_compare */
            0,                                        /* tp_repr */
            0,                                        /* tp_as_number */
            0,                                        /* tp_as_sequence */
            0,                                        /* tp_as_mapping */
            0,                                        /* tp_hash */
            0,                                        /* tp_call */
            0,                                        /* tp_str */
            0,                                        /* tp_getattro */
            0,                                        /* tp_setattro */
            0,                                        /* tp_as_buffer */
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
            "MarkerQuert objects",                    /* tp_doc */
            0,                                        /* tp_traverse */
            0,                                        /* tp_clear */
            0,                                        /* tp_richcompare */
            0,                                        /* tp_weaklistoffset */
            0,                                        /* tp_iter */
            0,                                        /* tp_iternext */
            MarkerQuery_methods,                      /* tp_methods */
            0,                                        /* tp_members */
            0,                                        /* tp_getset */
            0,                                        /* tp_base */
            0,                                        /* tp_dict */
            0,                                        /* tp_descr_get */
            0,                                        /* tp_descr_set */
            0,                                        /* tp_dictoffset */
            0,                                        /* tp_init */
            0,                                        /* tp_alloc */
            MarkerQuery_new,                          /* tp_new */
        };
///////////////////////////////////
// Python method implementations //
///////////////////////////////////

static PyObject *CorpusReader_new(PyTypeObject *type, PyObject *args,
  PyObject *kwds)
{
  char *path;
  if (!PyArg_ParseTuple(args, "s", &path))
    return NULL;

  CorpusReader *self;
  self = (CorpusReader *) type->tp_alloc(type, 0);
  try {
    if (self != NULL)
      self->reader = alpinocorpus::CorpusReader::open(path);
  } catch (std::runtime_error &e) {
    raise_exception(e.what());
    return NULL;
  }

  return (PyObject *) self;
}

static void CorpusReader_dealloc(CorpusReader *self)
{
  delete self->reader;
  self->ob_type->tp_free(self);
}

static PyObject *CorpusReader_read(CorpusReader *self, PyObject *args)
{
  char *entry;
  if (!PyArg_ParseTuple(args, "s", &entry))
    return NULL;

  std::string data;
  try {
    data = self->reader->read(entry);
  } catch (std::runtime_error &e) {
    raise_exception(e.what());
    return NULL;
  }

  return Py_BuildValue("s#", data.c_str(), data.size());
}

static PyObject *CorpusReader_readMarkQueries(CorpusReader *self, PyObject *args)
{
  char *entry;
  PyObject *markerList;
  if (!PyArg_ParseTuple(args, "sO!", &entry, &PyList_Type, &markerList))
    return NULL;

  std::list<alpinocorpus::CorpusReader::MarkerQuery> markerQueries;
  for (int i = 0; i < PyList_Size(markerList); ++i) {
    PyObject *entry = PyList_GetItem(markerList, (Py_ssize_t) i);
    if (entry->ob_type != &MarkerQueryType) {
      raise_exception("Marker list contains non-EntryMarker entries.");
      return NULL;
    }

    MarkerQuery *marker = reinterpret_cast<MarkerQuery *>(entry);
    markerQueries.push_back(alpinocorpus::CorpusReader::MarkerQuery(
      *marker->query, *marker->attr, *marker->value));
  }

  std::string data;
  try {
    data = self->reader->read(entry, markerQueries);
  } catch (std::runtime_error &e) {
    raise_exception(e.what());
    return NULL;
  }

  return Py_BuildValue("s#", data.c_str(), data.size());
}

static PyObject *CorpusReader_entries(CorpusReader *self)
{
  EntryIterator *iter;
  iter = (EntryIterator *) EntryIteratorType.tp_alloc(&EntryIteratorType, 0);
  try {
    if (iter != NULL) {
      iter->reader = self;
      iter->iter = new alpinocorpus::CorpusReader::EntryIterator(self->reader->begin());

      // Ensure the reader is not deallocated, since we need it.
      Py_INCREF(iter->reader);
    }
  } catch (std::runtime_error &e) {
    raise_exception(e.what());
    return NULL;
  }

  return (PyObject *) iter;
}

static PyObject *CorpusReader_query(CorpusReader *self, PyObject *args)
{
  char *query;
  if (!PyArg_ParseTuple(args, "s", &query))
    return NULL;

  EntryIterator *iter;
  iter = (EntryIterator *) EntryIteratorType.tp_alloc(&EntryIteratorType, 0);
  try {
    if (iter != NULL) {
      iter->reader = self;
      iter->iter = new alpinocorpus::CorpusReader::EntryIterator(self->reader->query(alpinocorpus::CorpusReader::XPATH, query));

      // Ensure the reader is not deallocated, since we need it.
      Py_INCREF(iter->reader);
    }
  } catch (std::runtime_error &e) {
    EntryIteratorType.tp_free(iter);
    raise_exception(e.what());
    return NULL;
  }

  return (PyObject *) iter;
}

static PyObject *CorpusReader_validQuery(CorpusReader *self, PyObject *args)
{
  char *query;
  if (!PyArg_ParseTuple(args, "s", &query))
    return NULL;
   
  try {
    if (self->reader->isValidQuery(alpinocorpus::CorpusReader::XPATH, false,
        query))
      return Py_True;
    else
      return Py_False;
  } catch (std::runtime_error &e) {
      raise_exception(e.what());
      return NULL;
  }
}

static void EntryIterator_dealloc(EntryIterator *self)
{
  // We clean up the iterator before decrementing the reference count of
  // the reader. If we hold the last reference, we'd deallocate the corpus
  // reader, while the iterator could still hold resources.
  CorpusReader *reader = self->reader;

  delete self->iter;
  self->ob_type->tp_free(self);

  Py_DECREF(reader);
}

static PyObject *EntryIterator_iter(PyObject *self)
{
  Py_INCREF(self);
  return self;
}

static PyObject *EntryIterator_iternext(PyObject *self)
{
  EntryIterator *entryIterator = reinterpret_cast<EntryIterator *>(self);

  if (*entryIterator->iter != entryIterator->reader->reader->end()) {
    std::string entry = **entryIterator->iter;
    ++(*entryIterator->iter);
    return Py_BuildValue("s#", entry.c_str(), entry.size());
  } else {
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
}

static PyObject *MarkerQuery_new(PyTypeObject *type, PyObject *args,
  PyObject *kwds)
{
  char *query, *attr, *value;

  if (!PyArg_ParseTuple(args, "sss", &query, &attr, &value))
    return NULL;

  MarkerQuery *self;
  if ((self = reinterpret_cast<MarkerQuery *>(type->tp_alloc(type, 0))) == NULL)
    return NULL;

  self->query = new std::string(query);
  self->attr = new std::string(attr);
  self->value = new std::string(value);

  return reinterpret_cast<PyObject *>(self);
}

static PyObject *MarkerQuery_dealloc(MarkerQuery *self)
{
  delete self->query;
  delete self->attr;
  delete self->value;

  self->ob_type->tp_free(self);
}

PyMODINIT_FUNC initalpinocorpus(void)
{
  PyObject *m;

  if (PyType_Ready(&CorpusReaderType) < 0)
    return;
  if (PyType_Ready(&EntryIteratorType) < 0)
    return;
  if (PyType_Ready(&MarkerQueryType) < 0)
    return;

  m = Py_InitModule("alpinocorpus", AlpinoCorpusMethods);
  if (m == NULL)
    return;
  
  Py_INCREF(&CorpusReaderType);
  PyModule_AddObject(m, "CorpusReader", (PyObject *) &CorpusReaderType);
  PyModule_AddObject(m, "EntryIterator", (PyObject *) &EntryIteratorType);
  PyModule_AddObject(m, "MarkerQuery", (PyObject *) &MarkerQueryType);
}

