cmake_minimum_required(VERSION 2.6)
project(AlpinoCorpus)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

include_directories(${AlpinoCorpus_SOURCE_DIR})

find_package(Qt4 COMPONENTS QtCore REQUIRED)
INCLUDE(${QT_USE_FILE})

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

set (HEADERS
  AlpinoCorpus/CorpusReader.hh
  AlpinoCorpus/dictzip.hh
  AlpinoCorpus/DictZipFile.hh
  AlpinoCorpus/DirectoryCorpusReader.hh
  AlpinoCorpus/DLLDefines.hh
  AlpinoCorpus/DzChunk.hh
  AlpinoCorpus/fdstream.hh
  AlpinoCorpus/gzip.hh
  AlpinoCorpus/IndexedCorpusReader.hh
  AlpinoCorpus/IndexedCorpusWriter.hh
  AlpinoCorpus/IndexNamePair.hh
  AlpinoCorpus/NullStream.hh
  AlpinoCorpus/util/base64.hh
  AlpinoCorpus/util/bufutil.hh
  AlpinoCorpus/util/stringutil.hh
  AlpinoCorpus/util/textfile.hh
)

set (SOURCES
  src/CorpusReader/CorpusReader.cpp
  src/DictZipFile/DictZipFile.cpp
  src/DirectoryCorpusReader/DirectoryCorpusReader.cpp
  src/IndexedCorpusReader/IndexedCorpusReader.cpp
  #src/IndexedCorpusWriter/IndexedCorpusWriter.cpp
  src/util/textfile/textfile.cpp
  src/util/base64.cpp
  src/IndexNamePair/IndexNamePair.cpp
)

add_library(alpino_corpus SHARED 
	${SOURCES}
	${HEADERS}
)

target_link_libraries(alpino_corpus ${QT_LIBRARIES})
target_link_libraries(alpino_corpus ${ZLIB_LIBRARIES})

install(TARGETS alpino_corpus
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY AlpinoCorpus 
  DESTINATION include)
