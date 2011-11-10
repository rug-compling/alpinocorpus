#!/usr/bin/python

import alpinocorpus
import libxml2
import libxslt
import os.path
import sys

class Transformer:
  def __init__(self, stylesheet):
    self.doc = libxml2.parseFile(stylesheet)
    self.style = libxslt.parseStylesheetDoc(self.doc)

  def close(self):
    self.style.freeStylesheet()

  def transform(self, data):
    doc = libxml2.parseMemory(data, len(data))
    result = self.style.applyStylesheet(doc, None)
    resultStr = self.style.saveResultToString(result)
    doc.freeDoc()
    result.freeDoc()

    return resultStr.strip()

def matchAndPrint(stylesheet, reader, query):
  trans = Transformer(stylesheet)

  markerQueries = [alpinocorpus.MarkerQuery(query, "active", "1")]
  for entry in reader.query(query):
    print trans.transform(reader.readMarkQueries(entry, markerQueries))

  trans.close()

if __name__ == "__main__":
  if (len(sys.argv) != 3):
    print "%s: query corpus" % sys.argv[0]
    sys.exit(1)

  styledir = os.path.dirname(sys.argv[0])
  stylesheet = os.path.join(styledir, "bracketed-sentence.xsl")

  query = sys.argv[1]
  reader = alpinocorpus.CorpusReader(sys.argv[2])

  matchAndPrint(stylesheet, reader, query)
