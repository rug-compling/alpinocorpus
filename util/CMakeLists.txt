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

set(ACT_SOURCES
  act/main.cpp
)

add_executable(alpinocorpus-act
  ${ACT_SOURCES}
  ${COMMON_HEADERS}
  ${COMMON_SOURCES}
)

target_link_libraries(alpinocorpus-act alpino_corpus)

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

install(TARGETS alpinocorpus-act
  RUNTIME DESTINATION bin
)

install(TARGETS alpinocorpus-xslt
  RUNTIME DESTINATION bin
)

install(TARGETS alpinocorpus-stats
  RUNTIME DESTINATION bin
)
