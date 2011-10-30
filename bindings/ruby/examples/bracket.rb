#!/usr/bin/ruby

require 'rubygems'
require 'alpinocorpus'
require 'nokogiri'

if ARGV.length != 2 then
  puts "#{$0} query corpus"
  exit
end

query = ARGV[0]
corpus = ARGV[1]

styledir = File.dirname($0)
stylesheet = "#{styledir}/bracketed-sentence.xsl"

xslt = Nokogiri::XSLT(File.read(stylesheet))

reader = AlpinoCorpus::Reader.new(corpus)

markers = [AlpinoCorpus::MarkerQuery.new(query, "active", "1")]

reader.query(query).each { |e|
  xml = Nokogiri.XML(reader.read(e, markers))
  puts xslt.apply_to(xml)
}
