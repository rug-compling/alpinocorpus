% ALPINOCORPUS-GET(1)
% Daniel de Kok
% Oct 8, 2014
NAME
====

**alpinocorpus-get** -- Print Alpino treebank entry to stdout

SYNOPSIS
========

**alpinocorpus-get** [*options*] *treebank* *entry*

DESCRIPTION
===========

The **alpinocorpus-get** utility outputs a treebank entry to stdout. An
XPath query can be used to mark nodes.

The following options are available:

`-a` *ATTRIBUTE*

:    Mark nodes selected with 'q' with *ATTRIBUTE*.

`-m` *MACROFILE*

:    Load macros from *MACROFILE*.

`-q` *QUERY*

:    Mark nodes in the output using *QUERY* (XPath 2.0).

`-v` *VALUE*

:    Mark nodes selected with 'q' with *VALUE*.

SEE ALSO
========

alpinocorpus-create(1), alpinocorpus-extract, alpinocorpus-stats(1),
alpinocorpus-xpath(1), alpinocorpus-xquery(1), alpinocorpus-xslt(1)
