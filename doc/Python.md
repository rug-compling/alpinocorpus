# Alpinocorpus Python bindings

## Introduction

The *alpinocorpus* Python binding that is included in the *alpinocorpus*
distribution can be used to query and read XML treebanks in Python.

## Opening a corpus

A corpus is opened by loading the `alpinocorpus` Python module and
constructing a `CorpusReader` instance. The `CorpusReader` constructor
takes the corpus file or directory as its first argument.

The following fragment constucts a `CorpusReader` using the `cdb.dact`
file:

~~~ {.python}
>>> import alpinocorpus
>>> reader = alpinocorpus.CorpusReader("cdb.dact")
~~~

## Listing and reading entries

The `entries` method of a `CorpusReader` object returns an iterator
over the entries in the corpus:

~~~ {.python}
>>> len(list(reader.entries()))
7136
~~~

The XML data of an entry can be read using the `read` method. For example,
in the following fragment, we read the entry named `175.xml` and return
its first 25 characters:

~~~ {.python}
>>> reader.read("178.xml")[:25]
'<?xml version="1.0" encod'
~~~

## Executing queries

Using a query, we can retrieve the entries that have a non-empty set of
nodes matching that query. Queries are in
[XPath](http://en.wikipedia.org/wiki/XPath) 1.0 format. If the corpus
that was opened is a Dact (Berkeley DB XML) corpus, XPath 2.0 expressions
can also be used.

The `query` method of `CorpusReader` executes a query and returns an
iterator over the results. The following fragment executes the query
`//node[@root='loop']`, and gives the returns five results as a list:

~~~ {.python}
>>> list(reader.query("//node[@root='loop']"))[:5]
['1012.xml', '103.xml', '1126.xml', '1133.xml', '1189.xml']
~~~

## Examples

Some more extensive examples using the Python module can be found in
the `bindings/python/examples` directory of the *alpinocorpus* distribution.
