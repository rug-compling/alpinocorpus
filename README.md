# libalpino_corpus

## Introduction

This library provides a generic interface to XML treebanks. An XML
treebank is a directory structure with XML files representing parser
output, such as dependency structure. Three types of XML treebanks
are supported:

* Simple directory-based treebanks.
* An indexed treebank, consisting of a data file and an index file.
  The data file is a concatenation of chunks of data, such as XML
  documents or compressed derivation trees. The index file contains
  names for each chunk, along with the offset and size of the chunk
  encoded in base64 format.
* Treebanks stored in Berkeley DB XML databases.

Treebanks can be iterated by file or by query result.

This library evolved from the libcorpus library of the Alpino parser,
adding query-based iteration, support for Berkeley DB XML treebanks,
and a Qt-ish API.

## Design

Nearly all functionality is modelled as C++ classes using RAII, meaning
that memory is managed by virtue of construction/destruction. Where
necessary, errors are reported as exceptions. Language-specific wrappers
can catch exceptions and translate them to the language's native error
reporting method.

## Documentation

Documentation for this library can be obtained by running 'doxygen' in
the root of the source archive.

## Downloading

Pre-built binaries for Ubuntu are available from the Dact PPA:

https://launchpad.net/~danieldk/+archive/dact

On Mac OS X, alpinocorpus (including dependencies) can be compiled easily
using [Homebrew](http://mxcl.github.io/homebrew/):

~~~
$ brew tap rug-compling/homebrew
$ brew install alpinocorpus
~~~

## Building

Requirements

- A C++ compiler.
- Meson
- Boost 1.47.0.
- Berkeley DB XML 6.1.4 (with a small patch to correct a query processing bug, see #131).
- libxml2
- libxslt

### Build against system packages

If Berkeley DB XML is installed as a system package, alpinocorpus can
be built as follows:

```bash
$ meson builddir
$ ninja -C builddir
# If you want to install the library:
$ ninja -C builddir install
```

### Build against DB XML bundle

If Berkeley DB XML, XQilla, Xerces-C, and Berkeley DB are installed as
a bundle using the upstream Berkeley DB XML distribution, there are
two options for building alpinocorpus.

Assuming that the DB XML bundle is installed in `/opt/dbxml`, the
first option is to alpinocorpus as follows:

```bash
$ meson builddir -D dbxml_bundle=/opt/dbxml
$ ninja -C builddir
# If you want to install the library:
$ ninja -C builddir install
```

This embeds the DB, Xerces-C, XQilla, and DB XML library paths in the
alpinocorpus library. This will allow you to use the library and
utilities without further ado.

The second option is to instead define some variables before building
to point Meson to the headers and libraries:

```bash
$ export LD_LIBRARY_PATH=/opt/dbxml/lib
$ export LIBRARY_PATH=/opt/dbxml/lib
$ export CPATH=/opt/dbxml/include
$ meson builddir
$ ninja -C builddir
# If you want to install the library:
$ ninja -C builddir install
```

This does not embed the DB XML library paths into the alpinocorpus
shared library. Consequently, `LD_LIBRARY_PATH` should always be set
when using alpinocorpus (`LIBRARY_PATH` and `CPATH` only have to be
set at build time).

## Bindings

Bindings for Python 2 and 3 are available from:

http://github.com/rug-compling/alpinocorpus-python

Bindings for Go are available from:

http://github.com/rug-compling/alpinocorpus-go


## Contributors

* Daniël de Kok &lt;me@danieldk.eu&gt;
* Jelmer van der Linde &lt;jelmer@ikhoefgeen.nl&gt;
* Lars Buitinck &lt;larsmans@gmail.com&gt;
* Peter Kleiweg &lt;p.c.j.kleiweg@rug.nl&gt;

## License

~~~
Copyright 2010-2017 Daniël de Kok
Copyright 2010-2012 University of Groningen

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the 
Free Software Foundation, Inc.,  51 Franklin Street, Fifth Floor,
Boston, MA  02110-1301  USA
~~~
