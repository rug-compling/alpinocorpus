% ALPINOCORPUS-XQUERY(1)
% Daniel de Kok
% Nov 19, 2012
NAME
====

**alpinocorpus-xquery** -- Apply an XQuery program to an Alpino treebank

SYNOPSIS
========

**alpinocorpus-xquery** [*options*] *treebank ...*

DESCRIPTION
===========

The **alpinocorpus-xquery** utility runs an XQuery program on each
*treebank*. If *treebank* is a directory, then **alpinocorpus-xquery** will
apply the program to each treebank below that directory.

Since an XQuery program normally specifies that collection that it should
query, we provide the default collection name *collection('corpus')*. Note
that the program is applied to each treebank.

The following options are available:

`-f` *XQUERY_PROGRAM*
:    Read XQuery program from the file *XQUERY_PROGRAM*.
`-m` *MACROFILE*
:    Load macros from *MACROFILE*.
`-q` *QUERY*
:    Only show entries that match *QUERY* (XPath 2.0).

SEE ALSO
========

alpinocorpus-create(1), alpinocorpus-stats(1), alpinocorpus-xpath(1),
alpinocorpus-xslt(1)
