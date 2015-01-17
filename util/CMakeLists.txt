if(USE_DBXML)

find_package(LibXslt REQUIRED)
if(LIBXSLT_FOUND)
  include_directories(${LIBXSLT_INCLUDE_DIR})
endif()

find_package(LibExslt REQUIRED)
if(LIBEXSLT_FOUND)
  include_directories(${LIBEXSLT_INCLUDE_DIR})
endif()

include_directories(${AlpinoCorpus_SOURCE_DIR}/util/common)

set(COMMON_HEADERS
  common/EqualsPrevious.hh
  common/ProgramOptions.hh
  common/util.hh
)

set(COMMON_SOURCES
  common/ProgramOptions.cpp
  common/util.cpp
)

set(CREATE_SOURCES
  create/main.cpp
)

add_executable(alpinocorpus-create
  ${CREATE_SOURCES}
  ${COMMON_HEADERS}
  ${COMMON_SOURCES}
)

target_link_libraries(alpinocorpus-create alpino_corpus)

set(XPATH_SOURCES
  xpath/main.cpp
)
set(EXTRACT_SOURCES
  extract/main.cpp
)

add_executable(alpinocorpus-extract
  ${EXTRACT_SOURCES}
  ${COMMON_HEADERS}
  ${COMMON_SOURCES}
)

target_link_libraries(alpinocorpus-extract
  alpino_corpus
)

set(GET_SOURCES
  get/main.cpp
)

add_executable(alpinocorpus-get
  ${GET_SOURCES}
  ${COMMON_HEADERS}
  ${COMMON_SOURCES}
)

target_link_libraries(alpinocorpus-get alpino_corpus)

add_executable(alpinocorpus-xpath
  ${XPATH_SOURCES}
  ${COMMON_HEADERS}
  ${COMMON_SOURCES}
)

target_link_libraries(alpinocorpus-xpath alpino_corpus)

set(XQUERY_SOURCES
  xquery/main.cpp
)

add_executable(alpinocorpus-xquery
  ${XQUERY_SOURCES}
  ${COMMON_HEADERS}
  ${COMMON_SOURCES}
)

target_link_libraries(alpinocorpus-xquery alpino_corpus)

set(XSLT_HEADERS
  common/Stylesheet.hh
)

set(XSLT_SOURCES
  xslt/main.cpp
  common/Stylesheet.cpp
)

add_executable(alpinocorpus-xslt
  ${XSLT_SOURCES}
  ${XSLT_HEADERS}
  ${COMMON_HEADERS}
  ${COMMON_SOURCES}
)

target_link_libraries(alpinocorpus-xslt
  alpino_corpus
  ${LIBXSLT_LIBRARIES}
  ${LIBEXSLT_LIBRARIES}
)

set(STATS_SOURCES
  stats/main.cpp
)

add_executable(alpinocorpus-stats
  ${STATS_SOURCES}
  ${COMMON_HEADERS}
  ${COMMON_SOURCES}
)

target_link_libraries(alpinocorpus-stats
  alpino_corpus
)

install(
  TARGETS
    alpinocorpus-create
    alpinocorpus-extract
    alpinocorpus-get
    alpinocorpus-stats
    alpinocorpus-xpath
    alpinocorpus-xquery
    alpinocorpus-xslt
  RUNTIME DESTINATION bin
)

endif(USE_DBXML)
