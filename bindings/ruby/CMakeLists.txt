find_package(Ruby REQUIRED)
if(RUBY_FOUND)
  include_directories(${RUBY_INCLUDE_DIRS})
endif()

set(RUBY_HEADERS
  src/alpinocorpus.h
)

set(RUBY_SOURCES
  src/alpinocorpus.c
  src/corpusreader.c
  src/markerquery.c
  src/query.c
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
