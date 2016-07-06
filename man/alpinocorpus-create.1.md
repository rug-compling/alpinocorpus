% ALPINOCORPUS-CREATE(1)
% Daniel de Kok
% Nov 19, 2012
NAME
====

**alpinocorpus-create** -- Create Alpino treebanks

SYNOPSIS
========

**alpinocorpus-create** [*options*] *treebank ...*

DESCRIPTION
===========

The **alpinocorpus-create** creates a Dact treebank or compact corpus based
on *treebank*. If *treebank* is a directory, then **alpinocorpus-create**
will create a treebank consisting of all the XML files below that directory.
An XPath query can be applied to use only the entries that match that query.

The following options are available:

`-c` *FILENAME*
:    Create a compact corpus.
`-d` *FILENAME*
:    Create a Dact corpus.
`-m` *MACROFILE*
:    Load macros from *MACROFILE*.
`-q` *QUERY*
:    Only include entries that match *QUERY* (XPath 2.0).
`-r`
:    If *treebank* is a directory, include the contents of any Dact corpus
     below it, rather than including XML files.

SEE ALSO
========

alpinocorpus-create(1), alpinocorpus-extract(1), alpinocorpus-stats(1),
alpinocorpus-xquery(1), alpinocorpus-xslt(1)
