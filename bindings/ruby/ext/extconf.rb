require 'mkmf'

dir_config('alpinocorpus')

if have_library('alpino_corpus', 'alpinocorpus_open') and
    have_header('AlpinoCorpus/capi.h')
  create_makefile('alpinocorpus_ext')
else
  puts "Could not find alpinocorpus!"
end
