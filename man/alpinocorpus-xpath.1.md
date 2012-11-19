% ALPINOCORPUS-XPATH(1)
% Daniel de Kok
% Nov 19, 2012
NAME
====

**alpinocorpus-xpath** -- Alpino treebank XPath search

SYNOPSIS
========

**alpinocorpus-xpath** [*options*] *treebank ...*

DESCRIPTION
===========

The **alpinocorpus-xpath** utility lists the entries in each Dact
*treebank*. If *treebank* is a directory, then **alpinocorpus-xpath** will
list the entries of each treebank below that directory. An XPath query can
be applied to list only the entries that match that query.

The following options are available:

`-m` *MACROFILE*
:    Load macros from *MACROFILE*.
`-q` *QUERY*
:    Only show entries that match *QUERY* (XPath 2.0).
`-s`
:    Print the sentence of each entry, fragments that match the query are
     colored.

SEE ALSO
========

alpinocorpus-create(1), alpinocorpus-stats(1), alpinocorpus-xquery(1),
alpinocorpus-xslt(1)
