% ALPINOCORPUS-XSLT(1)
% Daniel de Kok
% Nov 19, 2012
NAME
====

**alpinocorpus-xslt** -- Apply stylesheet to an Alpino treebank

SYNOPSIS
========

**alpinocorpus-xslt** [*options*] *stylesheet* *treebank ...*

DESCRIPTION
===========

The **alpinocorpus-xslt** utility applies *stylesheet* to each Dact
*treebank*. If *treebank* is a directory, then **alpinocorpus-xslt** will
apply *stylesheet* to each treebank below that directory. *stylesheet*
should be a XSLT stylesheet.

The following options are available:

`-g` *ENTRY*

:    Apply the stylesheet to *ENTRY*, rather than each entry in the treebank.

`-m` *MACROFILE*

:    Load macros from *MACROFILE*.

`-q` *QUERY*

:    Filter the treebank using *QUERY* (XPath 2.0). Nodes in the XML data
     that match *QUERY* get the attribute-value pair *active=1*.

SEE ALSO
========

alpinocorpus-create(1), alpinocorpus-extract(1), alpinocorpus-get(1)
alpinocorpus-stats(1), alpinocorpus-xpath(1), alpinocorpus-xquery(1)
