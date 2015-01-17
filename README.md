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

## Compilation

Requirements

- A C++ compiler.
- Boost 1.47.0.
- Berkeley DB XML 2.5.16 or later.
- libxml2
- libxslt

Execute *cmake .*, followed by *make* in the source directory.

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
Copyright 2010-2013 Daniël de Kok
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
