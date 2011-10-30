#!/usr/bin/python

import alpinocorpus
import libxml2
import libxslt
import os.path
import sys

import bracket

vorfeldQuery = """
//node[(  ancestor::node[@cat="smain"]/
             node[@rel="hd"]/number(@begin) 
           > node[@rel="hd" or @rel="cmp"]/number(@begin) 
       or ancestor::node[@cat="smain"]/
             node[@rel="hd"]/number(@begin) 
           > number(@begin) and @pos
       )
       and not (  ancestor::node[   (  
             ancestor::node[@cat="smain"]/
                node[@rel="hd"]/number(@begin) 
               > node[@rel="hd" or @rel="cmp"]/number(@begin)
              or ancestor::node[@cat="smain"]/
                node[@rel="hd"]/number(@begin) 
               > number(@begin) and @pos
                                    ) 
                                ]
               ) 
      ]
"""

if __name__ == "__main__":
  if (len(sys.argv) != 2):
    print "%s: corpus" % sys.argv[0]
    sys.exit(1)

  styledir = os.path.dirname(sys.argv[0])
  stylesheet = os.path.join(styledir, "bracketed-sentence.xsl")

  reader = alpinocorpus.CorpusReader(sys.argv[1])

  bracket.matchAndPrint(stylesheet, reader, vorfeldQuery)
