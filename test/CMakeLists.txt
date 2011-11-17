# Unit tests

# Public API

if(USE_DBXML)
  add_executable(writer_test writer.cpp)
  target_link_libraries(writer_test alpino_corpus)
  #add_test(writer writer_test)
endif()

# Private API

include_directories(${AlpinoCorpus_SOURCE_DIR}/src)

add_executable(geturl_test geturl.cpp)
target_link_libraries(geturl_test alpino_corpus)
