find_package(Ruby REQUIRED)
if(RUBY_FOUND)
  include_directories(${RUBY_INCLUDE_DIRS})
endif()

set(RUBY_HEADERS
  ext/alpinocorpus.h
)

set(RUBY_SOURCES
  ext/alpinocorpus.c
  ext/corpusreader.c
  ext/markerquery.c
  ext/query.c
)

add_library(alpinocorpus-rb MODULE
  ${RUBY_SOURCES}
)

set_target_properties(alpinocorpus-rb PROPERTIES LINKER_LANGUAGE C)
set_target_properties(alpinocorpus-rb PROPERTIES PREFIX "")
set_target_properties(alpinocorpus-rb PROPERTIES OUTPUT_NAME "alpinocorpus")

if(APPLE)
  set_target_properties(alpinocorpus-rb PROPERTIES SUFFIX ".bundle")
endif()

target_link_libraries(alpinocorpus-rb ${RUBY_LIBRARY})
target_link_libraries(alpinocorpus-rb alpino_corpus)
