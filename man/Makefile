all: alpinocorpus-create.1 alpinocorpus-stats.1 alpinocorpus-xpath.1 \
	alpinocorpus-xquery.1 alpinocorpus-xslt.1 alpinocorpus-get.1

%.1: %.1.md
	pandoc -s -w man -o $@ $<
