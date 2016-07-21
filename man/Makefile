all: alpinocorpus-create.1 alpinocorpus-stats.1 alpinocorpus-xpath.1 \
	alpinocorpus-xquery.1 alpinocorpus-xslt.1 alpinocorpus-get.1 \
	alpinocorpus-extract.1

web: alpinocorpus-create.1.html \
	alpinocorpus-stats.1.html \
	alpinocorpus-xpath.1.html \
	alpinocorpus-xquery.1.html \
	alpinocorpus-xslt.1.html \
	alpinocorpus-get.1.html \
	alpinocorpus-extract.1.html

%.1: %.1.md
	pandoc -s -w man -o $@ $<

%.1.html: %.1.md
	pandoc -s -c manpage.css -w html5 -o $@ $<
