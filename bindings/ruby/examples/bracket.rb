#!/usr/bin/ruby

require 'rubygems'
require 'alpinocorpus'
require 'rexml/document'
require 'xml/xslt'

if ARGV.length != 2 then
  puts "#{$0} query corpus"
  exit
end

query = ARGV[0]
corpus = ARGV[1]

styledir = File.dirname($0)
stylesheet = "#{styledir}/bracketed-sentence.xsl"

xslt = XML::XSLT.new()
xslt.xsl = REXML::Document.new(File.new(stylesheet))

reader = AlpinoCorpus::CorpusReader.new(corpus)

markers = [AlpinoCorpus::MarkerQuery.new(query, "active", "1")]

reader.query(query).each { |e|
  xslt.xml = REXML::Document.new(reader.read(e, markers))
  puts xslt.serve()
}
